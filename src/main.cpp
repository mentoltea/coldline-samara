#include "game.h"
#include <raylib.h>



using std::cout, std::endl;
int main() {
    MemManager::prealloc(PAGE_SIZE*2);
    // 2----1
    // |    |
    // 4----3

    Wall *w = NEW(Wall) Wall({{300, 200}, {200, 200}, {250, 400}, {200, 400},});
    Gobjects.push_back(w);

    Mirror *m = NEW(Mirror) Mirror({{500, 100}, {490, 100}, {600, 200}, {590, 200}}, {-1, 1});
    Gobjects.push_back(m);

    Mirror *m2 = NEW(Mirror) Mirror({{600, 200}, {590, 200}, {500, 300}, {490, 300}}, {-1, -1});
    Gobjects.push_back(m2);


    

    Enemy *en = NEW(Enemy) Enemy({450, 500}, {10, 20});
    en->direction = {-1, 0};
    Gobjects.push_back(en);
    
    Enemy *en2 = NEW(Enemy) Enemy({500, 500}, {10, 20});
    en2->direction = {0, -1};
    Gobjects.push_back(en2);

    Point pos = {700, 500};
    Player *p = NEW(Player) Player(pos, {10, 20});
    Gobjects.push_back(p);
    Gplayer = p;


    MemManager::page_info(0);

    InitWindow(WinX, WinY, "Coldline Samara");
    SetExitKey(KEY_F4);
    SetTargetFPS(60);
    WaitTime(0.1);
    for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
        if (!(*it)->active) continue;
        (*it)->update();
    }
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({25,25,25,255});
            
        Vector2 move = {0};
        if (IsKeyDown(KEY_A)) {
            move.x -= 4;
        } else if (IsKeyDown(KEY_D)) {
            move.x += 4;
        }

        if (IsKeyDown(KEY_W)) {
            move.y -= 4;
        } else if (IsKeyDown(KEY_S)) {
            move.y += 4;
        }
        p->move = move;
        
        for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
            if (!(*it)->active) continue;
            (*it)->update();
            // if ((*it)->visible) {
            //     (*it)->draw();
            // }
        }
        for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
            if (!(*it)->active) continue;
            // (*it)->update();
            if ((*it)->visible) {
                (*it)->draw();
            }
        }

        

        DrawFPS(0,0);
        EndDrawing();
    }
    CloseWindow();

    MemManager::page_info(0);
    MemManager::destroy_pages();

    return 0;
}