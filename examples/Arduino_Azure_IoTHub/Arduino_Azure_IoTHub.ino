/*
// This code: connecting 5 accelerometers to esp32. 

The processed data is sent to IoT Hub over LTE Cat-M. 

LTE cat-M connection over TinyGSM
https://github.com/vshymanskyy/TinyGSM

Note: even though wifi doesn't actually connect:
 <WiFi.h> must be included
 WiFi.begin(); must be initiated 

Esp32's internal clock, tcp, other components and features are only initiated
when wifi is initiated.  

There are some ways to circumvent this by using libraries such as 
https://github.com/fbiego/ESP32Time/blob/main/examples/esp32_time/esp32_time.ino

to manually configure esp32's RTC, but there's so much more that would need to be
maunally configured.


>>> future fix needed: come up with a way to not use <WiFi.h> because it's huge...

For raw accl,gyro data, check:
this repo --> Esp32/Device/esp32_multiple_MPU6050_raw_data.ino

IoT Hub connection, and device provisioning, etc. :

SimpleMQTT_esp32_azure_iothub.ino
downloads/ubc_5G/sensor_networks_azure/

IoT Hub has to be Standard Tier, Basic Tier creates a lot of issues

Azure ESP32 IoT DevKit Get Started
https://docs.microsoft.com/en-us/samples/azure-samples/esp32-iot-devkit-get-started/sample/
*/

/**
   A simple Azure IoT example for sending telemetry to Iot Hub.

  original code from:
  https://github.com/VSChina/ESP32_AzureIoT_Arduino
  se
*/

// TCA9548A I2C Switch between:
// adafruit LSM9DS1 at address, on TCA9548's channel 0
// GY521-MPU 6050 at address, on TCA9548's channel 1
// TCA9548 module I2C address:  0x70
//

//======================================================
//I2C Mutiplexing using TCA9548A at I2C address 0x70
//======================================================

// GY-521 / mpu6050 on esp32
// connections: 5v, GND, SDA, SCL, on sensor to 5v, GND, SDA, SCL on esp32 respectively


//======================================================
//////////////// LTE-M connection ////////////////

#define TINY_GSM_MODEM_SIM7000

#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* ESP32 should sleep more seconds  (note SIM7000 needs ~20sec to turn off if sleep is activated) */

RTC_DATA_ATTR int bootCount = 0;   /* number of boots is saved even after esp is rebooted */
HardwareSerial SerialAT(1);

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
// rogers
const char apn[] = "ciot";
const char nbiot_apn[] = "ciot";

#define isNBIOT false

const char user[] = "";
const char pass[] = "";

// TTGO T-SIM pin definitions
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_DTR 25
#define MODEM_TX 26
#define MODEM_RX 27
//#define I2C_SDA 21
//#define I2C_SCL 22
#define reading_samles 100



#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif


//////////////// LTE-M connection ////////////////
//======================================================

#define USE_GSM  //! Uncomment will use SIM7000 for GSM communication

#ifdef USE_GSM
#include "Esp32MQTTClient.h"
// Initialize GSM client
TinyGsmClient client(modem);
#include <WiFi.h>
//=============================================================================
#else

#include <WiFi.h>
#include "Esp32MQTTClient.h"
const char *ssid = "";
const char *password = "";
TinyGsmClient client(modem);
#endif
//=============================================================================


//=============================================================================
//////////////////////////// Accelerometer ////////////////////
// Include Wire Library for I2C
#include <Wire.h>
// Amazing MPU6050 library by rfetick.
#include <MPU6050_light.h>
MPU6050 mpu(Wire);

const int MPU = 0x68;
int16_t roll1, pitch1, yaw1, roll2, pitch2, yaw2, roll3, pitch3, yaw3, roll4, pitch4, yaw4, roll5, pitch5, yaw5;
//////////////////////////// Accelerometer ////////////////////

void TCA9548A(uint8_t bus)
{
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}
//=============================================================================
//=============================================================================


///////////////////////////MQTT device to cloud initial message setup///////
/////////////////////////////////////////////////////



// Set to true, if modem is connected
bool modemConnected = false;

void shutdown();
void wait_till_ready();
void modem_off();

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

void modem_reset()
{
  Serial.println("Modem hardware reset");
  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, LOW);
  delay(260); //Treset 252ms
  digitalWrite(MODEM_RST, HIGH);
  delay(4000); //Modem takes longer to get ready and reply after this kind of reset vs power on

  //modem.factoryDefault();
  //modem.restart(); //this results in +CGREG: 0,0
}

void modem_on()
{
  // Set-up modem  power pin
  pinMode(MODEM_PWKEY, OUTPUT);
  digitalWrite(MODEM_PWKEY, HIGH);
  delay(10);
  digitalWrite(MODEM_PWKEY, LOW);
  delay(1010); //Ton 1sec
  digitalWrite(MODEM_PWKEY, HIGH);

  //wait_till_ready();
  Serial.println("Waiting till modem ready...");
  delay(4510); //Ton uart 4.5sec but seems to need ~7sec after hard (button) reset
  //On soft-reset serial replies immediately.

  SerialMon.println("Wait...");
  SerialAT.begin(115200, SERIAL_8N1, MODEM_TX, MODEM_RX);
  modem.setBaud(115200);
  modem.begin();
  delay(10000);

  if (!modem.restart()) {
    SerialMon.println(F(" [fail]"));
    SerialMon.println(F("************************"));
    SerialMon.println(F(" Is your modem connected properly?"));
    SerialMon.println(F(" Is your serial speed (baud rate) correct?"));
    SerialMon.println(F(" Is your modem powered on?"));
    SerialMon.println(F(" Do you use a good, stable power source?"));
    SerialMon.println(
      F(" Try useing File -> Examples -> TinyGSM -> tools -> AT_Debug to find correct configuration"));
    SerialMon.println(F("************************"));
    delay(10000);
    return;
  }
  SerialMon.println(F("Step 2: [OK] was able to open modem"));
  String modemInfo = modem.getModemInfo();
  SerialMon.println("Step 3: Modem details: ");
  SerialMon.println(modemInfo);

  SerialMon.println("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }
  SerialMon.print("Step 4: Waiting for network...");
  if (!modem.waitForNetwork(1200000L)) {
    SerialMon.println(F(" [fail] while waiting for network"));
    SerialMon.println(F("************************"));
    SerialMon.println(F(" Is your sim card locked?"));
    SerialMon.println(F(" Do you have a good signal?"));
    SerialMon.println(F(" Is antenna attached?"));
    SerialMon.println(F(" Does the SIM card work with your phone?"));
    SerialMon.println(F("************************"));
    delay(10000);
    return;
  }
  SerialMon.println(F("Found network: [OK]"));

  SerialMon.print("Step 5: About to set network mode to LTE Only 38: ");
  // Might not be needed for your carrier 
  modem.setNetworkMode(38);

  delay(3000);

  SerialMon.print("Step 6: About to set network mode: to CAT=M");
  // Might not be needed for your carrier 
  modem.setPreferredMode(3);
  delay(500);

      Serial.print(F("Waiting for network..."));
      if (!modem.waitForNetwork(60000L))
      {
        Serial.println(" fail");
        modem_reset();
        shutdown();
      }
      Serial.println(" OK");

      Serial.print("Signal quality:");
      Serial.println(modem.getSignalQuality());
  delay(3000);

  // GPRS connection parameters are usually set after network registration
  SerialMon.println("Step 7: Connecting to Rogers APN at LTE Mode Only (channel--> 38): ");
  SerialMon.println(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(F(" [fail]"));
    SerialMon.println(F("************************"));
    SerialMon.println(F(" Is GPRS enabled by network provider?"));
    SerialMon.println(F(" Try checking your card balance."));
    SerialMon.println(F("************************"));
    delay(10000);
    return;
  }

  if (modem.isGprsConnected()) {
    SerialMon.println(F("Step 8: Connected to network: [OK]"));
    IPAddress local = modem.localIP();
    SerialMon.print("Step 9: Local IP: ");
    SerialMon.println(local);
    modem.enableGPS();
    delay(3000);
    String IMEI = modem.getIMEI();
    SerialMon.print("Step 10: IMEI: ");
    SerialMon.println(IMEI);
  } else {
    SerialMon.println(F("Step 8: FAIL NOT Connected to network: "));
  }

   modemConnected = true;
   Serial.println("Modem Connected to Rogers' LTE (channel--> 38) CAT-M (preferred network). TLE CAT-M OK");
    
}

void modem_off()
{
  //if you turn modem off while activating the fancy sleep modes it takes ~20sec, else its immediate
  Serial.println("Going to sleep now with modem turned off");
  //modem.gprsDisconnect();
  //modem.radioOff();
  modem.sleepEnable(false); // required in case sleep was activated and will apply after reboot
  modem.poweroff();
}

// fancy low power mode - while connected
void modem_sleep() // will have an effect after reboot and will replace normal power down
{
  Serial.println("Going to sleep now with modem in power save mode");
  // needs reboot to activa and takes ~20sec to sleep
  //modem.PSM_mode();    //if network supports will enter a low power sleep PCM (9uA)
  //modem.eDRX_mode14(); // https://github.com/botletics/SIM7000-LTE-Shield/wiki/Current-Consumption#e-drx-mode
  modem.sleepEnable(); //will sleep (1.7mA), needs DTR or PWRKEY to wake
  pinMode(MODEM_DTR, OUTPUT);
  digitalWrite(MODEM_DTR, HIGH);
}

void modem_wake()
{
  Serial.println("Wake up modem from sleep");
  // DTR low to wake serial
  pinMode(MODEM_DTR, OUTPUT);
  digitalWrite(MODEM_DTR, LOW);
  delay(50);
  //wait_till_ready();
}

void shutdown()
{

  //modem_sleep();
  modem_off();

  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}

void wait_till_ready() // NOT WORKING - Attempt to minimize waiting time
{

  for (int8_t i = 0; i < 100; i++) //timeout 100 x 100ms = 10sec
  {
    if (modem.testAT())
    {
      //Serial.println("Wait time:%F sec\n", i/10));
      Serial.printf("Wait time: %d\n", i);
      break;
    }
    delay(100);
  }
}

//////////////// LTE-M connection ////////////////


//=============================================================================

#define INTERVAL 10 //10seconds ==1000 intervals sending messages; this is not actually used because esp boots regularly
#define MESSAGE_MAX_LEN 400

/// Primaty Connection String///
/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char *connectionString = 
//const char *messageData = "{\"messageId\":%d, \"Temperature\":%f, \"Humidity\":%f}";
const char *messageData = "{\"DeviceID\":\"GreenSpaceTree1\",\"TreeType\":\"Cedar\", \"Board\":\"Esp32\", \"GSM_mod\":\"sim7000g\", \"Connect\":\"CatM\" ,\"location\":\"fairview\" ,\"bootCount\":%d, \"roll1\":%d, \"pitch1\":%d, \"yaw1\":%d, \"roll2\":%d, \"pitch2\":%d, \"yaw2\":%d, \"roll3\":%d, \"pitch3\":%d, \"yaw3\":%d, \"roll4\":%d, \"pitch4\":%d, \"yaw4\":%d, \"roll5\":%d, \"pitch5\":%d, \"yaw5\":%d}";
static bool hasIoTHub = false;
static bool hasWifi = false;
int messageCount = 1;
static bool messageSending = true;
static uint64_t send_interval_ms;

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    Serial.println("Send Confirmation Callback finished.");
  }
}

static void MessageCallback(const char *payLoad, int size)
{
  Serial.println("Message callback:");
  Serial.println(payLoad);
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  // Display Twin message.
  Serial.println(temp);
  free(temp);
}

static int DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  LogInfo("Try to invoke method %s", methodName);
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;

  if (strcmp(methodName, "start") == 0)
  {
    LogInfo("Start sending MPU - DeviceID: Cedar data");
    messageSending = true;
  }
  else if (strcmp(methodName, "stop") == 0)
  {
    LogInfo("Stop sending MPU - DeviceID: Cedar data");
    messageSending = false;
  }
  else
  {
    LogInfo("No method %s found", methodName);
    responseMessage = "\"No method found\"";
    result = 404;
  }

  *response_size = strlen(responseMessage) + 1;
  *response = (unsigned char *)strdup(responseMessage);

  return result;
}





void setup()
{

  Serial.begin(115200);
  WiFi.begin();
  
  delay(10);
  Serial.println(F("Started"));
  Serial.println("ESP32 Device");
  Serial.println("Initializing...");

  
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");

  // if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
  // {
  //   modem_wake();
  // }
  // else
  {
    modem_on();
    Serial.println(" > Modem On --> True");
    //modem_wake();
    //modem_reset();
  }

      
    Serial.println(" > IoT Hub");
    if (!Esp32MQTTClient_Init((const uint8_t *)connectionString, true, true))
    {
      hasIoTHub = false;
      Serial.println("Initializing IoT hub failed.");
      return;
    }
    
    hasIoTHub = true;
    Serial.println(" > IoT Hub --> True");
    Esp32MQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);
    Serial.println(" > Esp32MQTTClient_SetSendConfirmation --> True");
    Esp32MQTTClient_SetMessageCallback(MessageCallback);
    Serial.println(" > Esp32MQTTClient_SetMessageCallback --> True");
    Esp32MQTTClient_SetDeviceTwinCallback(DeviceTwinCallback);
    Serial.println(" > Esp32MQTTClient_SetDeviceTwinCallback --> True");
    Esp32MQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);
    Serial.println(" > Esp32MQTTClient_SetDeviceMethodCallback --> True");
    
    Serial.println("Start sending events.");
    randomSeed(analogRead(0));

Wire.begin();
  TCA9548A(3);
  mpu.begin();
  Serial.println(F("Calculating gyro offset, do not move MPU6050"));
  mpu.calcGyroOffsets(); // This does the calibration
  delay(20);

  TCA9548A(4);
  mpu.begin();
  Serial.println(F("Calculating gyro offset, do not move MPU6050"));
  mpu.calcGyroOffsets();
  delay(20);

  TCA9548A(5);
  mpu.begin();
  Serial.println(F("Calculating gyro offset, do not move MPU6050"));
  mpu.calcGyroOffsets();
  delay(20);

  TCA9548A(6);
  mpu.begin();
  Serial.println(F("Calculating gyro offset, do not move MPU6050"));
  mpu.calcGyroOffsets();
  delay(20);

  TCA9548A(7);
  mpu.begin();
  Serial.println(F("Calculating gyro offset, do not move MPU6050"));
  mpu.calcGyroOffsets();
  delay(20);
    
    send_interval_ms = millis();   
}

void loop()
{

  delay(100);

Serial.print("modemConnected --> True");
modemConnected =true;
messageSending=true;
hasIoTHub=true;
Serial.println((int)(millis() - send_interval_ms));
if (modemConnected && hasIoTHub)
  {
    if (messageSending &&
        (int)(millis() - send_interval_ms) >= INTERVAL)
    {
      Serial.println("reading MPU sensor data");
      read_MPU_1();
      read_MPU_2();
      read_MPU_3();
      read_MPU_4();
      read_MPU_5();
      delay(2000);
      ////  MPU data //////

      char messagePayload[MESSAGE_MAX_LEN];

      snprintf(messagePayload, MESSAGE_MAX_LEN, messageData, bootCount, roll1, pitch1, yaw1, roll2, pitch2, yaw2, roll3, pitch3, yaw3, roll4, pitch4, yaw4, roll5, pitch5, yaw5);
      Serial.println(messagePayload);
      EVENT_INSTANCE *message = Esp32MQTTClient_Event_Generate(messagePayload, MESSAGE);
      Esp32MQTTClient_SendEventInstance(message);
      send_interval_ms = millis();
    }
    else
    {
      Esp32MQTTClient_Check();
    }
  }
  delay(5000);


  //delay(1000); // required - else will shutdown too early and miss last value
  shutdown();
}




void read_MPU_1()
{
  // get pitch, roll, yaw accelerometer data /////
  TCA9548A(3);
      mpu.update();
      pitch1 = mpu.getAngleX();
      roll1 = mpu.getAngleY();
      yaw1 = mpu.getAngleZ();
      Serial.print(roll1);
      Serial.print("/");
      Serial.print(pitch1);
      Serial.print("/");
      Serial.println(yaw1);
      delay(1000);
}

void read_MPU_2()
{
  TCA9548A(4);
      mpu.update();
       pitch2 = mpu.getAngleX();
       roll2 = mpu.getAngleY();
       yaw2 = mpu.getAngleZ();
      Serial.print("pitch2: ");
      Serial.println(pitch2);
      Serial.print("roll2: ");
      Serial.println(roll2);
      Serial.print("yaw2: ");
      Serial.println(yaw2);
      delay(1000);
  }

 void read_MPU_3()
{
  TCA9548A(5);
      mpu.update();
       pitch3 = mpu.getAngleX();
       roll3 = mpu.getAngleY();
       yaw3 = mpu.getAngleZ();
      Serial.print("pitch3: ");
      Serial.println(pitch3);
      Serial.print("roll3: ");
      Serial.println(roll3);
      Serial.print("yaw3: ");
      Serial.println(yaw3);
      delay(100);
}

void read_MPU_4()
{
TCA9548A(6);
      mpu.update();
       pitch4 = mpu.getAngleX();
       roll4 = mpu.getAngleY();
       yaw4 = mpu.getAngleZ();
      Serial.print("pitch4: ");
      Serial.println(pitch4);
      Serial.print("roll4: ");
      Serial.println(roll4);
      Serial.print("yaw4: ");
      Serial.println(yaw4);
      delay(100);
}

void read_MPU_5()
{
  TCA9548A(7);
      mpu.update();
       pitch5 = mpu.getAngleX();
       roll5 = mpu.getAngleY();
       yaw5 = mpu.getAngleZ();
      Serial.print("pitch5: ");
      Serial.println(pitch5);
      Serial.print("roll5: ");
      Serial.println(roll5);
      Serial.print("yaw5: ");
      Serial.println(yaw5);

      delay(2000);
}
