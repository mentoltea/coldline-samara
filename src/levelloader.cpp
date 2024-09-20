#include "levelloader.h"

namespace ObjectExamples {
// THESE EXAMPLES ARE ONLY TO GET VPTR TO VTABLE
Wall WallExample;
Door DoorExample;
Mirror MirrorExample;
Enemy EnemyExample;
Player PlayerExample;
}

void InitializeObject(Object* obj) {
    switch (obj->type) {
    case WALL: {
        *(size_t*)obj = *(size_t*)&ObjectExamples::WallExample;
        // nothing special, just values
    }
    break;
    case MIRROR: {
        *(size_t*)obj = *(size_t*)&ObjectExamples::MirrorExample;
        // nothing special, just values
    }
    break;
    case DOOR: {
        *(size_t*)obj = *(size_t*)&ObjectExamples::DoorExample;
        // nothing special, just values
    }
    break;
    case PLAYER: {
        *(size_t*)obj = *(size_t*)&ObjectExamples::PlayerExample;
        Player *p = (Player*)obj;
        p->inters = *(new(&(p->inters)) std::vector<IntersectInfo>(p->Nray));
        for (int i=0; i<p->Nray; i++) {
            p->inters[i].points = *(new(&p->inters[i].points) std::vector<Point>);
        }
        p->intersBack = *(new(&(p->intersBack)) std::vector<IntersectInfo>(p->Nrayback));
        for (int i=0; i<p->Nrayback; i++) {
            p->intersBack[i].points = *(new(&p->intersBack[i].points) std::vector<Point>);
        }
        gamestate.Gplayer = p;
    }
    break;
    case ENEMY: {
        *(size_t*)obj = *(size_t*)&ObjectExamples::EnemyExample;
        Enemy *e = (Enemy*)obj;
        e->inters = *(new(&(e->inters)) std::vector<IntersectInfo>(e->Nray));
        for (int i=0; i<e->Nray; i++) {
            e->inters[i].points = *(new(&e->inters[i].points) std::vector<Point>);
        }
    }
    break;

    default:
        fprintf(stderr, "ERROR [READ]: Cannot figure out how to initialize the object.\n");
        assert(0 && "CANNOT INITIALIZE");
        break;
    }
}

size_t ObjectSize(Object* obj) {
    switch (obj->type) {
    case WALL: return sizeof(Wall);
    case MIRROR: return sizeof(Mirror);
    case DOOR: return sizeof(Door);
    case PLAYER: return sizeof(Player);
    case ENEMY: return sizeof(Enemy);
    
    default:
        fprintf(stderr, "ERROR [WRITE]: Cannot figure out the size of object.\n");
        break;
    }

    return MAX_OBJECT_SIZE;
}

void LoadLevel(std::string filename) {
    std::ifstream fd(filename, std::ios::binary);
    size_t length = 0;
    size_t maxsize = MAX_OBJECT_SIZE;
    fd.read((char*)&gamestate.MapX, sizeof(int));
    fd.read((char*)&gamestate.MapY, sizeof(int));
    gamestate.MapXf = gamestate.MapX;
    gamestate.MapYf = gamestate.MapY;
    fd.read((char*)&length, sizeof(size_t));
    size_t idx = 0;
    uint8_t *buffer = (uint8_t*)MemManager::memloc(maxsize*length);
    while (!fd.eof() && idx<length) {
        fd.read((char*)(buffer + idx*maxsize), maxsize);
        InitializeObject((Object*)(buffer + idx*maxsize));
        gamestate.GlevelReference.push_back((Object*)(buffer + idx*maxsize));
        idx++;
    }
}

void SaveLevel(std::string filename) {
    std::ofstream fd(filename, std::ios::binary);
    size_t length = gamestate.GlevelReference.size();
    size_t maxsize = MAX_OBJECT_SIZE;
    fd.write((char*)&gamestate.MapX, sizeof(int));
    fd.write((char*)&gamestate.MapY, sizeof(int));
    fd.write((char*)(&length), sizeof(size_t));
    size_t currsize;
    auto it = gamestate.GlevelReference.begin();
    while (it != gamestate.GlevelReference.end()) {
        currsize = ObjectSize(*it);
        fd.write((char*)*it, currsize);
        for (size_t i=0; i<maxsize-currsize; i++) fd.put(0);
        it++;
    }
}



void ReloadLevel() {
    auto git = gamestate.Gobjects.begin();
    auto it = gamestate.GlevelReference.begin();
    Object *temp;
    for (; it != gamestate.GlevelReference.end() && git != gamestate.Gobjects.end(); it++) {
        MemManager::memfree(*git);
        temp = (Object*)MemManager::memloc(ObjectSize(*it));
        memcpy((void*)temp, *it, ObjectSize(*it));
        *git = temp;
        if ((*git)->type == PLAYER) gamestate.Gplayer = (Player*) *git;
        git++;
    }
    // std::cout << "Step1" << std::endl;
    for (; git != gamestate.Gobjects.end();) {
        MemManager::memfree(*git);
        git = gamestate.Gobjects.erase(git);
    }
    // std::cout << "Step2" << std::endl;
    while (it != gamestate.GlevelReference.end()) {
        temp = (Object*)MemManager::memloc(ObjectSize(*it));
        memcpy((void*)temp, *it, ObjectSize(*it));
        gamestate.Gobjects.push_back(temp);
        if (temp->type == PLAYER) gamestate.Gplayer = (Player*) temp;
        it++;
    }
    // std::cout << "Step3" << std::endl;
}

void UnloadLevel() {

}