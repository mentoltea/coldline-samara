#ifndef EFFECTS_H
#define EFFECTS_H

#include "definitions.h"
#include "game.h"

class Effect {
public:
    int tick=0;

    virtual ~Effect() = default;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual bool finished() = 0;
};


class Effect_Glassbreak: public Effect {
public:
    float length;
    float speed;
    Point start;
    Vector2 direction;
    int count;
    std::vector<Vector2> debris;
    Color selfcolor = {213,245,242, 160};

    Effect_Glassbreak(Point start, Vector2 direction, int count);
    ~Effect_Glassbreak() override;
    void update() override;
    void draw() override;
    bool finished() override;
};


#endif