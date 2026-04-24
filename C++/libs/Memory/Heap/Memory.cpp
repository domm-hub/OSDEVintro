#include "Memory.h"
#include "Heap.h"

void memset(void* start, uint_64 value, uint_64 num){
    if (num < 8){
        uint_8* valPTR = (uint_8*)&value;
        for (uint_8* ptr = (uint_8*)start; ptr < (uint_8*)((uint_64)start+num); ptr++){
            *ptr = *valPTR;
        }
        return;
    }

    uint_64 remainder = num % 8;
    uint_64 newNum = num - remainder;

    for (uint_64* ptr = (uint_64*)start; ptr < (uint_64*)((uint_64)start+newNum); ptr++){
        *ptr = value;
    }

    uint_8* valPtr = (uint_8*)&value;
    for (uint_8* ptr = (uint_8*)((uint_64)start+newNum); ptr < (uint_8*)((uint_64)start+num); ptr++){
        *ptr = *valPtr;
    }
}


void memcpy(void* destination, void* source, uint_64 num){
    if (num < 8){
        uint_8* srcptr = (uint_8*)source;
        uint_8* destptr = (uint_8*)destination;
        for (uint_64 i = 0; i < num; i++){
            destptr[i] = srcptr[i];
        }
        return;
    }

    uint_64 remainder = num % 8;
    uint_64 newNum = num - remainder;
    uint_64* srcptr64 = (uint_64*)source;
    uint_64* destptr64 = (uint_64*)destination;

    for (uint_64 i = 0; i < (newNum / 8); i++){
        destptr64[i] = srcptr64[i];
    }

    uint_8* srcptr8 = (uint_8*)((uint_64)source + newNum);
    uint_8* destptr8 = (uint_8*)((uint_64)destination + newNum);
    for (uint_64 i = 0; i < remainder; i++){
        destptr8[i] = srcptr8[i];
    }
}

// Standard C++ operators
void* operator new(size_t size) {
    return malloc(size);
}

void* operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void* p) {
    free(p);
}

void operator delete[](void* p) {
    free(p);
}

void operator delete(void* p, size_t size) {
    free(p);
}

void operator delete[](void* p, size_t size) {
    free(p);
}

// C++ ABI Support
extern "C" int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {
    return 0;
}

extern "C" void* __dso_handle = (void*) &__dso_handle;

extern "C" int __cxa_guard_acquire (uint_64 *g) {
    return !*(char *)(g);
}

extern "C" void __cxa_guard_release (uint_64 *g) {
    *(char *)g = 1;
}

extern "C" void __cxa_guard_abort (uint_64 *g) {
}
