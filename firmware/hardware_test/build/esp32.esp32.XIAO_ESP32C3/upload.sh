#!/bin/bash
#
# POSIX-compliant upload script for the ESP32-C3.
#
# Usage:
#   ./upload.sh <SERIAL_PORT>
#
# Example:
#   ./upload.sh /dev/ttyUSB0
#

# --- Configuration ---
set -e # Exit immediately if a command exits with a non-zero status.
APP_NAME="hardware_test"
ESPTOOL="esptool.py" # Assumes esptool.py is in the system's PATH

# Check if a serial port was provided
if [ -z "$1" ]; then
    echo "Error: No serial port provided."
    echo "Usage: $0 <SERIAL_PORT>"
    exit 1
fi

echo "--> Uploading application: $APP_NAME to port $1"

# --- ESPTool Command ---
"$ESPTOOL" \
    --chip esp32c3 \
    --port "$1" \
    --baud 921600 \
    --before default_reset \
    --after hard_reset write_flash \
    -z --flash_mode keep --flash_freq keep --flash_size keep \
    0x0 "$APP_NAME.ino.bootloader.bin" \
    0x8000 "$APP_NAME.ino.partitions.bin" \
    0xe000 "boot_app0.bin" \
    0x10000 "$APP_NAME.ino.bin"

echo "--> Upload complete for $APP_NAME." 