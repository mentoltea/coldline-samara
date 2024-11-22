#ifndef GAME_H
#define GAME_H

#include "definitions.h"

extern bool STOP;
extern bool RELOAD;
extern bool SAFE_DRAWING;


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

std::tuple<Item*, int, float> nearestItem(const Point& p);

bool lineCircleIntersection(Point start, Point end, Point circle, float radius, Point& intersection);

Point projectToCamera(const Point& p);
Point projectToMap(const Point& p);

void DrawTexturePoly(Texture2D texture, Vector2 center, Vector2 *points, Vector2 *texcoords, int pointCount, Color tint);

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
    bool SafeToDraw = true;

    Object() {}
    virtual ~Object() {};
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool intersects(const Point&) = 0;
    virtual bool intersectsCircle(const Point& circle, float radius, Point& intersection) = 0;
    virtual void raycallback(Object* obj, float dist) = 0;
    virtual void collidecallback(Entity* obj, const Point& point, const Vector2& direction) = 0;
    virtual void projectilecallback(Projectile* proj) = 0;
};


class Item : public Object {
public:
    ObjType subgentype;
    Point position;
    bool onFloor;
    float usageDistance = 0;
    // bool isHolded;
    
    Item();

    virtual bool usable() = 0;
    virtual void use(Entity* user) = 0;

    bool intersectsCircle(const Point& circle, float radius, Point& intersection) {return false;};
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) {};
    void projectilecallback(Projectile* proj) {};
    void raycallback(Object* obj, float dist) override;
};

class Firearm: public Item {
public:
    int rounds = 7;
    int maxrounds = 7;
    int extrarounds = 35;
    float bulletsize = 3;
    float maxdeclining = 0; // in degrees

    int delay_tick;
    float delay = 0.5;
    bool reloading = false;
    int reloading_tick;

    Texture* selftexture = NULL;

    Poly body; // relative to position

    Color selfcolor = {80, 140, 190, 250};
    
    Firearm() {};
    Firearm(Point position, Poly body, bool onFloor);
    ~Firearm() override;
    void drawA(unsigned char a);
    void draw() override;
    bool usable() override;
    void use(Entity* user) override; 
    void update() override;
    bool intersects(const Point&) override;
};

class Pistol: public Firearm {
public:
    Pistol() {};
    Pistol(Point position, Poly body, bool onFloor);
    ~Pistol() override;
};

class Rifle: public Firearm {
public:
    Rifle() {};
    Rifle(Point position, Poly body, bool onFloor);
    ~Rifle() override;
};

class Shotgun: public Firearm {
public:
    int bulletcount = 7;
    float anglebetween = 7; // in degrees

    Shotgun() {};
    Shotgun(Point position, Poly body, bool onFloor);
    ~Shotgun() override;

    void use(Entity* user) override; 
};


class Entity : public Object {
public:
    Vector2 direction = {0,0};
    Vector2 move = {0,0};
    float speed = 1;
    Point position;
    Point drawPosition;
    float hitCircleSize;
    bool alive = true;
    int hp = 3;
    int selfitem = -1; // index in level objects

    Entity();
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
    virtual void pickItem();
    virtual void dropItem();
};

class Projectile: public Entity {
public:
    bool finished = false;
    Projectile();
    
    virtual void onDestroy() = 0;
};

class Punch: public Projectile {
public:
    Entity* ignore = NULL;
    int tick = 0;
    int tickdurancy = 0;

    Punch(Point position, float radius, int tickdur, Entity* ignore);
    ~Punch() override;
    void draw() override;
    void update() override;
    bool intersects(const Point&) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
    void raycallback(Object* obj, float dist) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
    void projectilecallback(Projectile* proj) override;
    void onDestroy() override;
};

class Bullet: public Projectile {
public:
    Color selfcolor = {230, 180, 50, 250};

    Bullet(Point position, Vector2 direction, float radius);
    ~Bullet() override;
    void drawA(unsigned char alfa);
    void draw() override;
    void update() override;
    bool intersects(const Point&) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
    void raycallback(Object* obj, float dist) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
    void projectilecallback(Projectile* proj) override;
    void onDestroy() override;
};



class Obtacle: public Object {
public:
    Obtacle();
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
    void projectilecallback(Projectile* proj) override;
};

class Mirror: public Obtacle {
public:
    Poly body;
    Poly drawBody;

    Mirror() {};
    Mirror(Poly b, Vector2 n);
    Mirror(Point start, Point end, float halfwidth);
    ~Mirror() override;
    
    void drawA(unsigned char alfa) ;
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
    void projectilecallback(Projectile* proj) override;
};

class Wall: public Obtacle {
public:
    Poly body;
    Poly drawBody;

    Wall() {}
    Wall(Poly b);
    Wall(Point start, Point end, float halfwidth);
    ~Wall() override;
    
    void drawA(unsigned char alfa);
    void draw() override;
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    bool intersectsCircle(const Point& circle, float radius, Point& intersection) override;
    void projectilecallback(Projectile* proj) override;
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
    Door(float minangle, float maxangle, float angle, Point origin, Point start, Point end, float halfwidth);
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
    float stepsize = 2;
    float hview = 30;
    static const int Nray = 100;
    static const int Nrayback = 60;
    float delta = 2*hview/Nray;
    float deltaback = (360-2*hview+5)/Nrayback;
    Texture *selfTexture = NULL;
    bool use_item=false;

    bool punching = false;
    bool punched = false;
    int punchtick = 0;
    float punchdurance = 0.2;
    float punchcooldown = 0.2;
    float punchsizeProcent = 0.75;

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
    void projectilecallback(Projectile* proj) override;
};

class EnemyBehaviour {
public:
    std::vector<int> selfway; // constant
    std::stack<int> currentway;
    int selfwayidx = 0;
    std::tuple<Point, int, float> near;

    bool see_player_way_updated = false;
    bool see_item_way_updated = false;
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
    static const int Nray = 50;
    float stepsize = 5;
    float delta = 2*hview/Nray;
    bool intersUpdated = false;
    size_t maxlen;

    bool firstUpdate = false;

    Point target;
    bool chase_target=false;
    bool turn_target=false;
    bool see_player;
    bool use_item;
    EnemyBehaviour behaviour;

    bool punching = false;
    bool punched = false;
    int punchtick = 0;
    float punchdurance = 0.3;
    float punchcooldown = 0.1;
    float punchsizeProcent = 0.65;

    bool shocked = false;
    int shocktick = 0;
    float shockdurancy = 0.8;


    Enemy() {}
    Enemy(Point pos, Vector2 size, std::vector<int> way);
    ~Enemy() override;

    void drawA(unsigned char alfa);
    void drawView(unsigned char alfa);
    void draw() override;
    void makemove();
    void update() override;
    bool intersects(const Point& p) override;
    void raycallback(Object* obj, float dist) override;
    void collidecallback(Entity* obj, const Point& point, const Vector2& direction) override;
    void projectilecallback(Projectile* proj) override;
};



namespace ObjectExamples {
// THESE EXAMPLES ARE ONLY TO GET VPTR TO VTABLE
extern Wall ExampleWall;
extern Door ExampleDoor;
extern Mirror ExampleMirror;
extern TextSegment ExampleTextSegment;
extern Player ExamplePlayer;
extern Enemy ExampleEnemy;
extern Pistol ExamplePistol;
extern Rifle ExampleRifle;
extern Shotgun ExampleShotgun;
}
#define MacroExample(T) ObjectExamples::Example##T 

#endif