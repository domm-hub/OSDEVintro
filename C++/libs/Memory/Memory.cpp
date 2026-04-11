#include "Memory.h"

void memset(void* start, uint_64 value, uint_64 num){
    if (num <= 8){
        uint_8* valPTR = (uint_8*)&value;
        for (uint_8* ptr = (uint_8*)start; ptr < (uint_8*)((uint_64)start+num); ptr++){
            *ptr = *valPTR;
            valPTR++;
        }
        return;
    }

    uint_64 proceedingBytes = num & 8;
    uint_64 newNum = num - proceedingBytes;

    for (uint_64* ptr = (uint_64*)start; ptr < (uint_64*)((uint_64)start+newNum); ptr++){
        *ptr = value;

    }

    uint_8* valPtr = (uint_8*)&value;
    for (uint_8* ptr = (uint_8*)((uint_64)start+newNum); ptr < (uint_8*)((uint_64)start+num); ptr++){
        *ptr = *valPtr;
        valPtr++;
    }


}


void memcpy(void* destination, void* source, uint_64 num){
    if (num <= 8){
        uint_8* valPTR = (uint_8*)source;
        for (uint_8* ptr = (uint_8*)destination; ptr < (uint_8*)((uint_64)destination+num); ptr++){
            *ptr = *valPTR;
            valPTR++;
        }
        return;
    }


    uint_64 proceedingBytes = num & 8;
    uint_64 newNum = num - proceedingBytes;
    uint_64* srcptr = (uint_64*)source;

    for (uint_64* destptr = (uint_64*)destination; destptr < (uint_64*)((uint_64)destination+newNum); destptr++){
        *destptr = *srcptr;
        srcptr++;

    }

    uint_8* srcPtr8 = (uint_8*)((uint_64)source+newNum);
    for (uint_8* destptr8 = (uint_8*)((uint_64)destination+newNum); destptr8 < (uint_8*)((uint_64)destination+num); destptr8++){
        *destptr8 = *srcPtr8;
        srcPtr8++;
    }


}