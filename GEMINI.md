# Current Issue Summary: IDT, Keyboard, and VGA Display

## 1. IDT and Linker Errors (`_idt`)
- **Symptom:** `undefined reference to _idt` during linking.
- **Cause:** The `_idt` symbol was defined in `link.ld` using `_idt = .;`, but C++ code was also declaring it. Because C++ mangles variable names, the assembly file (`IDT.asm`) and the C++ file were not referencing the same symbol in the object files.
- **Solution Strategy:** Declare `_idt` as an initialized array in C++ using `extern "C" IDT64ENTRY _idt[256] = {};` so the C++ compiler exports it as exactly `_idt`. Remove the block for `_idt` in `link.ld` to avoid duplicate symbol definitions. Additionally, updating NASM to use `lidt [rel idtDescriptor]` fixes the `implicit DEFAULT ABS` warning.

## 2. Keyboard Handler Printing "0"
- **Symptom:** Pressing keys results in "0" being printed. Backspace and arrow keys work, but normal characters don't.
- **Cause:** 
  - The `ScanCodeLookupTable` is returning `0` for certain key presses. Key releases send a scancode of `original_scancode + 0x80`. If the logic does not drop or handle release codes, it tries to look them up, likely fetching a `0` or null char from out-of-bounds memory, which then gets printed.
  - The original backspace check was using `0x8E` (the key release code for backspace) instead of `0x0E` (the key press code), causing backspace to only trigger when the key was let go.

## 3. Empty / Blank Screen
- **Symptom:** The screen is completely blank or clears before showing anything.
- **Cause:**
  - `clearScreen` was setting the VGA memory to literal `0`s (which means a Null character with a Black-on-Black color attribute). To make an empty colored background properly, you need to write the space character (`32` or `0x20`) combined with the desired color attribute.
  - The identity paging setup in `ASM/Sector2+/paging.asm` may have had a slight misalignment in mapping the 2MB page table, potentially causing the VGA text buffer (`0xB8000`) to be unmapped or overwritten depending on how the loop bounds were structured.
