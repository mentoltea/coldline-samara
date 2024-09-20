#include "game.h"

Entity::Entity() {
    active = true;
    collidable = true;
    visible = true;
    opaque = false;
    reflects = false;
}

bool Entity::intersectsCircle(const Point& circle, float radius, Point& intersection) {
    float dx = position.x - circle.x;
    float dy = position.y - circle.y;
    float sumrad = radius + hitCircleSize;
    float diffrad = radius - hitCircleSize;
    diffrad = diffrad<0? -diffrad : diffrad;

    if (dx*dx + dy*dy > sumrad*sumrad) return false;
    if (dx*dx + dy*dy < diffrad) return false;
    
    return true;
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
    DrawTriangle(drawBody.p1, drawBody.p2, drawBody.p3, {213,245,242,alfa} );
    DrawTriangle(drawBody.p3, drawBody.p2, drawBody.p4, {213,245,242,alfa} );
}
void Mirror::draw() {
    int max = 255;
    int base = 20;
    float rate = 2;
    int a = base + rate*(float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (max-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
}
void Mirror::update() {
    drawBody.p1 = projectToCamera(body.p1);
    drawBody.p2 = projectToCamera(body.p2);
    drawBody.p3 = projectToCamera(body.p3);
    drawBody.p4 = projectToCamera(body.p4);
}
bool Mirror::intersects(const Point& p) {
    return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
}
void Mirror::raycallback(Object* obj, float dist) {
    if (obj->type == PLAYER) raycount++;
}
bool Mirror::intersectsCircle(const Point& circle, float radius, Point& intersection) {
    return lineCircleIntersection(body.p1, body.p2, circle, radius, intersection)
        || lineCircleIntersection(body.p2, body.p4, circle, radius, intersection)
        || lineCircleIntersection(body.p4, body.p3, circle, radius, intersection)
        || lineCircleIntersection(body.p3, body.p1, circle, radius, intersection);
}



Wall::Wall(Poly b) {
    opaque = false;
    reflects = false;
    type = WALL;

    body = b;
}
Wall::~Wall() = default;

void Wall::drawA(unsigned char alfa) {
    DrawTriangle(drawBody.p1, drawBody.p2, drawBody.p3, {181,67,22,alfa} );
    DrawTriangle(drawBody.p3, drawBody.p2, drawBody.p4, {181,67,22,alfa} );
}
void Wall::draw()  {
    int max = 255;
    int base = 10;
    float rate = 2;
    int a = base + rate*(float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (max-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
}
void Wall::update()  {
    drawBody.p1 = projectToCamera(body.p1);
    drawBody.p2 = projectToCamera(body.p2);
    drawBody.p3 = projectToCamera(body.p3);
    drawBody.p4 = projectToCamera(body.p4);
}
bool Wall::intersects(const Point& p)  {
    return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
}
void Wall::raycallback(Object* obj, float dist)  {
    if (obj->type == PLAYER) raycount++;
}
bool Wall::intersectsCircle(const Point& circle, float radius, Point& intersection) {
    return lineCircleIntersection(body.p1, body.p2, circle, radius, intersection)
        || lineCircleIntersection(body.p2, body.p4, circle, radius, intersection)
        || lineCircleIntersection(body.p4, body.p3, circle, radius, intersection)
        || lineCircleIntersection(body.p3, body.p1, circle, radius, intersection);
}



Door::Door(float minangle, float maxangle, float angle, Point origin, Vector2 hitboxsize, Vector2 drawsize) {
    opaque = false;
    reflects = false;
    type = DOOR;

    this->maxangle = maxangle;
    this->minangle = minangle;
    this->angle = angle;
    this->origin = origin;
    float hl = sqrtf(hitboxsize.x*hitboxsize.x + hitboxsize.y*hitboxsize.y);
    this->hitboxsize = hl;
    this->hitSizeAngle = acos(hitboxsize.x/hl);
    float sl = sqrtf(drawsize.x*drawsize.x + drawsize.y*drawsize.y);
    dirSizeAngle = acosf(drawsize.x/sl);
    this->drawsize = sl;
    this->posoriginsize = this->hitboxsize/2;
}
Door::~Door() = default;

void Door::drawA(unsigned char alfa) {
    drawBody.p1 = projectToCamera({position.x + drawsize * cosf(anglerad - dirSizeAngle), position.y + drawsize * sinf(anglerad - dirSizeAngle)});
    drawBody.p2 = projectToCamera({position.x + drawsize * cosf(anglerad + dirSizeAngle), position.y + drawsize * sinf(anglerad + dirSizeAngle)});
    drawBody.p3 = projectToCamera({position.x + drawsize * cosf(PI + anglerad + dirSizeAngle), position.y + drawsize * sinf(PI + anglerad + dirSizeAngle)});
    drawBody.p4 = projectToCamera({position.x + drawsize * cosf(PI + anglerad - dirSizeAngle), position.y + drawsize * sinf(PI + anglerad - dirSizeAngle)});

    // DrawLineV(drawBody.p1, drawBody.p2, {255,0,0,250});
    // DrawLineV(drawBody.p2, drawBody.p4, {255,0,0,250});
    // DrawLineV(drawBody.p4, drawBody.p3, {255,0,0,250});
    // DrawLineV(drawBody.p3, drawBody.p1, {255,0,0,250});

    // DrawLineV(projectToCamera( body.p1), projectToCamera(body.p2), {255,255,0,250});
    // DrawLineV(projectToCamera(body.p2), projectToCamera(body.p4), {255,255,0,250});
    // DrawLineV(projectToCamera(body.p4), projectToCamera(body.p3), {255,255,0,250});
    // DrawLineV(projectToCamera(body.p3), projectToCamera(body.p1), {255,255,0,250});
    // DrawCircleV(projectToCamera( origin), 1, {0, 250, 0, 250});
    // DrawCircleV(projectToCamera( position), 1, {250, 0, 0, 250});
    
    DrawTriangle(drawBody.p3, drawBody.p2, drawBody.p1, {240,20,50,alfa} );
    DrawTriangle(drawBody.p2, drawBody.p3, drawBody.p4, {240,20,50,alfa} );
}
void Door::draw()  {
    int max = 255;
    int base = 20;
    float rate = 4;
    int a = base + rate*(float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (max-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
}
void Door::update() {
    angle += anglevel;
    anglevel *= koef;
    while (angle >= 360) {
        angle -= 360;
    }
    while (angle <= -360) {
        angle += 360;
    }

    if (angle>maxangle || angle < minangle) {
        angle -= anglevel;
        anglevel *= -koef;
    }
    anglerad = PI*angle/180;

    normal = {-sinf(anglerad), cosf(anglerad)};

    position = {origin.x + hitboxsize*cosf(anglerad), origin.y + hitboxsize*sinf(anglerad)};
    body.p1 = {position.x + hitboxsize * cosf(anglerad - hitSizeAngle), position.y + hitboxsize * sinf(anglerad - hitSizeAngle)};
    body.p2 = {position.x + hitboxsize * cosf(anglerad + hitSizeAngle), position.y + hitboxsize * sinf(anglerad + hitSizeAngle)};
    body.p3 = {position.x + hitboxsize * cosf(PI + anglerad + hitSizeAngle), position.y + hitboxsize * sinf(PI + anglerad + hitSizeAngle)};
    body.p4 = {position.x + hitboxsize * cosf(PI + anglerad - hitSizeAngle), position.y + hitboxsize * sinf(PI + anglerad - hitSizeAngle)};
}
bool Door::intersects(const Point& p) {
    return CheckCollisionPointTriangle(p, body.p1, body.p2, body.p3) || CheckCollisionPointTriangle(p, body.p3, body.p2, body.p4);
}
void Door::raycallback(Object* obj, float dist) {
    if (obj->type == PLAYER) raycount++;
}
bool Door::intersectsCircle(const Point& circle, float radius, Point& intersection) {
    return lineCircleIntersection(body.p1, body.p2, circle, radius, intersection)
        || lineCircleIntersection(body.p2, body.p4, circle, radius, intersection)
        || lineCircleIntersection(body.p4, body.p3, circle, radius, intersection)
        || lineCircleIntersection(body.p3, body.p1, circle, radius, intersection);
}
void Door::collidecallback(Entity* obj, const Point& point, const Vector2& direction) {
    // float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
    // if (dl==0) {
    //     anglevel = -anglevel;
    //     return;
    // }
    float dot = (normal.x*direction.x + normal.y*direction.y);
    anglevel += (dot*3 - anglevel)/2;
    obj->move.x -= direction.x/3;
    obj->move.y -= direction.y/3;
    // obj->position.x -= direction.x/3 + 1;
    // obj->position.y -= direction.y/3 + 1;
}




Player::Player(Point pos, Vector2 size): inters(Nray), intersBack(Nrayback) {
    position = pos;
    drawPosition = {gamestate.WinXf/2, gamestate.WinYf/2};
    float sl = sqrtf(size.x*size.x + size.y*size.y);
    hitCircleSize = sl*0.95;
    dirSizeAngle = acosf(size.x/sl);
    this->size = sl;
    type = PLAYER;
}
Player::~Player() = default;

void Player::drawViewAround() {
    float a = angle - hview + 4;
    for (int i=0; i<Nrayback; i++) {
        a -= deltaback;
        raycastLimited(intersBack[i], position, a, 1, this, this, viewAround);
        if (i>0) {
            // DrawCircleV(inters[i].points[0], 1, {255,255,0,250});
            DrawTriangle( drawPosition , projectToCamera( intersBack[i-1].points[0] ), projectToCamera(intersBack[i].points[0]),  viewAroundColor);
        }
    }
}
void Player::drawView() {
    float a = angle + hview;
    size_t maxlen = 0;
    for (int i=0; i<Nray; i++) {
        a -= delta;
        raycastLimitedReflections(inters[i], position, a, 2, this, this, viewLength);
        maxlen = inters[i].points.size() > maxlen ? inters[i].points.size() : maxlen;
        if (i>0) {
            DrawTriangle(drawPosition, projectToCamera(inters[i-1].points[0]), projectToCamera(inters[i].points[0]), viewColor);
        }
    }
    // {
    // Point curr = position;
    // Point next;
    // for (int i=0; i<(int)inters[Nray/2].points.size(); i++) {
    //     next = inters[Nray/2].points[i];
    //     DrawLineV(curr, next, {210, 0, 0, 210});
    //     curr = next;
    // }
    // }
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
                            DrawTriangle(projectToCamera(inters[j-1].points[idx-1]), projectToCamera(inters[j-1].points[idx]), 
                                projectToCamera(inters[j].points[idx-1]),   col);
                            DrawTriangle(projectToCamera(inters[j].points[idx-1]), projectToCamera(inters[j-1].points[idx]),
                                projectToCamera(inters[j].points[idx]),   col);
                        }
                        else {
                            DrawTriangle(projectToCamera(inters[j-1].points[idx-1]), projectToCamera(inters[j].points[idx-1]),
                                projectToCamera(inters[j-1].points[idx]),  col);
                            DrawTriangle(projectToCamera(inters[j].points[idx-1]), projectToCamera(inters[j].points[idx]), 
                                projectToCamera(inters[j-1].points[idx]),  col);
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

    drawPosition = projectToCamera(position);

    dp1 = {drawPosition.x + size * cosf(angleRad - dirSizeAngle), drawPosition.y + size * sinf(angleRad - dirSizeAngle)};
    dp2 = {drawPosition.x + size * cosf(angleRad + dirSizeAngle), drawPosition.y + size * sinf(angleRad + dirSizeAngle)};
    dp3 = {drawPosition.x + size * cosf(PI + angleRad + dirSizeAngle), drawPosition.y + size * sinf(PI + angleRad + dirSizeAngle)};
    dp4 = {drawPosition.x + size * cosf(PI + angleRad - dirSizeAngle), drawPosition.y + size * sinf(PI + angleRad - dirSizeAngle)};
    
    drawViewAround();
    drawView();
    DrawTriangle(dp2, dp1,  dp3, {100, 100, 200, 250});
    DrawTriangle(dp3, dp4, dp2,  {100, 100, 200, 250});
    // DrawCircleLinesV(drawPosition, hitCircleSize, {255, 50, 50, 250});
}
void Player::update()  {
    Vector2 mouse = GetMousePosition();
    direction = {mouse.x - drawPosition.x, mouse.y - drawPosition.y};
    float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
    if (dl>0) {direction.x /= dl; direction.y /= dl;}

    angleRad = atan2f(direction.y, direction.x);
    angle = angleRad*180/PI;
    
    Object* collision;
    position.x += move.x;
    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->type==WALL || collision->type==MIRROR) move.x *= 1.1;
        position.x -= move.x;
    }
    
    position.y += move.y;
    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->type==WALL || collision->type==MIRROR) move.y *= 1.1;
        position.y -= move.y;
    }

    p1 = {position.x + size * cosf(angleRad - dirSizeAngle), position.y + size * sinf(angleRad - dirSizeAngle)};
    p2 = {position.x + size * cosf(angleRad + dirSizeAngle), position.y + size * sinf(angleRad + dirSizeAngle)};
    p3 = {position.x + size * cosf(PI + angleRad + dirSizeAngle), position.y + size * sinf(PI + angleRad + dirSizeAngle)};
    p4 = {position.x + size * cosf(PI + angleRad - dirSizeAngle), position.y + size * sinf(PI + angleRad - dirSizeAngle)};
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
    hitCircleSize = sl*0.95;
    dirSizeAngle = acosf(size.x/sl);
    this->size = sl;
    type = ENEMY;
}
Enemy::~Enemy() = default;
void Enemy::drawView(unsigned char alfa) {
    Color color = {viewColor.r, viewColor.g, viewColor.b, alfa};
    for (int i=0; i<Nray; i++) {
        if (i>0) {
            DrawTriangle(drawPosition, projectToCamera(inters[i-1].points[0]), projectToCamera(inters[i].points[0]), 
            color);
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
                        int a = alfa-idx*20;
                        Color col = {viewColor.r,viewColor.g,viewColor.b, (unsigned char)(a>0? a: 0)};
                        if (idx%2==0) {
                            DrawTriangle(projectToCamera(inters[j-1].points[idx-1]), projectToCamera(inters[j-1].points[idx]), 
                                projectToCamera(inters[j].points[idx-1]),   col);
                            DrawTriangle(projectToCamera(inters[j].points[idx-1]), projectToCamera(inters[j-1].points[idx]),
                                projectToCamera(inters[j].points[idx]),   col);
                        }
                        else {
                            DrawTriangle(projectToCamera(inters[j-1].points[idx-1]), projectToCamera(inters[j].points[idx-1]),
                                projectToCamera(inters[j-1].points[idx]),  col);
                            DrawTriangle(projectToCamera(inters[j].points[idx-1]), projectToCamera(inters[j].points[idx]), 
                                projectToCamera(inters[j-1].points[idx]),  col);
                        }
                    }
                    from = -1;
                    to = -1;
                }
            } else {
                if (from == -1) {
                    from = i;
                }
                to = i;
            }
        }
        idx++;
    }
}

void Enemy::drawA(unsigned char alfa)  {
    if (alfa==0) return;
    // drawView(alfa);
    drawPosition = projectToCamera(position);
    dp1 = {drawPosition.x + size * cosf(angleRad - dirSizeAngle), drawPosition.y + size * sinf(angleRad - dirSizeAngle)};
    dp2 = {drawPosition.x + size * cosf(angleRad + dirSizeAngle), drawPosition.y + size * sinf(angleRad + dirSizeAngle)};
    dp3 = {drawPosition.x + size * cosf(PI + angleRad + dirSizeAngle), drawPosition.y + size * sinf(PI + angleRad + dirSizeAngle)};
    dp4 = {drawPosition.x + size * cosf(PI + angleRad - dirSizeAngle), drawPosition.y + size * sinf(PI + angleRad - dirSizeAngle)};
    selfColor.a = alfa;
    drawView(alfa/3);
    DrawTriangle(dp2, dp1,  dp3, selfColor);
    DrawTriangle(dp3, dp4, dp2,  selfColor);
    DrawLine(drawPosition.x, drawPosition.y, drawPosition.x+size*direction.x, drawPosition.y+size*direction.y, selfColor);
    DrawCircleLinesV(drawPosition, hitCircleSize, {255, 50, 50, 250});
}
void Enemy::draw()  {
    int max = 255;
    int base = 5;
    float rate = 12;
    int a = base + rate*(float)raycount/(float)(Player::Nray) * (max-base);
    drawA((unsigned char) (a > max ? max : a));
    raycount = 0;
}
void Enemy::update() {
    float a = angle + hview;
    maxlen = 0;
    int found_player = -1;
    for (int i=0; i<Nray; i++) {
        a -= delta;
        raycastLimitedReflections(inters[i], position, a, 2, this, this, viewLength);
        maxlen = inters[i].points.size() > maxlen ? inters[i].points.size() : maxlen;
        if (inters[i].ptr && inters[i].ptr->type==PLAYER) {
            found_player = i;
        }
    }
    if (found_player != -1) {
        selfColor = {200, 50, 50, 250};
        viewColor = {170,50,50,180};
        direction = {gamestate.Gplayer->position.x - position.x, gamestate.Gplayer->position.y - position.y};
        move.x += direction.x;
        move.y += direction.y;
    } else {
        selfColor = {150, 150, 50, 250};
        viewColor = {110,110,90,180};
    }
    float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
    if (dl>0) {
        direction.x /= dl; direction.y /= dl;
        move.x /= 2*dl; move.y /= 2*dl;
    }

    angleRad = atan2f(direction.y, direction.x);
    angle = angleRad*180/PI;
    
    
    Object* collision;
    position.x += move.x;
    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->type==WALL || collision->type==MIRROR) move.x *= 1.1; 
        position.x -= move.x;
    }
    
    position.y += move.y;

    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->type==WALL || collision->type==MIRROR) move.y *= 1.1;
        position.y -= move.y;
    }

    p1 = {position.x + size * cosf(angleRad - dirSizeAngle), position.y + size * sinf(angleRad - dirSizeAngle)};
    p2 = {position.x + size * cosf(angleRad + dirSizeAngle), position.y + size * sinf(angleRad + dirSizeAngle)};
    p3 = {position.x + size * cosf(PI + angleRad + dirSizeAngle), position.y + size * sinf(PI + angleRad + dirSizeAngle)};
    p4 = {position.x + size * cosf(PI + angleRad - dirSizeAngle), position.y + size * sinf(PI + angleRad - dirSizeAngle)};

    move = {0,0};
}
bool Enemy::intersects(const Point& p)  {
    return CheckCollisionPointTriangle(p, p1, p2, p3) || CheckCollisionPointTriangle(p, p3, p2, p4);
}
void Enemy::raycallback(Object* obj, float dist)  {
    if (obj->type == PLAYER) raycount++;
}
void Enemy::collidecallback(Entity* obj, const Point& point, const Vector2& direction)  {
    move.x += direction.x/2;
    move.y += direction.y/2;
}
