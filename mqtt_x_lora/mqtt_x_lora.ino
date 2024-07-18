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

// Update these with values suitable for your network.
const char* ssid = "JONO";
const char* password = "susukuda";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

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

  // Register the receive callback
  LoRa.onReceive(onReceive);

  // Put the radio into receive mode
  LoRa.receive();
  SDM = bacaSDMDariSD();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 10000) {
    
    float datasdm = SDM;
    char SDMString[100];
    dtostrf(datasdm, 1, 2, SDMString);
    Serial.print("Gedung SDM: ");
    Serial.println(SDMString);
    client.publish("/esp32-mqtt/PeruriGSDM", SDMString);
    lastMsg = now;
  }
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
  SDM = nilai;
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
