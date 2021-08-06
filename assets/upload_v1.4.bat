python -m esptool erase_flash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 921600 write_flash 0x0 cointhing_v1.4.2.bin 0x200000 spiffs_v1.4.0.bin

pio device monitor -b 115200
