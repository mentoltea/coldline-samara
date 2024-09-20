#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <raylib.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <array>
#include <math.h>
#include <iostream>
#include <assert.h>
#include <stdint.h>

namespace Cstd {
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
}

namespace MemManager {
extern "C" {
#include "memmanager.h"
}
}
namespace Json {
extern "C" {
#include "json.h"
}
}

float absf(float x);
size_t max(size_t, size_t);

#define NEW(T) new(MemManager::memloc(sizeof(T)))



typedef Vector2 Point;
typedef struct Poly {
    Point p1, p2, p3, p4;
} Poly;

typedef enum ObjType {
    UNKNOWN = 0,
    
    WALL,
    DOOR,
    MIRROR,

    ENTITY,
    PLAYER,
    ENEMY,

    ITEM,
} ObjType;

class Object;
class Obtacle;
class Wall; // final
class Mirror; // final
class Door; // final
class Entity;
class Player; // final
class Enemy; // final
#define MAX_OBJECT_SIZE \
    max(sizeof(Wall), \
    max(sizeof(Mirror), \
    max(sizeof(Door), \
    max(sizeof(Player),\
    max(sizeof(Enemy), 1)))))


typedef struct IntersectInfo {
    std::vector<Point> points;
    float distance;
    Object* ptr;
} IntersectInfo;


typedef struct GameState {
    size_t id_counter;
    bool fullscreen;
    int WinX, WinY;
    float WinXf, WinYf;
    int MapX, MapY;
    float MapXf, MapYf;
    int MAX_REFLECTIONS;
    std::list<Object*> Gobjects;
    std::list<Object*> GlevelReference;
    Player* Gplayer;
    bool pause;
    Point camera;
} GameState;

extern GameState gamestate;


#endif