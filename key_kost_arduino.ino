/*
PINOUT:
RC522 MODULE    Uno/Nano     MEGA
SDA             D10          D9
SCK             D13          D52
MOSI            D11          D51
MISO            D12          D50
IRQ             N/A          N/A
GND             GND          GND
RST             D9           D8
3.3V            3.3V         3.3V
*/
/* Include the standard Arduino SPI library */
#include <SPI.h>
/* Include the RFID library */
#include <RFID.h>
//#include <String.h>

#include "ESP8266.h"

#define SSID        "Wifi Kita"
#define PASSWORD    "kospj2014season2"
#define HOST_NAME   "ditoraharjo.co"
#define HOST_PORT   (80)

/* Define the DIO used for the SDA (SS) and RST (reset) pins. */
#define SDA_DIO 11
#define RESET_DIO 10
#define TAP 1
#define GA_NGETAP 0
#define BELUM_BUKA 0
#define SUDAH_BUKA 1

/* RFID LIST */
#define RFID_1 "22914384190128"
#define RFID_2 "481532092597"

/* Create an instance of the RFID library */
RFID RC522(SDA_DIO, RESET_DIO); 
int statusPintu;
int isOpen;
ESP8266 wifi(Serial1);

void setup()
{ 
  Serial.begin(9600);
  setWifi();
  //Selenoid
  pinMode(8,OUTPUT);

  statusPintu = GA_NGETAP;
  isOpen = BELUM_BUKA;
  
  //Sound
  pinMode(12,OUTPUT);
  
  //Sensor
  pinMode(4,INPUT);
  
  /* Enable the SPI interface */
  SPI.begin(); 
  /* Initialise the RFID reader */
  RC522.init();
}

void loop()
{
  //mematikan selenoid diawal
  digitalWrite(8,HIGH);
//  Serial.println(statusPintu);

  
  //LOW = Pintu Ketutup
  if(digitalRead(4)==LOW){
      statusPintu=GA_NGETAP;
    }
    
  //HIGH = Pintu Kebuka
  if(digitalRead(4)==HIGH){
      if(statusPintu==GA_NGETAP){
          soundOn();
        }      
  }
  
  
  /* Has a card been detected? */
  if (RC522.isCard())
  {    
    int tempLength=0;
    String idParsing="";
    /* If so then get its serial number */
    isOpen=SUDAH_BUKA;
    statusPintu=TAP;
    
    RC522.readCardSerial();
    Serial.println("Card detected:");
    for(int i=0;i<5;i++)
    {
      Serial.println(RC522.serNum[i]);
      tempLength+=strlen(RC522.serNum[i]);
      idParsing+=RC522.serNum[i];      
    }
    Serial.println(tempLength);
    char *temp = new char[40]{'\0'};
    Serial.println(idParsing);
    strcpy(temp,authRFID(idParsing));
    Serial.println(temp);
    
    if(strcmp(temp,"NO_AUTH")!=0){
      soundOn();
      digitalWrite(8,LOW);
      delay(1500);
      digitalWrite(8,HIGH);  
      setWifi();
      sendData(temp);
    }
    else{
      digitalWrite(12,HIGH);
      delay(500);
      digitalWrite(12,LOW);
      delay(100);
      digitalWrite(12,HIGH);
      delay(500);
      digitalWrite(12,LOW);
      delay(100);
      digitalWrite(12,HIGH);
      delay(500);
      digitalWrite(12,LOW);
      setWifi();
      sendData(temp);
    }

  }
  delay(500);
} 

void soundOn(){
  digitalWrite(12,HIGH);
  delay(500);
  digitalWrite(12,LOW);
}

void setWifi(){
    Serial1.begin(115200);
    
    Serial.print("setup begin\r\n");

    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");

        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");  
}

void sendData(char* idRFID){
    uint8_t buffer[1024] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    char *hello1 = "GET /keykost/api/v1/log?rfid_tag=";
    char *hello3 = " HTTP/1.1\r\nHost: ditoraharjo.co\r\nConnection: close\r\n\r\n";

    char *hello = new char[strlen(hello1)+strlen(idRFID)+strlen(hello3)] {'\0'};

    strcat(hello,hello1);
    strcat(hello,idRFID);
    strcat(hello,hello3);  
    
//    Serial.println(hello);
    wifi.send((const uint8_t*)hello, strlen(hello));
    
     uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }

    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }
    
    Serial.println("Data sudah terkirim");
}

char* authRFID(String rfidID){
    if(rfidID==RFID_1){
        Serial.println("Sukses authentikasi");
        return "RFID_1";
      }
    else if(rfidID==RFID_2){
        Serial.println("Sukses authentikasi RFID 2");
        return "RFID_2";
      }
    Serial.println("Gagal authentikasi");
    return "NO_AUTH";
  }
