/*
  FILE: ATdebug.ino
  AUTHOR: Koby Hale
  PURPOSE: test AT commands
  List of SIM7000 AT commands can be found here
  http://www.microchip.ua/simcom/LTE/SIM7000/SIM7000%20Series_AT%20Command%20Manual_V1.05.pdf
*/

#define SerialAT Serial1

#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

bool reply = false;

void modem_on() {
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
}

void setup() {
  Serial.begin(9600); // Set console baud rate
  SerialAT.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
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

void loop() {
  while (true) {
    if (SerialAT.available()) {
      Serial.write(SerialAT.read());
    }
    if (Serial.available()) {
      SerialAT.write(Serial.read());
    }
    delay(1);
  }
  Serial.println("Failed");
  setup();
}
