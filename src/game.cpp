#include "game.h"

Object* collide(Entity *obj, const Point& p, const Vector2& direction) {
    for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
        if (*it == obj || !(*it)->active) continue;
        if ((*it)->collidable && (*it)->intersects(p)) {
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
    for (auto it=Gobjects.begin(); it!=Gobjects.end(); it++) {
        if (*it == ignore || !(*it)->active) continue;
        if (!(*it)->opaque && (*it)->intersects(p)) return *it;
    }
    return NULL;
}
void raycastLimited(IntersectInfo& result, Point start, float angle, float step, Object* ignore, float limit) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;

    float dx = cosf(angle/180 * PI)*step;
    float dy = sinf(angle/180 * PI)*step;
    Object* intobj = NULL;

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(ignore, result.distance);
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
        
        if (start.x >= WinXf) {
            result.points.push_back({WinXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= WinYf) {
            result.points.push_back({start.x, WinYf});
            break;
        } else if (start.y < 0) {
            result.points.push_back({start.x, 0});
            break;
        }
        
    }
    result.ptr = intobj;
}
void raycast(IntersectInfo& result, Point start, float angle, float step, Object* ignore) {
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
            intobj->raycallback(ignore, result.distance);
            if (intobj->reflects && reflections < MAX_REFLECTIONS) {
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

        if (start.x >= WinXf) {
            result.points.push_back({WinXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= WinYf) {
            result.points.push_back({start.x, WinYf});
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
void raycast(IntersectInfo& result, Point start, Vector2 direct, Object* ignore) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;
    
    Object* intobj = NULL;
    float step = sqrtf(direct.x*direct.x + direct.y*direct.y);
    int reflections = 0;

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(ignore, result.distance);
            if (intobj->reflects && reflections < MAX_REFLECTIONS) {
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

        if (start.x >= WinXf) {
            result.points.push_back({WinXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= WinYf) {
            result.points.push_back({start.x, WinYf});
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
