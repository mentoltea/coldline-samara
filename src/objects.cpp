#include "game.h"

Entity::Entity() {
    active = true;
    collidable = true;
    visible = true;
    opaque = false;
    reflects = false;
}



Obtacle::Obtacle() {
    active = true;
    collidable = true;
    visible = true;
}
void Obtacle::collidecallback(Entity* obj, const Point& point, const Vector2& direction) {}




Mirror::Mirror(Poly b, Vector2 n) {
    opaque = false;
    type = MIRROR;
    reflects = true;

    float nl = sqrtf(n.x*n.x + n.y*n.y);
    normal = {n.x/nl, n.y/nl};
    body = b;
}

void Mirror::drawA(unsigned char alfa) {
    DrawTriangle(body.p1, body.p2, body.p3, {213,245,242,alfa} );
    DrawTriangle(body.p3, body.p2, body.p4, {213,245,242,alfa} );
}
void Mirror::draw() {
    int base = 20;
    int a = base + (float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (255-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
}
void Mirror::update() {}
bool Mirror::intersects(const Point& p) {
    return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
}
void Mirror::raycallback(Object* obj, float dist) {
    if (obj->type == PLAYER) raycount++;
}



Wall::Wall(Poly b) {
    opaque = false;
    reflects = false;
    type = WALL;

    body = b;
}
Wall::~Wall() = default;

void Wall::drawA(unsigned char alfa) {
    DrawTriangle(body.p1, body.p2, body.p3, {181,67,22,alfa} );
    DrawTriangle(body.p3, body.p2, body.p4, {181,67,22,alfa} );
}
void Wall::draw()  {
    int base = 10;
    int a = base + (float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (254-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
}
void Wall::update()  {}
bool Wall::intersects(const Point& p)  {
    return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
}
void Wall::raycallback(Object* obj, float dist)  {
    if (obj->type == PLAYER) raycount++;
}



Player::Player(Point pos, Vector2 size): inters(Nray), intersBack(Nrayback) {
    position = pos;
    float sl = sqrtf(size.x*size.x + size.y*size.y);
    dirSizeAngle = acosf(size.x/sl);
    this->size = sl;
    type = PLAYER;
}
Player::~Player() = default;

void Player::drawViewAround() {
    float a = angle - hview + 2;
    for (int i=0; i<Nrayback; i++) {
        a -= deltaback;
        inters[i] = raycastLimited(position, a, 1, this, viewAround);
        if (i>0) {
            // DrawCircleV(inters[i].points[0], 1, {255,255,0,250});
            DrawTriangle( position,inters[i-1].points[0], inters[i].points[0],  viewAroundColor);
        }
    }
}
void Player::drawView() {
    float a = angle + hview;
    size_t maxlen = 0;
    for (int i=0; i<Nray; i++) {
        a -= delta;
        inters[i] = raycast(position, a, 2, this);
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
                        Color col = {viewColor.r,viewColor.g,viewColor.b, (unsigned char)(a>0? a: 0)};//{120,120,200,180};
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
void Player::draw()  {
    drawViewAround();
    drawView();
    DrawTriangle(p2, p1,  p3, {100, 100, 200, 250});
    DrawTriangle(p3, p4, p2,  {100, 100, 200, 250});
}
void Player::update()  {
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
bool Player::intersects(const Point& p)  {
    // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
    return CheckCollisionPointTriangle(p, p1, p2, p3) || CheckCollisionPointTriangle(p, p3, p2, p4);
}
void Player::raycallback(Object* obj, float dist)  {
}
void Player::collidecallback(Entity* obj, const Point& point, const Vector2& direction)  {
}




Enemy::Enemy(Point pos, Vector2 size): inters(Nray) {
    position = pos;
    float sl = sqrtf(size.x*size.x + size.y*size.y);
    dirSizeAngle = acosf(size.x/sl);
    this->size = sl;
    type = ENEMY;
}
Enemy::~Enemy() = default;
void Enemy::drawView(unsigned char alfa) {
    float a = angle + hview;
    maxlen = 0;
    for (int i=0; i<Nray; i++) {
        a -= delta;
        if (i>0) {
            DrawTriangle(position, inters[i-1].points[0], inters[i].points[0], 
            {viewColor.r, viewColor.g, viewColor.g, alfa});
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
                        int a = alfa-idx*80;
                        Color col = {viewColor.r,viewColor.g,viewColor.b, (unsigned char)(a>0? a: 0)};//{120,120,200,180};
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

void Enemy::drawA(unsigned char alfa)  {
    // drawView(alfa);
    selfColor.a = alfa;
    DrawTriangle(p2, p1,  p3, selfColor);
    DrawTriangle(p3, p4, p2,  selfColor);
    DrawLine(position.x, position.y, position.x+size*direction.x, position.y+size*direction.y, selfColor);
}
void Enemy::draw()  {
    int base = 5;
    int a = base + 8*(float)raycount/(float)(Player::Nray) * (255-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
}
void Enemy::update() {
    float a = angle + hview;
    maxlen = 0;
    int found_player = -1;
    for (int i=0; i<Nray; i++) {
        a -= delta;
        inters[i] = raycast(position, a, 2, this);
        maxlen = inters[i].points.size() > maxlen ? inters[i].points.size() : maxlen;
        if (inters[i].ptr && inters[i].ptr->type==PLAYER) {
            found_player = i;
        }
    }
    if (found_player != -1) {
        selfColor = {200, 50, 50, 250};
        move = {Gplayer->position.x - position.x, Gplayer->position.y - position.y};
        direction = move;
    } else {
        selfColor = {150, 150, 50, 250};
        move = {0,0};
    }
    float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
    if (dl>0) {
        direction.x /= dl; direction.y /= dl;
        move.x /= 2*dl; move.y /= 2*dl;
    }

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
bool Enemy::intersects(const Point& p)  {
    return CheckCollisionPointTriangle(p, p1, p2, p3) || CheckCollisionPointTriangle(p, p3, p2, p4);
}
void Enemy::raycallback(Object* obj, float dist)  {
    if (obj->type == PLAYER) raycount++;
}
void Enemy::collidecallback(Entity* obj, const Point& point, const Vector2& direction)  {}
