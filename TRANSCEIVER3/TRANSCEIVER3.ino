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

// Definisi pin untuk OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

String induk;
long nilai;

// Initialize the OLED display using Wire library
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
long MAKO;
long lastMsg= 0;
long lastMsg1=0;
int grafik = 0;
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
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,15, "> Config Screeen");
  display.display();
  delay(1000);
  display.drawString(0,15, "> Config Screeen ok!");
  display.display();
  delay(1000);
  // aktivasi Oled END
  // Nyalakan serial monitor
  Serial.begin(115200);
  while (!Serial);
  // Inisialisasi SPI untuk SD card
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(500);
  display.drawString(0,25, "> Config LoRa Module");
  display.display();

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

   LoRa.setTxPower(14);  // Contoh: set daya transmisi ke 14 dBm
   
  display.drawString(0,25, "> Config LoRa Module ok!");
  display.display();
  display.drawString(0,35, "> Config Storage");
  display.display();
  delay(500);
  // Inisialisasi SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("Initialization of SD card failed!");
    display.drawString(0,35, "> Config Storage failed!");
    display.display();
    while (1);
  }
  Serial.println("SD card initialized.");
  display.drawString(0,35, "> Config Storage ok!");
  display.display();
  delay(1000);
  // Register the receive callback
  LoRa.onReceive(onReceive);

  // Put the radio into receive mode
  LoRa.receive();
  display.drawString(0,45, "> Read Data from SD card!");
  display.display();
  delay(1000);
  SDM = bacaSDMDariSD();
  display.drawString(0,55, "> *");
  display.display();
  delay(1000);
  LINIA = bacaLINIADariSD();
  display.drawString(0,55, "> ***");
  display.display();
  delay(1000);
  LINIB = bacaLINIBDariSD();
  display.drawString(0,55, "> ******");
  display.display();
  delay(1000);
  MAKO = bacaMAKODariSD();
  display.drawString(0,55, "> ************");
  display.display();
  delay(1000);
}

void loop() {
  perbaruiScreen();
  // Menampilkan hasil
  Serial.print("Variabel Induk: ");
  Serial.println(induk);
  Serial.print("Nilai: ");
  Serial.println(nilai);
  if (simpanOn == 1) {
    if (induk == "SDM") {
      simpanData();
      SDM = nilai;
    }
    if (induk == "LINIA") {
      simpanData();
      LINIA = nilai;
    }
    if (induk == "LINIB"){
      simpanData();
      LINIB = nilai;  
    }
    if (induk == "MAKO"){
      simpanData();
      MAKO = nilai;
    }
  }
  long now = millis();
  if (now - lastMsg > 3000){
    lastMsg = now;
    grafik++;
    if (grafik == 2){
      grafik = 0;
    }
  }
  long now1 = millis();
  if (now1 - lastMsg1 > 15000){
    lastMsg1 = now1;
    kirimData();
    Serial.println("Data send!");
  }
  
  delay(500);
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
  // Line 1
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "LORA IOT CONCENTRATOR!");
  // Line 2
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 3, "___________________________________________");
  // Line 3
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "Data In : " + String(Str1));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  // Line 4
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 27, "RSSI: " + String(LoRa.packetRssi()) + " | SNR:" + String(LoRa.packetSnr()));
  // Line 6
  if (grafik == 0){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "SDM          : " + String(SDM));
    // Line 6
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 50, "MAKO         : " + String(MAKO));
    }
  if (grafik == 1){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "LINIA        : " + String(LINIA));
    // Line 6
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 50, "LINIB        : " + String(LINIB));
    }
  
  display.display();
}

void simpanData() {
  // Rekam data kedalam file excel "SDM.csv"
  File dataFile = SD.open("/"+ String(induk)+ ".csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println(nilai);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening SDM.csv");
  }
  simpanOn = 0;
}

long bacaSDMDariSD() {
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
long bacaLINIADariSD() {
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
long bacaLINIBDariSD() {
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
long bacaMAKODariSD() {
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

void kirimData(){
  LoRa.beginPacket();
  LoRa.print("Concentrator|"+String(SDM)+"|"+String(MAKO)+"|"+String(LINIA)+"|"+String(LINIB));
  LoRa.endPacket();
  LoRa.receive();
  }
