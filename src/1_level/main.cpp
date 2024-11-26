#include "../game.h"
#include "../levelloader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <raylib.h>
#include <iostream>

#include <filesystem>

int TICK = 60;
double dt = 1/(double)TICK;
int FPS = 60;
double df = 1/(double)FPS;


std::vector<std::string> GetFilesFromDirExt(const std::string &path, const std::string& extension) {
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().has_extension() && entry.path().extension()==extension) result.push_back(entry.path().string());
    }
    return result;
}

std::vector<std::string> GetFilesFromDir(const std::string &path) {
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().has_extension()) result.push_back(entry.path().string());
    }
    return result;
}

extern "C" {    

void load_settings(FILE* fd) {
    Json::init_json();
    Json::json_child json = Json::read_json(fd);
    size_t len = Json::vec_metaptr(json.fields)->length;
    for (size_t i=0; i<len; i++) {
        if (Cstd::strcmp(json.fields[i].key, "fullscreen")==0) {
            gamestate.fullscreen = json.fields[i].value.data.num;
        } else if (Cstd::strcmp(json.fields[i].key, "WinX")==0) {
            gamestate.WinX = json.fields[i].value.data.num;
            gamestate.WinXf = gamestate.WinX;
        } else if (Cstd::strcmp(json.fields[i].key, "WinY")==0) {
            gamestate.WinY = json.fields[i].value.data.num;
            gamestate.WinYf = gamestate.WinY;
        } else if (Cstd::strcmp(json.fields[i].key, "MAX_REFLECTIONS")==0) {
            gamestate.MAX_REFLECTIONS = json.fields[i].value.data.num;
        }
    }
}
}

void default_level();


double updateLatency = 0;

using std::cout, std::endl;
int main(int argc, char** argv) {
    FILE* file = fopen("settings.json", "r");
    if (file) {
        load_settings(file);
        fclose(file);
        MemManager::page_info(0);
        MemManager::destroy_pages();
    } else fprintf(stderr, "Cannot load settings: using default\n");

    MemManager::prealloc(PAGE_SIZE*8);

    SetWindowState(FLAG_VSYNC_HINT); 
    SetWindowState(FLAG_WINDOW_ALWAYS_RUN); 
    // SetWindowState(FLAG_WINDOW_TRANSPARENT); 
    SetWindowState(FLAG_WINDOW_HIGHDPI); 
    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(gamestate.WinX, gamestate.WinY, "Coldline Samara");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    // SetWindowState(FLAG_WINDOW_UNDECORATED); 

    if (gamestate.fullscreen) ToggleFullscreen();
    SetExitKey(KEY_F4);
    SetTargetFPS(FPS);


    TM::LoadT("assets/pistol.png", TM::TPistol);
    TM::LoadT("assets/rifle.png", TM::TRifle);
    TM::LoadT("assets/shotgun.png", TM::TShotgun);
    
    gamestate.fileLevels = GetFilesFromDirExt("./levels/", ".lvl");

    default_level();


    ReloadLevel();
            
    // 2----1
    // |    |
    // 4----3

    MemManager::page_info(0);


    char buffer[64] = {0};
    int buffer_idx = 0;

    std::thread updateThread([&]() { 
        // SetTargetFPS(TICK);
        // EnableEventWaiting();
        using namespace std::chrono;
        SAFE_DRAWING = true;
        updateLatency = 0;
        do {
            
            time_point from = steady_clock::now();
            PollInputEvents();
            if (!gamestate.pause) update();
            time_point to = steady_clock::now();
            
            updateLatency = (double)duration_cast<milliseconds>(to-from).count() /1000.f;
            if (dt - updateLatency < 0) updateLatency = dt;

            WaitTime(dt - updateLatency);
            // cout << dt << "\t" << updateLatency << "\t"  << dt-updateLatency << endl;
        } while (!STOP);
    });

    WaitTime(0.1);
    bool swapped = false;
    while (!WindowShouldClose()) {
        
        if (IsWindowResized()) {
            gamestate.WinX = GetScreenWidth();
            gamestate.WinY = GetScreenHeight();
            gamestate.WinXf = gamestate.WinX;
            gamestate.WinYf = gamestate.WinY;
        }

        buffer[buffer_idx] = GetCharPressed();
        if (buffer[buffer_idx] > 32) {
            buffer_idx++;
            // cout << buffer << endl;
        }
        if (buffer_idx>=63) {
            memcpy(buffer, buffer+32, 31);
            buffer_idx = buffer_idx%63;
        }

        if (IsKeyPressed(KEY_ESCAPE)) gamestate.pause = !gamestate.pause;
        if (IsKeyPressed(KEY_F11)) {
            gamestate.fullscreen = !gamestate.fullscreen;
            ToggleFullscreen();
        }
        if (IsKeyPressed(KEY_R) && !gamestate.pause) {
            // SAFE_DRAWING = false;
            // RELOAD = true;
            // bool temp = gamestate.pause; 
            gamestate.pause = true;
            WaitTime(4*dt);
            ReloadLevel(); 
            gamestate.pause = false;
        }
        
        if (IsKeyPressed(KEY_I)) {
            gamestate.currentLevel.cheats.invisible = !gamestate.currentLevel.cheats.invisible;
        }

        BeginDrawing();
        // ClearBackground(BLACK);
        // if (!gamestate.pause) update();
        if (SAFE_DRAWING) {
            draw();
            DrawFPS(0,0);
            {
                char b[64];
                int tps = (double)(1.f/(dt+updateLatency));
                snprintf(b, 64, "%d TPS", tps);
                DrawText(b, 0, 30, 20, RED);
            }
            swapped = false;
        } else if (!swapped) {
            SwapScreenBuffer();
            swapped = true;
        }

        EndDrawing();
    }
    STOP = true;
    updateThread.join();
    CloseWindow();
    MemManager::page_info(0);

    SaveLevel(gamestate.levelReference, "levels/1.lvl");

    gamestate.currentLevel.clear();
    UnloadLevel(gamestate.levelReference);
    gamestate.levelReference.~Level();
    gamestate.currentLevel.~Level();

    TextureManager::UnloadT();
    
    MemManager::page_info(0);
    MemManager::destroy_pages();

    std::cout << "Finished succesfully" << std::endl;
    return 0;
}



#define ADD_OBJ(O) gamestate.levelReference.objects.push_back(O);

#define NEW_Wall(x1, y1, x2, y2, w) NEW(Wall) Wall(k*Point({x1,y1}), k*Point({x2,y2}), k*w)
#define NEW_Door(minangle, maxangle, angle, origin, x1, y1, x2, y2, w) NEW(Door) Door(minangle, maxangle, angle, k*origin, k*Point({x1,y1}), k*Point({x2,y2}), k*w)
#define NEW_Mirror(x1, y1, x2, y2, w) NEW(Mirror) Mirror(k*Point({x1,y1}), k*Point({x2,y2}), k*w)

#define AUTONEW(T, ...) ADD_OBJ(NEW_##T(__VA_ARGS__))

#define MP(x, y) gamestate.levelReference.MapPoints.push_back(ConnectedPoint({k*x,k*y}))
#define MP_Connect(p1, p2) \
    gamestate.levelReference.MapPoints[p1].connections.push_back(p2);\
    gamestate.levelReference.MapPoints[p2].connections.push_back(p1);

void default_level() {
    float k=3.5;
    
    gamestate.levelReference.clear();
    gamestate.levelReference.MapX = k*200;
    gamestate.levelReference.MapY = k*400;
    gamestate.levelReference.MapXf = gamestate.levelReference.MapX;
    gamestate.levelReference.MapYf = gamestate.levelReference.MapY;

    Player* p = NEW(Player) Player(k*Point({100.f, 350.f}), k/1.5*Vector2({5.f, 10.f}));
    p->viewAround = 80;
    p->viewLength = k*200;
    p->speed = 1.5;
    ADD_OBJ(p);

    AUTONEW(Wall, 0,0, 200,0, 2);
    AUTONEW(Wall, 0,0, 0,250, 2);
    AUTONEW(Wall, 0,250, 0,400, 2);
    AUTONEW(Wall, 200,0, 200,250, 2);
    AUTONEW(Wall, 200,250, 200,400, 2);
    Wall* w = NEW(Wall) Wall({
        k*(Vector2){200,397}, k*(Vector2){0,397},
        k*(Vector2){200,400}, k*(Vector2){0,400}
    });
    ADD_OBJ(w);
    

    AUTONEW(Wall, 0,250, 33,250, 2);
    AUTONEW(Wall, 80,250, 120,250, 2);
    AUTONEW(Wall, 167,250, 200,250, 2);

    Mirror* m = NEW(Mirror) Mirror({
        k*(Vector2){70,0}, k*(Vector2){65,0},
        k*(Vector2){0,70}, k*(Vector2){0,65},
    },
        {1/sqrt(2), 1/sqrt(2)}
    );
    ADD_OBJ(m);

    m = NEW(Mirror) Mirror((Poly){
        k*(Vector2){135,0}, k*(Vector2){130,0},
        k*(Vector2){200,65}, k*(Vector2){200,70},
    },
        (Vector2){-1/sqrt(2), 1/sqrt(2)}
    );
    ADD_OBJ(m);

    MP(-20,-20);

    MP(60, 40);
    MP(140, 40);

    MP(30, 80);
    MP(100, 80);
    MP(170, 80);

    MP(50, 120);
    MP(150, 120);

    MP(50, 160);
    MP(100,180);
    MP(150, 160);
    
    MP(50, 220);
    MP(150, 220);
    
    MP(50, 250);
    MP(150, 250);

    MP(50, 270);
    MP(100, 280);
    MP(150, 270);

    MP(30, 310);
    MP(170,310);

    MP(30, 360);
    MP(100,350);
    MP(170,360);
    
    MP_Connect(1, 2);
    MP_Connect(1, 3);
    MP_Connect(1, 4);
    MP_Connect(2, 4);
    MP_Connect(2, 5);
    MP_Connect(3, 4);
    MP_Connect(3, 6);
    MP_Connect(4, 5);
    MP_Connect(4, 6);
    MP_Connect(4, 7);
    MP_Connect(5, 7);
    MP_Connect(6, 7);
    MP_Connect(6, 8);
    MP_Connect(6, 9);
    MP_Connect(7, 9);
    MP_Connect(7, 10);
    MP_Connect(8, 9);
    MP_Connect(8, 11);
    MP_Connect(9, 10);
    MP_Connect(9, 11);
    MP_Connect(9, 12);
    MP_Connect(10, 12);
    MP_Connect(11, 13);
    MP_Connect(12, 14);
    MP_Connect(13, 15);
    MP_Connect(14, 17);
    MP_Connect(15, 16);
    MP_Connect(15, 18);
    MP_Connect(15, 21);
    MP_Connect(16, 17);
    MP_Connect(16, 21);
    MP_Connect(17, 19);
    MP_Connect(17, 21);
    MP_Connect(18, 20);
    MP_Connect(18, 21);
    MP_Connect(19, 21);
    MP_Connect(19, 22);
    MP_Connect(20, 21);
    MP_Connect(21, 22);
    
    Enemy *en;
    Item *it;

    en = NEW(Enemy) Enemy(gamestate.levelReference.MapPoints[1], k/1.5*Vector2({5.f, 10.f}), {});
    en->viewAround = p->viewAround*0.75;
    en->viewLength = p->viewLength*0.75;
    en->stepsize = 5;
    en->speed = 1;
    en->direction = {1,0};
    en->default_procent_found = 0.08;
    ADD_OBJ(en);

    it = NEW(Pistol) Pistol({0,0}, {2*Point({10,-5}), 2*Point({-10,-5}), 2*Point({10,5}), 2*Point({-10,5})}, false);
    ADD_OBJ(it);
    {int idx = gamestate.levelReference.objects.size()-1;
    en->selfitem = idx;}

    en = NEW(Enemy) Enemy(gamestate.levelReference.MapPoints[2], k/1.5*Vector2({5.f, 10.f}), {});
    en->viewAround = p->viewAround*0.75;
    en->viewLength = p->viewLength*0.75;
    en->stepsize = 5;
    en->speed = 1;
    en->direction = {-1,0};
    en->default_procent_found = 0.08;
    ADD_OBJ(en);

    it = NEW(Pistol) Pistol({0,0}, {2*Point({10,-5}), 2*Point({-10,-5}), 2*Point({10,5}), 2*Point({-10,5})}, false);
    ADD_OBJ(it);
    {int idx = gamestate.levelReference.objects.size()-1;
    en->selfitem = idx;}

    // gamestate.levelReference.cheats.see_MP = true;
    // gamestate.levelReference.cheats.invisible = true;
}