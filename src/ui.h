#ifndef UI_H
#define UI_H

#include "definitions.h"
namespace UI {

class Element {
public:
    Point position; // 0 to 1
    Vector2 box; // 0 to 1

    bool active = true;
    bool visible = true;

    Element(Point position, Point box);

    virtual void update() = 0;
    virtual void click() = 0;
    virtual void draw(int winx, int winy) = 0;
};

class Button: public Element {
public:
    void (*func)(int) = NULL;
    Color background;
    std::string text;
    Color textcolor;
    int clickcount = 0;
    Vector2 padding = {0.1, 0.1};
    Font font = GetFontDefault();
    bool clicked = true;

    Button(Point position, Point box, void (*funcptr)(int), std::string text, Color textcolor, Color backcolor);

    void update() override;
    void click() override;
    void draw(int winx, int winy) override;
};




}
#endif // UI_H