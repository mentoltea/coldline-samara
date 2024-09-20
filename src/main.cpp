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
    

    // Player *p = NEW(Player) Player({500, 400}, {10, 20});
    // gamestate.GlevelReference.push_back(p);

    // TextSegment *tg = NEW(TextSegment) TextSegment({{450, 100}, {100, 100}, {450, 250}, {100, 250}}, "THIS IS\n\n\n\nMENU LEVEL", 50);
    // tg->offset = {10, 10};
    // gamestate.GlevelReference.push_back(tg);

    LoadLevel("text.level");

    ReloadLevel();
    
    // 2----1
    // |    |
    // 4----3

    MemManager::page_info(0);


    InitWindow(gamestate.WinX, gamestate.WinY, "Coldline Samara");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_VSYNC_HINT); 
    SetWindowState(FLAG_WINDOW_UNDECORATED); 
    SetWindowState(FLAG_WINDOW_ALWAYS_RUN); 
    SetWindowState(FLAG_WINDOW_TRANSPARENT); 
    SetWindowState(FLAG_WINDOW_HIGHDPI); 
    SetWindowState(FLAG_MSAA_4X_HINT);

    if (gamestate.fullscreen) ToggleFullscreen();
    SetExitKey(KEY_F4);
    SetTargetFPS(60);
    WaitTime(0.1);

    char buffer[64] = {0};
    int buffer_idx = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
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
    // SaveLevel("text.level");
    UnloadLevel();


    MemManager::page_info(0);
    MemManager::destroy_pages();

    return 0;
}