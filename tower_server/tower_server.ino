//Load Library
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"


#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

#define SCK     5    // GPIO5  -- lora SCK
#define MISO    19   // GPIO19 -- lora MISO
#define MOSI    27   // GPIO27 -- lora MOSI
#define SS      18   // GPIO18 -- lora CS
#define RST     12   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    915E6

//replace default pin  OLED_SDA=4, OLED_SCL=15 with  OLED_SDA=21, OLED_SCL=22
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23   //try this, if problem change to 14 or 16


#define LED_BUILTIN 25

String induk;
long nilai;

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=4, OLED_SCL=15


//Receive Package Variable
int RxDataRSSI = 0;
char Str1[15];
//parsingData
String myString;
int i;
String dt[10];
boolean parsing = false;

void setup() {
  // START aktivas Oled
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high、

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
  

  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);

  Serial.println("LoRa Receiver Callback");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();

  File dataFile = SD.open("/SDM.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time (ms),Counter");
    dataFile.close();
    Serial.println("CSV header written to SD card.");
  } else {
    Serial.println("Error opening data.csv");
  }

}

void loop() {
  // do nothing
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "LORA IOT SERVER");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "rx Data:" + String(Str1));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 30, "RSSI : " + String(LoRa.packetRssi()));
  display.display();
  // Menampilkan hasil
  Serial.print("Variabel Induk: ");
  Serial.println(induk);
  Serial.print("Nilai: ");
  Serial.println(nilai);
  delay(500);
}

void onReceive(int packetSize) {
  // received a packet
  display.clear();
  Serial.println("Received packet '");
  memset(Str1, 0, sizeof(Str1));
  // read packet
  for (int i = 0; i < packetSize; i++) {
    Str1[i] = (char)LoRa.read();
  }
  Serial.print(Str1);
  // print RSSI of packet
  Serial.print("' with RSSI ");
  RxDataRSSI = LoRa.packetRssi();
  Serial.println(RxDataRSSI);
  //  digitalWrite(LED_BUILTIN, (state) ? HIGH : LOW);
  //  state = !state;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  display.clear();
  myString = String(Str1);
  Serial.println(myString);
  if (myString.length() > 0)
  {
    int separatorIndex = myString.indexOf('|');
    // Memisahkan variabel induk dan nilai berdasarkan posisi pemisah
    induk = myString.substring(0, separatorIndex);
    String nilai_str = myString.substring(separatorIndex + 1);

    // Konversi nilai dari string ke integer
    nilai = nilai_str.toInt();
  }
}
