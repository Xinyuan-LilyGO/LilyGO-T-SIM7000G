/*
Battery test T-SIM7000 & Thingsboard
 */

#define TINY_GSM_MODEM_SIM7000

#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define DUMP_AT_COMMANDS

#include "TinyGsmClient.h"
#include "ThingsBoard.h"

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* ESP32 should sleep more seconds  (note SIM7000 needs ~20sec to turn off if sleep is activated) */
RTC_DATA_ATTR int bootCount = 0;

HardwareSerial serialGsm(1);
#define SerialAT serialGsm
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// cosmote
const char apn[] = "internet";
const char nbiot_apn[] = "iot";

// 1nce
// const char apn[] = "iot.1nce.net";
// const char nbiot_apn[] = "iot.1nce.net";

#define isNBIOT true

const char user[] = "";
const char pass[] = "";

// TTGO T-SIM pin definitions
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_DTR 25

#define MODEM_TX 26
#define MODEM_RX 27

#define I2C_SDA 21
#define I2C_SCL 22

#define PIN_ADC_BAT 35
#define PIN_ADC_SOLAR 36
#define ADC_BATTERY_LEVEL_SAMPLES 100

#ifdef DUMP_AT_COMMANDS
#include "StreamDebugger.h"
StreamDebugger debugger(serialGsm, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(serialGsm);
#endif

#define TOKEN "xxxxxxxxxxxxxxxxxxxx"     // thingsboard token
#define THINGSBOARD_SERVER "192.168.1.1" //thingsboard server

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD 115200

// Initialize GSM client
TinyGsmClient client(modem);

// Initialize ThingsBoard instance
ThingsBoard tb(client);

// Set to true, if modem is connected
bool modemConnected = false;

void read_adc_bat(uint16_t *voltage);
void read_adc_solar(uint16_t *voltage);
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
  modem.PSM_mode();    //if network supports will enter a low power sleep PCM (9uA)
  modem.eDRX_mode14(); // https://github.com/botletics/SIM7000-LTE-Shield/wiki/Current-Consumption#e-drx-mode
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

void setup()
{
  // Set console baud rate
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(10);
  Serial.println(F("Started"));

  pinMode(PIN_ADC_BAT, INPUT);
  pinMode(PIN_ADC_SOLAR, INPUT);

  uint16_t v_bat = 0;
  uint16_t v_solar = 0;

  // while (1)
  // {
  read_adc_bat(&v_bat);
  Serial.print("BAT: ");
  Serial.print(v_bat);

  read_adc_solar(&v_solar);
  Serial.print(" SOLAR: ");
  Serial.println(v_solar);

  //   delay(1000);
  // }

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
    //modem_wake();
    //modem_reset();
  }

  // Set GSM module baud rate and UART pins
  SerialAT.begin(9600, SERIAL_8N1, MODEM_TX, MODEM_RX); //reversing them
  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem: "));
  Serial.println(modemInfo);

  if (!modemConnected)   
  {

    //SIM7000

    if (isNBIOT)
    {
      Serial.println("configuring NBIoT mode");
      modem.setPreferredMode(38);
      modem.setPreferredLTEMode(2);
      modem.setOperatingBand(20); // Required for cosmote Greece

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

      Serial.print(F("Connecting to apn:"));
      Serial.println(nbiot_apn);
      if (!modem.gprsConnect(nbiot_apn, user, pass))
      {
        Serial.println(" failed");
        modem_reset();
        shutdown();
      }

      modemConnected = true;
      Serial.println(" OK");
    }
    else
    {
      Serial.println("configuring GSM mode"); // AUTO or GSM ONLY

      modem.setPreferredMode(13); //2 Auto // 13 GSM only // 38 LTE only

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

      Serial.print(F("Connecting to "));
      Serial.print(apn);
      if (!modem.gprsConnect(apn, user, pass))
      {
        Serial.println(" failed");
        modem_reset();
        shutdown();
      }

      modemConnected = true;
      Serial.println(" GSM OK");
    }
  }

  if (!tb.connected())
  {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN))
    {
      Serial.println("Failed to connect");
      modem_reset();
      shutdown();
    }
  }

  Serial.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
  // for more details

  if (tb.sendAttributeBool("beat", true))
    ;
  Serial.println("send beat");

  if (tb.sendTelemetryInt("bootCount", bootCount))
    Serial.println("bootCount send");

  if (tb.sendTelemetryInt("d_gsm_CSQ", modem.getSignalQuality())) //CSQ need to convert to RSSI
    Serial.println("d_gsm_CSQ send");

  read_adc_bat(&v_bat);
  if (tb.sendTelemetryInt("d_bat", v_bat))
    Serial.print("read_adc_bat: ");
  Serial.print(v_bat);
  Serial.println(" send");

  read_adc_solar(&v_solar);
  if (tb.sendTelemetryInt("d_solar", v_solar))
    Serial.print("read_adc_solar: ");
  Serial.print(v_solar);
  Serial.println(" send");

  delay(1000); // required - else will shutdown too early and miss last value
  shutdown();
}

void loop()
{
}

void read_adc_bat(uint16_t *voltage)
{
  uint32_t in = 0;
  for (int i = 0; i < ADC_BATTERY_LEVEL_SAMPLES; i++)
  {
    in += (uint32_t)analogRead(PIN_ADC_BAT);
  }
  in = (int)in / ADC_BATTERY_LEVEL_SAMPLES;

  uint16_t bat_mv = ((float)in / 4096) * 3600 * 2;

  *voltage = bat_mv;
}

void read_adc_solar(uint16_t *voltage)
{
  uint32_t in = 0;
  for (int i = 0; i < ADC_BATTERY_LEVEL_SAMPLES; i++)
  {
    in += (uint32_t)analogRead(PIN_ADC_SOLAR);
  }
  in = (int)in / ADC_BATTERY_LEVEL_SAMPLES;

  uint16_t bat_mv = ((float)in / 4096) * 3600 * 2;

  *voltage = bat_mv;
}
