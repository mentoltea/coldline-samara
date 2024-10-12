#include "game.h"

Item::Item() {
    active = true;
    collidable = false;
    visible = true;
    opaque = true;
    reflects = false;
    gentype = ITEM;
}

void Item::raycallback(Object* obj, float dist) {
    // std::cout << "cal " << std::endl;
    if (obj->type==PLAYER) raycount++;
}

Firearm::Firearm(Point position, Poly body, bool onFloor) {
    this->position = position;
    this->onFloor = onFloor;
    this->body = body;
    visible = onFloor;
    subgentype = FIREARM;
}
Firearm::~Firearm() = default;
void Firearm::drawA(unsigned char alfa) {
    selfcolor.a = alfa;
    // std::cout << (int)alfa << std::endl;
    Point p1 = projectToCamera( position + body.p1 ); 
    Point p2 = projectToCamera( position + body.p2 );
    Point p3 = projectToCamera( position + body.p3 );
    Point p4 = projectToCamera( position + body.p4 );

    DrawTriangle(p1, p2, p3, selfcolor );
    DrawTriangle(p3, p2, p4, selfcolor );
    // DrawCircleV(p1, 3, {255,255,255,255});
    // DrawCircleV(p2, 3, selfcolor);
}
void Firearm::draw() {
    if (onFloor) {
        // std::cout << "draw" << std::endl;
        // int max = 255;
        // int base = 60;
        // float rate = 20;
        // int a = base + rate*(float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (max-base);
        drawA((unsigned char) (200));
    }
    raycount = 0;
}
void Firearm::update() {
    if (onFloor) {
        reloading_tick = 0;
        return;
    }
    if (reloading) {
        if (reloading_tick > (TICK)) {
            int transferrounds = maxrounds-rounds < extrarounds ? maxrounds-rounds : extrarounds;
            extrarounds -= transferrounds;
            rounds += transferrounds;

            reloading = false;
        }
        reloading_tick++;
    }
    if (delay_tick>0) delay_tick--;
}
bool Firearm::usable() {
    return (rounds + extrarounds > 0);
}
void Firearm::use(Entity* user) {
    if (!usable()) return;
    if (rounds>0) {
        if (delay_tick<=0) {
            Point startpoint = user->position;
            startpoint.x += user->direction.x *(user->hitCircleSize+bulletsize+1);
            startpoint.y += user->direction.y *(user->hitCircleSize+bulletsize+1);

            Projectile* newobj = NEW(Bullet) Bullet(startpoint, 50*user->direction, bulletsize);
            gamestate.currentLevel.projects.push_front(newobj);

            rounds--;
            delay_tick = TICK*delay;
        }
    } else if (!reloading) {
        reloading = true;
        reloading_tick = 0;
    }
}

bool Firearm::intersects(const Point& p) {
    return CheckCollisionPointTriangle(p, position + body.p1, position + body.p2, position + body.p3) 
    || CheckCollisionPointTriangle(p, position + body.p3, position + body.p2, position + body.p4);
}

Pistol::Pistol(Point position, Poly body, bool onFloor): Firearm(position, body, onFloor) {
    rounds = 7;
    maxrounds = 7;
    extrarounds = 35;
    bulletsize = 3;
    delay = 0.5;
    type = PISTOL;
    selfcolor = {80, 140, 190, 250};
}
Pistol::~Pistol() = default;

Rifle::Rifle(Point position, Poly body, bool onFloor): Firearm(position, body, onFloor) {
    rounds = 30;
    maxrounds = 30;
    extrarounds = 90;
    bulletsize = 2;
    delay = 0.01;
    type = RIFLE;
    selfcolor = {140, 100, 130, 250};
}
Rifle::~Rifle() = default;

// void Firearm::raycallback(Object* obj, float dist) {
//     if (obj->type==PLAYER) raycount++;
// }

Entity::Entity() {
    active = true;
    collidable = true;
    visible = true;
    opaque = false;
    reflects = false;
    gentype = ENTITY;
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

void Entity::pickItem() {
    auto itemnear = nearestItem(position);
    dropItem();
    if (std::get<0>(itemnear)) {
        if (std::get<2>(itemnear) < 2*hitCircleSize) {
            (std::get<0>(itemnear))->onFloor = false;
            (std::get<0>(itemnear))->visible = false;
            selfitem = std::get<1>(itemnear);
        }
    }
}

void Entity::dropItem() {
    if (selfitem==-1) return;
    Item* item = (Item*)gamestate.currentLevel.objects[selfitem];
    float dx = hitCircleSize*(2*randf()-1);
    float dy = hitCircleSize*(2*randf()-1);
    
    item->position = position + (Vector2){dx, dy};
    item->onFloor = true;
    item->visible = true;

    selfitem = -1;
}

Projectile::Projectile() {
    active = true;
    collidable = true;
    visible = true;
    opaque = false;
    reflects = false;
    gentype = PROJECTILE;
    finished = false;
}

Bullet::Bullet(Point position, Vector2 direction, float radius) {
    this->position = position;
    this->direction = direction;
    this->hitCircleSize = radius;
    type = BULLET;
}
Bullet::~Bullet() = default;
void Bullet::drawA(unsigned char alfa) {
    selfcolor.a = alfa;
    drawPosition = projectToCamera(position);
    DrawCircleV(drawPosition, hitCircleSize, selfcolor);
}
void Bullet::draw() {
    // std::cout << "draw" << std::endl;
    drawA(180);
    raycount = 0;
}
void Bullet::update() {
    Object* collision;
    float totalx = 0;
    float totaly = 0;

    while (totalx < absf(direction.x) || totaly < absf(direction.y)) {
        position.x += direction.x/10;
        position.y += direction.y/10;
        totalx += absf(direction.x/10);
        totaly += absf(direction.y/10);

        collision = collideCircle(this, position, hitCircleSize, direction);
        if (collision) {
            collision->projectilecallback(this);
            active = false;
            visible = false;
            onDestroy();
            break;
        }
    }
}
bool Bullet::intersects(const Point& p) {
    float dx = p.x - position.x;
    float dy = p.y - position.y;
    if (dx*dx + dy*dy < hitCircleSize*hitCircleSize) return true;
    return false;
}
bool Bullet::intersectsCircle(const Point& circle, float radius, Point& intersection) {
    float dx = position.x - circle.x;
    float dy = position.y - circle.y;
    float sumrad = radius + hitCircleSize;
    float diffrad = radius - hitCircleSize;
    diffrad = diffrad<0? -diffrad : diffrad;

    if (dx*dx + dy*dy > sumrad*sumrad) return false;
    if (dx*dx + dy*dy < diffrad) return false;
    
    return true;
}
void Bullet::raycallback(Object* obj, float dist) {
    if (obj->type==PLAYER) raycount++;
}
void Bullet::collidecallback(Entity* obj, const Point& point, const Vector2& direction) {
    obj->projectilecallback(this);
    active = false;
    visible = false;
    onDestroy();
}
void Bullet::projectilecallback(Projectile* proj) {
    // Two bullets met
    onDestroy();
}
void Bullet::onDestroy() {
    // BOOM
}



Obtacle::Obtacle() {
    active = true;
    collidable = true;
    visible = true;
    gentype = OBTACLE;
}
void Obtacle::collidecallback(Entity* obj, const Point& point, const Vector2& direction) {}
void Obtacle::projectilecallback(Projectile* proj) {
}



Mirror::Mirror(Poly b, Vector2 n) {
    opaque = false;
    type = MIRROR;
    reflects = true;

    float nl = sqrtf(n.x*n.x + n.y*n.y);
    normal = {n.x/nl, n.y/nl};
    body = b;
}

Mirror::~Mirror() = default;

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
void Mirror::projectilecallback(Projectile* proj) {
    active = false;
    collidable = false;
    visible = false;
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
void Wall::projectilecallback(Projectile* proj) {

}

TextSegment::TextSegment(Poly b, std::string text, int fontsize): Wall(b) {
    snprintf(this->text, 64, text.c_str());
    this->fontsize = fontsize;
    opaque = false;
    reflects = false;
    type = TEXTSEGMENT;
}
TextSegment::~TextSegment() = default;

void TextSegment::drawA(unsigned char alfa) {
    DrawTriangle(drawBody.p1, drawBody.p2, drawBody.p3, {181,67,22,alfa} );
    DrawTriangle(drawBody.p3, drawBody.p2, drawBody.p4, {181,67,22,alfa} );
    DrawText(text, drawBody.p2.x + offset.x, drawBody.p2.y + offset.y, fontsize, {textColor.r, textColor.g, textColor.b, alfa});
}

void TextSegment::draw()  {
    int max = 255;
    int base = 10;
    float rate = 2;
    int a = base + rate*(float)raycount/(float)(Player::Nray + Player::Nrayback/2) * (max-base);
    drawA((unsigned char) (a > 255 ? 255 : a));
    raycount = 0;
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

    selfTexture = TM::GetT(TM::Tid::TPlayer);
    if(selfTexture) this->size = sqrtf((float)selfTexture->width*(float)selfTexture->width + (float)selfTexture->height*(float)selfTexture->height);
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
    selfTexture = TM::GetT(TM::Tid::TPlayer);

    drawViewAround();
    drawView();
    if (!selfTexture) {
        dp1 = {drawPosition.x + size * cosf(angleRad - dirSizeAngle), drawPosition.y + size * sinf(angleRad - dirSizeAngle)};
        dp2 = {drawPosition.x + size * cosf(angleRad + dirSizeAngle), drawPosition.y + size * sinf(angleRad + dirSizeAngle)};
        dp3 = {drawPosition.x + size * cosf(PI + angleRad + dirSizeAngle), drawPosition.y + size * sinf(PI + angleRad + dirSizeAngle)};
        dp4 = {drawPosition.x + size * cosf(PI + angleRad - dirSizeAngle), drawPosition.y + size * sinf(PI + angleRad - dirSizeAngle)};
        DrawTriangle(dp2, dp1,  dp3, {100, 100, 200, 250});
        DrawTriangle(dp3, dp4, dp2,  {100, 100, 200, 250});
    }
    else {
        float k = 3;
        
        dp1 =  {drawPosition.x - ((k)/2)*size* cosf(angleRad - dirSizeAngle)/2,
                drawPosition.y - ((k)/2)*size* sinf(angleRad - dirSizeAngle)/2};
        dp2 =  {drawPosition.x - ((k)/2)*size * cosf(angleRad + dirSizeAngle)/2,
                drawPosition.y - ((k)/2)*size * sinf(angleRad + dirSizeAngle)/2};
        dp3 =  {drawPosition.x - ((k)/2)*size * cosf(PI + angleRad + dirSizeAngle)/2, 
                drawPosition.y - ((k)/2)*size * sinf(PI + angleRad + dirSizeAngle)/2};
        dp4 =  {drawPosition.x - ((k)/2)*size * cosf(PI + angleRad - dirSizeAngle)/2, 
                drawPosition.y - ((k)/2)*size * sinf(PI + angleRad - dirSizeAngle)/2};
        // DrawTextureEx(*selfTexture, {drawPosition.x - selfTexture->width/2, drawPosition.y - selfTexture->height/2,}, angle, 2, RAYWHITE);
        DrawTexturePro(*selfTexture, {.x=0, .y=0, .width=(float)selfTexture->width, .height=(float)selfTexture->height},
            {   .x= dp2.x,
                .y= dp2.y,
                .width= (k)*(float)selfTexture->width,
                .height= (k)*(float)selfTexture->height}, // dest
            {.x=(float)selfTexture->width/2, .y=(float)selfTexture->height/2}, // origin
            angle, 
            RAYWHITE);
    }
    // DrawCircleLinesV(drawPosition, hitCircleSize, {255, 50, 50, 250});
    // DrawCircleLinesV(dp1, 2, {50, 50, 250, 250});
    // DrawCircleLinesV(dp2, 2, {50, 50, 250, 250});
    // DrawCircleLinesV(dp3, 2, {50, 50, 250, 250});
    // DrawCircleLinesV(dp4, 2, {50, 50, 250, 250});
    // DrawCircleLinesV(drawPosition, 2, {50, 50, 250, 250});
}
void Player::update()  {
    if (!alive) return;
    Vector2 mouse = GetMousePosition();
    direction = {mouse.x - drawPosition.x, mouse.y - drawPosition.y};
    float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
    if (dl>0) {direction.x /= dl; direction.y /= dl;}

    angleRad = atan2f(direction.y, direction.x);
    // angleRad = 0;
    angle = angleRad*180/PI;
    Object* collision;
    position.x += move.x;
    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->gentype == OBTACLE) move.x *= 1.1;
        position.x -= move.x;
    }
    position.y += move.y;
    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->gentype == OBTACLE) move.y *= 1.1;
        position.y -= move.y;
    }
    p1 = {position.x + size * cosf(angleRad - dirSizeAngle), position.y + size * sinf(angleRad - dirSizeAngle)};
    p2 = {position.x + size * cosf(angleRad + dirSizeAngle), position.y + size * sinf(angleRad + dirSizeAngle)};
    p3 = {position.x + size * cosf(PI + angleRad + dirSizeAngle), position.y + size * sinf(PI + angleRad + dirSizeAngle)};
    p4 = {position.x + size * cosf(PI + angleRad - dirSizeAngle), position.y + size * sinf(PI + angleRad - dirSizeAngle)};
    
    if (use_item && selfitem!=-1) {
        Item* item = (Item*)gamestate.currentLevel.objects[selfitem];
        item->use(this);
    }
    use_item = false;
}
bool Player::intersects(const Point& p)  {
    // return CheckCollisionPointPoly(p, (Vector2*)&body, 4);
    if (gamestate.currentLevel.cheats.invisible) return false;
    return CheckCollisionPointTriangle(p, p1, p2, p3) || CheckCollisionPointTriangle(p, p3, p2, p4);
}
void Player::raycallback(Object* obj, float dist)  {
}
void Player::collidecallback(Entity* obj, const Point& point, const Vector2& direction)  {
}
void Player::projectilecallback(Projectile* proj) {
    alive = false;
}

void EnemyBehaviour::update(Enemy* self) {
    if (self->see_player || warned) {
        // std::cout << "see player" << std::endl;
        if (!see_player_way_updated && !warned) {
            near = gamestate.currentLevel.nearPoint(self->position);
            auto playernear = gamestate.currentLevel.nearPoint(gamestate.currentLevel.player->position);
            if (std::get<1>(near) == std::get<1>(playernear)) {
                while (!currentway.empty()) {
                    currentway.pop();
                }
                self->target = gamestate.currentLevel.player->position;
            }
            else {
                currentway = gamestate.currentLevel.way(std::get<1>(near), std::get<1>(playernear));
            }
            see_player_way_updated = true;
            warned = true;
            lost = false;
            tick_warned = 1;
            self->chase_target = true;
        } 
        if (!currentway.empty()) {
            self->target = gamestate.currentLevel.MapPoints[currentway.top()]; 
    
            if (distance(self->target, self->position) < self->hitCircleSize*2/5 && !currentway.empty()) {
                currentway.pop();
                if (currentway.empty()) {
                    self->target = gamestate.currentLevel.player->position;
                }
            }
        } else {
            // self->target = gamestate.currentLevel.player->position;
            if (self->see_player) {
                self->chase_target = true;
                self->target = gamestate.currentLevel.player->position;
                lost = false;
            }
            else {
                if (distance(self->target, self->position) < self->hitCircleSize*7/5) {
                    if (!lost) {
                        lost = true;
                        tick_lost = 1;
                        self->chase_target = false;
                    }
                    else {
                        tick_lost++;
                        if (tick_lost%(3*TICK)==0) {
                            warned = false;
                        }
                    }    
                }
            }
        }

        tick_warned++;
        if (tick_warned%(5*TICK) == 0 && !self->see_player) warned=false; 
    } 
    else if (selfway.size() > 0) {
        see_player_way_updated=false;

        if (currentway.empty()) {
            near = gamestate.currentLevel.nearPoint(self->position);
            currentway = gamestate.currentLevel.way(std::get<1>(near), selfway.at(selfwayidx));
            selfwayidx = (selfwayidx+1)%selfway.size();
        }
        
        self->target = gamestate.currentLevel.MapPoints[currentway.top()];
        
        // std::cout << currentway.top() << std::endl;

        if (distance(self->target, self->position) < self->hitCircleSize*2/5 && !currentway.empty()) {
            currentway.pop();
            if (currentway.empty()) {
                near = gamestate.currentLevel.nearPoint(self->position);
                currentway = gamestate.currentLevel.way(std::get<1>(near), selfway.at(selfwayidx));
                selfwayidx = (selfwayidx+1)%selfway.size();
                currentway.pop();
                if (currentway.empty()) {self->chase_target = false;}
                else {self->target = gamestate.currentLevel.MapPoints[currentway.top()];}
            } else {
                self->chase_target = true;
            }
        } else {
            self->chase_target = true;
        }
    }
    else {
        see_player_way_updated=false;
        self->chase_target = false;
    }
}

Enemy::Enemy(Point pos, Vector2 size, std::vector<int> way): inters(Nray), behaviour(way) {
    position = pos;
    float sl = sqrtf(size.x*size.x + size.y*size.y);
    hitCircleSize = sl*0.95;
    dirSizeAngle = acosf(size.x/sl);
    this->size = sl;
    type = ENEMY;
}
Enemy::~Enemy() {
    // decltype(inters)().swap(inters);
    // decltype(selfway)().swap(selfway);
};
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
    
    if (alive) {
        selfColor.a = alfa;
        drawView(alfa/3); 
    }
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
    if (!alive) {
        selfColor = GRAY;
        collidable = false;
        opaque = true;
        return;
    }
    
    float a = angle + hview;
    maxlen = 0;
    int found_player = -1;
    see_player = false;
    for (int i=0; i<Nray; i++) {
        a -= delta;
        raycastLimitedReflections(inters[i], position, a, 2, this, this, viewLength);
        maxlen = inters[i].points.size() > maxlen ? inters[i].points.size() : maxlen;
        if (inters[i].ptr && inters[i].ptr->type==PLAYER) {
            found_player = i;
        }
    }
    if (found_player != -1) see_player = true;
    
    behaviour.update(this);
    if (see_player) {
        selfColor = {200, 50, 50, 250};
        viewColor = {170,50,50,180};
    }
    else if (behaviour.warned) {
        selfColor = {150, 150, 50, 250};
        viewColor = {110,110,90,180};
    } else {
        selfColor = {130, 140, 90, 250};
        viewColor = {100,110,90,180};
    }

    // std::cout << "1" << std::endl;
    
    if (chase_target) {
        Vector2 targetdirection = {target.x - position.x, target.y - position.y};
        float targetangleRad = atan2f(targetdirection.y, targetdirection.x);
        
        
        float anglediffRad = targetangleRad - angleRad;
        
        // anglediffRad = constraintBetween(anglediffRad, -PI, PI);
        
        while (absf(anglediffRad) > PI) {
            int anglesign = signf(anglediffRad);
            anglediffRad = 2*PI - absf(anglediffRad);
            anglediffRad *= -anglesign;
        }
        // while (anglediffRad>2*PI)
        // {
        //     anglediffRad-=PI;
        // }
        // while (anglediffRad<-2*PI)
        // {
        //     anglediffRad+=PI;
        // }
        

        
        angleRad += anglediffRad/8;
        // angleRad = targetangleRad;

        direction = {cosf(angleRad), sinf(angleRad)};
        move.x += direction.x * (cosf(anglediffRad) + 0.1f);
        move.y += direction.y * (cosf(anglediffRad) + 0.1f);
        angleRad = constraintBetween(angleRad, -2*PI, 2*PI);

        // std::cout << angleRad << std::endl;
    }
    
    float dl = sqrtf(direction.x*direction.x + direction.y*direction.y);
    if (dl>0) {
        direction.x /= dl; direction.y /= dl;
        move.x /= dl/4;
        move.y /= dl/4;
    }

    // angleRad = atan2f(direction.y, direction.x);
    angle = angleRad*180/PI;
    
    
    Object* collision;
    position.x += move.x;
    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->gentype == OBTACLE) move.x *= 1.1; 
        position.x -= move.x;
    }
    
    position.y += move.y;

    if ((collision= collideCircle(this, position, hitCircleSize, move))) {
        if (collision->gentype == OBTACLE) move.y *= 1.1;
        position.y -= move.y;
    }

    p1 = {position.x + size * cosf(angleRad - dirSizeAngle), position.y + size * sinf(angleRad - dirSizeAngle)};
    p2 = {position.x + size * cosf(angleRad + dirSizeAngle), position.y + size * sinf(angleRad + dirSizeAngle)};
    p3 = {position.x + size * cosf(PI + angleRad + dirSizeAngle), position.y + size * sinf(PI + angleRad + dirSizeAngle)};
    p4 = {position.x + size * cosf(PI + angleRad - dirSizeAngle), position.y + size * sinf(PI + angleRad - dirSizeAngle)};

    move = {0,0};
    // std::cout << "4" << std::endl;
}
bool Enemy::intersects(const Point& p)  {
    return CheckCollisionPointTriangle(p, p1, p2, p3) || CheckCollisionPointTriangle(p, p3, p2, p4);
}
void Enemy::raycallback(Object* obj, float dist)  {
    if (obj->type == PLAYER) raycount++;
}
void Enemy::collidecallback(Entity* obj, const Point& point, const Vector2& direction)  {
    Vector2 dir = direction;
    if (obj->gentype == PROJECTILE) dir = dir * 0.1;
    move.x += dir.x/2;
    move.y += dir.y/2;
}

void Enemy::projectilecallback(Projectile* proj) {
    alive = false;
}
