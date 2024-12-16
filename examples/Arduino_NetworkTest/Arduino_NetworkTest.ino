/**
 * @file      Arduino_NetworkTest.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-02-01
 * @note      This example function is the SIM7000/SIM7070 network test to
 *            determine whether the module can access the network and obtain some access parameters
 */

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#define UART_BAUD           9600
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13
#define LED_PIN             12

void enableGPS(void)
{
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power HIGH Failed");
    }
    modem.enableGPS();
}

void disableGPS(void)
{
    // Set Modem GPS Power Control Pin to LOW ,turn off GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power LOW Failed");
    }
    modem.disableGPS();
}

void modemPowerOn()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1000);    //Datasheet Ton mintues = 1S
    digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1500);    //Datasheet Ton mintues = 1.2S
    digitalWrite(PWR_PIN, HIGH);
}


void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);

    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    modemPowerOn();

    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);
    if (!SD.begin(SD_CS)) {
        Serial.println("> It looks like you haven't inserted the SD card..");
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        String str = "> SDCard Size: " + String(cardSize) + "MB";
        Serial.println(str);
    }

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    Serial.println("> Check whether Modem is online");
    //test modem is online ?
    uint32_t  timeout = millis();
    while (!modem.testAT()) {
        Serial.print(".");
        if (millis() - timeout > 60000 ) {
            Serial.println("> It looks like the modem is not responding, trying to restart");
            modemPowerOff();
            delay(5000);
            modemPowerOn();
            timeout = millis();
        }
    }
    Serial.println("\nModem is online");

    //test sim card is online ?
    timeout = millis();
    Serial.print("> Get SIM card status");
    while (modem.getSimStatus() != SIM_READY) {
        Serial.print(".");
        if (millis() - timeout > 60000 ) {
            Serial.println("It seems that your SIM card has not been detected. Has it been inserted?");
            Serial.println("If you have inserted the SIM card, please remove the power supply again and try again!");
            return;
        }

    }
    Serial.println();
    Serial.println("> SIM card exists");


    Serial.println("> /**********************************************************/");
    Serial.println("> Please make sure that the location has 2G/NB-IOT signal");
    Serial.println("> SIM7000/SIM707G does not support 4G network. Please ensure that the USIM card you use supports 2G/NB access");
    Serial.println("> /**********************************************************/");

    String res = modem.getIMEI();
    Serial.print("IMEI:");
    Serial.println(res);
    Serial.println();

    /*
    * Tips:
    * When you are not sure which method of network access is supported by the network you use,
    * please use the automatic mode. If you are sure, please change the parameters to speed up the network access
    * * * * */

    //Set mobile operation band
    modem.sendAT("+CBAND=ALL_MODE");
    modem.waitResponse();

    // Args:
    // 1 CAT-M
    // 2 NB-IoT
    // 3 CAT-M and NB-IoT
    // Set network preferre to auto
    uint8_t perferred = 3;
    modem.setPreferredMode(perferred);

    if (perferred == 2) {
        Serial.println("When you select 2, please ensure that your SIM card operator supports NB-IOT");
    }

    // Args:
    // 2 Automatic
    // 13 GSM only
    // 38 LTE only
    // 51 GSM and LTE only
    // Set network mode to auto
    modem.setNetworkMode(2);


    // Check network signal and registration information
    Serial.println("> SIM7000/SIM7070 uses automatic mode to access the network. The access speed may be slow. Please wait patiently");
    SIM70xxRegStatus status;
    timeout = millis();
    do {
        int16_t sq =  modem.getSignalQuality();

        status = modem.getRegistrationStatus();

        if (status == REG_DENIED) {
            Serial.println("> The SIM card you use has been rejected by the network operator. Please check that the card you use is not bound to a device!");
            return;
        } else {
            Serial.print("Signal:");
            Serial.println(sq);
        }

        if (millis() - timeout > 360000 ) {
            if (sq == 99) {
                Serial.println("> It seems that there is no signal. Please check whether the"\
                               "LTE antenna is connected. Please make sure that the location has 2G/NB-IOT signal\n"\
                               "SIM7000G does not support 4G network. Please ensure that the USIM card you use supports 2G/NB access");
                return;
            }
            timeout = millis();
        }

        delay(800);
    } while (status != REG_OK_HOME && status != REG_OK_ROAMING);

    Serial.println("Obtain the APN issued by the network");
    modem.sendAT("+CGNAPN");
    if (modem.waitResponse(3000, res) == 1) {
        res = res.substring(res.indexOf(",") + 1);
        res.replace("\"", "");
        res.replace("\r", "");
        res.replace("\n", "");
        res.replace("OK", "");
        Serial.print("The APN issued by the network is:");
        Serial.println(res);
    }

    modem.sendAT("+CNACT=1");
    modem.waitResponse();


    // res = modem.getLocalIP();
    modem.sendAT("+CNACT?");
    if (modem.waitResponse("+CNACT: ") == 1) {
        modem.stream.read();
        modem.stream.read();
        res = modem.stream.readStringUntil('\n');
        res.replace("\"", "");
        res.replace("\r", "");
        res.replace("\n", "");
        modem.waitResponse();
        Serial.print("The current network IP address is:");
        Serial.println(res);
    }


    modem.sendAT("+CPSI?");
    if (modem.waitResponse("+CPSI: ") == 1) {
        res = modem.stream.readStringUntil('\n');
        res.replace("\r", "");
        res.replace("\n", "");
        modem.waitResponse();
        Serial.print("The current network parameter is:");
        Serial.println(res);
    }


    Serial.println("/**********************************************************/");
    Serial.println("After the network test is complete, please enter the  ");
    Serial.println("AT command in the serial terminal.");
    Serial.println("/**********************************************************/\n\n");

}


uint32_t check_timestamp = 0;

void loop()
{   
    // AT Debug
    // while (SerialAT.available()) {
    //     SerialMon.write(SerialAT.read());
    // }
    // while (SerialMon.available()) {
    //     SerialAT.write(SerialMon.read());
    // }
    
    if (millis() > check_timestamp) {
        Serial.print("[");
        Serial.print(millis() / 1000);
        Serial.print("]:");
        int16_t sq = modem.getSignalQuality();
        SIM70xxRegStatus s = modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING) {
            Serial.println("Devices lost network connect!");
        } else {
            modem.sendAT("+CPSI?");
            if (modem.waitResponse("+CPSI: ") == 1) {
                String res = modem.stream.readStringUntil('\n');
                res.replace("\r", "");
                res.replace("\n", "");
                modem.waitResponse();
                Serial.print("Network is connected. Signal Quality:");
                Serial.println(sq);
                Serial.print("The current network parameter is:");
                Serial.println(res);
            } else {
                Serial.println("get params failed!");
            }
        }
        check_timestamp = millis() + 5000;
    }


}
