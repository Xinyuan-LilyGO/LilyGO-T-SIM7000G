#include "DFRobot_SIM7000.h"

#define PIN_TX      27
#define PIN_RX      26
#define UART_BAUD   115200
#define PWR_PIN     4

HardwareSerial  mySerial(1);
DFRobot_SIM7000    sim7000;
static char        buff[350];

void setup()
{
    int signalStrength, dataNum;
    Serial.begin(UART_BAUD);

    mySerial.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    sim7000.begin(mySerial);


    Serial.println("Turn ON SIM7000......");
    if (sim7000.turnON(PWR_PIN)) {                                                                    //Turn ON SIM7000
        Serial.println("Turn ON !");
    }
    
    Serial.println("Check SIM card......");
    if (sim7000.checkSIMStatus()) {                                                            //Check SIM card
        Serial.println("SIM card READY");
    } else {
        Serial.println("SIM card ERROR");
        while (1);
    }
    delay(500);
    Serial.println("Set net mod......");
    if (sim7000.setNetMode(GPRS)) {                                                                  //Set net mod NB-IOT
        Serial.println("Set NB mode");
    } else {
        Serial.println("Fail to set mode");
    }
    Serial.println("Get signal quality......");
    delay(500);

    signalStrength = sim7000.checkSignalQuality();                                             //Check signal quality from (0-30)
    Serial.print("signalStrength =");
    Serial.println(signalStrength);
    delay(500);

    Serial.println("Attaching service......");
    if (sim7000.attacthService()) {                                                            //Open the connection
        Serial.println("Attach service");
    } else {
        Serial.println("Fail to Attach service");
        while (1);
    }

    Serial.println("Connecting......");
    if (sim7000.openNetwork(TCP, "www.taobao.com", 80)) {                                          //Start Up TCP or UDP Connection
        Serial.println("Connect OK");
    } else {
        Serial.println("Fail to connect");
        while (1);
    }
    sim7000.send("HEAD/HTTP/1.1\r\nHost:www.taobao.com\r\nConnection:keep-alive\r\n\r\n");     //Send Data Through TCP or UDP Connection
    dataNum = sim7000.recv(buff, 350, 0);                                                      //Receive data
    Serial.print("dataNum=");
    Serial.println(dataNum);
    Serial.println(buff);
    delay(500);
    if (sim7000.closeNetwork()) {                                                                     //End the connection
        Serial.println("Close connection");
    } else {
        Serial.println("Fail to close connection");
    }
    delay(2000);
    sim7000.turnOFF();                                                                         //Turn OFF SIM7000
}

void loop()
{
    delay(1000);
}