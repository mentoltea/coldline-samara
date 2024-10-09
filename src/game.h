#ifndef GAME_H
#define GAME_H

#include "definitions.h"


Object* intersect(const Point& p, Object* ignore);
void raycast(IntersectInfo& result,Point start, float angle, float step, Object* ignore, Object* origin);
// raycast with direction
void raycast(IntersectInfo& result,Point start, Vector2 direct, Object* ignore, Object* origin);
void raycastLimited(IntersectInfo& result,Point start, float angle, float step, Object* ignore, Object* origin, float limit);
void raycastLimitedReflections(IntersectInfo& result,Point start, float angle, float step, Object* ignore, Object* origin, float limit);

// @return normalized reflected vector
// @param v normalized vector
// @param normal vector of normal
Vector2 reflect(const Vector2& v, const Vector2& normal);

Object* collide(Entity *obj, const Point& p, const Vector2& direction);
Object* collideCircle(Entity *obj, const Point& circle, float radius, const Vector2& direction);

bool lineCircleIntersection(Point start, Point end, Point circle, float radius, Point& intersection);

Point projectToCamera(const Point& p);
Point projectToMap(const Point& p);

void DrawTexturePoly(Texture2D texture, Vector2 center, Vector2 *points, Vector2 *texcoords, int pointCount, Color tint);

extern int TICK;
extern int tickcount;
void draw();
void update();


class Object {
public:
    size_t emptyVar;
    size_t id = gamestate.id_counter++;
    bool active;
    bool collidable;
    bool visible;
    bool opaque;
    ObjType gentype;
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
    Vector2 direction = {0,0};
    Vector2 move = {0,0};
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

    Mirror() {};
    Mirror(Poly b, Vector2 n);
    ~Mirror() override;
    
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

    Wall() {}
    Wall(Poly b);
    ~Wall() override;
    
    void drawA(unsigned char alfa);
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
};

class TextSegment: public Wall {
public:
    int fontsize;
    Vector2 offset = {0,0};
    Color textColor = {50, 100, 50, 250};
    char text[64];

    TextSegment() {}
    TextSegment(Poly b, std::string text, int fontsize);
    ~TextSegment() override;

    void drawA(unsigned char alfa);
    void draw() override;
};

class Door: public Obtacle {
public:
    Poly body;
    Poly drawBody;
    Point origin;
    Point position;
    float anglevel;
    float angle, anglerad;
    float dirSizeAngle;
    float hitSizeAngle;
    float drawsize;
    float maxangle;
    float minangle;
    float hitboxsize;
    float posoriginsize;
    float koef = 0.95;

    Door() {}
    Door(float minangle, float maxangle, float angle, Point origin, Vector2 hitboxsize, Vector2 drawsize);
    ~Door() override;
    
    void drawA(unsigned char alfa) ;
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
};


class Player : public Entity {
public:
    float size;
    Point p1, p2, p3, p4;
    Point dp1, dp2, dp3, dp4;
    Vector2 staticDrawingBox = {100, 100};
    float angle, angleRad;
    float dirSizeAngle;
    float viewAround = 50;
    float viewLength = 800;
    std::vector<IntersectInfo> inters;
    std::vector<IntersectInfo> intersBack;
    Color viewColor = {110,110,110,180};
    Color viewAroundColor = {110,110,110,100};
    float hview = 30;
    static const int Nray = 150;
    static const int Nrayback = 80;
    float delta = 2*hview/Nray;
    float deltaback = (360-2*hview+5)/Nrayback;
    Texture *selfTexture = NULL;

    Player() {}
    // Player(const Player& other) = default;
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

class EnemyBehaviour {
public:
    std::vector<int> selfway; // constant
    std::stack<int> currentway;
    int selfwayidx = 0;
    std::tuple<Point, int, float> near;

    bool see_player_way_updated = false;
    bool warned = false;
    int tick_warned;
    bool lost;
    int tick_lost;

    EnemyBehaviour() {}    
    EnemyBehaviour(std::vector<int> way): selfway(way) {};

    void update(Enemy* self);
};

class Enemy : public Entity {
public:
    float size;
    Point p1, p2, p3, p4;
    Point dp1, dp2, dp3, dp4;
    float angle, angleRad;
    float dirSizeAngle;

    float viewAround = 30;
    float viewLength = 650;
    std::vector<IntersectInfo> inters; // constant
    Color viewColor = {110,50,50,180};
    Color selfColor = {150, 150, 50, 250};
    float hview = 30;
    static const int Nray = 60;
    float delta = 2*hview/Nray;
    size_t maxlen;

    Point target;
    bool chase_target=true;
    bool see_player;
    EnemyBehaviour behaviour;

    Enemy() {}
    Enemy(Point pos, Vector2 size, std::vector<int> way);
    ~Enemy() override;

    void drawA(unsigned char alfa);
    void drawView(unsigned char alfa);
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
};



namespace ObjectExamples {
// THESE EXAMPLES ARE ONLY TO GET VPTR TO VTABLE
extern Wall ExampleWall;
extern Door ExampleDoor;
extern Mirror ExampleMirror;
extern Enemy ExampleEnemy;
extern Player ExamplePlayer;
extern TextSegment ExampleTextSegment;
}
#define MacroExample(T) ObjectExamples::Example##T 

#endif