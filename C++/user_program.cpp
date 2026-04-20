#include "SysCalls/IO.cpp"
#include "SysCalls/Heap.h"

extern "C" __attribute__((section(".entry"))) void _start() {
    exit(main());
}

int main(){
    const char* msg = "Hello from Ring 3! I am a user program.\n";
    
    println(msg);
    uint_8 items = 15;
    uint_8* array = (uint_8*) malloc(items*sizeof(uint_8));
    int i = 0; 
    char* str = "Hello!\n";

    while (str[i] != '\0'){
        array[i] = str[i];
        i++;
    }
    array[i] = '\0';

    println(array);
    newMem = (char*)realloc((void*)array, 30);
    newMem = "1234567890 123456789 123456789";

    println(newMem);
    println("\nExcellent!\n");

    println("Exiting...\n");
    
    return 0;
}

