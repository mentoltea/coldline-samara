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
    float angle = 0;


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
                DrawTriangle(pos, inters[i-1].points[0], inters[i].points[0], {110,110,110,180});
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
        
        size_t idx = 1;
        int from = -1;
        int to = -1;
        int loopstep = 2;
        while (idx < maxlen) {
            for (int i=0; i<Nray; i+=loopstep) {
                if (inters[i].points.size() <= idx || i>=Nray-loopstep) {
                    if (to != -1) {
                        for (int j=from+1; j<=to; j++) {
                            if (i==j) continue;
                            int a = 180-idx*80;
                            Color col = {180,180,180, (unsigned char)(a>0? a: 0)};//{120,120,200,180};
                            if (idx%2==0) {
                                DrawTriangle(inters[j-1].points[idx-1], inters[j-1].points[idx],inters[j].points[idx-1],   col);
                                DrawTriangle(inters[j].points[idx-1], inters[j-1].points[idx],inters[j].points[idx],   col);
                            }
                            else {
                                DrawTriangle(inters[j-1].points[idx-1], inters[j].points[idx-1], inters[j-1].points[idx],  col);
                                DrawTriangle(inters[j].points[idx-1], inters[j].points[idx], inters[j-1].points[idx],  col);
                            }
                        }
                        from = -1;
                        to = -1;
                    }
                } else {
                    if (from == -1) {
                        from = i;
                        // DrawLineV(inters[from].points[idx-1],inters[from].points[idx], {140,140,250,220});
                    }
                    to = i;
                }
            }
            idx++;
            // break;
        }

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