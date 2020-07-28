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

void setup() {
  // Set console and modem baud rate
  Serial.begin(9600);
  SerialAT.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(500);

  // Set-up modem  power pin
  Serial.println("\nStarting Up Modem...");
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);
  delay(10000);                 //Wait for the SIM7000 communication to be normal, and will quit when receiving OK

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

void loop() {

}
