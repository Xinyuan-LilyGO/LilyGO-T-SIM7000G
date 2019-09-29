#include "DFRobot_SIM7000.h"

#define PIN_TX      27
#define PIN_RX      26
#define UART_BAUD   115200
#define PWR_PIN     4


DFRobot_SIM7000    sim7000;
HardwareSerial  mySerial(1);

void setup()
{
    int signalStrength, dataNum;
    Serial.begin(115200);
    while (!Serial);

    mySerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);

    sim7000.begin(mySerial);
    Serial.println("Getting position......");

    Serial.println("Turn ON SIM7000......");
    if (sim7000.turnON(PWR_PIN)) {                                        //Turn ON SIM7000
        Serial.println("Turn ON !");
    }

    Serial.println("Init positioning function......");
    while (1) {
        if (sim7000.initPos()) {
            Serial.println("Positioning function initialized");
            break;
        } else {
            Serial.println("Fail to init positioning function");
            delay(1000);
        }
    }
}

void loop()
{
    Serial.println("Getting position......");
    if (sim7000.getPosition()) {                                   //Get the current position
        Serial.print(" Longitude : ");
        Serial.println(sim7000.getLongitude());                    //Get longitude
        Serial.print(" Latitude : ");
        Serial.println(sim7000.getLatitude());                     //Get latitude
    } else {
        Serial.println("Wrong data try again");
    }
}
