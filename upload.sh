#!/usr/bin/env bash
set -e

# ECE3849 Upload Script for TM4C1294 (Tiva C Series)
# Usage: ./upload.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/build/lab0.bin"

if [[ ! -f "$BIN" ]]; then
    echo "Error: Binary not found at $BIN"
    echo "Run ./build.sh first"
    exit 1
fi

echo "Uploading $BIN to TM4C1294..."

# Use nix-shell for lm4flash on NixOS, fall back to system lm4flash
if command -v nix-shell &> /dev/null; then
    nix-shell -p lm4flash --run "lm4flash '$BIN'"
elif command -v lm4flash &> /dev/null; then
    lm4flash "$BIN"
else
    echo "Error: lm4flash not found"
    echo "On NixOS: nix-shell -p lm4flash"
    echo "On Ubuntu/Debian: sudo apt install lm4flash"
    exit 1
fi

echo "Upload complete!"
