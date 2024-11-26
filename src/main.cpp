#include "game.h"
#include "levelloader.h"
#include "ui.h"
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

bool LevelStateUpdate(int idx){
    // gamestate.fileLevels = GetFilesFromDirExt("./levels/", ".lvl");
    if (idx<0) {
        default_level();
        gamestate.levelIdx = idx;
        return true;
    }
    if (gamestate.fileLevels.empty()) {
        std::cout << "Levels not found, loading default" << std::endl;
        default_level();
        return true;
    }
    else if (idx >= (int)gamestate.fileLevels.size()) {
        return false;
    }
    else {
        gamestate.levelIdx = idx;
        do {
            std::cout << "Loading level " << gamestate.levelIdx << ": " << gamestate.fileLevels[gamestate.levelIdx] << std::endl;
            gamestate.levelReference = LoadLevel( gamestate.fileLevels[gamestate.levelIdx] );
            gamestate.levelIdx++;
        } while (gamestate.levelReference.errorFlag && gamestate.levelIdx < (int)gamestate.fileLevels.size());
        if (gamestate.levelReference.errorFlag) {
            gamestate.levelIdx = 0;
            gamestate.fileLevels.clear();
            std::cout << "All levels failed to load" << std::endl;
            default_level();
        }
    }
    return true;
};

bool LoadNextLevel() {
    if (gamestate.levelIdx<0) gamestate.levelIdx = 0;
    if (LevelStateUpdate(gamestate.levelIdx)){ 
        if (!gamestate.fileLevels.empty() && gamestate.levelIdx > 0) {
            gamestate.levelIdx = gamestate.levelIdx % gamestate.fileLevels.size();
        }
        ReloadLevel();
        return true;
    }
    gamestate.levelIdx = 0;
    return false;
}

int currentpage = 0;
int onpagecount = 7;
std::vector<UI::Button> MainMenuLevelsList;
std::vector<UI::Element*> MainMenuUI;
    
std::vector<UI::Element*> GameMenuUI;

void LevelsListUpdate() {
    gamestate.fileLevels = GetFilesFromDirExt("./levels/", ".lvl");
    if (!gamestate.fileLevels.empty()) {
        currentpage = currentpage % ((gamestate.fileLevels.size()-1)/onpagecount + 1);
    } else currentpage = 0;
    float padding = 0.15;
    float spacing = 0.03;
    float bsize = (1 - 2*padding)/(onpagecount);
    MainMenuLevelsList.clear();
    for (int i = currentpage*onpagecount;
        i < currentpage*onpagecount + std::min(onpagecount, (int)gamestate.fileLevels.size()-currentpage*onpagecount);
        i++) 
    {
        std::string name = gamestate.fileLevels[i];
        if (name.find('/') != std::string::npos) {
            name = name.substr(name.find_last_of('/')+1);
        }
        
        UI::Button b = UI::Button(
                (Vector2){padding, padding + (i-currentpage*onpagecount)*(bsize)},
                (Vector2){0.5, bsize-spacing},
                [](int){},
                name,
                RED, GRAY);
        b.padding.x = 0.03;
        b.padding.y = 0.13;
        MainMenuLevelsList.push_back(b);
    }
};

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
    MemManager::prealloc(PAGE_SIZE*8);

    SetWindowState(FLAG_WINDOW_HIGHDPI); 
    SetWindowState(FLAG_MSAA_4X_HINT);
    SetWindowState(FLAG_VSYNC_HINT); 
    SetWindowState(FLAG_WINDOW_ALWAYS_RUN); 
    // SetWindowState(FLAG_WINDOW_TRANSPARENT); 
    InitWindow(gamestate.WinX, gamestate.WinY, "Coldline Samara");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    // SetWindowState(FLAG_WINDOW_UNDECORATED); 

    if (gamestate.fullscreen) ToggleFullscreen();
    SetExitKey(KEY_F4);
    SetTargetFPS(FPS);


    TM::LoadT("assets/pistol.png", TM::TPistol);
    TM::LoadT("assets/rifle.png", TM::TRifle);
    TM::LoadT("assets/shotgun.png", TM::TShotgun);
    
    
    // 2----1
    // |    |
    // 4----3
    MemManager::page_info(0);

    auto updateFunc = [&]() { 
        using namespace std::chrono;
        SAFE_DRAWING = true;
        updateLatency = 0;
        int NSD_ticks = 0;
        int tk = 0;
        do {
            time_point from = steady_clock::now();
            if (!gamestate.pause) {
                PollInputEvents();
                update();
            }
            // cout << "here" << endl;
            time_point to = steady_clock::now();
            
            updateLatency = (double)duration_cast<microseconds>(to-from).count() /1000000.f;
            if (dt - updateLatency < 0) updateLatency = dt;

            if (!SAFE_DRAWING) {
                NSD_ticks++;
                if (NSD_ticks == 3) {
                    SAFE_DRAWING = true;
                    NSD_ticks = 0;
                }
            }
            tk++;
            WaitTime(dt - updateLatency);
            // cout << dt << "\t" << updateLatency << "\t"  << dt-updateLatency << endl;
        } while (!STOP);
    };
    bool updateThreadRunning = true;
    std::thread updateThread;

    bool swapped = false;
    
    UI::Button q((Vector2){0.7,0.1}, (Vector2){0.1,0.1},
        [] (int click) {QUIT = true;},
        std::string("QUIT"), GREEN, GRAY);
    MainMenuUI.push_back(&q);

    UI::Button d((Vector2){0.7,0.3}, (Vector2){0.25,0.12},
        [] (int click) {
            UDLevel();
            if (LevelStateUpdate(-1)) {ReloadLevel(); gamestate.gamestep = GAME;}
        },
        std::string("DEFAULT\n\n\nLEVEL"), GREEN, GRAY);
    MainMenuUI.push_back(&d);


    UI::Button bp = UI::Button((Vector2){0.15,0.05}, (Vector2){0.05,0.05},
        [] (int) {currentpage++; LevelsListUpdate();},
        std::string("+") , GREEN, GRAY);
    MainMenuUI.push_back(&bp);

    UI::Button bm = UI::Button((Vector2){0.22,0.05}, (Vector2){0.05,0.05},
        [] (int) {currentpage--; LevelsListUpdate();},
        std::string("-") , GREEN, GRAY);
    MainMenuUI.push_back(&bm);



    UI::Button men = UI::Button((Vector2){0.05,0.8}, (Vector2){0.12,0.1},
        [] (int) {MENU = true;},
        std::string("MENU") , GREEN, GRAY);
    GameMenuUI.push_back(&men);

    UI::Button rest = UI::Button((Vector2){0.05,0.65}, (Vector2){0.2,0.1},
        [] (int) {RELOAD = true; gamestate.pause = false;},
        std::string("RESTART") , GREEN, GRAY);
    GameMenuUI.push_back(&rest);

    while (!WindowShouldClose() && !QUIT) {
        if (IsKeyPressed(KEY_F11)) {
            gamestate.fullscreen = !gamestate.fullscreen;
            ToggleFullscreen();
        }
        switch (gamestate.gamestep) {
            case MAIN_MENU: {
                if (updateThreadRunning) {
                    MemManager::page_info(0);
                    STOP = true;
                    WaitTime(0.05);
                    if (updateThread.joinable()) updateThread.join();
                    updateThreadRunning = false;
                    currentpage = 0;
                    LevelsListUpdate();
                }

                bool clicked = false;
                Point clickpoint;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    clicked = true;
                    clickpoint = GetMousePosition();
                    clickpoint.x /= gamestate.WinX;
                    clickpoint.y /= gamestate.WinY;
                }

                BeginDrawing();
                ClearBackground({25,25,25,150});
                for (auto elem: MainMenuUI) {
                    elem->update();
                    if (clicked) {
                        if ((clickpoint.x >= elem->position.x && clickpoint.x <= elem->position.x + elem->box.x)
                        && (clickpoint.y >= elem->position.y && clickpoint.y <= elem->position.y + elem->box.y)) {
                            elem->click();
                        }
                    }
                    elem->draw(gamestate.WinX, gamestate.WinY);
                }

                int listidx = 0;
                for (auto &but: MainMenuLevelsList) {
                    but.update();
                    if (clicked) {
                        if ((clickpoint.x >= but.position.x && clickpoint.x <= but.position.x + but.box.x)
                        && (clickpoint.y >= but.position.y && clickpoint.y <= but.position.y + but.box.y)) {
                            but.click();
                            if (LevelStateUpdate(listidx + currentpage*onpagecount)) {
                                ReloadLevel();
                                gamestate.gamestep = GAME;
                            }
                        }
                    }
                    but.draw(gamestate.WinX, gamestate.WinY);
                    listidx++;
                }
                char textbuff[16];
                snprintf(textbuff, 16, "Page %d", currentpage);
                DrawText(textbuff, gamestate.WinX*0.1, gamestate.WinY*0.05, 15, GREEN);

                EndDrawing();
            } break;
            case GAME: {
                if (!updateThreadRunning) {
                    STOP = false;
                    SAFE_DRAWING = false;
                    gamestate.pause = false;
                    updateThread = std::thread(updateFunc);
                    WaitTime(0.15);
                    updateThreadRunning = true;
                }

                if (IsWindowResized()) {
                    gamestate.WinX = GetScreenWidth();
                    gamestate.WinY = GetScreenHeight();
                    gamestate.WinXf = gamestate.WinX;
                    gamestate.WinYf = gamestate.WinY;
                }

                if (IsKeyPressed(KEY_ESCAPE)) gamestate.pause = !gamestate.pause;
                if ((IsKeyPressed(KEY_R) || RELOAD) && !gamestate.pause) {
                    gamestate.pause = true;
                    RELOAD = true;
                    WaitTime(4*dt);
                    ReloadLevel(); 
                    RELOAD = false;
                    gamestate.pause = false;
                }
                
                if (IsKeyPressed(KEY_I)) {
                    gamestate.currentLevel.cheats.invisible = !gamestate.currentLevel.cheats.invisible;
                }
                if (IsKeyPressed(KEY_P)) {
                    gamestate.currentLevel.cheats.see_MP = !gamestate.currentLevel.cheats.see_MP;
                }

                if (IsKeyPressed(KEY_M)) {
                    MENU = true;
                }
                if (MENU) {
                    MENU = false;
                    gamestate.pause = true;
                    WaitTime(2*dt);

                    UDLevel();
                    gamestate.gamestep = MAIN_MENU;
                    continue;
                }

                if (IsKeyPressed(KEY_N) && gamestate.levelComplete) {
                    gamestate.pause = true;
                    RELOAD = true;
                    WaitTime(4*dt);

                    UDLevel();
                    if (LoadNextLevel()) {
                        RELOAD = false;
                        gamestate.pause = false;
                    } else {

                        UDLevel();
                        RELOAD = false;
                        gamestate.gamestep = MAIN_MENU;
                        continue;
                    }
                }

                BeginDrawing();

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

                if (gamestate.pause) {
                    bool clicked = false;
                    Point clickpoint;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        clicked = true;
                        clickpoint = GetMousePosition();
                        clickpoint.x /= gamestate.WinX;
                        clickpoint.y /= gamestate.WinY;

                    }
                    for (auto elem: GameMenuUI) {
                        elem->update();
                        if (clicked) {
                            if ((clickpoint.x >= elem->position.x && clickpoint.x <= elem->position.x + elem->box.x)
                            && (clickpoint.y >= elem->position.y && clickpoint.y <= elem->position.y + elem->box.y)) {
                                elem->click();
                                
                            }
                        }
                        elem->draw(gamestate.WinX, gamestate.WinY);
                    }
                }

                EndDrawing();
            } break;
        }
    }
    STOP = true;
    if (updateThread.joinable()) updateThread.join();
    CloseWindow();
    MemManager::page_info(0);

    // SaveLevel(gamestate.levelReference, "levels/l1.lvl");

    UDLevel();


    TextureManager::UnloadT();
    
    MemManager::page_info(0);
    MemManager::destroy_pages();

    std::cout << "Finished succesfully" << std::endl;
    return 0;
}






void default_level() {
    gamestate.levelReference.destroy();
    gamestate.levelReference.errorFlag = false;
    gamestate.levelReference.MapX = 1000;
    gamestate.levelReference.MapY = 800;
    gamestate.levelReference.MapXf = gamestate.levelReference.MapX;
    gamestate.levelReference.MapYf = gamestate.levelReference.MapY;
    gamestate.levelReference.name = "Default";

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