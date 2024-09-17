#ifndef GAME_H
#define GAME_H

#include "definitions.h"

namespace MemManager {
extern "C" {
#include <memmanager.h>
}
}
#define NEW(T) new(MemManager::memloc(sizeof(T)))

Object* intersect(const Point& p, Object* ignore);
void raycast(IntersectInfo& result,Point start, float angle, float step, Object* ignore);
// raycast with direction
void raycast(IntersectInfo& result,Point start, Vector2 direct, Object* ignore);
void raycastLimited(IntersectInfo& result,Point start, float angle, float step, Object* ignore, float limit);

// @return normalized reflected vector
// @param v normalized vector
// @param normal vector of normal
Vector2 reflect(const Vector2& v, const Vector2& normal);

Object* collide(Entity *obj, const Point& p, const Vector2& direction);
Object* collideCircle(Entity *obj, const Point& circle, float radius, const Vector2& direction);

bool lineCircleIntersection(Point start, Point end, Point circle, float radius, Point& intersection);

Point projectToCamera(const Point& p);
Point projectToMap(const Point& p);

void draw();
void update();


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
    int raycount;

    Object() {}
    virtual ~Object() {};
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool intersects(const Point&) = 0;
    virtual bool intersectsCircle(const Point& circle, float radius, Point& intersection) = 0;
    virtual void raycallback(Object* obj, float dist) = 0;
    virtual void collidecallback(Entity* obj, const Point& point, const Vector2& direction) = 0;
};

class Entity : public Object {
public:
    Vector2 direction;
    Vector2 move;
    Point position;
    Point drawPosition;
    float hitCircleSize;

    Entity();
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
};

class Obtacle: public Object {
public:
    Obtacle();
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
};

class Mirror: public Obtacle {
public:
    Poly body;
    Poly drawBody;

    Mirror(Poly b, Vector2 n);
    ~Mirror() = default;
    
    void drawA(unsigned char alfa) ;
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
};

class Wall: public Obtacle {
public:
    Poly body;
    Poly drawBody;

    Wall(Poly b);
    ~Wall() override;
    
    void drawA(unsigned char alfa);
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
};



class Player : public Entity {
public:
    float size;
    Point p1, p2, p3, p4;
    Point dp1, dp2, dp3, dp4;
    float angle, angleRad;
    float dirSizeAngle;
    float viewAround = 50;
    std::vector<IntersectInfo> inters;
    std::vector<IntersectInfo> intersBack;
    Color viewColor = {110,110,110,180};
    Color viewAroundColor = {110,110,110,100};
    float hview = 30;
    static const int Nray = 150;
    static const int Nrayback = 80;
    float delta = 2*hview/Nray;
    float deltaback = (360-2*hview+2)/Nrayback;
    Player(Point pos, Vector2 size);
    ~Player() override;

    void drawViewAround();
    void drawView();
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
};


class Enemy : public Entity {
public:
    float size;
    Point p1, p2, p3, p4;
    Point dp1, dp2, dp3, dp4;
    float angle, angleRad;
    float dirSizeAngle;
    float viewAround = 30;
    std::vector<IntersectInfo> inters;
    Color viewColor = {110,50,50,180};
    Color selfColor = {150, 150, 50, 250};
    float hview = 30;
    static const int Nray = 60;
    float delta = 2*hview/Nray;
    size_t maxlen;
    Enemy(Point pos, Vector2 size);
    ~Enemy() override;

    void drawA(unsigned char alfa);
    void drawView(unsigned char alfa);
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
};



#endif