#include "game.h"
#include <raylib.h>
#include <iostream>
#include <libloaderapi.h>
#include <errhandlingapi.h>

extern "C" {
#include <string.h>
void load_settings(FILE* fd) {
    Json::init_json();
    Json::json_child json = Json::read_json(fd);
    size_t len = Json::vec_metaptr(json.fields)->length;
    for (size_t i=0; i<len; i++) {
        if (strcmp(json.fields[i].key, "fullscreen")==0) {
            gamestate.fullscreen = json.fields[i].value.data.num;
        } else if (strcmp(json.fields[i].key, "WinX")==0) {
            gamestate.WinX = json.fields[i].value.data.num;
            gamestate.WinXf = gamestate.WinX;
        } else if (strcmp(json.fields[i].key, "WinY")==0) {
            gamestate.WinY = json.fields[i].value.data.num;
            gamestate.WinYf = gamestate.WinY;
        } else if (strcmp(json.fields[i].key, "MAX_REFLECTIONS")==0) {
            gamestate.MAX_REFLECTIONS = json.fields[i].value.data.num;
        }
    }
}
}


HMODULE gamelib = NULL;
bool LoadGameLibrary(const char* filename) {
    if (gamelib!=NULL) FreeLibrary(gamelib);
    gamelib = LoadLibraryA(filename);

    if (gamelib==NULL) return false;

    void (*load_level)(GameState* gs) = (void (*)(GameState*))GetProcAddress(gamelib, "load_level");
    if (load_level==NULL) return false;
    load_level(&gamestate);
    // FreeLibrary(gamelib);
    gamelib = NULL;

    return true;
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

    
    char* libfilename = (char*)"levels/level0.dll";
    if (argc>=2) {
        libfilename = argv[1];
    }

    MemManager::prealloc(PAGE_SIZE*2);
    
    // if (!LoadGameLibrary(libfilename)) {
    //     MemManager::destroy_pages();
    //     fprintf(stderr, "Cannot open %s. ERROR: %ld\n", libfilename, GetLastError());
    //     return 1;
    // }
    
    Wall *w = NEW(Wall) Wall({{300, 200}, {200, 200}, {250, 400}, {200, 400},});
    gamestate.Gobjects.push_back(w);

    Mirror *m = NEW(Mirror) Mirror({{500, 100}, {490, 100}, {600, 200}, {590, 200}}, {-1, 1});
    gamestate.Gobjects.push_back(m);

    Mirror *m2 = NEW(Mirror) Mirror({{600, 200}, {590, 200}, {500, 300}, {490, 300}}, {-1, -1});
    gamestate.Gobjects.push_back(m2);

    Enemy *en = NEW(Enemy) Enemy({450, 500}, {10, 10});
    en->direction = {-1, 0};
    gamestate.Gobjects.push_back(en);
    
    Enemy *en2 = NEW(Enemy) Enemy({500, 500}, {10, 20});
    en2->direction = {0, 1};
    gamestate.Gobjects.push_back(en2);

    Point pos = {700, 500};
    Player *p = NEW(Player) Player(pos, {10, 20});
    gamestate.Gobjects.push_back(p);
    gamestate.Gplayer = p;

    // 2----1
    // |    |
    // 4----3

    MemManager::page_info(0);


    InitWindow(gamestate.WinX, gamestate.WinY, "Coldline Samara");
    if (gamestate.fullscreen) ToggleFullscreen();
    SetExitKey(KEY_F4);
    SetTargetFPS(60);
    WaitTime(0.1);

    
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (IsKeyPressed(KEY_ESCAPE)) gamestate.pause = !gamestate.pause;
        

        if (!gamestate.pause) update();
        draw();
        DrawFPS(0,0);
        EndDrawing();
    }
    CloseWindow();

    MemManager::page_info(0);
    MemManager::destroy_pages();

    return 0;
}