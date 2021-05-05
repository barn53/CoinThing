# CoinThing

- [Manual](#manual)
- [Update The Software](#update-the-software)
- [Available Versions To Download](#available-versions-to-download)

-------

## Manual
### Setup WiFi
First, the CoinThing have to know the access data for your WLAN router.
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
If there is an update with a change in the second version number (i.e. from 1.__0__.xx to 1.__1__.xx), things are a bit more complicated :-).
You first have to download the __spiffs_v1.1.xx.bin__ and install it with the following procedure:

##### You Need
- python installed: [download python](https://www.python.org/downloads/)
- this python script installed: [esptool](https://github.com/espressif/esptool)

This is the command line to execute:
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266
       --baud 460800 write_flash 0x200000 <path_to_download>/spiffs_v1.1.xx.bin
```

After this was successful, you have to install the matching firmware itself.

Either via browser as described above, or now you have learned something new, you can also install it with the python script :-).
Download the firmware e.g. __cointhing_v1.1.xx.bin__ and execute:
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266
       --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.1.xx.bin 
```

Or both together:
```bash
python -m esptool --before default_reset --after hard_reset --chip esp8266
       --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.1.xx.bin 0x200000 <path_to_download>/spiffs_v1.1.xx.bin
```

-------

## Available Versions To Download

### Version 1.1.0:
The second currency was always USD, it is now also configurable in the settings.

For an update from Version 1.0, please also upload the spiffs_v1.1.0.bin file as described above to the CoinThing.

[cointhing_v1.1.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.1.0/cointhing_v1.1.0.bin)

[spiffs_v1.1.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.1.0/spiffs_v1.1.0.bin)

-------

### Version 1.0.3:

[cointhing_v1.0.3.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.3/cointhing_v1.0.3.bin)

### Version 1.0.2:

[cointhing_v1.0.2.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.2/cointhing_v1.0.2.bin)

### Version 1.0.1:

[cointhing_v1.0.1.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.1/cointhing_v1.0.1.bin)

### Version 1.0.0:

[cointhing_v1.0.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.0/cointhing_v1.0.0.bin)

