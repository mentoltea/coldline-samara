#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <raylib.h>
#include <rlgl.h>
#include <vector>
#include <list>
#include <array>
#include <queue>
#include <stack>
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

typedef Vector2 Point;

float absf(float x);
size_t max(size_t, size_t);
float distance(const Point& p1, const Point& p2);
float distSquare(const Point& p1, const Point& p2);
float constraintBetween(float value, float low, float hight);

#define NEW(T) new(MemManager::memloc(sizeof(T)))

// #define NEW(T, ...) new(MemManager::memloc(sizeof(T))) T(__VA_ARGS__)

#define DELETE(T, O) (O)->~T(); MemManager::memfree((O));


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
// #define MAX_OBJECT_SIZE 
//     max(sizeof(Wall), 
//     max(sizeof(Mirror), 
//     max(sizeof(Door), 
//     max(sizeof(Player),
//     max(sizeof(Enemy), 1)))))

size_t ObjectSize(Object* obj);
// @note DOES NOT ALLOCATE MEMORY, ONLY COPING
void CopyObject(Object* to, Object* from);

typedef struct IntersectInfo {
    std::vector<Point > points;
    float distance;
    Object* ptr;
} IntersectInfo;

typedef struct CheatFlags {
    bool HideAndSeek = false; // See enemies hitboxes
    bool BigBrother = false; // See everything brightly
    bool InfinityIsNotLimit = false; // Unlimit your vision length
    bool ShowFPS = true;
} CheatFlags;

struct ConnectedPoint: public Point {
    std::vector<int > connections; // indexes in MapPoints
};

struct Level {
    int MapX, MapY;
    float MapXf, MapYf;
    std::vector<ConnectedPoint> MapPoints;
    CheatFlags cheats = {0};
    std::list<Object*> objects;
    Player* player = NULL;

    Level();
    Level(const Level& other);
    ~Level();

    Level& operator=(const Level& other);
    Level& operator=(Level&& other) = delete;

    void clear();
    void destroy();

    std::tuple<Point, int, float> nearPoint(const Point& p) const; // Point, index, distance
    std::stack<int> way(int fromIdx, int toIdx) const; // Next point on a way
};


typedef struct GameState {
    size_t id_counter;
    bool fullscreen;
    int WinX, WinY;
    float WinXf, WinYf;
    int MAX_REFLECTIONS;
    Level currentLevel;
    Level levelReference;
    bool pause;
    Point camera;
} GameState;

extern GameState gamestate;

namespace TextureManager {
typedef enum Tid {
    TPlayer,
} Tid;
}

namespace TM = TextureManager;

#endif