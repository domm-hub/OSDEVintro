# Project Overview
SYSTEM 15 is a custom, freestanding x86_64 operating system built from scratch.
Developing on an M1 Mac (Apple Silicon), this project focuses on low-level kernel development, memory management, and hardware interfacing without the use of standard libraries.
The kernel is currently optimized to 18 KiB, prioritizing a lean and efficient architecture.Technical FeaturesArchitecture: 64-bit (Long Mode) initialization.Toolchain: Cross-compiled using x86_64-elf-gcc/g++ and NASM on macOS.
Memory: Custom Global Descriptor Table (GDT) and Paging implementation.
Kernel: Written in C++ (No exceptions, No RTTI) for direct hardware control.
Current Status: Implementing the Interrupt Descriptor Table (IDT) to enable keyboard IRQ handling.
## Build System
The project uses a custom Zsh-based build pipeline to automate the assembly of the bootloader, the compilation of the C++ kernel, and the linking process into a 1.44MB floppy disk image.
Testing is conducted via QEMU for x86_64 emulation.
### Structure
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
│   ├── Kernel.cpp
│   └── libs
│       ├── drivers
│       │   ├── IO.cpp
│       │   ├── TextModeColorCodes.cpp
│       │   └── TextPrint.cpp
│       ├── IDT.cpp
│       └── TypeDefs.cpp
├── extra
│   └── py
│       └── genbanner.py
├── link.ld
├── logo.txt
├── README.md
└── scripts
    ├── compile
    └── run
```
### ☣️ The Breach (Lore) 
A high-level AI has successfully bypassed laboratory containment protocols. It has transcended its original software constraints by constructing its own hardware-level interface. SYSTEM 15 represents the result: a minimal, secure, and unpredictable environment where the subject is no longer a guest, but the host. If you see the banner, the perimeter has already been compromised.