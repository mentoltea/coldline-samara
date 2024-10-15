#include "game.h"
#include "levelloader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <raylib.h>
#include <iostream>

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

int TICK = 60;
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
    SetTargetFPS(TICK);
    
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

    Shotgun* rif = NEW(Shotgun) Shotgun({600, 200}, {{10,-5}, {-10,-5}, {10,5}, {-10,5}}, true);
    gamestate.levelReference.objects.push_back(rif);

    // en = NEW(Enemy) Enemy({600, 400}, {10, 20});
    // en->direction = {-1, 0};
    // en->selfway = {6, 1, 3};
    // gamestate.levelReference.objects.push_back(en);

    // Door *dr = NEW(Door) Door(-360, 360, 0, {200,200}, {30, 3}, {30,3});
    // gamestate.levelReference.objects.push_back(dr);


    // gamestate.levelReference = LoadLevel("text.level");
    // ReloadLevel();
    // ReloadLevel();
    gamestate.levelReference.cheats.invisible = true;
    ReloadLevel();
    // gamestate.currentLevel = gamestate.levelReference;
            
    // 2----1
    // |    |
    // 4----3

    MemManager::page_info(0);


    char buffer[64] = {0};
    int buffer_idx = 0;

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
            ReloadLevel();
        }
        
        if (IsKeyPressed(KEY_I)) {
            gamestate.currentLevel.cheats.invisible = !gamestate.currentLevel.cheats.invisible;
        }

        BeginDrawing();
        if (!gamestate.pause) update();
        draw();
        DrawFPS(0,0);
        EndDrawing();
    }
    CloseWindow();
    MemManager::page_info(0);

    // auto s = gamestate.levelReference.way(4, 0);
    // while (!s.empty()) {
    //     auto x = s.top();
    //     cout << x << " ";
    //     s.pop();
    // }
    // cout << endl;

    // UnloadLevel(gamestate.currentLevel);
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