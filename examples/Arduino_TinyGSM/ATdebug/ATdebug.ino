/*
  FILE: ATdebug.ino
  AUTHOR: Koby Hale
  PURPOSE: test AT commands
  List of SIM7000 AT commands can be found here
  http://www.microchip.ua/simcom/LTE/SIM7000/SIM7000%20Series_AT%20Command%20Manual_V1.05.pdf
*/
#include <Arduino.h>

#define SerialAT Serial1

#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

bool reply = false;

uint32_t dectModemBaud()
{
    static uint32_t rates[] = {115200, 57600,  38400, 19200, 9600,  74400, 74880,
                               230400, 460800, 2400,  4800,  14400, 28800
                              };

    for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
        uint32_t rate = rates[i];
        Serial.printf("[INFO]:Trying baud rate:%d\n", rate);
        SerialAT.updateBaudRate(rate);
        delay(10);
        for (int j = 0; j < 10; j++) {
            SerialAT.print("AT\r\n");
            String input = SerialAT.readString();
            if (input.indexOf("OK") >= 0) {
                Serial.printf("[INFO]:Modem responded at rate:%d\n", rate);
                SerialAT.println("AT+IPREX=115200");
                return rate;
            }
        }
    }
    SerialAT.updateBaudRate(115200);
    Serial.println("[ERROR]:Modem is not online!!!");
    return 0;
}


void modem_on()
{
    // Set-up modem  power pin
    Serial.println("\nStarting Up Modem...");
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    // Starting the machine requires at least 1 second of low level, and with a level conversion, the levels are opposite
    delay(1000);
    digitalWrite(PWR_PIN, LOW);
    delay(10000);                 //Wait for the SIM7000 communication to be normal, and will quit when receiving OK

    int i = 10;

    if(dectModemBaud() == 0){
        Serial.println("Unable to communicate with modem.");while(1);
    }

    Serial.println("\nTesting Modem Response...\n");
    Serial.println("****");
    while (i) {
        SerialAT.println("AT");
        delay(500);
        if (SerialAT.available()) {
            String r = SerialAT.readString();
            Serial.println(r);
            if ( r.indexOf("OK") >= 0 ) {
                reply = true;
                break;;
            }
        }
        delay(500);
        i--;
    }
    Serial.println("****\n");
}

void setup()
{
    Serial.begin(115200); // Set console baud rate
    SerialAT.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
    delay(100);

    modem_on();
    if (reply) {
        Serial.println(F("***********************************************************"));
        Serial.println(F(" You can now send AT commands"));
        Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
        Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
        Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
        Serial.println(F(" can have undesired consiquinces..."));
        Serial.println(F("***********************************************************\n"));
    } else {
        Serial.println(F("***********************************************************"));
        Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
        Serial.println(F("***********************************************************\n"));
    }
}

void loop()
{
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }
    delay(1);
}
