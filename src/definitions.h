#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <raylib.h>
#include <rlgl.h>
#include <vector>
#include <list>
#include <array>
#include <unordered_map>
#include <math.h>
#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <memory>

namespace Cstd {
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
}


#include "MManager.hpp"
#include "texturemanager.h"

namespace Json {
extern "C" {
#include "json.h"
}
}

float absf(float x);
size_t max(size_t, size_t);

#define NEW(T) new(MemManager::memloc(sizeof(T)))
#define DELETE(T, O) O->~T(); MemManager::memfree(O);


typedef Vector2 Point;
typedef struct Poly {
    Point p1, p2, p3, p4;
} Poly;

typedef enum ObjType {
    UNKNOWN = 0,
    
    OBTACLE, // General
    WALL,
    TEXTSEGMENT,
    DOOR,
    MIRROR,

    ENTITY, // General
    PLAYER,
    ENEMY,

    ITEM, // General

    PROJECTILE, // General
} ObjType;

class Object;
class Obtacle;
class Wall; // final
class TextSegment; //final
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
    std::vector<Point, MemManager::Allocator<Point> > points;
    float distance;
    Object* ptr;
} IntersectInfo;

typedef struct CheatFlags {
    bool HideAndSeek = false; // See enemies hitboxes
    bool BigBrother = false; // See everything brightly
    bool InfinityIsNotLimit = false; // Unlimit your vision length
    bool ShowFPS = true;
} CheatFlags;

typedef struct GameState {
    size_t id_counter;
    bool fullscreen;
    int WinX, WinY;
    float WinXf, WinYf;
    int MapX, MapY;
    float MapXf, MapYf;
    int MAX_REFLECTIONS;
    std::list<Object*, MemManager::Allocator<Object*> > Gobjects;
    std::list<Object*, MemManager::Allocator<Object*> > GlevelReference;
    Player* Gplayer;
    bool pause;
    Point camera;
    CheatFlags cheats;
} GameState;

extern GameState gamestate;

namespace TextureManager {
typedef enum Tid {
    TPlayer,
} Tid;
}

namespace TM = TextureManager;

#endif