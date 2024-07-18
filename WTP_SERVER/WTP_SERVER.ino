// Load Library
#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <WiFi.h>
#include <PubSubClient.h>

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
long MAKO;
long LINIA;
long LINIB;
long SSDM;
long SMAKO;
long SLINIA;
long SLINIB;
long lastMsg=0;
long lastMsg1=0;
int grafik = 0;
String activity = "";
// Definisi pin untuk antarmuka SPI
#define SD_CS   13   // CS pin untuk SD card module
#define SD_MOSI 15   // MOSI pin untuk SD card module
#define SD_SCK  14   // SCK pin untuk SD card module
#define SD_MISO 2    // MISO pin untuk SD card module

// Update these with values suitable for your network.
const char* ssid = "JONO";
const char* password = "susukuda";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/esp32/mqtt/in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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

  // Put the radio into receive mode
  LoRa.receive();
  bacaDariSD(); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if((SSDM != SDM) || (SMAKO != MAKO) || (SLINIA != LINIA) || (SLINIB != LINIB)){
    SDM = SSDM;
    MAKO = SMAKO;
    LINIA = SLINIA;
    LINIB = SLINIB;
    simpanData();
  }
  
  
  perbaruiScreen();
  receiveData();
  long now = millis();
  if (now - lastMsg > 3000){
    lastMsg = now;
    grafik++;
    if (grafik == 2){
      grafik = 0;
    }
    }
  long now1 = millis();
  if (now - lastMsg1 > 30000){
    lastMsg1 = now1;
    sendMqtt();
  }
    
}

void sendMqtt(){
  Serial.println("____________________________________________");
  Serial.println("Send MQTT Data:");
  long dataSDM = SDM;
  long dataMAKO = MAKO;
  long dataLINIA = LINIA;
  long dataLINIB = LINIB;

  char SDMString[100];
  char MAKOString[100];
  char LINIAString[100];
  char LINIBString[100];
  dtostrf(dataSDM,1,2,SDMString);
  Serial.print("Gedung SDM : ");
  Serial.println(SDMString);
  client.publish("/esp32-mqtt/PeruriGSDM",SDMString);
  dtostrf(dataMAKO,1,2,MAKOString);
  Serial.print("Gedung MAKO : ");
  Serial.println(MAKOString);
  client.publish("/esp32-mqtt/PeruriGMAKO",MAKOString);
  dtostrf(dataLINIA,1,2,LINIAString);
  Serial.print("Gedung LINI A : ");
  Serial.println(LINIAString);
  client.publish("/esp32-mqtt/PeruriGLINIA",LINIAString);
  dtostrf(dataLINIB,1,2,LINIBString);
  Serial.print("Gedung LINI B : ");
  Serial.println(LINIBString);
  client.publish("/esp32-mqtt/PeruriGLINIB",LINIBString);
}


void receiveData() {
  activity = "Receive Data";
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // baca packet
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    Serial.println("____________________________________________");
    Serial.print("Received: ");
    Serial.println(receivedData);

    // Parse the received data
    if (receivedData.startsWith("Concentrator|")) {
      int firstPipe = receivedData.indexOf('|');
      int secondPipe = receivedData.indexOf('|', firstPipe + 1);
      int thirdPipe = receivedData.indexOf('|', secondPipe + 1);
      int fourthPipe = receivedData.indexOf('|', thirdPipe + 1);

      if (secondPipe != -1 && thirdPipe != -1 && fourthPipe != -1) {
        SSDM    = receivedData.substring(firstPipe + 1, secondPipe).toInt();
        SMAKO   = receivedData.substring(secondPipe + 1, thirdPipe).toInt();
        SLINIA  = receivedData.substring(thirdPipe + 1, fourthPipe).toInt();
        SLINIB  = receivedData.substring(fourthPipe + 1).toInt();

        Serial.println("Extracted Data:");
        Serial.print("SDM: ");
        Serial.println(SDM);
        Serial.print("MAKO: ");
        Serial.println(MAKO);
        Serial.print("LINIA: ");
        Serial.println(LINIA);
        Serial.print("LINIB: ");
        Serial.println(LINIB);
        Serial.println("____________________________________________");
      }
    }
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
  display.drawString(0, 16, "Status : " + activity);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 28, "RSSI: " + String(LoRa.packetRssi()) + " | SNR:" + String(LoRa.packetSnr()));
  
  if (grafik == 0){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "LINI A : " + String(LINIA));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 52, "LINI B : " + String(LINIB));
  }
  if (grafik == 1){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "SDM   : " + String(SDM));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 52, "MAKO : " + String(MAKO));
  }
  display.display();
}

void simpanData() {
  Serial.println("____________________________________________");
  // Rekam data kedalam file excel "SDM.csv"
  File dataFile = SD.open("/DATABASE.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.print(SDM);
    dataFile.print(",");
    dataFile.print(MAKO);
    dataFile.print(",");
    dataFile.print(LINIA);
    dataFile.print(",");
    dataFile.println(LINIB);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening DATABASE.csv");
  }
  Serial.println("____________________________________________");
  simpanOn = 0;
  activity = "Stand by";
}

void bacaDariSD() {
  File dataFile = SD.open("/DATABASE.csv", FILE_READ);
  if (dataFile) {
    String lastLine = "";
    // Baca seluruh isi file dan simpan baris terakhir
    while (dataFile.available()) {
      lastLine = dataFile.readStringUntil('\n');
    }
    dataFile.close();
    
    if (lastLine.length() > 0) {
      Serial.println("Last line read from SD card: " + lastLine);
      
      // Pisahkan data menggunakan koma sebagai pemisah
      int firstComma = lastLine.indexOf(',');
      int secondComma = lastLine.indexOf(',', firstComma + 1);
      int thirdComma = lastLine.indexOf(',', secondComma + 1);
      
      if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
        SDM = lastLine.substring(0, firstComma).toInt();
        SSDM = lastLine.substring(0, firstComma).toInt();
        MAKO = lastLine.substring(firstComma + 1, secondComma).toInt();
        SMAKO = lastLine.substring(firstComma + 1, secondComma).toInt();
        LINIA = lastLine.substring(secondComma + 1, thirdComma).toInt();
        SLINIA = lastLine.substring(secondComma + 1, thirdComma).toInt();
        LINIB = lastLine.substring(thirdComma + 1).toInt();
        SLINIB = lastLine.substring(thirdComma + 1).toInt();
        
        Serial.println("Extracted Data:");
        Serial.print("SDM: ");
        Serial.println(SDM);
        Serial.print("MAKO: ");
        Serial.println(MAKO);
        Serial.print("LINIA: ");
        Serial.println(LINIA);
        Serial.print("LINIB: ");
        Serial.println(LINIB);
      } else {
        Serial.println("Error parsing the last line.");
      }
    } else {
      Serial.println("No data found in SD card.");
    }
  } else {
    Serial.println("Error opening DATABASE.csv to read last line.");
  }
}
