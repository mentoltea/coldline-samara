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
    case PISTOL: return sizeof(Pistol);
    case RIFLE: return sizeof(Rifle);
    case SHOTGUN: return sizeof(Shotgun);
    
    default:
        fprintf(stderr, "ERROR : Cannot figure out the size of object.\n");
        assert(0 && "UNKNOWN SIZE");
        break;
    }
}

#define CopyCaseDereferenceCast(Tenum, T, TO, FROM) case Tenum:\
    *(size_t*)TO = *(size_t*)& MacroExample(T);\
    *(T*)(TO) = *(T*)(FROM);

void CopyObject(Object* to, Object* from) {
    switch (from->type){
    CopyCaseDereferenceCast(WALL, Wall, to, from); break;
    CopyCaseDereferenceCast(TEXTSEGMENT, TextSegment, to, from); break;
    CopyCaseDereferenceCast(MIRROR, Mirror, to, from); break;
    CopyCaseDereferenceCast(DOOR, Door, to, from); break;
    CopyCaseDereferenceCast(PLAYER, Player, to, from); break;
    CopyCaseDereferenceCast(ENEMY, Enemy, to, from); break;
    CopyCaseDereferenceCast(PISTOL, Pistol, to, from); break;
    CopyCaseDereferenceCast(RIFLE, Rifle, to, from); break;
    CopyCaseDereferenceCast(SHOTGUN, Shotgun, to, from); break;

    default:
        fprintf(stderr, "COPY ERROR : unknown type.\n");
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
        // memcpy((void*)temp, *it, ObjectSize(*it));
        CopyObject(temp, *it);
        objects.push_back(temp);
        if (temp->type == PLAYER) player = (Player*) temp;
    }
    MapPoints = other.MapPoints;
}
Level::~Level() {
    clear();
}

Level& Level::operator=(const Level& other) {
    if (this != &other) {
        clear();
        MapX = other.MapX;
        MapY = other.MapY;
        MapXf = other.MapXf;
        MapYf = other.MapYf;
        cheats = other.cheats;
        // std::cout << "for start" << std::endl;
        for (auto it = other.objects.cbegin(); it != other.objects.end(); it++) {
            Object* temp = (Object*)MemManager::memloc(ObjectSize(*it));
            // memcpy((void*)temp, *it, ObjectSize(*it));
            CopyObject(temp, *it);
            objects.push_back(temp);
            if (temp->type == PLAYER) player = (Player*) temp;
        }
        // std::cout << "for end" << std::endl;
        MapPoints = other.MapPoints;
    }
    return *this;
}
// Level& Level::operator=(Level&& other) {
//     if (this != &other) {
//         clear();
//         MapX = other.MapX;
//         MapY = other.MapY;
//         MapXf = other.MapXf;
//         MapYf = other.MapYf;
//         cheats = other.cheats;
//         objects = other.objects;
//         player = other.player;
//         other.objects.clear();
//         MapPoints = other.MapPoints;
//     }
//     return *this;
// }

void Level::clear() {
    for (auto it=objects.begin(); it!=objects.end(); it++) {
        MemManager::memfree(*it);
        // DELETE(Object, *it);
    }
    objects.clear();
    for (auto it=projects.begin(); it!=projects.end(); it++) {
        DELETE(Projectile, (*it));
        // DELETE(Object, *it);
    }
    projects.clear();
    MapPoints.clear();
    player = NULL;
}

void Level::destroy() {
    for (auto it=objects.begin(); it!=objects.end(); it++) {
        DELETE(Object, *it);
    }
    objects.clear();
    for (auto it=projects.begin(); it!=projects.end(); it++) {
        DELETE(Projectile, (*it));
        // DELETE(Object, *it);
    }
    projects.clear();
    MapPoints.clear();
    player = NULL;
}


std::tuple<Point, int, float> Level::nearPoint(const Point& p) const {
    int idx = -1;
    float mindist = INFINITY;
    int size = MapPoints.size();
    for (int i=0; i<size; i++) {
        float dx = MapPoints[i].x - p.x;
        float dy = MapPoints[i].y - p.y;
        float dist = dx*dx + dy*dy;
        if (dist < mindist) {
            idx = i;
            mindist = dist;
        }
    }
    if (idx==-1) return {{0}, -1, mindist};
    return {MapPoints[idx], idx, mindist};
}


std::stack<int > Level::way(int fromIdx, int toIdx) const {
    // std::cout << "in0\n";

    std::queue<int> Q;
    std::vector<int> visited(MapPoints.size(), -1);
    Q.push(fromIdx);
    visited[fromIdx] = 0;
    int curr;

    // std::cout << "in1\n";

    while (!Q.empty()) {
        curr = Q.front();
        Q.pop();
        if (curr == toIdx) break;
        for (int i=0; i<(int)MapPoints[curr].connections.size(); i++) {
            int near = MapPoints[curr].connections[i];
            if (visited[near] == -1) {
                Q.push(near);
                visited[near] = visited[curr]+1;
            } else if (visited[curr]+1 < visited[near]) {
                visited[near] = visited[curr]+1;
            }
        }
    }
    // std::cout << "in2\n";
    std::stack<int> S;
    if (curr != toIdx) {
        S.push(fromIdx);
        return S;
    }
    S.push(toIdx);
    // std::cout << "in3\n";
    int revcurr = curr;
    // std::cout << fromIdx << " " << toIdx << " " <<  revcurr << std::endl;
    while (revcurr != fromIdx) {
        int min = MapPoints.size() + 1;
        int minidx = -1;
        for (int i=0; i<(int)MapPoints[revcurr].connections.size(); i++) {
            int near = MapPoints[revcurr].connections[i];
            if (visited[near]!=-1 && visited[near] < min) {
                minidx = near;
                min = visited[near];
            } 
        }
        S.push(minidx);
        revcurr = minidx;
    }
    // std::cout << "in4\n";

    auto copy = S;
    // std::cout << "-----" << std::endl;
    // while (!copy.empty()) {
    //     std::cout << copy.top() << " ";
    //     copy.pop();
    // }
    // std::cout << std::endl;
    
    return S;
}