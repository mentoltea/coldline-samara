#ifndef GAME_H
#define GAME_H

#include "definitions.h"

extern "C" {
#include <memmanager.h>
}


class Object {
public:
    size_t id = id_counter++;
    bool active;
    bool collidable;
    bool visible;
    bool opaque;
    ObjType type;
    bool reflects;
    Vector2 normal;

    Object() : active(true) {}
    virtual ~Object() {};
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool intersects(const Point&) = 0;
    virtual void raycallback(Object* obj, float dist) = 0;
    virtual void collidecallback(Object* obj, Point point, Vector2 direction) = 0;
};



class Obtacle: public Object {
public:
    Obtacle() {
        active = true;
        collidable = true;
        visible = true;
    }
};

class Mirror: public Obtacle {
public:
    Poly body;

    Mirror(Poly b, Vector2 n) {
        opaque = false;
        type = MIRROR;
        reflects = true;

        float nl = sqrtf(n.x*n.x + n.y*n.y);
        normal = {n.x/nl, n.y/nl};
        body = b;
    }
    ~Mirror() = default;
    
    void drawA(unsigned char alfa) {
        DrawTriangle(body.p1, body.p2, body.p3, {213,245,242,alfa} );
        DrawTriangle(body.p3, body.p2, body.p4, {213,245,242,alfa} );
    }
    void draw() override {
        drawA(80);
        
    }
    void update() override {}
    bool intersects(const Point& p) override {
        // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
        return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
    }
    void raycallback(Object* obj, float dist) override {
        drawA(13);
    }
    void collidecallback(Object* obj, Point point, Vector2 direction) override {}
};

class Wall: public Obtacle {
public:
    Poly body;

    Wall(Poly b) {
        opaque = false;
        reflects = false;
        type = WALL;

        body = b;
    }
    ~Wall() = default;
    
    void drawA(unsigned char alfa) {
        DrawTriangle(body.p1, body.p2, body.p3, {181,67,22,alfa} );
        DrawTriangle(body.p3, body.p2, body.p4, {181,67,22,alfa} );
    }
    void draw() override {
        drawA(10);
    }
    void update() override {}
    bool intersects(const Point& p) override {
        // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
        return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
    }
    void raycallback(Object* obj, float dist) override {
        drawA(3);
    }
    void collidecallback(Object* obj, Point point, Vector2 direction) override {}
};




Object* intersect(const Point& p, Object* ignore);
IntersectInfo raycast(Point start, float angle, float step, Object* ignore);
// raycast with direction
IntersectInfo raycast(Point start, Vector2 direct, Object* ignore);

// @return normalized reflected vector
// @param v normalized vector
// @param normal vector of normal
Vector2 reflect(Vector2 v, Vector2 normal);


#endif