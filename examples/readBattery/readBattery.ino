/**
 * @file      readBattery.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-03-14
 *
 */
#include <esp_adc_cal.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#define ADC_PIN     35


// WiFi network name and password:
const char *networkName = "your-ssid";
const char *networkPswd = "your-password";

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char *udpAddress = "192.168.36.11";
const int udpPort = 3333;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

int vref = 1100;
uint32_t timeStamp = 0;


void connectToWiFi(const char *ssid, const char *pwd)
{
    Serial.println("Connecting to WiFi network: " + String(ssid));

    // delete old config
    WiFi.disconnect(true);
    //register event handler
    WiFi.onEvent(WiFiEvent);

    //Initiate connection
    WiFi.begin(ssid, pwd);

    Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event)
{
    switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        //When connected set
        Serial.print("WiFi connected! IP address: ");
        Serial.println(WiFi.localIP());
        //initializes the UDP state
        //This initializes the transfer buffer
        udp.begin(WiFi.localIP(), udpPort);
        connected = true;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        connected = false;
        break;
    default: break;
    }
}


void setup()
{
    Serial.begin(115200); // Set console baud rate
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.println("Default Vref: 1100mV");
    }

    //Connect to the WiFi network
    connectToWiFi(networkName, networkPswd);

}

void loop()
{
    //only send data when connected
    if (connected) {
        if (millis() - timeStamp > 1000) {
            timeStamp = millis();
            uint16_t v = analogRead(ADC_PIN);
            float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
            String voltage = "Voltage :" + String(battery_voltage) + "V\n";

            // When connecting USB, the battery detection will return 0,
            // because the adc detection circuit is disconnected when connecting USB
            Serial.println(voltage);
            if (voltage == "0.00") {
                Serial.println("USB is connected, please disconnect USB.");
            }

            //Send a packet
            udp.beginPacket(udpAddress, udpPort);
            udp.printf(voltage.c_str());
            udp.endPacket();
        }

    }
}
