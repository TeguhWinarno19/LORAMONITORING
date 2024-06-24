//modified lora library by Sandeep Mistry for TTGO ESP32 Lora
// lora Sender 915MHz V2.1-1.6
// Modified by HwThinker
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

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

int counter = 0;
int state = 0;

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=4, OLED_SCL=15

void setup() {
  // START  Oled ********
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high、

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  // clear the display
  display.clear();
  //  END Oled ********

  Serial.begin(115200);
  while (!Serial); //If just the the basic function, must connect to a computer

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
}

void loop() {

  //ukuran Form
  display.setFont(ArialMT_Plain_10);
  //colom, baris, isitext 
  display.drawString(0, 0, "Nabil Try to connected!");
  display.display();

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 14, "Water Meter Value:");
  display.display();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 28, "Sending packet : " + String(counter));
  display.display();

  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");  
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
  delay(2200);
  
  //  digitalWrite(LED_BUILTIN, (state) ? HIGH : LOW);
  //  state = !state;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  display.clear();
}
