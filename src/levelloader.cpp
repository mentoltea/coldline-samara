#include "levelloader.h"

void InitializeObject(Object* obj) {
    switch (obj->type) {
    case WALL: {
        *(size_t*)obj = *(size_t*)& MacroExample(Wall);
        // nothing special, just values
    }
    break;
    case TEXTSEGMENT: {
        *(size_t*)obj = *(size_t*)& MacroExample(TextSegment);
        // nothing special, just values
    }
    break;
    case DOOR: {
        *(size_t*)obj = *(size_t*)& MacroExample(Door);
        // nothing special, just values
    }
    break;
    case MIRROR: {
        *(size_t*)obj = *(size_t*)& MacroExample(Mirror);
        // nothing special, just values
    }
    break;
    case PLAYER: {
        *(size_t*)obj = *(size_t*)& MacroExample(Player);
        Player *p = (Player*)obj;
        p->inters = *(new(&(p->inters)) std::vector<IntersectInfo>(p->Nray));
        for (int i=0; i<p->Nray; i++) {
            p->inters[i].points = *(new(&p->inters[i].points) std::vector<Point>);
        }
        p->intersBack = *(new(&(p->intersBack)) std::vector<IntersectInfo>(p->Nrayback));
        for (int i=0; i<p->Nrayback; i++) {
            p->intersBack[i].points = *(new(&p->intersBack[i].points) std::vector<Point>);
        }
        // gamestate.Gplayer = p;
    }
    break;
    case ENEMY: {
        *(size_t*)obj = *(size_t*)& MacroExample(Enemy);
        Enemy *e = (Enemy*)obj;
        e->inters = *(new(&(e->inters)) std::vector<IntersectInfo>(e->Nray));
        for (int i=0; i<e->Nray; i++) {
            e->inters[i].points = *(new(&e->inters[i].points) std::vector<Point>);
        }
    }
    break;

    default:
        fprintf(stderr, "ERROR [READ]: Cannot figure out how to initialize the object %d.\n", obj->type);
        assert(0 && "CANNOT INITIALIZE OBJECT");
        break;
    }
}

Level LoadLevel(std::string filename) {
    Level level;
    std::ifstream fd(filename, std::ios::binary);
    fd.read((char*)&level.MapX, sizeof(int));
    fd.read((char*)&level.MapY, sizeof(int));
    level.MapXf = level.MapX;
    level.MapYf = level.MapY;
    size_t length = 0;
    fd.read((char*)&length, sizeof(size_t));
    size_t idx = 0;
    while (!fd.eof() && idx<length) {
        size_t objsize;
        fd.read((char*)&objsize, sizeof(size_t));
        Object *buffer = (Object*)MemManager::memloc(objsize);
        fd.read((char*)buffer, objsize);
        assert((objsize == ObjectSize(buffer)) && "Level of incompatible version");
        InitializeObject(buffer);
        level.objects.push_back(buffer);
        idx++;
    }
    fd.close();
    gamestate.currentLevel = level;
    return level;
}

void SaveLevel(Level& level, std::string filename) {
    std::ofstream fd(filename, std::ios::binary);
    fd.write((char*)&level.MapX, sizeof(int));
    fd.write((char*)&level.MapY, sizeof(int));
    size_t length = level.objects.size();
    fd.write((char*)(&length), sizeof(size_t));
    for (auto it = level.objects.begin(); it != level.objects.end(); it++) {
        size_t objsize = ObjectSize(*it);
        fd.write((char*)&objsize, sizeof(size_t));
        fd.write((char*)*it, objsize);               
    }
    fd.close();
}

void UnloadLevel(Level& level) {
    level.clear();
}

void ReloadLevel() {
    // std::cout << "before" << std::endl;
    gamestate.currentLevel = gamestate.levelReference;
    gamestate.levelComplete = false;
    // std::cout << "after" << std::endl;
}