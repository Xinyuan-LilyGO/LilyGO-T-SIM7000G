#define TINY_GSM_MODEM_SIM800 // sim800 and sim7000 are very identical, no sim7000 reset tool yet but sim800 works

#include <TinyGsmClient.h>

// Define pins
#define MODEM_RST     5
#define MODEM_PWKEY   4
#define MODEM_DTR     25
#define MODEM_TX      27
#define MODEM_RX      26

// Set serial for debug console (to the Serial Monitor, speed 9600)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT Serial1

// Uncomment to see all AT commands, if wanted
// #define DUMP_AT_COMMANDS

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
  pinMode(MODEM_PWKEY, OUTPUT);
  digitalWrite(MODEM_PWKEY, HIGH);
  delay(10);
  digitalWrite(MODEM_PWKEY, LOW);
  delay(1010); //Ton 1sec
  digitalWrite(MODEM_PWKEY, HIGH);
  delay(4510);

  // Set baud rate
  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(6000);

  bool ret = modem.factoryDefault();

  SerialMon.println(F("\n***********************************************************"));
  SerialMon.print  (F(" Reset settings to Factory Default: "));
  SerialMon.println((ret) ? "OK" : "FAIL");
  SerialMon.println(F("***********************************************************"));
}

void loop() {

}
