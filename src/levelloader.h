#include "game.h"
#include "fstream"

void InitializeObject(Object* obj);

size_t ObjectSize(Object* obj);

void LoadLevel(std::string filename);

void SaveLevel(std::string filename);

void ReloadLevel();

void UnloadLevel();