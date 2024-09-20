#include "game.h"

bool lineCircleIntersection(Point start, Point end, Point circle, float radius, Point& intersection) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    if (dx==0 && dy==0) return false;

    float xdif = start.x - circle.x;
    float ydif = start.y - circle.y;

    float alfa = dx*dx + dy*dy;
    float beta = dx*xdif + dy*ydif;
    float gamma = xdif*xdif + ydif*ydif - radius*radius;
    // alfa t^2 + 2*beta t + gamma = 0 

    float D = beta*beta - alfa*gamma;
    if (D < 0) return false;
    D = sqrtf(D);

    float t = (-beta + D)/alfa;
    if (t>=0 && t<=1) {
        intersection.x = start.x + dx*t;
        intersection.y = start.y + dy*t;
        return true;
    }
    t = (-beta - D)/alfa;
    if (t>=0 && t<=1) {
        intersection.x = start.x + dx*t;
        intersection.y = start.y + dy*t;
        return true;
    }

    return false;
}


Object* collide(Entity *obj, const Point& p, const Vector2& direction) {
    for (auto it=gamestate.Gobjects.begin(); it!=gamestate.Gobjects.end(); it++) {
        if (*it == obj || !(*it)->active) continue;
        if ((*it)->collidable && (*it)->intersects(p)) {
            (*it)->collidecallback(obj, p, direction);
            return (*it);
        }
    }
    return NULL;
}

Object* collideCircle(Entity *obj, const Point& circle, float radius, const Vector2& direction) {
    Point p;
    for (auto it=gamestate.Gobjects.begin(); it!=gamestate.Gobjects.end(); it++) {
        if (*it == obj || !(*it)->active) continue;
        if ((*it)->collidable && (*it)->intersectsCircle(circle, radius, p)) {
            (*it)->collidecallback(obj, p, direction);
            return (*it);
        }
    }
    return NULL;
}

Vector2 reflect(const Vector2&  v, const Vector2&  normal) {
    float dot = v.x*normal.x + v.y*normal.y;
    return {v.x - 2*dot*normal.x, v.y - 2*dot*normal.y};
}

Object* intersect(const Point& p, Object* ignore) {
    for (auto it=gamestate.Gobjects.begin(); it!=gamestate.Gobjects.end(); it++) {
        if (*it == ignore || !(*it)->active) continue;
        if (!(*it)->opaque && (*it)->intersects(p)) return *it;
    }
    return NULL;
}

void raycastLimitedReflections(IntersectInfo& result,Point start, float angle, float step, Object* ignore, Object* origin, float limit) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;

    float dx = cosf(angle/180 * PI)*step;
    float dy = sinf(angle/180 * PI)*step;
    Object* intobj = NULL;
    int reflections = 0;
    

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(origin, result.distance);
            if (intobj->reflects && reflections < gamestate.MAX_REFLECTIONS) {
                Vector2 r = reflect({dx, dy}, intobj->normal);
                dx = r.x; dy = r.y;
                ignore = intobj;
                intobj = NULL;
                result.points.push_back(start);
                start.x += 2*dx;
                start.y += 2*dy;
                reflections++;
            } else break;
        }
        start.x += dx;
        start.y += dy;
        result.distance += step;
        if (result.distance >= limit) {
            result.points.push_back(start);
            break;
        }

        if (start.x >= gamestate.MapXf) {
            result.points.push_back({gamestate.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.MapYf) {
            result.points.push_back({start.x, gamestate.MapYf});
            break;
        } else if (start.y < 0) {
            result.points.push_back({start.x, 0});
            break;
        }
    }
    if (intobj) {
        // result.type = intobj->type;
        result.points.push_back(start);
    }
    result.ptr = intobj;
}

void raycastLimited(IntersectInfo& result, Point start, float angle, float step, Object* ignore, Object* origin, float limit) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;

    float dx = cosf(angle/180 * PI)*step;
    float dy = sinf(angle/180 * PI)*step;
    Object* intobj = NULL;

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(origin, result.distance);
            result.points.push_back(start);
            break;
        }
        start.x += dx;
        start.y += dy;
        result.distance += step;
        if (result.distance >= limit) {
            result.points.push_back(start);
            break;
        }
        
        if (start.x >= gamestate.MapXf) {
            result.points.push_back({gamestate.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.MapYf) {
            result.points.push_back({start.x, gamestate.MapYf});
            break;
        } else if (start.y < 0) {
            result.points.push_back({start.x, 0});
            break;
        }
        
    }
    result.ptr = intobj;
}
void raycast(IntersectInfo& result, Point start, float angle, float step, Object* ignore, Object* origin) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;

    float dx = cosf(angle/180 * PI)*step;
    float dy = sinf(angle/180 * PI)*step;
    Object* intobj = NULL;
    int reflections = 0;
    

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(origin, result.distance);
            if (intobj->reflects && reflections < gamestate.MAX_REFLECTIONS) {
                Vector2 r = reflect({dx, dy}, intobj->normal);
                dx = r.x; dy = r.y;
                ignore = intobj;
                intobj = NULL;
                result.points.push_back(start);
                start.x += 2*dx;
                start.y += 2*dy;
                reflections++;
            } else break;
        }
        start.x += dx;
        start.y += dy;
        result.distance += step;

        if (start.x >= gamestate.MapXf) {
            result.points.push_back({gamestate.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.MapYf) {
            result.points.push_back({start.x, gamestate.MapYf});
            break;
        } else if (start.y < 0) {
            result.points.push_back({start.x, 0});
            break;
        }
    }
    if (intobj) {
        // result.type = intobj->type;
        result.points.push_back(start);
    }
    result.ptr = intobj;
}

// raycast with direction
void raycast(IntersectInfo& result, Point start, Vector2 direct, Object* ignore, Object* origin) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;

    Object* intobj = NULL;
    float step = sqrtf(direct.x*direct.x + direct.y*direct.y);
    int reflections = 0;

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(origin, result.distance);
            if (intobj->reflects && reflections < gamestate.MAX_REFLECTIONS) {
                direct = reflect(direct, intobj->normal);
                // direct.x *= 3/2;
                // direct.y *= 3/2;
                ignore = intobj;
                intobj = NULL;
                result.points.push_back(start);
                start.x += 2*direct.x;
                start.y += 2*direct.y;
                reflections++;
            } else break;
        }
        start.x += direct.x;
        start.y += direct.y;
        result.distance += step;

        if (start.x >= gamestate.MapXf) {
            result.points.push_back({gamestate.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.MapYf) {
            result.points.push_back({start.x, gamestate.MapYf});
            break;
        } else if (start.y < 0) {
            result.points.push_back({start.x, 0});
            break;
        }
    }
    if (intobj) {
        // result.type = intobj->type;
        result.points.push_back(start);
    }
    result.ptr = intobj;
}

Point projectToCamera(const Point& p) {
    return {p.x - gamestate.camera.x, p.y - gamestate.camera.y};
}

Point projectToMap(const Point& p) {
    return {p.x + gamestate.camera.x, p.y + gamestate.camera.y};
}

void update() {        
    // std::cout << "Update" << std::endl;
    Vector2 move = {0};
    if (IsKeyDown(KEY_A)) {
        move.x -= 4;
    } else if (IsKeyDown(KEY_D)) {
        move.x += 4;
    }

    if (IsKeyDown(KEY_W)) {
        move.y -= 4;
    } else if (IsKeyDown(KEY_S)) {
        move.y += 4;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT)){
        move.x *= 1.5;
        move.y *= 1.5;
    }
    

    Point mouse = GetMousePosition();
    float koef = 1.2;
    if (gamestate.Gplayer) {
        gamestate.camera.x = (koef*(gamestate.Gplayer->position.x) + gamestate.camera.x + mouse.x)/(1+koef) - (gamestate.WinXf)/2;
        gamestate.camera.y = (koef*(gamestate.Gplayer->position.y) + gamestate.camera.y + mouse.y)/(1+koef) - (gamestate.WinYf)/2;
        gamestate.Gplayer->move = move;
        gamestate.Gplayer->update();
    } else {
        gamestate.camera.x += move.x;
        gamestate.camera.y += move.y;
    }
    for (auto it=gamestate.Gobjects.begin(); it!=gamestate.Gobjects.end(); it++) {
        if (!(*it)->active || (*it)==gamestate.Gplayer) continue;
        // std::cout << (*it)->type << std::endl;
        (*it)->update();
    }
}


void draw() {
    ClearBackground({0,0,0,255});
    DrawRectangleV(projectToCamera({0,0}), {gamestate.MapXf, gamestate.MapYf}, {25,25,25,255});
    if (gamestate.Gplayer) gamestate.Gplayer->draw();
    for (auto it=gamestate.Gobjects.begin(); it!=gamestate.Gobjects.end(); it++) {
        if (!(*it)->active || (*it)==gamestate.Gplayer) continue;
        if ((*it)->visible) {
            (*it)->draw();
        }
    }
    if (gamestate.pause) {
        DrawText("PAUSED", gamestate.WinX - 6*20 - 5, 0, 30, GREEN);
    }
}