#include "game.h"

Vector2 operator+(const Vector2& v1, const Vector2& v2) {
    return (Vector2){.x = v1.x + v2.x, .y = v1.y + v2.y};
}
Vector2 operator-(const Vector2& v1, const Vector2& v2) {
    return (Vector2){.x = v1.x - v2.x, .y = v1.y - v2.y};
}

Vector2 operator*(const Vector2 v, float k) {
    return (Vector2){.x = v.x*k, .y = v.y*k};
}
Vector2 operator*(float k, const Vector2 v) {
    return (Vector2){.x = v.x*k, .y = v.y*k};
}

float absf(float x) {
    if (x<0) return -x;
    return x;
}

int signf(float x) {
    if (x<0) return -1;
    if (x>0) return 1;
    return 0;
}

float randf() {
    return (float) rand() / (float) RAND_MAX;
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

namespace ObjectExamples {
// THESE EXAMPLES ARE ONLY TO GET VPTR TO VTABLE
Wall ExampleWall;
Door ExampleDoor;
Mirror ExampleMirror;
Enemy ExampleEnemy;
Player ExamplePlayer;
TextSegment ExampleTextSegment;
Pistol ExamplePistol;
Rifle ExampleRifle;
}
