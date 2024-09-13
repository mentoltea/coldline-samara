#ifndef GAME_H
#define GAME_H

#include <raylib.h>
// #include <rlgl.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <array>
#include <math.h>
#include "iostream"

extern "C" {
#include <memmanager.h>
}

extern int WinX, WinY;
extern float WinXf, WinYf;
extern int MAX_REFLECTIONS;

extern float hview;

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

extern size_t id_counter;
class Object {
public:
    bool active;
    bool collidable;
    bool visible;
    bool opaque;
    ObjType type;
    size_t id = id_counter++;
    bool reflects;
    Vector2 normal;

    virtual ~Object() {};
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool intersects(const Point&) = 0;
    virtual void raycallback(Object* obj) = 0;
};
extern std::list<Object*> Gobjects;


class Obtacle: public Object {
public:

};

class Mirror: public Obtacle {
public:
    Poly body;

    Mirror(Poly b, Vector2 n) {
        active = true;
        collidable = true;
        visible = true;
        opaque = false;
        type = MIRROR;
        reflects = true;

        body = b;
        float nl = sqrtf(n.x*n.x + n.y*n.y);
        normal = {n.x/nl, n.y/nl};
    }
    ~Mirror() = default;
    
    void draw() override {
        // DrawTriangleFan((Vector2*)&body, 4, {213,245,242,180});
        DrawTriangle(body.p1, body.p2, body.p3, {213,245,242,80} );
        DrawTriangle(body.p3, body.p2, body.p4, {213,245,242,80} );
    }
    void update() override {}
    bool intersects(const Point& p) override {
        // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
        return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
    }
    void raycallback(Object* obj) override {
        DrawTriangle(body.p1, body.p2, body.p3, {213,245,242,180} );
        DrawTriangle(body.p3, body.p2, body.p4, {213,245,242,180} );
    }
};

class Wall: public Obtacle {
public:
    Poly body;

    Wall(Poly b) {
        active = true;
        collidable = true;
        visible = true;
        opaque = false;
        type = WALL;
        reflects = false;

        body = b;
    }
    ~Wall() = default;
    
    void draw() override {
        DrawTriangle(body.p1, body.p2, body.p3, {181,67,22,30} );
        DrawTriangle(body.p3, body.p2, body.p4, {181,67,22,30} );
        // DrawTriangleFan((Vector2*)&body, 4, {181,67,22,250});
    }
    void update() override {}
    bool intersects(const Point& p) override {
        // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
        return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
    }
    void raycallback(Object* obj) override {
        DrawTriangle(body.p1, body.p2, body.p3, {181,67,22,230} );
        DrawTriangle(body.p3, body.p2, body.p4, {181,67,22,230} );
    }
};


typedef struct IntersectInfo {
    std::vector<Point> points;
    float distance;
    ObjType type;
    Object* ptr;
} IntersectInfo;

Object* intersect(const Point& p, Object* ignore);
IntersectInfo raycast(Point start, float angle, float step, Object* ignore);
// raycast with direction
IntersectInfo raycast(Point start, Vector2 direct, Object* ignore);

// @return normalized reflected vector
// @param v normalized vector
// @param normal vector of normal
Vector2 reflect(Vector2 v, Vector2 normal);


#endif