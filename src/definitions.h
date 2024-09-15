#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <raylib.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <array>
#include <math.h>
#include "iostream"

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
class Entity;

typedef struct IntersectInfo {
    std::vector<Point> points;
    float distance;
    Object* ptr;
} IntersectInfo;

extern size_t id_counter;
extern int WinX, WinY;
extern float WinXf, WinYf;
extern int MAX_REFLECTIONS;
extern float hview;
extern int Nray;
extern float delta;
extern std::list<Object*> Gobjects;


#endif