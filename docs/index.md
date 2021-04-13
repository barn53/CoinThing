# CoinThing

# Table Of Contents

- [Software Updates](#software-updates)
- [Available Versions](#available-versions)


## Software Updates

### Update Procedure

#### Update Firmware Via Browser (Settings Sage)
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
```python
python -m esptool --before default_reset --after hard_reset --chip esp8266
       --baud 460800 write_flash 0x0 <path_to_download>/spiffs_v1.1.xx.bin
       <-- flash address is wrong!
```

After this was successful, you have to install the matching firmware itself.

Either via browser as described above, or now you have learned something new, you can also install it with the python script :-).
Download the firmware e.g. __cointhing_v1.1.xx.bin__ and execute:
```python
python -m esptool --before default_reset --after hard_reset --chip esp8266
       --baud 460800 write_flash 0x0 <path_to_download>/cointhing_v1.1.xx.bin 
```

## Available Versions

### Version 1.0.3:

[cointhing_v1.0.3.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.3/cointhing_v1.0.3.bin)

### Version 1.0.2:

[cointhing_v1.0.2.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.2/cointhing_v1.0.2.bin)

### Version 1.0.1:

[cointhing_v1.0.1.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.1/cointhing_v1.0.1.bin)

### Version 1.0.0:

[cointhing_v1.0.0.bin](https://github.com/barn53/CoinThing/releases/download/v1.0.0/cointhing_v1.0.0.bin)

