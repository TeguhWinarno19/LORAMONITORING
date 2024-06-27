#include <SPI.h>
#include <SD.h>

// Definisikan pin untuk antarmuka SPI
#define SD_CS   13   // CS pin untuk SD card module
#define SD_MOSI 15   // MOSI pin untuk SD card module
#define SD_SCK  14   // SCK pin untuk SD card module
#define SD_MISO 2    // MISO pin untuk SD card module

int counter = 0;
void setup() {
  Serial.begin(115200);
  while (!Serial); // tunggu hingga serial port tersambung

  // Set up SPI interface
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("Initialization of SD card failed!");
    while (1);
  }
  Serial.println("SD card initialized.");

  // Menulis data uji ke SD card
  File dataFile = SD.open("/data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("SD card test");
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening test.txt");
  }

  // Membaca data dari SD card
  dataFile = SD.open("/data.csv");
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
    Serial.println("\nData read from SD card.");
  } else {
    Serial.println("Error opening test.txt");
  }
}

void loop() {
  counter++;
  delay(1000);
  // Menulis data uji ke SD card
  File dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println(counter);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening test.txt");
  }
  delay(1000);

  // Membaca data dari SD card
  dataFile = SD.open("/data.csv");
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
    Serial.println("\nData read from SD card.");
  } else {
    Serial.println("Error opening test.txt");
  }

}
