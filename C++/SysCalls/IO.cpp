#pragma once

typedef unsigned long long uint_64;

// Print string
void print(const char* msg) {
    register unsigned long long rax asm("rax") = 1;
    register const char* rdi asm("rdi") = msg;
    asm volatile (
        "syscall"
        : "+r" (rax), "+r" (rdi)
        :
        : "rcx", "r11", "memory"
    );
}

void println(const char* msg) {
    print(msg);
    print("\n");
}

void printHex(unsigned long long value) {
    char hex[17];
    for (int i = 0; i < 16; i++) {
        unsigned long long nibble = (value >> (60 - i * 4)) & 0xF;
        if (nibble < 10) hex[i] = nibble + '0';
        else hex[i] = nibble - 10 + 'A';
    }
    hex[16] = '\0';
    print("0x");
    print(hex);
}

// Exit
void exit(unsigned char returncode) {
    register unsigned long long rax asm("rax") = 0;
    asm volatile (
        "syscall"
        : "+r" (rax)
        :
        : "rcx", "r11", "memory"
    );
    while(1);
}

// Sleep for a certain amount of time
void Sleep(uint_64 milliseconds){
    register unsigned long long rax asm("rax") = 3;
    register uint_64 rdi asm("rdi") = milliseconds;

    asm volatile (
        "syscall"
        : "+r" (rax), "+r" (rdi)
        :
        : "rcx", "r11", "memory"
    );
}

void Yield() {
    register unsigned long long rax asm("rax") = 4;
    asm volatile (
        "syscall"
        : "+r" (rax)
        :
        : "rcx", "r11", "memory"
    );
}

// Request more memory
void* sbrk(int increment) {
    register unsigned long long rax asm("rax") = 5;
    register long long rdi asm("rdi") = increment;
    asm volatile (
        "syscall"
        : "+r" (rax), "+r" (rdi)
        :
        : "rcx", "r11", "memory"
    );
    return (void*)rax;
}
