import os
import shutil
from datetime import datetime

# 1. Setup Local Target
flder = datetime.now().strftime("%Y%m%d_%H%M%S")
local_target = f"Backups/{flder}"
mkdir -p @(local_target)

# 2. Setup External Drive Target
# This searches for a mounted drive.
external_root = "/Volumes"
external_target = None

if os.path.exists(external_root):
    for drive in os.listdir(external_root):
        # We look for a drive that isn't the system drive (Macintosh HD)
        if drive == "Macintosh HD": continue
        
        # We'll create a folder on the first external drive we find
        external_path = os.path.join(external_root, drive, "OS_Backups", flder)
        external_target = external_path
        break

# 3. Perform Copy
# Skip folders that are too large (OVMF) or redundant (Backups, .git)
items_to_backup = [i for i in os.listdir(".") if i not in ["Backups", ".git", "OVMF"]]

for i in items_to_backup:
    if os.path.isdir(i):
        cp -r @(i) @(local_target)
        if external_target:
            mkdir -p @(external_target)
            cp -r @(i) @(external_target)
    else:
        cp @(i) @(local_target)
        if external_target:
            mkdir -p @(external_target)
            cp @(i) @(external_target) 

print(f"✅ Local Backup: {local_target}")
if external_target:
    print(f"✅ External Backup: {external_target}")
else:
    print("⚠️ External drive not found in /Volumes/")