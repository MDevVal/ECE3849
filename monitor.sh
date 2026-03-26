#!/usr/bin/env bash

# ECE3849 Serial Monitor for TM4C1294 (Tiva C Series)
# Usage: ./monitor.sh [device] [baudrate]
# Example: ./monitor.sh /dev/ttyACM0 115200

DEVICE="${1:-/dev/ttyACM0}"
BAUD="${2:-115200}"

# Find the device if not specified
if [[ ! -e "$DEVICE" ]]; then
    # Try common device names
    for dev in /dev/ttyACM0 /dev/ttyACM1 /dev/ttyUSB0 /dev/ttyUSB1; do
        if [[ -e "$dev" ]]; then
            DEVICE="$dev"
            break
        fi
    done
fi

if [[ ! -e "$DEVICE" ]]; then
    echo "Error: No serial device found"
    echo "Available devices:"
    ls -la /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo "  None found"
    echo ""
    echo "Make sure the board is connected and you have permissions:"
    echo "  sudo usermod -a -G dialout \$USER"
    echo "  (then log out and back in)"
    exit 1
fi

echo "Connecting to $DEVICE at $BAUD baud..."
echo "Press Ctrl+A then Ctrl+X to exit (minicom) or Ctrl+A then K (screen)"
echo ""

# Try different terminal programs
if command -v nix-shell &> /dev/null; then
    # NixOS: use nix-shell
    nix-shell -p minicom --run "minicom -D '$DEVICE' -b $BAUD"
elif command -v minicom &> /dev/null; then
    minicom -D "$DEVICE" -b "$BAUD"
elif command -v screen &> /dev/null; then
    screen "$DEVICE" "$BAUD"
elif command -v picocom &> /dev/null; then
    picocom -b "$BAUD" "$DEVICE"
else
    echo "Error: No serial terminal found"
    echo "Install one of: minicom, screen, picocom"
    echo "On NixOS: nix-shell -p minicom"
    exit 1
fi
