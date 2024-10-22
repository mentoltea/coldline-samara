#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include "game.h"
#include "fstream"

void InitializeObject(Object* obj);

void FinishLoadObject(std::ifstream &fd, Object* obj);

Level LoadLevel(std::string filename);

void SaveObject(std::ofstream &fd, Object* obj);

void SaveLevel(Level& level, std::string filename);

void UnloadLevel(Level& level);

void ReloadLevel();

#endif