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

// Set serial for debug console (to the Serial Monitor, speed 9600)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT Serial1

// See all AT commands, if wanted
 #define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(9600);
  delay(10);

  // Start GSM module
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  // Set baud rate
  SerialAT.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
  
  //Wait for the SIM7000 communication to be normal, and will quit when receiving any byte
  int i = 10;
  delay(5000);
  Serial.println("\nTesting Modem Response\n");
  Serial.println("****");
  while (i) {
    SerialAT.println("AT");
    delay(500);
    if (SerialAT.available()) {
        String r = SerialAT.readString();
        Serial.println(r);
        if ( r.indexOf("OK") >= 0 ) break;;
    }
    delay(500);
    i--;
  }
  Serial.println("****\n");

  bool ret = modem.factoryDefault();

  SerialMon.println(F("\n***********************************************************"));
  SerialMon.print  (F(" Reset settings to Factory Default: "));
  SerialMon.println((ret) ? "OK" : "FAIL");
  SerialMon.println(F("***********************************************************"));
}

void loop() {

}
