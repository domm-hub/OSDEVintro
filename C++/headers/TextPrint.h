#pragma once
#include "IO.h"
#include "TypeDefs.h"
#include "TextModeColorCodes.h"

extern char HexToStringOutput[128];
extern char IntegerToStringOutput[128];
extern char FloatToStringOutput[128];

template<typename T>
const char* HexToString(T value){
    T* valPtr = &value;
    uint_8* ptr;
    uint_8 temp;
    uint_8 size = sizeof(T) * 2 - 1;
    uint_8 i;
    for (i = 0; i < size; i++){
        ptr = ((uint_8*)valPtr + i);
        temp = ((*ptr & 0xF0) >> 4);
        HexToStringOutput[size - (i * 2 + 1)] = temp + (temp > 9 ? 55 : 48);
        temp = ((*ptr & 0x0F));
        HexToStringOutput[size - (i * 2)] = temp + (temp > 9 ? 55 : 48);
    }
    HexToStringOutput[size + 1] = 0;

    return HexToStringOutput;
}

template<typename T>
const char* IntegerToString(T value) {
    // 1. Handle Zero explicitly
    if (value == 0) {
        IntegerToStringOutput[0] = '0';
        IntegerToStringOutput[1] = '\0';
        return IntegerToStringOutput;
    }

    uint_64 temp;
    bool isNegative = false;

    // 2. Handle Negatives for signed types
    // We check if T is signed. If value is negative, mark it and flip it.
    if (value < 0) {
        isNegative = true;
        temp = (uint_64)(-value);
    } else {
        temp = (uint_64)value;
    }

    int i = 0;
    // 3. Extract digits (they will be in reverse order)
    while (temp > 0) {
        IntegerToStringOutput[i++] = (temp % 10) + '0';
        temp /= 10;
    }

    if (isNegative) {
        IntegerToStringOutput[i++] = '-';
    }

    IntegerToStringOutput[i] = '\0'; // Null terminate

    // 4. Reverse the string in place
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char t = IntegerToStringOutput[start];
        IntegerToStringOutput[start] = IntegerToStringOutput[end];
        IntegerToStringOutput[end] = t;
        start++;
        end--;
    }

    return IntegerToStringOutput;
}


const char* FloatToString(double value, int precision = 6);

