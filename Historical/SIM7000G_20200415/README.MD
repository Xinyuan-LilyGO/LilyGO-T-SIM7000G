<h1 align = "center">LilyGO T-SIM7000G 20200415</h1>


### PinOut

![](./pins.jpg)

### Dimensions
![](./dimensions.png)


## Examples
- Arduino + TinyGSM library: [examples/Arduino_TinyGSM](../../examples/Arduino_TinyGSM/)
- Platformio_Arduino_Thingsboard :[/exaples/Platformio_Arduino_Thingsboard](../../exaples/Platformio_Arduino_Thingsboard)


## SIM7000G connection
| Name           | Pins  |
| -------------- | ----- |
| Modem DTR      | 25    |
| Modem Rx       | 26    |
| Modem Tx       | 27    |
| Modem PowerKey | 4     |
| Modem GPS      | GPIO4 |
| BAT ADC        | 35    |
| SOLAR ADC      | 36    |
| SD MISO        | 2     |
| SD MOSI        | 15    |
| SD CLK         | 14    |
| SD CS          | 13    |

**Modem GPS** is the internal GPIO of the SIM7000G module. When using GPS, GPIO4 needs to be set to high level, as shown below:

```
// Set SIM7000G GPIO4 HIGH ,Open GPS power
modem.sendAT("+SGPIO=0,4,1,1");

// Set SIM7000G GPIO4 LOW ,Close GPS power
modem.sendAT("+SGPIO=0,4,1,0");
```


