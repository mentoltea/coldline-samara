#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include "game.h"
#include "fstream"

bool InitializeObject(Object* obj);

bool FinishLoadObject(std::ifstream &fd, Object* obj);

Level LoadLevel(std::string filename);

bool SaveObject(std::ofstream &fd, Object* obj);

bool SaveLevel(Level& level, std::string filename);

void UnloadLevel(Level& level);

void DestroyLevel(Level& level);

void UDLevel();

void ReloadLevel();

#endif