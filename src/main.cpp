#include "game.h"
#include "levelloader.h"
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

    if (gamestate.fileLevels.empty()) {
        std::cout << "Levels not found, loading default" << std::endl;
        default_level();
    }
    else {
        gamestate.levelIdx = 0;
        do {
            gamestate.levelReference = LoadLevel( gamestate.fileLevels[gamestate.levelIdx] );
            gamestate.levelIdx++;
        } while (gamestate.levelReference.errorFlag && gamestate.levelIdx < gamestate.fileLevels.size());
        if (gamestate.levelReference.errorFlag) {
            gamestate.fileLevels.clear();
            default_level();
        }
    }

    ReloadLevel();
            
    // 2----1
    // |    |
    // 4----3

    MemManager::page_info(0);


    char buffer[64] = {0};
    int buffer_idx = 0;

    std::thread updateThread([&]() { 
        using namespace std::chrono;
        SAFE_DRAWING = true;
        updateLatency = 0;
        do {
            
            time_point from = steady_clock::now();
            PollInputEvents();
            if (!gamestate.pause) update();
            time_point to = steady_clock::now();
            
            updateLatency = (double)duration_cast<microseconds>(to-from).count() /1000000.f;
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






void default_level() {
    gamestate.levelReference.clear();
    gamestate.levelReference.MapX = 1000;
    gamestate.levelReference.MapY = 800;
    gamestate.levelReference.MapXf = gamestate.levelReference.MapX;
    gamestate.levelReference.MapYf = gamestate.levelReference.MapY;

    ConnectedPoint cp;

    //0
    cp.x = 40;
    cp.y = 40;
    cp.connections = {1, 2};
    gamestate.levelReference.MapPoints.push_back(cp);

    //1
    cp.x = 200;
    cp.y = 50;
    cp.connections = {0, 4, 5};
    gamestate.levelReference.MapPoints.push_back(cp);

    //2
    cp.x = 40;
    cp.y = 100;
    cp.connections = {0, 3};
    gamestate.levelReference.MapPoints.push_back(cp);

    //3
    cp.x = 140;
    cp.y = 140;
    cp.connections = {2, 4, 5};
    gamestate.levelReference.MapPoints.push_back(cp);
    
    //4
    cp.x = 250;
    cp.y = 250;
    cp.connections = {1, 3, 6};
    gamestate.levelReference.MapPoints.push_back(cp);

    //5
    cp.x = 300;
    cp.y = 140;
    cp.connections = {1, 3, 6};
    gamestate.levelReference.MapPoints.push_back(cp);

    //6
    cp.x = 500;
    cp.y = 500;
    cp.connections = {4, 5};
    gamestate.levelReference.MapPoints.push_back(cp);

    Player *p = NEW(Player) Player({500, 400}, {10, 20});
    gamestate.levelReference.objects.push_back(p);


    Wall *w;

    // w = NEW(Wall) Wall({ 
    //     {gamestate.levelReference.MapXf, 0}, 
    //     {0, 0},
    //     {gamestate.levelReference.MapXf, 10}, 
    //     {0, 10} });
    // gamestate.levelReference.objects.push_back(w);

    w = NEW(Wall) Wall({ 
        {gamestate.levelReference.MapXf, gamestate.levelReference.MapYf-10}, 
        {0, gamestate.levelReference.MapYf-10},
        {gamestate.levelReference.MapXf, gamestate.levelReference.MapYf}, 
        {0, gamestate.levelReference.MapYf} });
    gamestate.levelReference.objects.push_back(w);


    w = NEW(Wall) Wall({ 
        {10, 0}, 
        {0, 0},
        {10, gamestate.levelReference.MapYf}, 
        {0, gamestate.levelReference.MapYf} });
    gamestate.levelReference.objects.push_back(w);

    w = NEW(Wall) Wall({ 
        {gamestate.levelReference.MapXf, 0}, 
        {gamestate.levelReference.MapXf-10, 0},
        {gamestate.levelReference.MapXf, gamestate.levelReference.MapYf}, 
        {gamestate.levelReference.MapXf-10, gamestate.levelReference.MapYf} });
    gamestate.levelReference.objects.push_back(w);


    Enemy *en = NEW(Enemy) Enemy({200, 600}, {10, 20}, { 4, 1, 0, 2});
    en->direction = {-1, 0};
    gamestate.levelReference.objects.push_back(en);

    Pistol* pis = NEW(Pistol) Pistol({600, 200}, {{10,-5}, {-10,-5}, {10,5}, {-10,5}}, false);
    gamestate.levelReference.objects.push_back(pis);
    pis->extrarounds = 0;
    {int idx = gamestate.levelReference.objects.size()-1;
    en->selfitem = idx;}

    Shotgun* rif = NEW(Shotgun) Shotgun({600, 200}, {(Vector2){30,-15}, (Vector2){-30,-15}, (Vector2){30,15}, (Vector2){-30,15}}, true);
    gamestate.levelReference.objects.push_back(rif);

    // en = NEW(Enemy) Enemy({600, 400}, {10, 20});
    // en->direction = {-1, 0};
    // en->selfway = {6, 1, 3};
    // gamestate.levelReference.objects.push_back(en);

    Mirror *dr = NEW(Mirror) Mirror({{500, 700}, {495, 700}, {550, 750}, {545, 750}, }, {-1, 1});
    gamestate.levelReference.objects.push_back(dr);


    // gamestate.levelReference = LoadLevel("text.level");
    // ReloadLevel();
    // ReloadLevel();
    gamestate.levelReference.cheats.invisible = true;

    // gamestate.levelReference = LoadLevel("levels/l1.lvl");
}