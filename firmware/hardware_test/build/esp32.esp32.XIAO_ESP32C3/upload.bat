rem Usage: upload.bat COM Port
rem Replace the location of the ESPTool.exe for your specific configuration
set APP_NAME=hardware_test
set PATH=%PATH%;C:\Users\bpijls\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.9.dev3\

esptool.exe ^
--chip esp32c3 ^
--port %1 ^
--baud 921600 ^
--before default_reset ^
--after hard_reset write_flash ^
-z --flash_mode keep --flash_freq keep --flash_size keep ^
0x0 "%APP_NAME%.ino.bootloader.bin" ^
0x8000 "%APP_NAME%.ino.partitions.bin" ^
0xe000 "boot_app0.bin" ^
0x10000 "%APP_NAME%.ino.bin"