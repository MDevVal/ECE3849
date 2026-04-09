#!/usr/bin/env bash

# ECE3849 Serial Monitor for TM4C1294 (Tiva C Series)
# Usage: ./monitor.sh [device] [baudrate]
# Example: ./monitor.sh /dev/ttyACM0 115200

DEVICE="${1:-/dev/ttyACM2}"
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
echo "Press Ctrl+C to exit"
echo ""

while true; do
    if [[ -e "$DEVICE" ]]; then
        stty -F "$DEVICE" "$BAUD" cs8 -cstopb -parenb -crtscts -ixon -ixoff raw -echo 2>/dev/null
        cat "$DEVICE" 2>/dev/null
        echo "[disconnected]"
    fi
    sleep 1
done
