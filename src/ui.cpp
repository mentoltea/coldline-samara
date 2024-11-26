#include "ui.h"
using namespace UI;

Element::Element(Point position, Point box) {
    this->position = position;
    this->box = box;
}

Button::Button(Point position, Point box, void (*funcptr)(int), std::string text, Color textcolor, Color backcolor): Element(position, box) {
    this->func = funcptr;
    this->text = text;
    this->textcolor = textcolor;
    this->background = backcolor;
}

void Button::update()  {
    clicked = false;
    return;
}
void Button::click() {
    if (func) func(clickcount);
    clickcount++;
    clicked = true;
}
void Button::draw(int winx, int winy) {
    Vector2 screensize = {box.x * winx, box.y * winy};
    Point screenpos = {position.x * winx, position.y * winy};

    if (!clicked) DrawRectangleV(screenpos, screensize, background);
    else DrawRectangleV(screenpos, screensize, { (uint8_t)(255-background.r), (uint8_t)(255-background.g), (uint8_t)(255-background.b), background.a});

    Point textpos = screenpos + (Vector2){screensize.x * padding.x, screensize.y * padding.y};    
    float fontsize = screensize.y * (1 - 2*padding.y) / 2;
    if (fontsize < 2) fontsize = 2;
    if (!clicked) DrawTextEx(font, text.c_str(), textpos, fontsize, fontsize/2, textcolor);
    else DrawTextEx(font, text.c_str(), textpos, fontsize, fontsize/2, 
        { (uint8_t)(255-textcolor.r), (uint8_t)(255-textcolor.g), (uint8_t)(255-textcolor.b), textcolor.a});
}