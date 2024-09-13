#include "game.h"
#include <raylib.h>


using std::cout, std::endl;
int main() {
    InitWindow(WinX, WinY, "Coldline Samara");
    SetTargetFPS(60);
    SetExitKey(KEY_F4);

    Wall w({{300, 200}, {200, 200}, {250, 400},    {200, 400},});
    Gobjects.push_back(&w);

    Mirror m({{500, 100}, {495, 100}, {500, 140}, {495, 140}}, {-1, 0});
    Gobjects.push_back(&m);

    Point pos = {700, 500};
    float angle = 0;
    int Nray = 120;
    float delta = 2*hview/Nray;

    std::vector<IntersectInfo> inters(Nray);

    Point mouse;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({25,25,25,255});

        for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
            if (!(*it)->active) continue;
            (*it)->update();
        }

        mouse = GetMousePosition();
        angle = atan2f(mouse.y-pos.y, mouse.x-pos.x)*180/PI;
        float a = angle + hview;
        size_t maxlen = 0;
        for (int i=0; i<Nray; i++) {
            a -= delta;
            inters[i] = raycast(pos, a, 1, NULL);
            maxlen = inters[i].points.size() > maxlen ? inters[i].points.size() : maxlen;
            if (i>0) {
                DrawTriangle(pos, inters[i-1].points[0], inters[i].points[0], {180,180,180,180});
            }
        }

        {
        Point curr = pos;
        Point next;
        for (int i=0; i<(int)inters[Nray/2].points.size(); i++) {
            next = inters[Nray/2].points[i];
            DrawLineV(curr, next, {210, 0, 0, 210});
            curr = next;
        }
        }

        // size_t idx = 1;
        // int from = -1;
        // int to = -1;
        // while (idx <= maxlen) {
        //     for (int i=0; i<(int)inters.size(); i++) {
        //         if (inters[i].points.size() >= idx) {
        //             if (from == -1) from = i;
        //         } else {
        //             if (from != -1) {
        //                 to = i;
        //                 DrawTriangle(inters[from].points[idx-1], inters[from].points[idx], inters[to].points[idx], {140,140,180,180});
        //                 DrawTriangle(inters[to].points[idx-1], inters[from].points[idx], inters[to].points[idx], {140,140,180,180});
        //                 from = -1;
        //             }
        //         }
        //     }
        //     idx++;
        // }

        if (IsKeyDown(KEY_A)) {
            pos.x -= 2;
        } else if (IsKeyDown(KEY_D)) {
            pos.x += 2;
        }

        if (IsKeyDown(KEY_W)) {
            pos.y -= 2;
        } else if (IsKeyDown(KEY_S)) {
            pos.y += 2;
        }
        
        for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
            if (!(*it)->active) continue;
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