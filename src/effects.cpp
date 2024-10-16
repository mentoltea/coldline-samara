#include "effects.h"

Effect_Glassbreak::Effect_Glassbreak(Point start, Vector2 direction, int count) {
    this->start = start;
    this->direction = direction;
    this->count = count;
    this->debris = std::vector<Point>(this->count);

    speed = sqrtf(direction.x*direction.x + direction.y*direction.y);

    float anglebetween = PI/3/count;
    float angle = atan2f(direction.y, direction.x);
    float deviation = 10; // in degrees

    for (int i=0; i<count; i++) {
        float curangle = angle + anglebetween*((float)i - (float)(count-1)/2.f);
        curangle += deviation * (2*randf() - 1) * PI / 180;
        Vector2 curdir = {cosf(curangle), sinf(curangle)};

        debris[i] = curdir;
    }

    length = 0.75;
    tick=1;
}
Effect_Glassbreak::~Effect_Glassbreak() = default;
void Effect_Glassbreak::update() {
    // float t = (float)tick / ((float)TICK*length);
    // for (int i=0; i<count; i++) {
    //     Vector2 dir = debris[i];
    //     float dl = sqrtf(dir.x*dir.x + dir.y*dir.y);
    //     if (dl>0) {
    //         dir = dir*t/dl;
    //     }
    //     debris[i] = dir;
    // }
    tick++;
}
void Effect_Glassbreak::draw() {
    float t = (float)tick / ((float)TICK*length);
    for (int i=0; i<count; i++) {
        // std::cout << i << std::endl;
        Vector2 dir = debris[i];
        float dl = sqrtf(dir.x*dir.x + dir.y*dir.y);
        Point pos = start + dir*t*speed;

        Point p1 = projectToCamera( pos + (Vector2){1,-1}  );
        Point p2 = projectToCamera( pos + (Vector2){-1,-1} );
        Point p3 = projectToCamera( pos + (Vector2){1,1}   );
        Point p4 = projectToCamera( pos + (Vector2){-1,1}  );

        float alfa;
        if (dl>0) alfa = 1/dl/dl;
        else {
            alfa = 1;
            // std::cout << 0 << std::endl;
        }
        alfa *= 255;
        if (alfa>255) alfa = 255;

        Color col = selfcolor;
        col.a = alfa;

        DrawTriangle(p1, p2, p3, col );
        DrawTriangle(p3, p2, p4, col );

        // col.a = 255;
        // DrawCircleV(p1, 3, col);
        // DrawCircleV(p2, 4, col);
        // DrawCircleV(p3, 5, col);
        // DrawCircleV(p4, 6, col);
    }
}
bool Effect_Glassbreak::finished() {
    return (tick>TICK*length);
}