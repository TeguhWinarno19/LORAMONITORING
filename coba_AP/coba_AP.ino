// WIFI
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

//Variable Wifi
const char *ssid = "yourAP";
const char *password = "yourPassword";
WiFiServer server(80);

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
  display.clear();
  // aktivasi Oled END

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

  // Inisialisasi SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("Initialization of SD card failed!");
    while (1);
  }
  Serial.println("SD card initialized.");

  // Register the receive callback
  LoRa.onReceive(onReceive);

  // Put the radio into receive mode
  LoRa.receive();
  SDM = bacaSDMDariSD();
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}
void loop(){
  wifi();
  perbaruiScreen();
  // Menampilkan hasil
  Serial.print("Variabel Induk: ");
  Serial.println(induk);
  Serial.print("Nilai: ");
  Serial.println(nilai);
  if (simpanOn == 1) {
    if (induk == "SDM") {
      simpanSDMData();
      SDM = nilai;
    }
  }
  delay(500);
  
}

void wifi(){
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
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
  // Line 1
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "LORA IOT SERVER");
  // Line 2
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 3, "___________________________________________");
  // Line 3
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 18, "Receive Data : " + String(Str1));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  // Line 4
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 30, "RSSI: " + String(LoRa.packetRssi()) + " | SNR:" + String(LoRa.packetSnr()));
  // Line 6
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 43, "SDM          : " + String(SDM));
  display.display();
}

void simpanSDMData() {
  // Rekam data kedalam file excel "SDM.csv"
  File dataFile = SD.open("/SDM.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println(nilai);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening SDM.csv");
  }
  simpanOn = 0;
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
