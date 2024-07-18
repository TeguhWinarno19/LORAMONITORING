#include <WiFi.h>
#include <WebServer.h>

// SSID dan password untuk Access Point
const char* ssid = "ESP32-AP";
const char* password = "123456789";

// Inisialisasi server web pada port 80
WebServer server(80);

void setup() {
  // Memulai serial communication
  Serial.begin(115200);

  // Menginisialisasi Access Point
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("Access Point started, IP address: ");
  Serial.println(WiFi.softAPIP());

  // Menangani request ke root URL "/"
  server.on("/", handleRoot);

  // Memulai server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Menangani request klien
  server.handleClient();
}

// Fungsi untuk menangani request ke root URL "/"
void handleRoot() {
  // Menghasilkan data acak
  int randomNumber = random(0, 100);

  // Menampilkan data acak pada halaman web
  String html = "<!DOCTYPE html><html><head><title>ESP32 Web Server</title></head><body>";
  html += "<h1>Random Number</h1>";
  html += "<p>" + String(randomNumber) + "</p>";
  html += "</body></html>";

  // Mengirim respons ke klien
  server.send(200, "text/html", html);
}
