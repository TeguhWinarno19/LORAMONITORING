//Load Library
#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"

// Definisi pin untuk antarmuka SPI
#define SD_CS   13   // CS pin untuk SD card module
#define SD_MOSI 15   // MOSI pin untuk SD card module
#define SD_SCK  14   // SCK pin untuk SD card module
#define SD_MISO 2    // MISO pin untuk SD card module

// Definisi pin untuk LoRa
#define LORA_SCK 5    // GPIO5  -- lora SCK
#define LORA_MISO 19  // GPIO19 -- lora MISO
#define LORA_MOSI 27  // GPIO27 -- lora MOSI
#define LORA_SS 18    // GPIO18 -- lora CS
#define LORA_RST 12   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define LORA_DI0 26   // GPIO26 -- IRQ(Interrupt Request)
#define LORA_BAND 915E6
#define Node  "SDM"

// Definisi pin untuk OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

// Definisi pin untuk LED
#define LED_BUILTIN 25
int counter = 0;
int state = 0;

// Definisi pi  untuk Input
#define INPUT_PIN 4
int stack = 0;

// inisialisasi OLED display menggunakan Wire library
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=21, OLED_SCL=22

// SPI untuk SD card
SPIClass sdSPI(VSPI);

// SPI untuk LoRa
SPIClass loraSPI(HSPI);

void setup() {
  //set pinMode
  pinMode(INPUT_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);

  // Inisialisasi OLED
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  // Nyakalan Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Inisialisasi SPI untuk SD card 
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Inisialisasi SPI untuk LoRa 
  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  // Inisialisasi LoRa
  LoRa.setSPI(loraSPI);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  Serial.println("LoRa Sender");
  if (!LoRa.begin(LORA_BAND)){
    Serial.println("Starting LoRa failed!");
    while (1);}
  Serial.println("LoRa Initial OK!");

  // Inisialisasi SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("Initialization of SD card failed!");
    while (1);}
  Serial.println("SD card initialized.");
  
//  File dataFile = SD.open("/data.csv", FILE_WRITE);
//  if (dataFile) {
//    dataFile.println("Time (ms),Counter");
//    dataFile.close();
//    Serial.println("CSV header written to SD card.");
//  } else {
//    Serial.println("Error opening data.csv");
//  }

  
  // Baca nilai counter terakhir dari SD card
  counter = bacaCounterDariSD();
  Serial.print("Counter initialized to: ");
  Serial.println(counter);
  perbaruiScreen();
}

void loop() {  
  //Baca data sensor
  int sensor = digitalRead(INPUT_PIN);
  if (sensor == 0 && stack == 0){
    stack = 1;
  }
  if (sensor == 1 && stack == 1){
    counter ++;
    stack =0;
    perbaruiScreen();
    kirimData();
    simpanData();
   }
   Serial.println("------------------------");
   Serial.print("nilai sensor   : ");
   Serial.println(sensor);
   Serial.print("nilai stack    : ");
   Serial.println(stack);
   Serial.print("Sending packet : ");
   Serial.println(counter);
   delay(1000);
   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
   display.clear();
}

void perbaruiScreen(){
  // untuk Update tampilan OLED
  //line 1
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 1, "IOT LORA32 PROJECT");
  display.display();
    //line 2
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "Node Identity  : " + String(Node));
  display.display();
    //line3
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 29, "data packet    : " + String(counter));
  display.display();
}

void kirimData(){
  // kirimkan paket isi data via LoRa
  LoRa.beginPacket();
  LoRa.print(Node);
  LoRa.print("|");
  LoRa.print(counter);
  LoRa.endPacket();
}

void simpanData(){
  // rekam data kedalam file excel "data.csv"
  File dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println(counter);
    dataFile.close();
    Serial.println("Data written to SD card.");
    } else {
      Serial.println("Error opening data.csv");
      }
}

int bacaCounterDariSD() {
  int lastCounter = 0;
  File dataFile = SD.open("/data.csv", FILE_READ);
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
    Serial.println("Error opening data.csv to read last counter.");
  }
  return lastCounter;
}
