#include "game.h"

size_t id_counter = 0;
std::list<Object*> Gobjects;
bool fullscreen = false;
int WinX = 16*80;
int WinY = 9*80;
float WinXf = WinX; 
float WinYf = WinY;
int MapX = 800;
int MapY = 600;
float MapXf = MapX; 
float MapYf = MapY;
int MAX_REFLECTIONS = 20;
Player* Gplayer = NULL;
bool pause = false;
Point camera;