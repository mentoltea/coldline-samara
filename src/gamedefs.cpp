#include "game.h"

size_t id_counter = 0;
std::list<Object*> Gobjects;
bool fullscreen = 0;
int WinX = 16*80;
int WinY = 9*80;
float WinXf = WinX; 
float WinYf = WinY;
int MapX = 1000;
int MapY = 800;
float MapXf = MapX; 
float MapYf = MapY;
int MAX_REFLECTIONS = 20;
Player* Gplayer = NULL;
bool pause = false;
Point camera;
