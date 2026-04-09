#pragma once
#include "IO.cpp"
#include "TypeDefs.h"
#include "TextModeColorCodes.h"

#include "TextPrint.h"


#define VGA_MEMORY (uint_8*)0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

uint_16 CursorPosition;



void SetCursorPosition(uint_16 position){
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint_8)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint_8)(position >> 8) & 0xFF);

    CursorPosition = position;
}



uint_16 PositionFromCoords(uint_8 x, uint_8 y){
    return y * VGA_WIDTH + x;
}


void PrintString(const char* str, uint_8 color = BACKGROUND_BLACK | FOREGROUND_WHITE){
    uint_8* charPtr = (uint_8*)str;
    uint_16 index = CursorPosition;
    while (*charPtr != 0){
        switch (*charPtr){
            case 10:
                index += VGA_WIDTH;
                index -= index % VGA_WIDTH;
                break;
            case 13:
                index -= index % VGA_WIDTH;
                break;
            default:
                *(VGA_MEMORY + index * 2) = *charPtr;
                *(VGA_MEMORY + index * 2 + 1) = color;
                index++;
        }

        charPtr++;
    }
    SetCursorPosition(index);
}

// Global iteration counter

void PrintChar(char chr, uint_8 color = BACKGROUND_BLACK | FOREGROUND_WHITE){
    uint_16 index = CursorPosition;
    switch (chr){
        case 10:
            index += VGA_WIDTH;
            index -= index % VGA_WIDTH;
            break;
        case 13:
            index -= index % VGA_WIDTH;
            break;
        default:
            *(VGA_MEMORY + index * 2) = chr;
            *(VGA_MEMORY + index * 2 + 1) = color;
            index++;
    }
    SetCursorPosition(index);

}


void clearScreen(uint_64 clearclr = BACKGROUND_BLACK | FOREGROUND_WHITE){
    uint_64 value = 0;
    // We want 4 copies of [Attribute][Space] in a 64-bit value
    // 0x20 is ' ' (Space)
    uint_64 character = 0x20; 
    
    uint_64 pair = (clearclr << 8) | character;
    
    value |= (pair << 0);
    value |= (pair << 16);
    value |= (pair << 32);
    value |= (pair << 48);

    for (uint_64* i = (uint_64*)VGA_MEMORY; i < (uint_64*)(VGA_MEMORY + 4000); i++) {
        *i = value;
    }
}

char HexToStringOutput[128];
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


char IntegerToStringOutput[128];
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

char FloatToStringOutput[128];

const char* FloatToString(double value, int precision = 6) {
    int i = 0;
    bool isNegative = false;

    // 1. Handle Negative
    if (value < 0) {
        isNegative = true;
        value = -value;
    }

    // 2. Rounding (Optional but recommended)
    // Adds 0.5 to the last digit we care about to round correctly
    double rounding = 0.5;
    for (int p = 0; p < precision; ++p) rounding /= 10.0;
    value += rounding;

    // 3. Extract Integer Part
    unsigned long long integerPart = (unsigned long long)value;
    double fractionalPart = value - (double)integerPart;

    // 4. Convert Fractional Part first (to know where the '.' goes)
    // We do this in reverse order just like your integer function
    for (int p = 0; p < precision; p++) {
        fractionalPart *= 10;
        int digit = (int)fractionalPart;
        // We'll store these temporarily or just handle them after the decimal
        // For simplicity in this logic, let's build the string from left to right
    }

    // --- Let's restart the buffer fill to be more efficient ---
    i = 0;

    // Add negative sign
    if (isNegative) FloatToStringOutput[i++] = '-';

    // Convert Integer Part (using your reverse logic)
    int intStart = i;
    if (integerPart == 0) {
        FloatToStringOutput[i++] = '0';
    } else {
        while (integerPart > 0) {
            FloatToStringOutput[i++] = (integerPart % 10) + '0';
            integerPart /= 10;
        }
        // Reverse only the integer portion
        int start = intStart;
        int end = i - 1;
        while (start < end) {
            char t = FloatToStringOutput[start];
            FloatToStringOutput[start] = FloatToStringOutput[end];
            FloatToStringOutput[end] = t;
            start++; end--;
        }
    }

    // 5. Add Decimal Point
    if (precision > 0) {
        FloatToStringOutput[i++] = '.';

        // 6. Extract and Add Fractional Digits
        fractionalPart = value - (unsigned long long)value; 
        for (int p = 0; p < precision; p++) {
            fractionalPart *= 10;
            int digit = (int)fractionalPart;
            FloatToStringOutput[i++] = digit + '0';
            fractionalPart -= digit;
        }
    }

    FloatToStringOutput[i] = '\0';
    return FloatToStringOutput;
}