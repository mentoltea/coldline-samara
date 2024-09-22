#ifndef CPPMemManager
#define CPPMemManager

namespace MemManager {
extern "C" {
#include "memmanager.h"
}

template <typename T>
class Allocator {
public:
    typedef T value_type;
    Allocator() noexcept {}
    Allocator(void* ptr) noexcept {
    }
    Allocator(const size_t sizeBytes, void* const start) noexcept {
        prealloc(sizeBytes);
    }
    template <class U>
    Allocator(const Allocator<U>& other) {
    };

    ~Allocator() = default;

    T* allocate(size_t n) {
        return (T*)memloc(n * sizeof(T));
    }
    void deallocate(T* p, size_t n) noexcept {
        memfree(p);
    }
};

template<class T, class U>
bool operator==(const Allocator<T>&, const Allocator<U>&) {
    return true;
}
template<class T, class U>
bool operator!=(const Allocator<T>&, const Allocator<U>&) {
    return false;
}

}

#endif