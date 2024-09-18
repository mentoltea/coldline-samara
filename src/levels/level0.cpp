#include "../game.h"

extern "C" {
void load_level(GameState *gamestate) {
    Wall *w = NEW(Wall) Wall({{300, 200}, {200, 200}, {250, 400}, {200, 400},});
    gamestate->Gobjects.push_back(w);

    Mirror *m = NEW(Mirror) Mirror({{500, 100}, {490, 100}, {600, 200}, {590, 200}}, {-1, 1});
    gamestate->Gobjects.push_back(m);

    Mirror *m2 = NEW(Mirror) Mirror({{600, 200}, {590, 200}, {500, 300}, {490, 300}}, {-1, -1});
    gamestate->Gobjects.push_back(m2);

    Enemy *en = NEW(Enemy) Enemy({450, 500}, {10, 10});
    en->direction = {-1, 0};
    gamestate->Gobjects.push_back(en);
    
    Enemy *en2 = NEW(Enemy) Enemy({500, 500}, {10, 20});
    en2->direction = {0, 1};
    gamestate->Gobjects.push_back(en2);

    Point pos = {700, 500};
    Player *p = NEW(Player) Player(pos, {10, 20});
    gamestate->Gobjects.push_back(p);
    gamestate->Gplayer = p;
}
}