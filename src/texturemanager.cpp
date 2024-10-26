#include "texturemanager.h"
namespace TextureManager {

// std::unordered_map<int,
//     Texture,
//     std::hash<int>,
//     std::equal_to<int>,
//     MemManager::Allocator< std::pair< int, Texture > >
//     > THolder; // Textures 
std::unordered_map<int, Texture> THolder;

void LoadT(const char* filename, int id, bool force) {
    if (!FileExists(filename)) return;
    if (!force && THolder.count(id) != 0) return;
    Texture T = LoadTexture(filename);
    THolder.insert({id, T});
}
void LoadTfromI(Image I, int id, bool force) {
    if (!force && THolder.count(id) != 0) return;
    Texture T = LoadTextureFromImage(I);
    THolder.insert({id, T});
}
Texture *GetT(int id) {
    auto it = THolder.find(id);
    if (it == THolder.end()) return NULL;
    return &(it->second);
}
Texture *GetOrLoadT(const char* filename, int id) {
    auto it = THolder.find(id);
    if (it == THolder.end()) {
        if (!FileExists(filename)) return &(THolder.begin()->second);
        LoadT(filename, id, true);
        return GetT(id);
    }
    return &(it->second);
}
void UnloadT() {
    for (auto it = THolder.begin(); it != THolder.end(); it++) {
        UnloadTexture(it->second);
    }
    THolder.clear();
}

}