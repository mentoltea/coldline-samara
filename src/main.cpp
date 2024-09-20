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
    
    // Wall *w;
    // Enemy *en2 = NEW(Enemy) Enemy({600, 700}, {10, 20});
    // en2->direction = {0, 1};
    // gamestate.Gobjects.push_back(en2);

    // en2 = NEW(Enemy) Enemy({300, 700}, {30, 10});
    // en2->direction = {0, 1};
    // gamestate.Gobjects.push_back(en2);

    // Mirror *m = NEW(Mirror) Mirror({{500, 100}, {490, 100}, {600, 200}, {590, 200}}, {-1, 1});
    // gamestate.Gobjects.push_back(m);

    // Mirror *m2 = NEW(Mirror) Mirror({{600, 200}, {590, 200}, {500, 300}, {490, 300}}, {-1, -1});
    // gamestate.Gobjects.push_back(m2);

    // w = NEW(Wall) Wall({{300, 200}, {200, 200}, {250, 400}, {200, 400},});
    // gamestate.Gobjects.push_back(w);

    // Door *d = NEW(Door) Door(-170, 170, 0, {450,500}, {50, 5}, {50, 5});
    // gamestate.Gobjects.push_back(d);

    // w = NEW(Wall) Wall({{450, 495}, {0, 495}, {450, 505}, {0, 505},});
    // gamestate.Gobjects.push_back(w);

    //  w = NEW(Wall) Wall({{gamestate.MapXf, 495}, {550, 495}, {gamestate.MapXf, 505}, {550, 505},});
    // gamestate.Gobjects.push_back(w);

    // Point pos = {700, 550};
    // Player *p = NEW(Player) Player(pos, {10, 20});
    // gamestate.Gplayer = p;
    // gamestate.Gobjects.push_back(p);

    
    LoadLevel("demo.level");
    ReloadLevel();

    
    // 2----1
    // |    |
    // 4----3

    MemManager::page_info(0);


    InitWindow(gamestate.WinX, gamestate.WinY, "Coldline Samara");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    // ToggleBorderlessWindowed();
    if (gamestate.fullscreen) ToggleFullscreen();
    SetExitKey(KEY_F4);
    SetTargetFPS(60);
    WaitTime(0.1);

    
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (IsWindowResized()) {
            gamestate.WinX = GetScreenWidth();
            gamestate.WinY = GetScreenHeight();
            gamestate.WinXf = gamestate.WinX;
            gamestate.WinYf = gamestate.WinY;
        }

        if (IsKeyPressed(KEY_ESCAPE)) gamestate.pause = !gamestate.pause;
        if (IsKeyPressed(KEY_F11)) {
            gamestate.fullscreen = !gamestate.fullscreen;
            ToggleFullscreen();
        }
        if (IsKeyPressed(KEY_R)) {
            ReloadLevel();
        }
        if (!gamestate.pause) update();
        draw();
        DrawFPS(0,0);
        EndDrawing();
    }
    CloseWindow();

    // SaveLevel("demo.level");

    MemManager::page_info(0);
    MemManager::destroy_pages();

    return 0;
}