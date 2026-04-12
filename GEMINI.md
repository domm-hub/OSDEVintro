# Gemini.md
Youre not allowed to write or do commands except
`./scripts/compile`
and compiling commands
All `git` commands and any `sudo` commands are forbidden

Poncho is an OS dev teacher on YT.

### Things to acknoledge but not be sure of:
  - Wierd RemapPic from tutorial
  - Every body says my handler logic is incorrect; Poncho did it this way.

## Prompt:
  - Tell me the   problem
  - Tell me the    reason
  - Tell me what to    do
  - Tell me how to backup

### What I'm experiencing

### Basic info to reduce requests time
  - Tree: 
    ```
    .
    ├── ASM
    │   ├── SecImport
    │   │   └── Binaries.asm
    │   ├── Sector1
    │   │   ├── bootloader.asm
    │   │   ├── DiskRead.asm
    │   │   └── print.asm
    │   └── Sector2+
    │       ├── CPUID.asm
    │       ├── ExtendedProgram.asm
    │       ├── gdt.asm
    │       ├── IDT.asm
    │       └── paging.asm
    ├── bin
    │   └── O
    │       ├── Binaries.o
    │       ├── bootloader.bin
    │       ├── ExtendedProgram.o
    │       └── Kernel.o
    ├── bootable
    ├── C++
    │   ├── include
    │   ├── Kernel.cpp
    │   └── libs
    │       ├── drivers
    │       │   ├── IO.cpp
    │       │   ├── Keyboard.cpp
    │       │   ├── TextModeColorCodes.cpp
    │       │   └── TextPrint.cpp
    │       ├── IDT.cpp
    │       ├── Sets
    │       │   └── KBSCodesS1.cpp
    │       └── TypeDefs.cpp
    ├── extra
    │   └── py
    │       └── genbanner.py
    ├── gemini-2.5-flash:generateContent
    ├── GEMINI.md
    ├── link.ld
    ├── logo.txt
    ├── main.py
    ├── README.md
    └── scripts
    ├── compile
    └── run
  ```

### NOTE:
  - I strictly forbid any commands that can or will delete my progress
  - WRITING TO FILES IS FORBIDDEN
  - If wanna to provide an edit, make a temp file with the files name + _diff.temp
### Status:
  Not working

