#pragma once
#include "TypeDefs.h"

class Bitmap {
public:
    uint_64 Size;
    uint_8* Buffer;

    bool operator[](uint_64 index) {
        if (index >= Size * 8) return false;
        uint_64 byteIndex = index / 8;
        uint_8 bitIndex = index % 8;
        uint_8 bitIndexer = 0b10000000 >> bitIndex;
        return (Buffer[byteIndex] & bitIndexer) > 0;
    }

    bool Set(uint_64 index, bool value) {
        if (index >= Size * 8) return false;
        uint_64 byteIndex = index / 8;
        uint_8 bitIndex = index % 8;
        uint_8 bitIndexer = 0b10000000 >> bitIndex;
        
        if (value) Buffer[byteIndex] |= bitIndexer;
        else Buffer[byteIndex] &= ~bitIndexer;
        
        return true;
    }
};
