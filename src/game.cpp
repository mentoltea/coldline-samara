#include "game.h"
#include "effects.h"

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

std::tuple<Item*, int, float> nearestItem(const Point& p) {
    Item* obj = NULL;
    int minidx = -1;
    float mindist = INFINITY;
    int idx = -1;
    for (auto it=gamestate.currentLevel.objects.begin(); it!=gamestate.currentLevel.objects.end(); it++) {
        idx++;
        if (!(*it)->active || (*it)->gentype!=ITEM) continue;
        Item* cur = (Item*) (*it);
        if (!cur->onFloor) continue;
        float dx = p.x - cur->position.x;
        float dy = p.y - cur->position.y;
        float dist = dx*dx + dy*dy;
        if (dist < mindist) {
            mindist = dist;
            obj = cur;
            minidx = idx;
        }
    }
    return {obj, minidx, sqrtf(mindist)};
}

Object* collide(Entity *obj, const Point& p, const Vector2& direction) {
    for (auto it=gamestate.currentLevel.objects.begin(); it!=gamestate.currentLevel.objects.end(); it++) {
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
    for (auto it=gamestate.currentLevel.objects.begin(); it!=gamestate.currentLevel.objects.end(); it++) {
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
    for (auto it=gamestate.currentLevel.objects.begin(); it!=gamestate.currentLevel.objects.end(); it++) {
        if (*it == ignore || !(*it)->active) continue;
        if (!(*it)->opaque && (*it)->intersects(p)) return *it;
    }
    return NULL;
}

void raycastLimitedReflections(IntersectInfo& result,Point start, float angle, float initialstep, Object* ignore, Object* origin, float limit) {
    result.distance = 0;
    result.points.clear();
    result.ptr = NULL;
    float step = initialstep;
    int stepcount = 1;
    int stepchangecount = 200;

    float initial_dx = cosf(angle/180 * PI)*step;
    float initial_dy = sinf(angle/180 * PI)*step;

    float dx = initial_dx;
    float dy = initial_dy;

    Object* intobj = NULL;
    int reflections = 0;
    

    while (true) {
        intobj = intersect(start, ignore);
        if (intobj) {
            intobj->raycallback(origin, result.distance);
            // if (intobj->opaque) continue;
            if (intobj->reflects && reflections < gamestate.MAX_REFLECTIONS) {
                Vector2 r = reflect({dx, dy}, intobj->normal);
                initial_dx = r.x; initial_dy = r.y;
                ignore = intobj;
                intobj = NULL;
                result.points.push_back(start);
                start.x += 2*dx;
                start.y += 2*dy;
                reflections++;
                dx = initial_dx;
                dy = initial_dy;
                step = initialstep;
                stepcount = 1;
            } else break;
        }
        start.x += dx;
        start.y += dy;
        result.distance += step;
        stepcount++;
        if (result.distance >= limit) {
            result.points.push_back(start);
            break;
        }
        if (stepcount%stepchangecount == 0) {
            step = (1 + (float)stepcount/(float)stepchangecount/2.f)*initialstep;
            dx = (1 + (float)stepcount/(float)stepchangecount/2.f)*initial_dx;
            dy = (1 + (float)stepcount/(float)stepchangecount/2.f)*initial_dy;
        }

        if (start.x >= gamestate.currentLevel.MapXf) {
            result.points.push_back({gamestate.currentLevel.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.currentLevel.MapYf) {
            result.points.push_back({start.x, gamestate.currentLevel.MapYf});
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
            // if (intobj->opaque) continue;
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
        
        if (start.x >= gamestate.currentLevel.MapXf) {
            result.points.push_back({gamestate.currentLevel.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.currentLevel.MapYf) {
            result.points.push_back({start.x, gamestate.currentLevel.MapYf});
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
            // if (intobj->opaque) continue;
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

        if (start.x >= gamestate.currentLevel.MapXf) {
            result.points.push_back({gamestate.currentLevel.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.currentLevel.MapYf) {
            result.points.push_back({start.x, gamestate.currentLevel.MapYf});
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
            // if (intobj->opaque) continue;
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

        if (start.x >= gamestate.currentLevel.MapXf) {
            result.points.push_back({gamestate.currentLevel.MapXf, start.y});
            break;
        } else if (start.x < 0) {
            result.points.push_back({0, start.y});
            break;
        }
        if (start.y >= gamestate.currentLevel.MapYf) {
            result.points.push_back({start.x, gamestate.currentLevel.MapYf});
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

void DrawTexturePoly(Texture2D texture, Vector2 center, Vector2 *points, Vector2 *texcoords, int pointCount, Color tint) {
    rlSetTexture(texture.id);

    // Texturing is only supported on RL_QUADS
    rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        for (int i = 0; i < pointCount - 1; i++)
        {
            rlTexCoord2f(0.5f, 0.5f);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(texcoords[i].x, texcoords[i].y);
            rlVertex2f(points[i].x + center.x, points[i].y + center.y);

            rlTexCoord2f(texcoords[i + 1].x, texcoords[i + 1].y);
            rlVertex2f(points[i + 1].x + center.x, points[i + 1].y + center.y);

            rlTexCoord2f(texcoords[i + 1].x, texcoords[i + 1].y);
            rlVertex2f(points[i + 1].x + center.x, points[i + 1].y + center.y);
        }
    rlEnd();

    rlSetTexture(0);
}

int ticksFromPicking = 0;
void update() {        
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
    // std::cout << gamestate.currentLevel.projects.size() << std::endl;
    for (auto it=gamestate.currentLevel.projects.begin(); it!=gamestate.currentLevel.projects.end(); it++) {
        if ((*it)->finished) {
            // std::cout << "delete" << std::endl;
            DELETE(Projectile, (*it));
            gamestate.currentLevel.projects.erase(it);
            continue;
        }
        if ((*it)->active) {
            (*it)->update();
            // std::cout << "update" << std::endl;
        }
    }
    
    Point mouse = GetMousePosition();
    float koef = 1.2;
    if (gamestate.currentLevel.player) {
        gamestate.camera.x = (koef*(gamestate.currentLevel.player->position.x) + gamestate.camera.x + mouse.x)/(1+koef) - (gamestate.WinXf)/2;
        gamestate.camera.y = (koef*(gamestate.currentLevel.player->position.y) + gamestate.camera.y + mouse.y)/(1+koef) - (gamestate.WinYf)/2;
        gamestate.currentLevel.player->move = move;
        if (ticksFromPicking) ticksFromPicking--;
        else {
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                gamestate.currentLevel.player->pickItem();
                ticksFromPicking = TICK/3;
            }
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            gamestate.currentLevel.player->use_item = true;
        }
        // std::cout << "player" << std::endl;
        gamestate.currentLevel.player->update();
    } else {
        gamestate.camera.x += move.x;
        gamestate.camera.y += move.y;
    }

    // std::cout << "objects" << std::endl;
    int enemies = 0;
    for (auto it=gamestate.currentLevel.objects.begin(); it!=gamestate.currentLevel.objects.end(); it++) {
        if (!(*it)->active || (*it)==gamestate.currentLevel.player) continue;
        (*it)->update();
        if ((*it)->type == ENEMY) {
            if (((Enemy*)(*it))->alive) enemies++;
        }
    }
    if (enemies==0) gamestate.levelComplete = true;

    for (auto it = gamestate.currentLevel.effects.begin(); it != gamestate.currentLevel.effects.end(); it++) {
        if ((*it)->finished()) {
            DELETE(Effect, (*it));
            gamestate.currentLevel.effects.erase(it);
            continue;
        }
        (*it)->update();
    }

    tickcount++;
    tickcount = tickcount%(120*TICK);
}


void draw() {
    // BeginDrawing();
    char buffer[32];
    ClearBackground({0,0,0,255});
    DrawRectangleV(projectToCamera({0,0}), {gamestate.currentLevel.MapXf, gamestate.currentLevel.MapYf}, {25,25,25,255});
    if (gamestate.currentLevel.player) {
        gamestate.currentLevel.player->draw();
        // std::cout << "player" << std::endl;
    }
    // std::cout << "not player" << std::endl;
    for (auto it=gamestate.currentLevel.projects.begin(); it!=gamestate.currentLevel.projects.end(); it++) {
        if (!(*it)->finished && (*it)->visible) (*it)->draw();
    }
    
    for (auto it=gamestate.currentLevel.objects.begin(); it!=gamestate.currentLevel.objects.end(); it++) {
        if ((*it)==gamestate.currentLevel.player) continue;
        // std::cout << (*it) << std::endl;
        // std::cout << (*it)->type << " ";
        if ((*it)->visible) {
            (*it)->draw();
        }
        // std::cout << "object" << std::endl;
    }

    for (auto it = gamestate.currentLevel.effects.begin(); it != gamestate.currentLevel.effects.end(); it++) {
        (*it)->draw();
    }



    // Draw bot path graph
    if (gamestate.currentLevel.cheats.see_MP) {
        for (int i=0; i<(int)gamestate.currentLevel.MapPoints.size(); i++) {
            // DrawCircleV(projectToCamera({40, 40}), 4, {255,0,0,255});
            auto curr = gamestate.currentLevel.MapPoints[i];
            DrawCircleV(projectToCamera(curr), 4, {0, 255, 0, 255});
            for (auto it2 = curr.connections.begin(); it2 != curr.connections.end(); it2++) {
                // std::cout << *it2 << std::endl;
                DrawLineV(projectToCamera(curr), projectToCamera(gamestate.currentLevel.MapPoints[*it2]), {0, 200, 100, 250});
            }
            snprintf(buffer, 32, "%d", i);
            DrawText(buffer, projectToCamera(curr).x+5, projectToCamera(curr).y+5, 25, {0, 150, 200, 250});
        }
    }

    if (gamestate.currentLevel.player && gamestate.currentLevel.player->selfitem!=-1) {
        Item* git = (Item*) gamestate.currentLevel.objects[gamestate.currentLevel.player->selfitem];
        switch (git->subgentype) {
        case FIREARM: {
            Firearm* it = (Firearm*) git;
            snprintf(buffer, 32, "%d/%d", it->rounds, it->extrarounds);
            DrawText(buffer, gamestate.WinX - 6*20 - 5, gamestate.WinY-40, 30, GREEN);
            float a = ((float)it->delay_tick)/TICK/it->delay;
            float linesize = 120;
            DrawLine(gamestate.WinX - 40 - linesize, gamestate.WinY-60, gamestate.WinX - 40 - linesize*a, gamestate.WinY-60, GREEN); 
            if (it->reloading) {
                DrawText("RELOADING", gamestate.WinX - 180, gamestate.WinY-100, 30, GREEN);
            }
            // std::cout << it->delay_tick << std::endl;
            break;
        }
        
        default:
            break;
        }
    }
    if (gamestate.currentLevel.player && !gamestate.currentLevel.player->alive) {
        DrawText("RESTART - R", gamestate.WinXf/2 - 6*50, gamestate.WinYf/2 - 20, 80, GREEN);
    } 
    else if (gamestate.levelComplete) {
        DrawText("LEVEL COMPLETED", gamestate.WinXf/2 - 7*50, gamestate.WinYf/2 - 40, 80, GREEN);
        DrawText("NEXT LEVEL - N", gamestate.WinXf/2 - 6*50, gamestate.WinYf/2 + 20, 80, GREEN);
    }
    if (gamestate.pause) {
        DrawText("PAUSED", gamestate.WinX - 6*20 - 5, 0, 30, GREEN);
    }

    DrawText(gamestate.currentLevel.name.c_str(), 10, gamestate.WinY-30, 20, RED);

    // std::cout << "d3" << std::endl;

    // EndDrawing();
    // SwapScreenBuffer();
}