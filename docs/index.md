# CoinThing

- [What's New](#whats-new)
- [Manual](#manual)
- [Update The Software](#update-the-software)
- [Available Versions To Download](#available-versions-to-download)

-------

## What's New

### Version 1.3 is ready!

-------

## Manual
### Setup WiFi
First, the CoinThing has to know the access data for your WLAN router.
To get the data, CoinThing first starts itself as a WLAN access point. You can connect to this via smartphone or PC.

When the CoinThing is powered for the first time, a QR code appears that represents the connection data to the access point.

The easiest way to connect is to scan this code via a smartphone camera. Once you are connected to the access point, a configuration portal (WiFiManager) opens. Enter the access data for your WLAN router there. These are the SSID (available SSIDs are listed) and the corresponding password.

After saving, the CoinThing starts again and tries to connect to the WLAN with the given credentials. If successful, again a QR code appears, this time for the settings.

### Settings
If you open the address of this QR code (the assigned IP address of the CoinThing), the settings page will open in the browser. 
You can bookmark this page to easily access the settings again later.
This QR code is only displayed if no settings have been made yet. However, the settings can always be accessed via this IP address.

In the settings, you enter all desired data. After clicking Apply, the settings are checked and taken over and displayed by the CoinThing.

-------

## Update The Software
### Update Procedure
#### Update Firmware Via Browser (Settings Page)
- download one of the firmware binary files below
- from the settings page, go to about (link in the footer)
- click on _Update_, then on _Really?_
- CoinThing reboots in update mode
- the page changes to http:://&lt;ip&gt;/update (corresponding to the shown QR code)
- select the downloaded firmware file and click on update button
- CoinThing reboots and shows its update message
- that's it – enjoy :-)

#### Update Firmware Via Python Script
If there is an update with a change in the second version number (i.e. from 1.__1__.xx to 1.__2__.xx), things are a bit more complicated :-).

##### You need
- a PC or Mac
- python installed: [download python](https://www.python.org/downloads/)
- this python module installed: [esptool](https://github.com/espressif/esptool)
- the CoinThing connected to the PC via an USB data cable
- typically under windows, the correct USB drivers will be installed automtically. If not, you need to install the USB drivers, you can find the right ones here under downloads: <https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers>

##### Is python installed?
```bash
> python --version
```
Should return something like ```Python 3.9.4```

##### This is how to install esptool
```bash
> pip install esptool
```

##### Is the esptool installed?
```bash
> python -m esptool version
```
Should return something like
```
esptool.py v3.0
3.0
```

##### Is the CoinThing connected correctly?
```bash
> python -m esptool --no-stub chip_id
```
Should return something like
```
esptool.py v3.0
Serial port COM3
Connecting....
Detecting chip type... ESP8266
Chip is ESP8266EX
Features: WiFi
Crystal is 26MHz
MAC: 50:02:91:58:03:37
Enabling default SPI flash mode...
Chip ID: 0x00580337
Hard resetting via RTS pin...
```

##### Download the SPIFFS file e.g. __spiffs_v1.2.xx.bin__ and execute
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 460800 write_flash 0x200000 <path_to_download>/spiffs_v1.2.xx.bin
```
##### ATTENTION! For version 1.3 the spiffs upload address has changed to 0x100000!
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 460800 write_flash 0x100000 <path_to_download>/spiffs_v1.3.0.bin
```


After this was successful, you have to install the matching firmware itself.<br>

##### Download the firmware file e.g. __cointhing_v1.2.xx.bin__ and execute
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.2.xx.bin 
```

##### Or both together
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.2.xx.bin 0x200000 <path_to_download>/spiffs_v1.2.xx.bin
```
##### ATTENTION! For version 1.3 the spiffs upload address has changed to 0x100000!
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.3.0.bin 0x100000 <path_to_download>/spiffs_v1.3.0.bin
```

##### If the CoinThing does not startup after update attempts
You can erase the CoinThing completely (you cannot damage it).
After that install the firmware again.
Unfortunately after this a new setup ist necessary.
```bash
python -m esptool erase_flash
```

That's all.<br>
After a reboot, the CoinThing starts with the new version – enjoy!

-------

## Available Versions To Download

### Version 1.3.0:
- Added ExtraCondensed font
- Added new icons
- Increased CPU clock

For an update to Version 1.3 you need to upload both files via python as described above.

[cointhing_v1.3.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.3.0/cointhing_v1.3.0.bin)

[spiffs_v1.3.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.3.0/spiffs_v1.3.0.bin)

#### ATTENTION!
##### The upload address for the spiffs file has changed to 0x100000 for version 1.3!
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.3.0.bin 0x100000 <path_to_download>/spiffs_v1.3.0.bin
```

-------

### Version 1.2.0:
#### New functionality With Version 1.2
- Show 2 coins simultaneously on the screen (no chart)
- Cycle through up to 10 coins (with chart)
- New cypto coin icons

For an update to Version 1.2 you need to upload both files via python as described above.

[cointhing_v1.2.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.2.0/cointhing_v1.2.0.bin)

[spiffs_v1.2.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.2.0/spiffs_v1.2.0.bin)

-------

### Version 1.1.0:
The second currency was always USD, it is now also configurable in the settings.

For an update from Version 1.0, please also upload the spiffs_v1.1.0.bin file as described above to the CoinThing.

[cointhing_v1.1.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.1.0/cointhing_v1.1.0.bin)

[spiffs_v1.1.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.1.0/spiffs_v1.1.0.bin)

-------

### Version 1.0.3:

[cointhing_v1.0.3.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.3/cointhing_v1.0.3.bin)

[spiffs_v1.0.3.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.3/spiffs_v1.0.3.bin)

### Version 1.0.2:

[cointhing_v1.0.2.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.2/cointhing_v1.0.2.bin)

### Version 1.0.1:

[cointhing_v1.0.1.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.1/cointhing_v1.0.1.bin)

### Version 1.0.0:

[cointhing_v1.0.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.0/cointhing_v1.0.0.bin)

