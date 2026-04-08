#pragma once
#include "IO.cpp"
#include "../TypeDefs.cpp"
#include "TextModeColorCodes.cpp"


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
    value += clearclr << 8;
    value += clearclr << 24;
    value += clearclr << 40;
    value += clearclr << 56;
    for (uint_64* i = (uint_64*)VGA_MEMORY; i < (uint_64*)(VGA_MEMORY+4000); i += 8) {
        *(i + 0) = value; // Bytes 0-7
        *(i + 1) = value; // Bytes 8-15
        *(i + 2) = value; // Bytes 16-23
        *(i + 3) = value; // Bytes 24-31
        *(i + 4) = value; // Bytes 24-31
        *(i + 5) = value; // Bytes 24-31
        *(i + 6) = value; // Bytes 24-31
        *(i + 7) = value; // Bytes 24-31
        
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
const char* IntegerToString(T value){
    uint_8 isNegative = 0;
    uint_64 temp;

    if (value < 0){
        isNegative = 1;
        temp = (uint_64)(-value);
    } else {
        temp = (uint_64)value;
    }

    int i = 0;
    do {
        IntegerToStringOutput[i++] = (temp % 10) + '0';
        temp /= 10;
    } while (temp);

    if (isNegative)
        IntegerToStringOutput[i++] = '-';

    IntegerToStringOutput[i] = 0;

    // reverse
    for (int j = 0; j < i / 2; j++){
        char t = IntegerToStringOutput[j];
        IntegerToStringOutput[j] = IntegerToStringOutput[i - j - 1];
        IntegerToStringOutput[i - j - 1] = t;
    }

    return IntegerToStringOutput;
}