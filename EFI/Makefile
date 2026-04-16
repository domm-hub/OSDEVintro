LIMA = limactl shell debian
CC = $(LIMA) clang
CXX = $(LIMA) clang++
LD = $(LIMA) ld.lld
BINF = bin/obj/

# Directories
SRC_DIR = ../C++
LIB_DIR = $(SRC_DIR)/libs

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
       Syscalls.o setjmp.o)

all: $(EFI_TARGET) $(KERNEL_BIN) $(USER_BIN)

# 1. UEFI Bootloader
$(BINF)bootloader.o: bootloader.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EFI_TARGET): $(BINF)bootloader.o
	$(LD) $(LDFLAGS) $< -out:$@

# 2. Pattern Rules for Kernel (Cleans up your file significantly)
$(BINF)%.o: $(LIB_DIR)/drivers/%.cpp
	$(CXX) $(if $(filter Framebuffer.o BasicRenderer.o IO.o Keyboard.o PIT.o, $@), $(ISR_FLAGS), $(CXXFLAGS)) -c $< -o $@

$(BINF)%.o: $(LIB_DIR)/Memory/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)Kernel.o: $(SRC_DIR)/Kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINF)IDT.o: $(LIB_DIR)/IDT.cpp
	$(CXX) $(ISR_FLAGS) -c $< -o $@

# 3. Assembly Files
$(BINF)%.o: $(LIB_DIR)/%.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)gdt_wrapper.o: $(LIB_DIR)/Memory/gdt_wrapper.S
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)font.o: font_wrapper.S font.psf
	$(CC) -target x86_64-unknown-none -c $< -o $@

$(BINF)str.o: $(LIB_DIR)/data/str.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Linking
$(KERNEL_BIN): $(OBJS)
	$(LD) -flavor gnu -T ../link.ld -o $@ $(OBJS)

$(USER_BIN): $(BINF)user_program.o
	$(LD) -flavor gnu --section-start .text=0x0 --oformat binary -o $@ $<

$(BINF)user_program.o: $(SRC_DIR)/user_program.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Deployment & Run ---
deploy: all
	mkdir -p esp/EFI/BOOT
	cp $(EFI_TARGET) esp/EFI/BOOT/BOOTX64.EFI
	cp $(KERNEL_BIN) esp/kernel.bin
	cp $(USER_BIN) esp/user_program.bin
	printf "\\\EFI\\\BOOT\\\BOOTX64.EFI\r\n" > esp/startup.nsh

run: deploy $(FS_IMG)
	qemu-system-x86_64 -drive if=pflash,format=raw,readonly=on,file=OVMF/OVMF_CODE-pure-efi.fd \
		-drive if=pflash,format=raw,file=OVMF/OVMF_VARS-pure-efi.fd \
		-drive format=raw,file=fat:rw:esp -m 256M -net none

clean:
	rm -rf $(BINF) *.o $(EFI_TARGET) $(KERNEL_BIN) $(USER_BIN) esp/ $(FS_IMG)

.PHONY: all deploy run clean
