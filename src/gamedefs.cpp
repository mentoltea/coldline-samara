#include "game.h"

float absf(float x) {
    if (x<0) return -x;
    return x;
}

size_t max(size_t s1, size_t s2) {
    if (s1>s2) return s1;
    return s2;
}

float distance(const Point& p1, const Point& p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return sqrtf(dx*dx + dy*dy);
}
float distSquare(const Point& p1, const Point& p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return dx*dx + dy*dy;
}

float constraintBetween(float value, float low, float hight) {
    float diff = (hight-low)/2;
    while (value < low) {
        value += diff;
    }
    while (value > hight) {
        value -= diff;
    }
    return value;
}

GameState gamestate = {
    .id_counter = 0,
    // std::list<Object*> Gobjects ,
    .fullscreen = 0 ,
    .WinX = 16*80 ,
    .WinY = 9*80 ,
    .WinXf = (float)gamestate.WinX , 
    .WinYf = (float)gamestate.WinY ,
    // .MapX = 1000 ,
    // .MapY = 800 ,
    // .MapXf = (float)gamestate.MapX , 
    // .MapYf = (float)gamestate.MapY ,
    .MAX_REFLECTIONS = 20 ,
    // .Gplayer = NULL ,
    .pause = false ,
    .camera = {0,0},
};