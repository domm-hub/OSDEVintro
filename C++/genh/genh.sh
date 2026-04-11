#!/bin/sh

# 1. Setup Paths
# SCRIPT_DIR is where makeheaders.c and this script live
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
# BASE_DIR is the root of your project
BASE_DIR=$(cd "$SCRIPT_DIR/.." && pwd)

# 2. Compile the tool
# Compiling it fresh ensures it's ready for your architecture
echo "Compiling mkhdr..."
gcc "$SCRIPT_DIR/makeheaders.c" -o "$SCRIPT_DIR/mkhdr"

# 3. Prepare target folder
mkdir -p "$BASE_DIR/headers"

# 4. Generate headers 1:1
echo "Generating individual headers for libs..."

# Find all .cpp files in libs and subfolders, excluding the root Kernel.cpp
find "$BASE_DIR/libs" -name "*.cpp" | while read -r file; do
    
    # Get just the filename (e.g., Keyboard.cpp)
    filename=$(basename "$file")
    # Change extension for the target (e.g., Keyboard.h)
    headername="${filename%.cpp}.h"
    
    # -plusplus: Silences the "C code" warnings and enables C++ support
    # -h: Writes header content to stdout
    # > : Redirects that content into our headers folder
    "$SCRIPT_DIR/mkhdr" -plusplus -h "$file" > "$BASE_DIR/headers/$headername"
    
    echo "  -> Generated: $headername"
done

# 5. Clean up the binary
if [ -f "$SCRIPT_DIR/mkhdr" ]; then
    rm "$SCRIPT_DIR/mkhdr"
fi

echo "------------------------------------------------"
echo "Success: Headers generated in $BASE_DIR/headers"