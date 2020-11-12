/*
* Depend  https://github.com/sandeepmistry/arduino-LoRa
* */

#include <SPI.h>
#include <SD.h>
#include <LoRa.h>

#define PIN_TX              27
#define PIN_RX              26
#define UART_BAUD           115200
#define PWR_PIN             4

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#define LORA_RST            12
#define LORA_DI0            32
#define RADIO_DIO_1         33
#define RADIO_DIO_2         34
#define LORA_SS             5
#define LORA_MISO           19
#define LORA_MOSI           23
#define LORA_SCK            18

#define BAND                470E6

SPIClass SPI1(HSPI);

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    Serial.println("LoRa Receiver");

    // Specify pin to initialize SPI
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS)) {
        Serial.println("SDCard MOUNT FAIL");
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        String str = "SDCard Size: " + String(cardSize) + "MB";
        Serial.println(str);
    }

    // Specify pin to initialize SPI1
    SPI1.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setSPI(SPI1);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
    Serial.printf("Setup Lora freq : %.0f\n", BAND);
    if (!LoRa.begin(BAND)) {
        Serial.println("LORA Begin FAIL");
        while (1);
    }
    Serial.println("LORA Begin PASS");
}

void loop()
{
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // received a packet
        Serial.print("Received packet '");

        // read packet
        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
        }

        // print RSSI of packet
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
    }
}
