/**
 * @file      Serial2Example.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-02-04
 * The example mainly shows how to use the second serial port
 *
 */
#include <Arduino.h>

#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define LED_PIN     12


void begin()
{

    /*Please note that the pins used need not be connected to other peripherals by hardware.
    Above 33 can only be used as input. IO12 and IO0 are not recommended*/
    uint8_t rxPin = 34;
    uint8_t txPin = 33;

    // For debug
    Serial.begin(115200);

    //Hardware connected to modem
    Serial1.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);

    // Note that you need to exchange RX and TX pins on the module
    Serial2.begin(115200, SERIAL_8N1, rxPin, txPin);

}

void loop()
{
    while (Serial.available()) {
        Serial1.write(Serial.read());
    }
    //uart 1
    while (Serial1.available()) {
        Serial.write(Serial1.read());
    }
    //uart 2
    while (Serial2.available()) {
        Serial.write(Serial2.read());
    }
}