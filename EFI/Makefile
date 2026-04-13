# Use your 'lima' alias
LIMA = limactl shell debian

CC = $(LIMA) clang
CXX = $(LIMA) clang++
LD = $(LIMA) ld.lld
OBJCOPY = $(LIMA) objcopy

FS_IMG = fat32_disk.img
FS_CONTENTS = Text1.txt Text2.txt


# UEFI Flags
CFLAGS = -target x86_64-unknown-windows \
         -ffreestanding \
         -fshort-wchar \
         -mno-red-zone \
         -mgeneral-regs-only \
         -I/usr/include/efi \
         -I/usr/include/efi/x86_64 \
         -I/usr/include/efi/protocol \
         -Dtarget_x86_64 \
         -DGNU_EFI_USE_MS_ABI \
         -Wall \
         -O2 \
         -I ../C++/headers

# Kernel Flags (Base)
CXXFLAGS = -target x86_64-unknown-none \
           -ffreestanding \
           -fno-stack-protector \
           -fno-stack-check \
           -fno-exceptions \
           -fno-rtti \
           -fno-lto \
           -mno-red-zone \
           -I. \
           -I ../C++/headers \
           -Wall \
           -O2

# Special flags for ISR-related files and their dependencies
ISR_CXXFLAGS = $(CXXFLAGS) -mgeneral-regs-only

# EFI Linker Flags
LDFLAGS = -flavor link \
          -subsystem:efi_application \
          -entry:efi_main \
          -nodefaultlib

# Kernel Objects - Full List
KERNEL_OBJS = entry.o \
              Kernel.o \
              Framebuffer.o \
              BasicRenderer.o \
              IO.o \
              IDT.o \
              Keyboard.o \
              PIT.o \
              PCI.o \
              AHCI.o \
              fat32.o \
              TextPrint.o \
              Heap.o \
              Memory.o \
              MemoryMap.o \
              GDT.o \
              gdt_wrapper.o \
              str.o \
              font.o

all: bootx64.efi kernel.bin

# Compile UEFI Bootloader
bootloader.o: bootloader.c
	$(CC) $(CFLAGS) -c $< -o $@

bootx64.efi: bootloader.o
	$(LD) $(LDFLAGS) $< -out:$@

# Compile Kernel Files
entry.o: ../C++/libs/entry.S
	$(CC) -target x86_64-unknown-none -fno-pic -fno-plt -c $< -o $@

Kernel.o: ../C++/Kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Framebuffer.o: ../C++/libs/drivers/Framebuffer.cpp
	$(CXX) $(ISR_CXXFLAGS) -c $< -o $@

BasicRenderer.o: ../C++/libs/drivers/BasicRenderer.cpp
	$(CXX) $(ISR_CXXFLAGS) -c $< -o $@

IO.o: ../C++/libs/drivers/IO.cpp
	$(CXX) $(ISR_CXXFLAGS) -c $< -o $@

IDT.o: ../C++/libs/IDT.cpp
	$(CXX) $(ISR_CXXFLAGS) -c $< -o $@

Keyboard.o: ../C++/libs/drivers/Keyboard.cpp
	$(CXX) $(ISR_CXXFLAGS) -c $< -o $@

PIT.o: ../C++/libs/drivers/PIT.cpp
	$(CXX) $(ISR_CXXFLAGS) -c $< -o $@

PCI.o: ../C++/libs/drivers/PCI.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

AHCI.o: ../C++/libs/drivers/AHCI.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

fat32.o: ../C++/libs/drivers/fat32.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

TextPrint.o: ../C++/libs/drivers/TextPrint.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Heap.o: ../C++/libs/Memory/Heap.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Memory.o: ../C++/libs/Memory/Memory.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

MemoryMap.o: ../C++/libs/Memory/MemoryMap.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

str.o: ../C++/libs/data/str.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

GDT.o: ../C++/libs/Memory/GDT.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

gdt_wrapper.o: ../C++/libs/Memory/gdt_wrapper.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

setjmp.o: ../C++/libs/setjmp.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

# Convert font.psf to an object file using clang (cross-compiles correctly)
font.o: font_wrapper.S font.psf
	$(CC) -target x86_64-unknown-none -c $< -o $@

# Link the kernel binary
kernel.bin: $(KERNEL_OBJS)
	$(LD) -flavor gnu -T ../link.ld -o $@ $(KERNEL_OBJS)

deploy: all
	@mkdir -p esp/EFI/BOOT
	@cp bootx64.efi esp/EFI/BOOT/BOOTX64.EFI
	@cp kernel.bin esp/kernel.bin
	@printf "\\\EFI\\\BOOT\\\BOOTX64.EFI\r\n" > esp/startup.nsh

$(FS_IMG):
	@echo "Creating 20MB FAT32 disk image..."
	dd if=/dev/zero of=$(FS_IMG) bs=1M count=20
	mkfs.fat -F 32 $(FS_IMG)
	echo "Hello from Text1!" > Text1.txt
	echo "Hello from Text2! \n HeHeHe" > Text2.txt
	mcopy -i $(FS_IMG) Text1.txt ::/
	mcopy -i $(FS_IMG) Text2.txt ::/


run: deploy $(FS_IMG)
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=OVMF/OVMF_CODE-pure-efi.fd \
		-drive if=pflash,format=raw,file=OVMF/OVMF_VARS-pure-efi.fd \
		-drive format=raw,file=fat:rw:esp \
		-drive id=disk,file=$(FS_IMG),if=none,format=raw \
		-device ahci,id=ahci \
		-device ide-hd,drive=disk,bus=ahci.0 \
		-m 256M \
		-net none \
		-d int \
		-D logs.txt

# Hi

clean:
	rm -rf *.o bootx64.efi kernel.bin esp/

.PHONY: all deploy run clean
NY: clean all deploy run clean
