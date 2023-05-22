/*
  FILE: ResetModem.ino
  AUTHOR: Koby Hale
  PURPOSE: reset the SIM7000
*/

#define TINY_GSM_MODEM_SIM800 // sim800 and sim7000 are very identical, no sim7000 reset tool yet but sim800 works

#include <TinyGsmClient.h>

// Define pins
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

// Set serial for AT commands (to the module)
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>    // if enabled it requires the streamDebugger lib
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif


bool reply = false;

uint32_t TinyGsmAutoBaud()
{
    static uint32_t rates[] = {115200, 57600,  38400, 19200, 9600,  74400, 74880,
                               230400, 460800, 2400,  4800,  14400, 28800
                              };

    for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
        uint32_t rate = rates[i];
        if (rate < 9600 || rate > 115200) continue;

        DBG("Trying baud rate", rate, "...");
        SerialAT.updateBaudRate(rate);
        delay(10);
        for (int j = 0; j < 10; j++) {
            SerialAT.print("AT\r\n");
            String input = SerialAT.readString();
            if (input.indexOf("OK") >= 0) {
                DBG("Modem responded at rate", rate);
                return rate;
            }
        }
    }
    return 0;
}

void setup()
{
    // Set console and modem baud rate
    Serial.begin(9600);


    // Set-up modem  power pin
    Serial.println("\nStarting Up Modem...");
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    // Starting the machine requires at least 1 second of low level, and with a level conversion, the levels are opposite
    delay(1000);
    digitalWrite(PWR_PIN, LOW);
    delay(10000);                 //Wait for the SIM7000 communication to be normal, and will quit when receiving OK


    SerialAT.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);


    if (TinyGsmAutoBaud() == 0) {
        Serial.println("Failed start modem ! Hardware is not respone!");
        while (1)delay(500);
    }


    int i = 10;
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

    if (reply) {
        bool ret = modem.factoryDefault();
        Serial.println(F("\n***********************************************************"));
        Serial.print  (F(" Reset settings to Factory Default: "));
        Serial.println((ret) ? "OK" : "FAIL");
        Serial.println(F("***********************************************************"));
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
