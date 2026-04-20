#pragma once

#include "TypeDefs.h"

typedef struct Framebuffer {
    void* BaseAddress;
    uint_64 BufferSize;
    uint_32 Width;
    uint_32 Height;
    uint_32 PixelsPerScanLine;
} Framebuffer;

typedef struct PSF1_Header {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charsize;
} PSF1_Header;

typedef struct PSF1_Font {
    PSF1_Header* header;
    void* glyphBuffer;
} PSF1_Font;

// Renamed to avoid conflict with efi.h
typedef struct MemoryDescriptor {
    uint_32 Type;
    void* PhysicalStart;
    void* VirtualStart;
    uint_64 NumberOfPages;
    uint_64 Attribute;
} MemoryDescriptor;

typedef struct BootInfo {
    Framebuffer* fb;
    PSF1_Font* font;
    MemoryDescriptor* memoryMap;
    uint_64 memoryMapSize;
    uint_64 descriptorSize;
} BootInfo;
