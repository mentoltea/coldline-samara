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
        SAFE_DRAWING = true;
        do {
            WaitTime(dt);
            PollInputEvents();
            if (!gamestate.pause) update();

            // if (!SAFE_DRAWING) SAFE_DRAWING = true;
            // draw();
        } while (!STOP);
    });

    WaitTime(0.1);
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
        }

        DrawFPS(0,0);
        EndDrawing();
    }
    STOP = true;
    updateThread.join();
    CloseWindow();
    MemManager::page_info(0);

    // SaveLevel(gamestate.levelReference, "levels/l1.lvl");

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
    float k=4.5;
    
    gamestate.levelReference.clear();
    gamestate.levelReference.MapX = k*300;
    gamestate.levelReference.MapY = k*320;
    gamestate.levelReference.MapXf = gamestate.levelReference.MapX;
    gamestate.levelReference.MapYf = gamestate.levelReference.MapY;

    Player* p = NEW(Player) Player(k*Point({140.f, 280.f}), k/1.5*Vector2({5.f, 10.f}));
    p->viewAround = 80;
    p->viewLength = 460;
    p->speed = 1.5;
    ADD_OBJ(p);

    

    AUTONEW(Wall, 0,0, 300,0, 1);
    AUTONEW(Wall, 300,0, 300,240, 1);
    AUTONEW(Wall, 0,0, 0,320, 1);
    AUTONEW(Wall, 0,240, 120,240, 1);
    AUTONEW(Wall, 160,240, 230,240, 1);
    AUTONEW(Wall, 270,240, 300,240, 1);

    AUTONEW(Wall, 120,240, 120,320, 1);
    AUTONEW(Wall, 160,240, 160,320, 1);
    AUTONEW(Wall, 120,320, 160,320, 1);
    
    // AUTONEW(Door, 100, 300, 180, Point({160,240}), 120,240, 160,240, 1/2);

    AUTONEW(Wall, 230,240, 230,320, 1);
    AUTONEW(Wall, 270,240, 270,320, 1);
    AUTONEW(Wall, 230,320, 270,320, 1);

    AUTONEW(Door, 95, 355, 180, Point({270,240}), 230,240, 270,240, 1/2)

    float thick = 1.5;
    AUTONEW(Wall, 20,0, 0,20,  (thick+2));
    AUTONEW(Wall, 130,0, 170,40, thick);
    AUTONEW(Wall, 170,40, 210,40, thick);
    AUTONEW(Wall, 210,40, 260,60, thick);
    AUTONEW(Wall, 40,20, 40,70, thick);
    AUTONEW(Wall, 40,70, 0,100,  (thick+2));
    AUTONEW(Wall, 80,30, 80,80, thick);
    AUTONEW(Wall, 80,55, 130,55, thick);
    AUTONEW(Wall, 170,40, 170,100, thick);
    AUTONEW(Wall, 210,40, 210,100, thick);
    AUTONEW(Wall, 80,80, 80,130, thick);
    AUTONEW(Wall, 80,80, 120,100, thick);
    AUTONEW(Mirror, 167,100, 123,126, (thick+4));
    AUTONEW(Wall, 210,100, 160,130, (thick+3));
    AUTONEW(Mirror, 80,130, 120,100, (thick+2));
    AUTONEW(Wall, 120,130, 160,130, thick);
    AUTONEW(Wall, 100,160, 140,160, thick);
    AUTONEW(Wall, 280,80, 300,100, thick);
    AUTONEW(Wall, 40,110, 40,160, thick);
    AUTONEW(Wall, 40,160, 0,200, (thick+2));
    AUTONEW(Wall, 140,160, 170,200, thick);
    AUTONEW(Wall, 180,160, 230,120, (thick+2));
    AUTONEW(Wall, 180,160, 220,200, (thick));
    AUTONEW(Wall, 220,200, 180,240, (thick+2));
    AUTONEW(Wall, 100,200, 100,240, thick);
    AUTONEW(Wall, 250,200, 300,200, thick);
    // AUTONEW(Wall, , 1);
    // AUTONEW(Wall, , 1);
    // AUTONEW(Wall, , 1);

    MP(15,70); // 0
    MP(20,25);
    MP(40, 10);
    MP(60,20);
    MP(60,60);
    MP(20,100);
    MP(60,100);
    MP(20,160);
    MP(60,160);
    MP(60,180);
    MP(20,200); // 10
    MP(20,220);
    MP(60,220);
    MP(80,180);
    MP(80,155);
    MP(90,145);
    MP(120,117);
    MP(140,100);
    MP(95,70);
    MP(140,80);
    MP(150,60); // 20
    MP(120,20);
    MP(95,105);
    MP(190,50);
    MP(190,70);
    MP(190,100);
    MP(160,120);
    MP(120,180);
    MP(120,220);
    MP(160,220);
    MP(190,200); // 30
    MP(170,170);
    MP(160,145);
    MP(180,140);
    MP(230,100);
    MP(220,60);
    MP(280,60);
    MP(280,20);
    MP(220,15);
    MP(180,15);
    MP(260,80); // 40
    MP(270,130);
    MP(230,160);
    MP(270,160);
    MP(235,220);
    MP(270,220); // 45
    
    MP_Connect(0,1);
    MP_Connect(1,2);
    MP_Connect(2,3);
    MP_Connect(3,4);
    MP_Connect(3,21);
    MP_Connect(4,5);
    MP_Connect(4,6);
    MP_Connect(5,6);
    MP_Connect(5,7);
    MP_Connect(6,8);
    MP_Connect(6,14);
    MP_Connect(8,9);
    MP_Connect(8,14);
    MP_Connect(9,10);
    MP_Connect(9,12);
    MP_Connect(9,13);
    MP_Connect(9,14);
    MP_Connect(10,11);
    MP_Connect(11,12);
    MP_Connect(13,14);
    MP_Connect(13,27);
    MP_Connect(14,15);
    MP_Connect(15,16);
    MP_Connect(15,32);
    MP_Connect(16,17);
    MP_Connect(17,18);
    MP_Connect(17,19);
    MP_Connect(18,19);
    MP_Connect(18,20);
    MP_Connect(19,20);
    MP_Connect(20,21);
    MP_Connect(23,24);
    MP_Connect(24,25);
    MP_Connect(25,26);
    MP_Connect(27,28);
    MP_Connect(27,29);
    MP_Connect(28,29);
    MP_Connect(29,30);
    MP_Connect(30,31);
    MP_Connect(31,32);
    MP_Connect(32,33);
    MP_Connect(33,34);
    MP_Connect(34,35);
    MP_Connect(34,40);
    MP_Connect(34,41);
    MP_Connect(35,40);
    MP_Connect(36,40);
    MP_Connect(36,37);
    MP_Connect(37,38);
    MP_Connect(38,39);
    MP_Connect(40,41);
    MP_Connect(41,42);
    MP_Connect(41,43);
    MP_Connect(42,43);
    MP_Connect(42,44);
    MP_Connect(44,45);


    Enemy* en;
    Item* it;

    en = NEW(Enemy) Enemy(gamestate.levelReference.MapPoints[11], k/1.5*Vector2({5.f, 10.f}), {12, 9, 10, 11});
    en->viewAround = p->viewAround*0.75;
    en->viewLength = p->viewLength*0.75;
    en->speed = 1.5;
    ADD_OBJ(en);

    it = NEW(Pistol) Pistol({0,0}, {2*Point({10,-5}), 2*Point({-10,-5}), 2*Point({10,5}), 2*Point({-10,5})}, false);
    ADD_OBJ(it);
    {int idx = gamestate.levelReference.objects.size()-1;
    en->selfitem = idx;}

    en = NEW(Enemy) Enemy(gamestate.levelReference.MapPoints[22] + Vector2({-1,-1}), k/1.5*Vector2({5.f, 10.f}), {});
    en->viewAround = p->viewAround*0.75;
    en->viewLength = p->viewLength*0.75;
    en->speed = 1.5;
    en->direction = {1/sqrtf(2), -1/sqrtf(2)};
    ADD_OBJ(en);

    it = NEW(Pistol) Pistol({0,0}, {2*Point({10,-5}), 2*Point({-10,-5}), 2*Point({10,5}), 2*Point({-10,5})}, false);
    ADD_OBJ(it);
    {int idx = gamestate.levelReference.objects.size()-1;
    en->selfitem = idx;}

    it = NEW(Rifle) Rifle(k*Point({240,280}), {2*Point({10,-5}), 2*Point({-10,-5}), 2*Point({10,5}), 2*Point({-10,5})}, true);
    ADD_OBJ(it);

    en = NEW(Enemy) Enemy(gamestate.levelReference.MapPoints[0], k/1.5*Vector2({5.f, 10.f}), {3, 8, 19, 21});
    en->viewAround = p->viewAround*0.75;
    en->viewLength = p->viewLength*0.75;
    en->speed = 1.5;
    ADD_OBJ(en);

    it = NEW(Pistol) Pistol({0,0}, {2*Point({10,-5}), 2*Point({-10,-5}), 2*Point({10,5}), 2*Point({-10,5})}, false);
    ADD_OBJ(it);
    {int idx = gamestate.levelReference.objects.size()-1;
    en->selfitem = idx;}


    en = NEW(Enemy) Enemy(gamestate.levelReference.MapPoints[39], k/1.5*Vector2({5.f, 10.f}), {37, 35, 44, 43});
    en->viewAround = p->viewAround*0.75;
    en->viewLength = p->viewLength*0.75;
    en->speed = 1.5;
    ADD_OBJ(en);

    
}