#include "definitions.h"
#include "game.h"

size_t ObjectSize(Object* obj) {
    switch (obj->type) {
    case WALL: return sizeof(Wall);
    case MIRROR: return sizeof(Mirror);
    case DOOR: return sizeof(Door);
    case PLAYER: return sizeof(Player);
    case ENEMY: return sizeof(Enemy);
    case TEXTSEGMENT: return sizeof(TextSegment);
    
    default:
        fprintf(stderr, "ERROR : Cannot figure out the size of object.\n");
        assert(0 && "UNKNOWN SIZE");
        break;
    }
}


Level::Level() = default;

Level::Level(const Level& other) {
    MapX = other.MapX;
    MapY = other.MapY;
    MapXf = other.MapXf;
    MapYf = other.MapYf;
    cheats = other.cheats;
    for (auto it = other.objects.cbegin(); it != other.objects.end(); it++) {
        Object* temp = (Object*)MemManager::memloc(ObjectSize(*it));
        memcpy((void*)temp, *it, ObjectSize(*it));
        objects.push_back(temp);
        if (temp->type == PLAYER) player = (Player*) temp;
    }
}
Level::~Level() {
    clear();
}

Level& Level::operator=(const Level& other) {
    clear();
    MapX = other.MapX;
    MapY = other.MapY;
    MapXf = other.MapXf;
    MapYf = other.MapYf;
    cheats = other.cheats;
    for (auto it = other.objects.cbegin(); it != other.objects.end(); it++) {
        Object* temp = (Object*)MemManager::memloc(ObjectSize(*it));
        memcpy((void*)temp, *it, ObjectSize(*it));
        objects.push_back(temp);
        if (temp->type == PLAYER) player = (Player*) temp;
    }
    return *this;
}
Level& Level::operator=(Level&& other) {
    clear();
    MapX = other.MapX;
    MapY = other.MapY;
    MapXf = other.MapXf;
    MapYf = other.MapYf;
    cheats = other.cheats;
    objects = other.objects;
    player = other.player;
    other.objects.clear();
    return *this;
}

void Level::clear() {
    for (auto it=objects.begin(); it!=objects.end(); it++) {
        MemManager::memfree(*it);
    }
    objects.clear();
    player = NULL;
}

void Level::destroy() {
    for (auto it=objects.begin(); it!=objects.end(); it++) {
        DELETE(Object, *it);
    }
    objects.clear();
    player = NULL;
}