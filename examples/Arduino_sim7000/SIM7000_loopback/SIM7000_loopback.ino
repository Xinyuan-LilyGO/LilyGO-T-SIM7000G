#include "DFRobot_SIM7000.h"

#define PIN_TX      27
#define PIN_RX      26
#define UART_BAUD   115200
#define PWR_PIN     4

DFRobot_SIM7000    sim7000;
HardwareSerial  mySerial(1);

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    mySerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);

    sim7000.begin(mySerial);

    Serial.println("Turn ON SIM7000......");

    if (sim7000.turnON(PWR_PIN)) {                           //Turn ON SIM7000
        Serial.println("Turn ON !");
    }

    Serial.println("For example, if you type AT\\r\\n, OK\\r\\n will be responsed!");
    Serial.println("Enter your AT command :");
}

void loop()
{
    while (mySerial.available()) {
        Serial.write(mySerial.read());
    }
    while (Serial.available()) {
        mySerial.write(Serial.read());
    }
}