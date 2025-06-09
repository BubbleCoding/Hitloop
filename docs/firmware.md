# Flashing the Firmware

This guide provides instructions for flashing the pre-compiled firmware binaries onto the ESP32-C3 scanner and beacon controller hardware.

## Prerequisites

Before you begin, ensure you have the following tools installed on your system:

1.  **ESPTool:** This is the official command-line utility for communicating with the ESP32 ROM bootloader. It is a Python-based tool. If you have Python installed, you can install it via pip:
    ```bash
    pip install esptool
    ```

2.  **Serial Port Driver:** Ensure you have the necessary USB-to-UART bridge driver installed for your hardware. For the XIAO ESP32-C3, you may need the CH340 driver, depending on your operating system.

## Flashing Instructions

The process involves using a script to upload the four necessary binary files (`bootloader`, `partitions`, `boot_app0`, and the main application) to the ESP32-C3 at specific memory addresses.

### 1. Identify the Serial Port

First, connect the ESP32-C3 device to your computer via USB. You must identify the serial port it has been assigned.

*   **Windows:** Open the Device Manager. Look under the "Ports (COM & LPT)" section. Your device will likely appear as `COM3`, `COM4`, etc.

*   **Linux:** Open a terminal and run the `dmesg | grep tty` command after plugging in the device. It will typically appear as `/dev/ttyUSB0` or `/dev/ttyACM0`.

*   **macOS:** Open a terminal and run `ls /dev/tty.*`. It will likely be listed as `/dev/tty.usbserial-XXXXXXXX`.

### 2. Run the Upload Script

Navigate to the correct `build` directory for the firmware you wish to upload (`firmware/Scanner/build/...` or `firmware/hardware_test/build/...`).

*   **Windows (Command Prompt or PowerShell):**

    Use the `upload.bat` script, passing the COM port you identified as an argument.

    ```powershell
    # Example for COM3
    ./upload.bat COM3
    ```

*   **Linux / macOS (Bash/Zsh):**

    Use the `upload.sh` script. You may need to make it executable first (`chmod +x upload.sh`).

    ```bash
    # Example for /dev/ttyUSB0
    ./upload.sh /dev/ttyUSB0
    ```

### 3. Verification

Once the script starts, `esptool.py` will connect to the device and begin writing the binaries. If successful, you will see output similar to this, followed by the device rebooting:

```
esptool.py v4.5.1
Serial port /dev/ttyUSB0
Connecting...
Chip is ESP32-C3 (revision v0.3)
...
Writing at 0x00010000... (100 %)
Wrote 1572016 bytes at 0x00010000 in 16.7s...
...
Hard resetting via RTS pin...
```

The device is now flashed with the new firmware. You can open a serial monitor at baud rate `115200` to view its output. 