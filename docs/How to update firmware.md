
## Quick Step

1. To solder the USB to the upgrade solder joint, you can choose a flying lead or directly solder the USB to the contact. 
2. Power on the SIM7000G board, and at the same time connect the USB interface to the computer port (please note that you need to insert the SIM card during the upgrade process) 
    ![](../image/16.png)
    ![](../image/17.png)
3. Download [SIM7000X Driver](https://1drv.ms/u/s!AmbpOqVezk5drS-ateuVXXDK1ulv?e=yc0kXz), and decompress the corresponding compressed package according to the system you are using. 
4. Open the computer device manager and follow the steps below to add the driver. 
    ![](../image/18.png)
    ![](../image/19.png)
    ![](../image/20.png)
    ![](../image/21.png)
    ![](../image/22.png)
    ![](../image/23.png)
    Follow the above steps to install the driver for the remaining ports that are not installed.
    ![](../image/24.png)

5. Unzip `SIM7000-Update-tools.7z`
6. Open `setup.exe` , install update tools
7. Follow the steps below to install 
    ![](../image/1.png)
    ![](../image/2.png)
    ![](../image/3.png)
    ![](../image/4.png)
    ![](../image/5.png)
    ![](../image/6.png)
    ![](../image/7.png)
    ![](../image/8.png)

8. Open the upgrade tool and follow the diagram below 

    ![](../image/9.png)
    ![](../image/10.png)
    ![](../image/11.png)
    ![](../image/12.png)
    ![](../image/13.png)
    ![](../image/14.png)
    ![](../image/15.png)
    ![](../image/25.png)
    ![](../image/26.png)
    Note that when the following figure is executed, it will prompt that the new port has no driver installed. After adding the driver according to the above steps to install the driver, click Start again to update. 
    ![](../image/27.png)

    When prompted `Update Success!`, click Stop to stop the update. At this point, the firmware update has been completed. 

    ![](../image/28.png)

9. Open the serial terminal tool, or the built-in serial tool of `Arduino IDE`, select `AT Port` for the port, and enter `AT+CGMR` to view the firmware version 
    ![](../image/29.png)


## Resources 
- [SIM7000G FW](https://1drv.ms/u/s!AmbpOqVezk5drSysFSgYu6bGRKy3?e=al2Ioh)
- [SIM7000X FlashTools](https://1drv.ms/u/s!AmbpOqVezk5drS1BascoH0_8EjTT?e=um3Q6A)
- [SIM7000X Dirver](https://1drv.ms/u/s!AmbpOqVezk5drS-ateuVXXDK1ulv?e=PN9QdC)


