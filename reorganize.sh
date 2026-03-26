#!/usr/bin/env bash
set -e

# Reorganize project to flat structure
cd "$(dirname "$0")"

echo "Creating new directory structure..."
mkdir -p src libs linker build

echo "Moving main source files..."
mv src/lab_0_Workspace/lab0/main.cpp src/
mv src/lab_0_Workspace/lab0/startup_ccs.c src/

echo "Moving linker script..."
mv src/lab_0_Workspace/lab0/blinky_ccs.cmd linker/tm4c1294.cmd

echo "Moving libraries..."
mv src/libraries/* libs/

echo "Applying updated scripts..."
mv build.sh.new build.sh
mv upload.sh.new upload.sh
mv compile_flags.txt.new compile_flags.txt
chmod +x build.sh upload.sh monitor.sh

echo "Cleaning up old CCS workspace cruft..."
rm -rf src/lab_0_Workspace
rm -rf src/libraries

echo ""
echo "Done! New structure:"
echo "  src/        - main.cpp, startup_ccs.c"
echo "  libs/       - HAL_TM4C1294, buttonsDriver, display, etc."
echo "  linker/     - tm4c1294.cmd"
echo "  build/      - output (.obj, .out, .bin)"
echo ""
echo "You can delete this script: rm reorganize.sh"
