#include "SysCalls/IO.cpp"

extern "C" __attribute__((section(".entry"))) void _start() {
    println("User Program Started.");

    // Test sbrk
    char* my_mem = (char*)sbrk(4096);
    if (my_mem) {
        my_mem[0] = 'H';
        my_mem[1] = 'e';
        my_mem[2] = 'y';
        my_mem[3] = '!';
        my_mem[4] = '\0';
        
        print("Dynamic Memory Allocated at: ");
        printHex((unsigned long long)my_mem);
        print("\n");
        
        println(my_mem);
    } else {
        println("SBRK Failed.");
    }

    exit(0);
}
