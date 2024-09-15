#include "game.h"
#include <raylib.h>


using std::cout, std::endl;
int main() {
    InitWindow(WinX, WinY, "Coldline Samara");
    SetTargetFPS(60);
    SetExitKey(KEY_F4);

    Wall w({{300, 200}, {200, 200}, {250, 400},    {200, 400},});
    Gobjects.push_back(&w);

    Mirror m({{500, 100}, {495, 100}, {600, 200}, {595, 200}}, {-1, 1});
    Gobjects.push_back(&m);

    Mirror m2({{600, 200}, {595, 200}, {500, 300}, {495, 300}}, {-1, -1});
    Gobjects.push_back(&m2);

    Point pos = {700, 500};

    Player p(pos, {10, 10});
    Gobjects.push_back(&p);


    WaitTime(0.1);
    for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
        if (!(*it)->active) continue;
        (*it)->update();
    }
    Point mouse;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({25,25,25,255});

        // for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
        //     if (!(*it)->active) continue;
        //     (*it)->update();
        // }

        mouse = GetMousePosition();
        
        
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
        p.move = move;
        
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

    // page_info(0);
    destroy_pages();


    return 0;
}