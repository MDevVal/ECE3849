#!/usr/bin/env bash
set -e

# ECE3849 Build Script for TM4C1294 (Tiva C Series)
# Usage: ./build.sh [clean]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Paths
SRC="$SCRIPT_DIR/src"
LIBS="$SCRIPT_DIR/libs"
LINKER="$SCRIPT_DIR/linker"
BUILD="$SCRIPT_DIR/build"
TIVA="$HOME/ti/sw-tm4c-2.2.0.295"
CCS="$HOME/ti/ccs1281/ccs"
CC="$CCS/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl"
CCSLIB="$CCS/tools/compiler/ti-cgt-arm_20.2.7.LTS/lib"
CCSINC="$CCS/tools/compiler/ti-cgt-arm_20.2.7.LTS/include"

# Check if clean requested
if [[ "$1" == "clean" ]]; then
    echo "Cleaning build artifacts..."
    rm -rf "$BUILD"/*.obj "$BUILD"/*.out "$BUILD"/*.bin "$BUILD"/*.map 2>/dev/null || true
    echo "Clean complete"
    exit 0
fi

# Verify paths exist
if [[ ! -f "$CC" ]]; then
    echo "Error: TI compiler not found at $CC"
    echo "Please install CCS to ~/ti/ccs1281/"
    exit 1
fi

if [[ ! -d "$TIVA" ]]; then
    echo "Error: TivaWare not found at $TIVA"
    echo "Please install TivaWare to ~/ti/sw-tm4c-2.2.0.295/"
    exit 1
fi

mkdir -p "$BUILD"

# Compiler flags
CFLAGS="-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O1"
CFLAGS="$CFLAGS --include_path=$TIVA"
CFLAGS="$CFLAGS --include_path=$SRC"
CFLAGS="$CFLAGS --include_path=$LIBS/HAL_TM4C1294"
CFLAGS="$CFLAGS --include_path=$LIBS/buttonsDriver"
CFLAGS="$CFLAGS --include_path=$LIBS/display"
CFLAGS="$CFLAGS --include_path=$LIBS/elapsedTime"
CFLAGS="$CFLAGS --include_path=$LIBS/joystickDriver"
CFLAGS="$CFLAGS --include_path=$LIBS/pll"
CFLAGS="$CFLAGS --include_path=$LIBS/timerLib"
CFLAGS="$CFLAGS --include_path=$LIBS/FreeRTOS"
CFLAGS="$CFLAGS --include_path=$LIBS/FreeRTOS/FreeRTOS/include"
CFLAGS="$CFLAGS --include_path=$LIBS/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F"
CFLAGS="$CFLAGS --include_path=$CCSINC"
CFLAGS="$CFLAGS --define=ccs --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA1"
CFLAGS="$CFLAGS -g --gcc --diag_warning=225 --abi=eabi"

echo "Building..."

# Compile source files
compile() {
    local src="$1"
    local obj="$2"
    echo "  Compiling $(basename "$src")..."
    "$CC" $CFLAGS -c "$src" --output_file="$obj"
}

compile "$SRC/main.cpp" "$BUILD/main.obj"
compile "$SRC/startup_ccs.c" "$BUILD/startup_ccs.obj"
compile "$LIBS/pll/sysctl_pll.c" "$BUILD/sysctl_pll.obj"
compile "$LIBS/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.c" "$BUILD/HAL_display.obj"
compile "$LIBS/display/Crystalfontz128x128_ST7735.c" "$BUILD/Crystalfontz.obj"
compile "$LIBS/timerLib/timerLib.cpp" "$BUILD/timerLib.obj"
compile "$LIBS/buttonsDriver/button.cpp" "$BUILD/button.obj"
compile "$LIBS/joystickDriver/joystick.cpp" "$BUILD/joystick.obj"

# FreeRTOS kernel sources
compile "$LIBS/FreeRTOS/FreeRTOS/tasks.c" "$BUILD/freertos_tasks.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/queue.c" "$BUILD/freertos_queue.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/list.c" "$BUILD/freertos_list.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/timers.c" "$BUILD/freertos_timers.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/event_groups.c" "$BUILD/freertos_event_groups.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/stream_buffer.c" "$BUILD/freertos_stream_buffer.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F/port.c" "$BUILD/freertos_port.obj"
compile "$LIBS/FreeRTOS/FreeRTOS/portable/MemMang/heap_4.c" "$BUILD/freertos_heap.obj"

# FreeRTOS port assembly
echo "  Compiling portasm.asm..."
"$CC" $CFLAGS -c "$LIBS/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F/portasm.asm" --output_file="$BUILD/freertos_portasm.obj"

echo "  Linking..."
"$CC" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O1 \
    --define=ccs --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA1 \
    -g --gcc --diag_warning=225 --abi=eabi \
    -z -m"$BUILD/lab0.map" --heap_size=0 --stack_size=2048 \
    -i"$TIVA/driverlib/ccs/Debug" \
    -i"$CCSLIB" \
    -i"$CCSINC" \
    --reread_libs --rom_model \
    -o "$BUILD/lab0.out" \
    "$BUILD/main.obj" \
    "$BUILD/startup_ccs.obj" \
    "$BUILD/sysctl_pll.obj" \
    "$BUILD/HAL_display.obj" \
    "$BUILD/Crystalfontz.obj" \
    "$BUILD/timerLib.obj" \
    "$BUILD/button.obj" \
    "$BUILD/joystick.obj" \
    "$BUILD/freertos_tasks.obj" \
    "$BUILD/freertos_queue.obj" \
    "$BUILD/freertos_list.obj" \
    "$BUILD/freertos_timers.obj" \
    "$BUILD/freertos_event_groups.obj" \
    "$BUILD/freertos_stream_buffer.obj" \
    "$BUILD/freertos_port.obj" \
    "$BUILD/freertos_heap.obj" \
    "$BUILD/freertos_portasm.obj" \
    "$LINKER/tm4c1294.cmd" \
    -l"$TIVA/driverlib/ccs/Debug/driverlib.lib" \
    -l"$TIVA/grlib/ccs/Debug/grlib.lib" \
    -llibc.a

echo "  Creating binary..."
# Use nix-shell for objcopy on NixOS, fall back to system objcopy
if command -v nix-shell &> /dev/null; then
    nix-shell -p gcc-arm-embedded --run "arm-none-eabi-objcopy -O binary '$BUILD/lab0.out' '$BUILD/lab0.bin'"
elif command -v arm-none-eabi-objcopy &> /dev/null; then
    arm-none-eabi-objcopy -O binary "$BUILD/lab0.out" "$BUILD/lab0.bin"
else
    echo "Warning: Could not create .bin file (no objcopy found)"
    echo "The .out file can still be used with some debuggers"
fi

echo ""
echo "Build complete!"
ls -lh "$BUILD/lab0.out" "$BUILD/lab0.bin" 2>/dev/null || ls -lh "$BUILD/lab0.out"
