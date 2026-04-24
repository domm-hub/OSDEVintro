#include <efi.h>
#include "../C++/headers/BootInfo.h"


// Helper to open the root directory
EFI_STATUS GetRootHandle(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, EFI_FILE_PROTOCOL **Root) {
    EFI_GUID LoadedImageGuid = LOADED_IMAGE_PROTOCOL;
    EFI_GUID FileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;

    SystemTable->BootServices->HandleProtocol(ImageHandle, &LoadedImageGuid, (void**)&LoadedImage);
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &FileSystemGuid, (void**)&FileSystem);
    return FileSystem->OpenVolume(FileSystem, Root);
}

// Initialize Graphics Output Protocol
EFI_STATUS InitializeGOP(EFI_SYSTEM_TABLE *SystemTable, Framebuffer* out_fb) {
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status = SystemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if (EFI_ERROR(status)) return status;

    out_fb->BaseAddress = (void*)gop->Mode->FrameBufferBase;
    out_fb->BufferSize = gop->Mode->FrameBufferSize;
    out_fb->Width = gop->Mode->Info->HorizontalResolution;
    out_fb->Height = gop->Mode->Info->VerticalResolution;
    out_fb->PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"UEFI Bootloader Initializing...\r\n");

    // 1. Initialize Framebuffer
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Locating GOP...\r\n");
    Framebuffer* fb;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(Framebuffer), (void**)&fb);
    if (InitializeGOP(SystemTable, fb) != EFI_SUCCESS) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to locate GOP!\r\n");
        return EFI_DEVICE_ERROR;
    }

    // ... (rest of the drive opening code) ...
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Opening drive...\r\n");
    EFI_FILE_PROTOCOL *Root;
    if (GetRootHandle(ImageHandle, SystemTable, &Root) != EFI_SUCCESS) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to open drive.\r\n");
        return EFI_LOAD_ERROR;
    }

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Opening kernel.bin...\r\n");
    EFI_FILE_PROTOCOL *KernelFile;
    if (Root->Open(Root, &KernelFile, L"kernel.bin", EFI_FILE_MODE_READ, 0) != EFI_SUCCESS) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"kernel.bin not found!\r\n");
        return EFI_NOT_FOUND;
    }

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Loading kernel...\r\n");
    
    // Get file info to know the exact size
    EFI_FILE_INFO *FileInfo;
    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 128; // Extra for filename
    SystemTable->BootServices->AllocatePool(EfiLoaderData, InfoSize, (void**)&FileInfo);
    EFI_GUID FileInfoGuid = EFI_FILE_INFO_ID;
    status = KernelFile->GetInfo(KernelFile, &FileInfoGuid, &InfoSize, FileInfo);
    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to get kernel info!\r\n");
        return status;
    }
    UINTN size = FileInfo->FileSize;
    SystemTable->BootServices->FreePool(FileInfo);

    void* kernel_entry = (void*)0x100000;
    UINTN pages = (size / 4096) + 1;
    status = SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, pages, (EFI_PHYSICAL_ADDRESS*)&kernel_entry);
    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to allocate pages at 0x100000!\r\n");
        if (status == EFI_NOT_FOUND) SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Reason: Memory region already in use.\r\n");
        if (status == EFI_OUT_OF_RESOURCES) SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Reason: Out of resources.\r\n");
        if (status == EFI_INVALID_PARAMETER) SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Reason: Invalid parameter.\r\n");
        return status;
    }

    status = KernelFile->Read(KernelFile, &size, kernel_entry);
    if (EFI_ERROR(status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to read kernel file!\r\n");
        return status;
    }

    // 4. Get Memory Map
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Getting Memory Map...\r\n");
    UINTN MapSize = 0, MapKey, DescriptorSize;
    UINT32 DescriptorVersion;
    SystemTable->BootServices->GetMemoryMap(&MapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    MapSize += 2 * DescriptorSize; // Buffer for growth
    void* pMap = NULL;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, &pMap);
    SystemTable->BootServices->GetMemoryMap(&MapSize, pMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    // 5. Prepare BootInfo
    BootInfo* bootInfo;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(BootInfo), (void**)&bootInfo);
    bootInfo->fb = fb;
    bootInfo->memoryMap = (MemoryDescriptor*)pMap; 
    bootInfo->memoryMapSize = MapSize;
    bootInfo->descriptorSize = DescriptorSize;

    // 6. Handover
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Exiting Boot Services...\r\n");
    
    status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    
    if (EFI_ERROR(status)) {
        // If it failed, the MapKey was probably out of date. 
        // Get the map one more time and try again.
        SystemTable->BootServices->GetMemoryMap(&MapSize, pMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
        
        if (EFI_ERROR(status)) {
            // If it fails twice, something is seriously wrong.
            SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Fatal: ExitBootServices failed!\r\n");
            while(1) { __asm__("hlt"); }
        }
    }
    
    void (*KernelEntry)(BootInfo*) = (void (*)(BootInfo*))kernel_entry;
    KernelEntry(bootInfo);

    while(1) { __asm__("hlt"); }
    return EFI_SUCCESS;
}
