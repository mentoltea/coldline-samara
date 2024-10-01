#include "game.h"
#include "fstream"

void InitializeObject(Object* obj);

Level LoadLevel(std::string filename);

void SaveLevel(Level& level, std::string filename);

void UnloadLevel(Level& level);

void ReloadLevel();