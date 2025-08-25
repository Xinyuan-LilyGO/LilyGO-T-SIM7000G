#define SerialMon Serial

// Set serial for AT commands (to the module)`
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS
String FINALLATI="0",FINALLOGI="0",FINALSPEED="0";
// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "YOUR_APN";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "your Traccar ip or url";
const int port = 5055; // your traccar port
String myid = "Your Traccar id ";

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>


#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
HttpClient http(client, server, port);

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 60 // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD 115200
#define PIN_DTR 25
#define PIN_TX 27
#define PIN_RX 26
#define PWR_PIN 4
#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13
#define LED_PIN 12

void modemPowerOn()
{
pinMode(PWR_PIN, OUTPUT);
digitalWrite(PWR_PIN, LOW);
delay(1000); //Datasheet Ton mintues = 1S
digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff()
{
pinMode(PWR_PIN, OUTPUT);
digitalWrite(PWR_PIN, LOW);
delay(1500); //Datasheet Ton mintues = 1.2S
digitalWrite(PWR_PIN, HIGH);
}

void modemRestart()
{
modemPowerOff();
delay(1000);
modemPowerOn();
}

void enableGPS(void)
{

Serial.println("Start positioning . Make sure to locate outdoors.");
Serial.println("The blue indicator light flashes to indicate positioning.");
modem.sendAT("+SGPIO=0,4,1,1");
if (modem.waitResponse(10000L) != 1)
{
DBG(" SGPIO=0,4,1,1 false ");
}
modem.enableGPS();
}

void disableGPS(void)
{
modem.sendAT("+SGPIO=0,4,1,0");
if (modem.waitResponse(10000L) != 1)
{
DBG(" SGPIO=0,4,1,0 false ");
}
modem.disableGPS();
}

void sendCoords(float lat, float lon)
{


String SerialData="";
      SerialData = String(lat,6);
      String SerialData1="";
      SerialData1 = String(lon,6);
      FINALLATI=SerialData;
      FINALLOGI=SerialData1;




int err = http.post("/?id="+myid+"&lat="+FINALLATI+"&lon="+FINALLOGI+"");
if (err != 0)
{
SerialMon.println(F("failed to connect"));
delay(10000);
return;
}

int status = http.responseStatusCode();

if (!status)
{
delay(10000);
return;
}

String body = http.responseBody();
SerialMon.println(F("Response:"));
SerialMon.println(body);

// Shutdown
http.stop();
SerialMon.println(F("Server disconnected bye bye will connect soon"));
}

void setup()
{
// Set console baud rate
SerialMon.begin(115200);

delay(10);

// Set LED OFF
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH);

SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

delay(10000);

modemPowerOn();

Serial.println("Initializing modem...");
if (!modem.restart())
{
Serial.println("Failed to restart modem, attempting to continue without restarting");
}

// Unlock your SIM card with a PIN if needed
if (GSM_PIN && modem.getSimStatus() != 3)
{
modem.simUnlock(GSM_PIN);
}
}

void loop()
{

// GPRS connection parameters are usually set after network registration
SerialMon.print(F("Connecting to "));
SerialMon.print(apn);
if (!modem.gprsConnect(apn, gprsUser, gprsPass))
{
SerialMon.println(" fail");
delay(30000);
return;
}
SerialMon.println(" success");

if (modem.isGprsConnected())
{
SerialMon.println("GPRS connected");
}

enableGPS();

float lat, lon;
while (1)
{
if (modem.getGPS(&lat, &lon))
{
sendCoords(lat, lon);
}
digitalWrite(LED_PIN, !digitalRead(LED_PIN));
delay(30000);
}
}
