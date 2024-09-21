#ifndef TextureManagerH
#define TextureManagerH

#include <raylib.h>
#include <unordered_map>
#include <cstdlib>
#include <iostream>
#include "MManager.hpp"

namespace TextureManager {

void LoadT(const char* filename, int id);
Texture *GetT(int id);
Texture *GetOrLoadT(const char* filename, int id);
void UnloadT();



// std::unordered_map<int, Texture> IHolder; // Images

// void LoadI(const char* filename, int id);
// Texture *GetI(int id);
// Texture *GetOrLoadI(int id);
// void UnloadI();

}


#endif