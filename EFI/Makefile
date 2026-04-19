LIMA = limactl shell debian
CC = $(LIMA) clang
CXX = $(LIMA) clang++
LD = $(LIMA) ld.lld
BINF = bin/obj/

# Directories
SRC_DIR = ../C++
LIB_DIR = $(SRC_DIR)/libs
DRV_DIR = $(LIB_DIR)/drivers

# Targets
EFI_TARGET = bootx64.efi
KERNEL_BIN = kernel.bin
USER_BIN = user_program.bin
FS_IMG = fat32_disk.img

# Logic to ensure the object directory exists
$(shell mkdir -p $(BINF))

# --- Flags ---
CFLAGS = -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone \
         -mgeneral-regs-only -I/usr/include/efi -I/usr/include/efi/x86_64 \
         -Dtarget_x86_64 -DGNU_EFI_USE_MS_ABI -Wall -O2 -I $(SRC_DIR)/headers

CXXFLAGS = -target x86_64-unknown-none -ffreestanding -fno-stack-protector \
           -fno-exceptions -fno-rtti -mno-red-zone -Wall -O2 \
           -I. -I $(SRC_DIR)/headers \
		   -mgeneral-regs-only

ISR_FLAGS = $(CXXFLAGS) -mgeneral-regs-only

LDFLAGS = -flavor link -subsystem:efi_application -entry:efi_main -nodefaultlib

# --- Object List ---
OBJS = $(addprefix $(BINF), entry.o Kernel.o Framebuffer.o BasicRenderer.o IO.o \
       IDT.o Keyboard.o PIT.o PCI.o AHCI.o fat32.o TextPrint.o Heap.o \
       Memory.o MemoryMap.o GDT.o gdt_wrapper.o str.o font.o userspace.o \
       Syscalls.o setjmp.o PageFrameAllocator.o Paging.o)

all: $(EFI_TARGET) $(KERNEL_BIN) $(USER_BIN)

# 1. UEFI Bootloader
$(BINF)bootloader.o: bootloader.c
	mkdir -p $(BINF)
	$(CC) $(CFLAGS) -c $< -o $@

$(EFI_TARGET): $(BINF)bootloader.o
	$(LD) $(LDFLAGS) $< -out:$@

# 2. Kernel Core & Drivers
$(BINF)Kernel.o: $(SRC_DIR)/Kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)str.o: $(LIB_DIR)/data/str.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Graphics
$(BINF)Framebuffer.o: $(DRV_DIR)/Graphics/Framebuffer.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

$(BINF)BasicRenderer.o: $(DRV_DIR)/Graphics/BasicRenderer.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

# COM / Basic
$(BINF)IO.o: $(DRV_DIR)/COM/Basic/IO.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

$(BINF)PCI.o: $(DRV_DIR)/COM/PCI.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)AHCI.o: $(DRV_DIR)/COM/AHCI.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)fat32.o: $(DRV_DIR)/COM/fat32.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# IDT
$(BINF)IDT.o: $(DRV_DIR)/IDT/IDT.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

$(BINF)Keyboard.o: $(DRV_DIR)/IDT/Keyboard.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

$(BINF)PIT.o: $(DRV_DIR)/IDT/PIT.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

# Misc Drivers
$(BINF)TextPrint.o: $(DRV_DIR)/TextPrint.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Memory & GDT
$(BINF)GDT.o: $(LIB_DIR)/Memory/GDT.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)PageFrameAllocator.o: $(LIB_DIR)/Memory/PageFrameAllocator.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)Paging.o: $(LIB_DIR)/Memory/Paging.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)Heap.o: $(LIB_DIR)/Memory/Heap/Heap.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)Memory.o: $(LIB_DIR)/Memory/Heap/Memory.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)MemoryMap.o: $(LIB_DIR)/Memory/Heap/MemoryMap.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 3. Assembly Files
$(BINF)entry.o: $(LIB_DIR)/entry.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)gdt_wrapper.o: $(LIB_DIR)/Memory/gdt_wrapper.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)userspace.o: $(DRV_DIR)/User/userspace.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)setjmp.o: $(DRV_DIR)/User/setjmp.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)Syscalls.o: $(DRV_DIR)/User/Syscalls.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)font.o: font_wrapper.S font.psf
	$(CC) -target x86_64-unknown-none -c $< -o $@

# 4. Linking
$(KERNEL_BIN): $(OBJS)
	$(LD) -flavor gnu -T ../Conf/link.ld -o $@ $(OBJS)

$(USER_BIN): $(BINF)user_program.o
	$(LD) -flavor gnu --section-start .text=0x0 --oformat binary -o $@ $<

$(BINF)user_program.o: $(SRC_DIR)/user_program.cpp
	mkdir -p $(BINF)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Filesystem Image Generation ---
# --- Filesystem Image Generation (Finder Compatible) ---
$(FS_IMG):
	@echo "Creating 64MB FAT32 disk image for Finder..."
	# Use hdiutil to create a layout macOS understands (MBR partition map)
	hdiutil create -size 64m -fs "MS-DOS FAT32" -volname "MY_OS_DISK" -layout MBRSPUD $(FS_IMG).dmg
	# Rename to your expected filename
	mv $(FS_IMG).dmg $(FS_IMG)
	
	# Use mtools via LIMA to add your files and folders
	@echo "Hello from Text1!" > Text1.txt
	$(LIMA) mmd -i $(FS_IMG) ::/my_folder
	$(LIMA) mcopy -i $(FS_IMG) Text1.txt ::/my_folder/
	$(LIMA) mcopy -i $(FS_IMG) $(USER_BIN) ::/


# --- Deployment & Run ---
deploy: all $(FS_IMG)
	mkdir -p esp/EFI/BOOT
	cp $(EFI_TARGET) esp/EFI/BOOT/BOOTX64.EFI
	cp $(KERNEL_BIN) esp/kernel.bin
	cp $(USER_BIN) esp/user_program.bin
	printf "\\\EFI\\\BOOT\\\BOOTX64.EFI\r\n" > esp/startup.nsh

run: deploy
	qemu-system-x86_64 -M q35 -drive if=pflash,format=raw,readonly=on,file=OVMF/OVMF_CODE-pure-efi.fd \
		-drive if=pflash,format=raw,file=OVMF/OVMF_VARS-pure-efi.fd \
		-drive format=raw,file=fat:rw:esp \
		-drive id=disk,file=$(FS_IMG),if=none,format=raw \
		-device ich9-ahci,id=ahci \
		-device ide-hd,drive=disk,bus=ahci.0 \
		-m 256M -net none

clean:
	rm -rf $(BINF) *.o $(EFI_TARGET) $(KERNEL_BIN) $(USER_BIN) esp/ $(FS_IMG) Text1.txt Text2.txt

.PHONY: all deploy run clean
