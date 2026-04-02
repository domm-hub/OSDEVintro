import pyfiglet
import os

def generate_os_banner(text1, text2):
    # 1. Initialize the Figlet fonts
    # 'slant' or 'block' are usually the best for OS headers
    header_font = pyfiglet.Figlet(font='slant', width=80)
    sub_font = pyfiglet.Figlet(font='small', width=80)

    # 2. Generate the ASCII patterns
    header_ascii = header_font.renderText(text1)
    sub_ascii = sub_font.renderText(text2)

    # 3. Add your custom System 15 status block
    status_block = (
        "\n"
        "       [SYSTEM 15] Integrity Check: PASSED\n"
        "       [KERNEL]    Memory Usage:    32 KiB\n"
        "       [STATUS]    Ready.\n"
    )

    full_banner = header_ascii + sub_ascii + status_block

    # 4. Save it to a file for your NASM IncBin macro
    with open("logo.txt", "w") as f:
        f.write(full_banner)
    
    print("✅ Banner generated and saved to logo.txt!")
    print(full_banner)

if __name__ == "__main__":
    # You can change these to whatever your OS name is!
    generate_os_banner("SYSTEM", "15")