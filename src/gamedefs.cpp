#include "game.h"

size_t id_counter = 0;
std::list<Object*> Gobjects;
int WinX = 800;
int WinY = 600;
float WinXf = WinX; 
float WinYf = WinY;
int MAX_REFLECTIONS = 10;
float hview = 30;
int Nray = 150;
float delta = 2*hview/Nray;
std::vector<IntersectInfo> inters(Nray);