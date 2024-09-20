#include "game.h"

float absf(float x) {
    if (x<0) return -x;
    return x;
}

size_t max(size_t s1, size_t s2) {
    if (s1>s2) return s1;
    return s2;
}

GameState gamestate = {
    .id_counter = 0,
    // std::list<Object*> Gobjects ,
    .fullscreen = 0 ,
    .WinX = 16*80 ,
    .WinY = 9*80 ,
    .WinXf = (float)gamestate.WinX , 
    .WinYf = (float)gamestate.WinY ,
    .MapX = 1000 ,
    .MapY = 800 ,
    .MapXf = (float)gamestate.MapX , 
    .MapYf = (float)gamestate.MapY ,
    .MAX_REFLECTIONS = 20 ,
    .Gplayer = NULL ,
    .pause = false ,
    .camera = {0,0},
};