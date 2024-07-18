// Load Library
#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"

// Definisi pin untuk LoRa
#define LORA_SCK 5    // GPIO5  -- lora SCK
#define LORA_MISO 19  // GPIO19 -- lora MISO
#define LORA_MOSI 27  // GPIO27 -- lora MOSI
#define LORA_SS 18    // GPIO18 -- lora CS
#define LORA_RST 12   // GPIO12 -- RESET
#define LORA_DI0 26   // GPIO26 -- IRQ(Interrupt Request)
#define LORA_BAND 923E6
#define Node  "SDM"

// Definisi pin untuk OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

String induk;
long nilai;
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

// SPI untuk SD card
SPIClass sdSPI(VSPI);

// SPI untuk LoRa
SPIClass loraSPI(HSPI);

// Receive Package Variable
int RxDataRSSI = 0;
float RxDataSNR;
char Str1[15];
// Parsing Data
String myString;
int i;
String dt[10];
boolean parsing = false;
int simpanOn = 0;
long SDM;
long LINIA;
long LINIB;
long PUSYANTEK;
long MAKO;
long LOGAM;
long WTP;
long TASGANU;
long lastMsg;
long lastMsg1;
long lastMsg2;
int dis = 0;
// Definisi pin untuk antarmuka SPI
#define SD_CS   13   // CS pin untuk SD card module
#define SD_MOSI 15   // MOSI pin untuk SD card module
#define SD_SCK  14   // SCK pin untuk SD card module
#define SD_MISO 2    // MISO pin untuk SD card module

void setup() {
  // START aktivas Oled
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH);   // while OLED is running, must set GPIO16 in high、

  // INISIALISASI DISPLAY
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  // clear the display
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "SERVER ON");
  display.display();
  delay(1000);
  // aktivasi Oled END
    display.setFont(ArialMT_Plain_10);
  display.drawString(0, 18, "-> Starting LORA Module");
  display.display();
  // Nyalakan serial monitor
  Serial.begin(115200);
  while (!Serial);
  // Inisialisasi SPI untuk SD card
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Inisialisasi SPI untuk LoRa
  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  // Inisialisasi LoRa
  LoRa.setSPI(loraSPI);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  Serial.println("LoRa Receiver");
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
  delay(1000);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 28, "-> Starting SD card Module");
  display.display();

  // Inisialisasi SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS, sdSPI)){
    Serial.println("Initialization of SD card failed!");
    while (1);
    }
  Serial.println("SD card initialized.");

  // Register the receive callback
  LoRa.onReceive(onReceive);

  // Put the radio into receive mode
  LoRa.receive();
  delay(1000);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 38, "-> Get Data from SD card");
  display.display();
  SDM = bacaSDMDariSD();
  delay(100);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, "*");
  display.display();
  LINIA = bacaLINIADariSD();
  delay(100);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, "**");
  display.display();
  LINIB = bacaLINIBDariSD();
  delay(100);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, "***");
  display.display();
  PUSYANTEK = bacaPUSYANTEKDariSD();
  delay(100);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, "****");
  display.display();
  LOGAM = bacaLOGAMDariSD();
  delay(100);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, "*****");
  display.display();
  MAKO = bacaMAKODariSD();
  delay(100);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, "******");
  display.display();
  TASGANU = bacaTASGANUDariSD();
  delay(100);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "STARTING..");
  display.display();
  delay(1000);
}

void loop() {
  long now2 = millis();
  if (now2 - lastMsg2 > 500){
    lastMsg2 = now2;
    // Menampilkan hasil
    Serial.print("Variabel Induk: ");
    Serial.println(induk);
    Serial.print("Nilai: ");
    Serial.println(nilai);

  }
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    perbaruiScreen();
    switch (dis) {
      case 1:
        detail1();
        break;
      case 2:
        detail2();
        break;
      case 3:
        detail3();
        break;
    }
    dis = (dis % 3) + 1;
  }
  long now1 = millis();
  if ( now1 - lastMsg1 > 60000){
    lastMsg1 = now1;
    kirimData();
  }  
  if (simpanOn == 1) {
    if (induk == "SDM") {
      simpanSDMData();
      SDM = nilai;
    }
    if (induk == "LINIA") {
      simpanSDMData();
      LINIA = nilai;
    }
    if (induk == "LINIB") {
      simpanSDMData();
      LINIB = nilai;
      }
    if (induk == "PUSYANTEK") {
      simpanSDMData();
      PUSYANTEK = nilai;
      }
    if (induk == "MAKO") {
      simpanSDMData();
      MAKO = nilai;
      }
    if (induk == "LOGAM") {
      simpanSDMData();
      LOGAM = nilai;
      }
  }
}

void onReceive(int packetSize) {
  // Received a packet
  display.clear();
  Serial.println("Received packet '");
  memset(Str1, 0, sizeof(Str1));
  
  // Read packet
  for (int i = 0; i < packetSize; i++) {
    Str1[i] = (char)LoRa.read();
  }
  Serial.print(Str1);
  
  // Print RSSI of packet
  Serial.print("' with RSSI ");
  RxDataRSSI = LoRa.packetRssi();
  RxDataSNR = LoRa.packetSnr();
  Serial.println(RxDataRSSI);
  Serial.println(RxDataSNR);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  display.clear();
  myString = String(Str1);
  Serial.println(myString);
  if (myString.length() > 0) {
    int separatorIndex = myString.indexOf('|');
    // Memisahkan variabel induk dan nilai berdasarkan posisi pemisah
    induk = myString.substring(0, separatorIndex);
    String nilai_str = myString.substring(separatorIndex + 1);

    // Konversi nilai dari string ke integer
    nilai = nilai_str.toInt();
    simpanOn = 1;
  }
}

void perbaruiScreen() {
  display.clear();
  // Line 0
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "LORA CONCENTRATOR!");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  // Line 1
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 5, "_____________________________________");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  // Line 2
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "Receive Data : " + String(Str1));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  // Line 3
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "RSSI: " + String(LoRa.packetRssi()) + " | SNR:" + String(LoRa.packetSnr()));
}
void detail1(){
  // Line 4
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "Lini A : " + String(LINIA));
  // Line 5
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 45, "Lini B : " + String(LINIB));
  display.display();
  }
  
void detail2(){
  // Line 4
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "Logam   : " + String(LOGAM));
  // Line 5
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 45, "Tasganu : " + String(TASGANU));
  display.display();
  }

void detail3(){
  // Line 4
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "MAKO : " + String(MAKO));
  // Line 5
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 45, "SDM  : " + String(SDM));
  display.display();
  }
void simpanSDMData() {
  // Rekam data kedalam file excel "SDM.csv"
  File dataFile = SD.open("/" + String(induk) +".csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println(nilai);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening SDM.csv");
  }
  simpanOn = 0;
}

int bacaLINIADariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/LINIA.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening LINIA.csv to read last counter.");
  }
  return lastCounter;
}
int bacaLINIBDariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/LINIB.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening LINIB.csv to read last counter.");
  }
  return lastCounter;
}
int bacaLOGAMDariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/LOGAM.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening LOGAM.csv to read last counter.");
  }
  return lastCounter;
}
int bacaPUSYANTEKDariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/PUSYANTEK.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening PUSYANTEK.csv to read last counter.");
  }
  return lastCounter;
}
int bacaMAKODariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/MAKO.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening MAKO.csv to read last counter.");
  }
  return lastCounter;
}
int bacaTASGANUDariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/TASGANU.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening TASGANU.csv to read last counter.");
  }
  return lastCounter;
}

int bacaSDMDariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/SDM.csv", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        lastCounter = line.toInt();
      }
    }
    dataFile.close();
    Serial.println("Last counter read from SD card: " + String(lastCounter));
  } else {
    Serial.println("Error opening SDM.csv to read last counter.");
  }
  return lastCounter;
}

void kirimData(){
  // kirimkan paket isi data via LoRa
  LoRa.beginPacket();
  LoRa.print(+"LINIA|"+String(LINIA)+"LINIB|"+String(LINIB)+"TASGANU|"+String(TASGANU)+"LOGAM|"+String(LOGAM)+"SDM|"+String(SDM)+"MAKO|"+String(MAKO));
  LoRa.endPacket();
}
