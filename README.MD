<h1 align = "center">LilyGo SIM7000G</h1>

Two main versions of the LilyGo SIM7000G have been released with the current board having better power management than the old board.
### Version 20200415

   This board is an improvement of the original with fixes added that where recommended from feedback.
1. Added active GPS antenna power control, when the module GPIO4 is not turned on the antenna consumes only the static current of the LDO.
2. Replaced TP4056 with CN3065 for solar charge input management.
3. Added reverse battery protection
4. Added battery overcharge protection
5. Added battery over-discharge protection

 ### Version 20191227
 
   First board release
   
<h2 align = "left">Product 📷:</h2>

|                                    Examples                                     |                                  Product  Link                                  |                     Schematic                      | Status |
| :-----------------------------------------------------------------------------: | :-----------------------------------------------------------------------------: | :------------------------------------------------: | :----: |
| [Version 20200415 (点击查看)](./Historical/SIM7000G_20200415/README.MD) | [Product link](https://www.aliexpress.com/item/4000542688096.html) | [New Version](./schematic/SIM7000G_20200415.pdf) |   ✅    |
| [Version 20191227 (点击查看)](./Historical/SIM7000G_20191227/README.MD) |                                    Obsolete                                     | [Old Version](./schematic/SIM7000G_20191227.pdf) |   ❌    |



## Notes for new Users

1. pin VIN is a USB input, if a battery is being used to power the device there will be no voltage output from VIN meaning 3.3v is the only power output.

2. When using the built-in battery pack the on/off switch will function as normal, supplying/cutting off power to the board, However, if an external battery pack is used and attached to the VBAT pin the on/off switch will be bypassed meaning the only way to shut off will be to disconnect the batteries.

3. On/off switch is for battery use only, it will not function when plugged into USB.

4. Battery voltage can not be read when plugged into USB using the onboard BAT ADC(35) 

5. Recommended solar panels are 4.4v to 6v **DO NOT EXCEED OR BOARD MAY BE DAMAGED** 

6. When an SD card in you can not upload software in Arduino IDE since the SD card uses IO2 as CS, the SD card must be removed when uploading a new sketch. 

7. On Version 20200415, When using GPS the sim7000 GPIO4 needs to be set to HIGH to enable the antenna `modem.sendAT("+SGPIO=0,4,1,1");` once finished the pin can be set LOW `modem.sendAT("+SGPIO=0,4,1,0");` to prevent power leaking to the GPS antenna, This will make help battery life. If using USB or battery life is not a concern, it can be left HIGH.



