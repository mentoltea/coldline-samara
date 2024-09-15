#ifndef GAME_H
#define GAME_H

#include "definitions.h"

extern "C" {
#include <memmanager.h>
}

Object* intersect(const Point& p, Object* ignore);
IntersectInfo raycast(Point start, float angle, float step, Object* ignore);
// raycast with direction
IntersectInfo raycast(Point start, Vector2 direct, Object* ignore);
IntersectInfo raycastLimited(Point start, float angle, float step, Object* ignore, float limit);

// @return normalized reflected vector
// @param v normalized vector
// @param normal vector of normal
Vector2 reflect(Vector2 v, Vector2 normal);

Object* collide(Entity *obj, Point p, Vector2 direction);

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

    Object() {}
    virtual ~Object() {};
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool intersects(const Point&) = 0;
    virtual void raycallback(Object* obj, float dist) = 0;
    virtual void collidecallback(Entity* obj, Point point, Vector2 direction) = 0;
};

class Entity : public Object {
public:
    Vector2 direction;
    Vector2 move;
    Point position;

    Entity() {
        active = true;
        collidable = true;
        visible = true;
        opaque = false;
    }
    // ~Entity() = default;
};

class Obtacle: public Object {
public:
    Obtacle() {
        active = true;
        collidable = true;
        visible = true;
    }

    void collidecallback(Entity* obj, Point point, Vector2 direction) override {
        // obj->move.x -= 2*direction.x;
        // obj->move.y -= 2*direction.y;
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
};



class Player : public Entity {
public:
    float size;
    Point p1, p2, p3, p4;
    float angle, angleRad;
    float dirSizeAngle;
    float viewAround = 50;
    std::vector<IntersectInfo> inters;
    Color viewColor = {110,110,110,180};
    Color viewAroundColor = {110,110,110,100};

    Player(Point pos, Vector2 size): inters(Nray) {
        position = pos;
        float sl = sqrtf(size.x*size.x + size.y*size.y);
        dirSizeAngle = acosf(size.x/sl);
        this->size = sl;
    }
    ~Player() = default;

    void drawViewAround() {
        float a = angle - hview + 2;
        float full = (360-2*hview+2);
        for (int i=0; i<Nray; i++) {
            a -= full/Nray;
            inters[i] = raycastLimited(position, a, 1, this, viewAround);
            if (i>0) {
                // DrawCircleV(inters[i].points[0], 1, {255,255,0,250});
                DrawTriangle( position,inters[i-1].points[0], inters[i].points[0],  viewAroundColor);
            }
        }
    }
    void drawView() {
        float a = angle + hview;
        size_t maxlen = 0;
        for (int i=0; i<Nray; i++) {
            a -= delta;
            inters[i] = raycast(position, a, 1, this);
            maxlen = inters[i].points.size() > maxlen ? inters[i].points.size() : maxlen;
            if (i>0) {
                DrawTriangle(position, inters[i-1].points[0], inters[i].points[0], viewColor);
            }
        }
        {
        Point curr = position;
        Point next;
        for (int i=0; i<(int)inters[Nray/2].points.size(); i++) {
            next = inters[Nray/2].points[i];
            DrawLineV(curr, next, {210, 0, 0, 210});
            curr = next;
        }
        }
        size_t idx = 1;
        int from = -1;
        int to = -1;
        int loopstep = 2;
        while (idx < maxlen) {
            for (int i=0; i<Nray; i+=loopstep) {
                if (inters[i].points.size() <= idx || i>=Nray-loopstep) {
                    if (to != -1) {
                        for (int j=from+1; j<=to; j++) {
                            if (i==j) continue;
                            int a = 180-idx*80;
                            Color col = {180,180,180, (unsigned char)(a>0? a: 0)};//{120,120,200,180};
                            if (idx%2==0) {
                                DrawTriangle(inters[j-1].points[idx-1], inters[j-1].points[idx],inters[j].points[idx-1],   col);
                                DrawTriangle(inters[j].points[idx-1], inters[j-1].points[idx],inters[j].points[idx],   col);
                            }
                            else {
                                DrawTriangle(inters[j-1].points[idx-1], inters[j].points[idx-1], inters[j-1].points[idx],  col);
                                DrawTriangle(inters[j].points[idx-1], inters[j].points[idx], inters[j-1].points[idx],  col);
                            }
                        }
                        from = -1;
                        to = -1;
                    }
                } else {
                    if (from == -1) {
                        from = i;
                        // DrawLineV(inters[from].points[idx-1],inters[from].points[idx], {140,140,250,220});
                    }
                    to = i;
                }
            }
            idx++;
            // break;
        }
    }
    void draw() override {
        drawViewAround();
        drawView();
        DrawTriangle(p2, p1,  p3, {100, 100, 200, 250});
        DrawTriangle(p3, p4, p2,  {100, 100, 200, 250});
        // DrawCircleV(p1, 1, {100, 100, 200, 250});
        // DrawCircleV(p2, 1, {100, 100, 200, 250});
        // DrawCircleV(p3, 1, {100, 100, 200, 250});
        // DrawCircleV(p4, 1, {100, 100, 200, 250});
    }
    void update() override {
        Vector2 mouse = GetMousePosition();
        direction = {mouse.x - position.x, mouse.y - position.y};
        float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
        if (dl>0) {direction.x /= dl; direction.y /= dl;}

        angleRad = atan2f(direction.y, direction.x);
        angle = angleRad*180/PI;
        
        
        position.x += move.x;
        p1 = {position.x + size * cosf(angleRad - dirSizeAngle), position.y + size * sinf(angleRad - dirSizeAngle)};
        p2 = {position.x + size * cosf(angleRad + dirSizeAngle), position.y + size * sinf(angleRad + dirSizeAngle)};
        p3 = {position.x + size * cosf(PI + angleRad + dirSizeAngle), position.y + size * sinf(PI + angleRad + dirSizeAngle)};
        p4 = {position.x + size * cosf(PI + angleRad - dirSizeAngle), position.y + size * sinf(PI + angleRad - dirSizeAngle)};

        Object* collision;
        if ((collision= collide(this, position, move))
        || (collision=collide(this, p1, move)) || (collision=collide(this, p2, move))
        || (collision=collide(this, p3, move)) || (collision=collide(this, p4, move))) {
            move.x *= 1.1;
            
            position.x -= move.x;
            p1.x -= move.x;
            p2.x -= move.x;
            p3.x -= move.x;
            p4.x -= move.x;
        }
        
        position.y += move.y;
        p1.y += move.y;
        p2.y += move.y;
        p3.y += move.y;
        p4.y += move.y;

        if ((collision= collide(this, position, move))
        || (collision=collide(this, p1, move)) || (collision=collide(this, p2, move))
        || (collision=collide(this, p3, move)) || (collision=collide(this, p4, move))) {
            // move.x *= 1.5;
            move.y *= 1.1;
            
            position.y -= move.y;
            p1.y -= move.y;
            p2.y -= move.y;
            p3.y -= move.y;
            p4.y -= move.y;
        }
    }
    bool intersects(const Point& p) override {
        // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
        return CheckCollisionPointTriangle(p, p1, p2, p3) || CheckCollisionPointTriangle(p, p3, p2, p4);
    }
    void raycallback(Object* obj, float dist) override {
    }
    void collidecallback(Entity* obj, Point point, Vector2 direction) override {
    }
};




#endif