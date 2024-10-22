#include "levelloader.h"

#define FD_WRITE(fd, P, S) fd.write((char*)P, S) 
#define FD_AUTOWRITE(fd, O) fd.write((char*)&O, sizeof(O))

#define FD_READ(fd, P, S) fd.read((char*)P, S)
#define FD_AUTOREAD(fd, O) fd.read((char*)&O, sizeof(O))

#define VTABLECASECAST(Tenum, T, TO) case Tenum:\
    *(size_t*)TO = *(size_t*)& MacroExample(T);

void InitializeObject(Object* obj) {
    switch (obj->type) {
    VTABLECASECAST(WALL, Wall, obj) {} break;
    VTABLECASECAST(MIRROR, Mirror, obj) {} break;
    VTABLECASECAST(DOOR, Door, obj) {} break;
    VTABLECASECAST(TEXTSEGMENT, TextSegment, obj) {} break;

    VTABLECASECAST(PLAYER, Player, obj) {
        Player *p = (Player*)obj;
        p->inters = *(new(&(p->inters)) std::vector<IntersectInfo>(p->Nray));
        for (int i=0; i<p->Nray; i++) {
            p->inters[i].points = *(new(&p->inters[i].points) std::vector<Point>);
        }
        p->intersBack = *(new(&(p->intersBack)) std::vector<IntersectInfo>(p->Nrayback));
        for (int i=0; i<p->Nrayback; i++) {
            p->intersBack[i].points = *(new(&p->intersBack[i].points) std::vector<Point>);
        }
    }
    break;

    VTABLECASECAST(ENEMY, Enemy, obj) {
        Enemy *e = (Enemy*)obj;
        e->inters = *(new(&(e->inters)) std::vector<IntersectInfo>(e->Nray));
        for (int i=0; i<e->Nray; i++) {
            e->inters[i].points = *(new(&e->inters[i].points) std::vector<Point>);
        }
        e->behaviour.selfway = *(new(&(e->behaviour.selfway)) std::vector<int>);
        e->behaviour.currentway = *(new(&(e->behaviour.currentway)) std::stack<int>);
        e->behaviour.near = *(new(&(e->behaviour.near)) std::tuple<Point, int, float>);
    }
    break;

    VTABLECASECAST(PISTOL, Pistol, obj) {} break;
    VTABLECASECAST(RIFLE, Rifle, obj) {} break;
    VTABLECASECAST(SHOTGUN, Shotgun, obj) {} break;

    default:
        fprintf(stderr, "ERROR [READ]: Cannot figure out how to initialize the object %d.\n", obj->type);
        assert(0 && "CANNOT INITIALIZE OBJECT");
        break;
    }
}

void FinishLoadObject(std::ifstream &fd, Object* obj) {
    switch (obj->gentype) {
    case OBTACLE:
        break;

    case ITEM:
        break;

    case ENTITY: {
        switch (obj->type) {
            case PLAYER:
                break;
            
            case ENEMY:{
                Enemy *en = (Enemy*) obj;
                size_t SW_size;
                FD_AUTOREAD(fd, SW_size);
                for (size_t i=0; i<SW_size; i++) {
                    int curr;
                    FD_AUTOREAD(fd, curr);
                    en->behaviour.selfway.push_back(curr);
                }
            }    break;

            default:
                assert(0&&"Unreachable");    
        }
    }   break;
    
    
    default:
        assert(0&&"Unknown object cant be read");
    }
}

Level LoadLevel(std::string filename) {
    Level level;
    std::ifstream fd(filename, std::ios::binary);

    // Byte-to-byte
    FD_AUTOREAD(fd, level.MapX);
    FD_AUTOREAD(fd, level.MapY);
    FD_AUTOREAD(fd, level.cheats);
    level.MapXf = level.MapX;
    level.MapYf = level.MapY;

    // Special

    // MapPoints
    size_t MP_size;
    FD_AUTOREAD(fd, MP_size);
    for (size_t i=0; i<MP_size; i++) {
        ConnectedPoint temp;

        FD_AUTOREAD(fd, temp.x);
        FD_AUTOREAD(fd, temp.y);

        size_t CP_size;
        FD_AUTOREAD(fd, CP_size);
        for (size_t conn=0; conn<CP_size; conn++) {
            int curr;
            FD_AUTOREAD(fd, curr);
            temp.connections.push_back(curr);
        }

        level.MapPoints.push_back(temp);
    }

    // Objects
    size_t OB_size;
    FD_AUTOREAD(fd, OB_size);
    for (size_t i=0; i<OB_size; i++) {
        size_t objs;
        FD_AUTOREAD(fd, objs);
        Object* obj = (Object*)MemManager::memloc(objs);
        FD_READ(fd, obj, objs);
        assert(objs == ObjectSize(obj) && "Level of incompatible version");

        InitializeObject(obj);
        FinishLoadObject(fd, obj);

        level.objects.push_back(obj);
    }

    fd.close();
    return level;
}

void SaveObject(std::ofstream &fd, Object* obj) {
    size_t objs = ObjectSize(obj);
    FD_AUTOWRITE(fd, objs);
    FD_WRITE(fd, obj, objs);
    
    switch (obj->gentype) {
    case OBTACLE:
        break;

    case ITEM:
        break;

    case ENTITY: {
        switch (obj->type) {
            case PLAYER:
                break;
            
            case ENEMY:{
                Enemy* en = (Enemy*)obj;
                size_t SW_size = en->behaviour.selfway.size();
                FD_AUTOWRITE(fd, SW_size);
                for (size_t i=0; i<SW_size; i++) {
                    int curr = en->behaviour.selfway[i];
                    FD_AUTOWRITE(fd, curr);
                }
            }    break;

            default:
                assert(0&&"Unreachable");    
        }
    }   break;
    
    
    default:
        assert(0&&"Unknown object cant be saved");
    }
}


void SaveLevel(Level& level, std::string filename) {
    std::ofstream fd(filename, std::ios::binary);
    
    // Byte-to-byte
    FD_AUTOWRITE(fd, level.MapX);
    FD_AUTOWRITE(fd, level.MapY);
    FD_AUTOWRITE(fd, level.cheats);


    // Special

    // MapPoints
    size_t MP_size = level.MapPoints.size();
    FD_AUTOWRITE(fd, MP_size);
    for (size_t point=0; point<MP_size; point++) {
        ConnectedPoint &temp = level.MapPoints[point];

        FD_AUTOWRITE(fd, temp.x);
        FD_AUTOWRITE(fd, temp.y);

        size_t CP_size = temp.connections.size();
        FD_AUTOWRITE(fd, CP_size);
        for (size_t conn=0; conn<CP_size; conn++) {
            int c = temp.connections[conn];
            FD_AUTOWRITE(fd, c);
        }
    }

    // Objects
    size_t OB_size = level.objects.size();
    FD_AUTOWRITE(fd, OB_size);
    for (size_t i=0; i<OB_size; i++) {
        Object* current = level.objects[i];
        SaveObject(fd, current);
    }

    fd.close();
}

void UnloadLevel(Level& level) {
    level.clear();
}

void ReloadLevel() {
    gamestate.currentLevel = gamestate.levelReference;
    gamestate.levelComplete = false;
}