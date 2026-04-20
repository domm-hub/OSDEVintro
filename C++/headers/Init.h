#include "BootInfo.h"
#include "BasicRenderer.h"
#include "IDT.h"
#include "GDT.h"
#include "Keyboard.h"
#include "IO.h"
#include "Heap.h"
#include "PIT.h"
#include "str.h"
#include "fat32.h"
#include "Vector.h"
#include "AHCI.h"
#include "MBR.h"
#include "Syscalls.h"
#include "Memory.h"
#include "PageFrameAllocator.h"
#include "Paging.h"

extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;

extern "C" void JumpToUser(uint_64 rip, uint_64 rsp);

extern "C" int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle) { return 0; }
void* __dso_handle = (void*)0;


void* operator new(unsigned long size);
void* operator new[](unsigned long size);

void operator delete(void* p);
void operator delete(void* p, unsigned long size);
void operator delete[](void* p);
void operator delete[](void* p, unsigned long size);

FAT32::Driver* InitStorage();


FAT32::Driver* globalFat32Driver;
uint32_t currentDirCluster; 
String* currentPath;

Paging::PageTableManager* GlobalPageTableManager;

static Framebuffer kernel_fb;
static PSF1_Font kernel_font;

void Init(BootInfo* bootInfo);


uint64_t read_return_code();

void InputMan(String input);

