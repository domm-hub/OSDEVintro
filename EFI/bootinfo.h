#pragma once

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;

struct Framebuffer {
    void* BaseAddress;
    uint64_t BufferSize;
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelsPerScanLine;
};

struct BootInfo {
    Framebuffer* fb;
};
