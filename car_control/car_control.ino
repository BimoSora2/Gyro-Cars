#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "RC_Car_AP";
const char* password = "12345678";

WiFiUDP udp;
unsigned int localPort = 4210;
char incomingPacket[32];  // Reduced buffer size for faster processing

void setup() {
  Serial.begin(115200);  // Increased baud rate for faster serial communication
  WiFi.softAP(ssid, password, 1, true);
  udp.begin(localPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
    if (len > 0) {
      incomingPacket[len] = 0;
      Serial.write(incomingPacket, len);  // Directly write to Serial without additional processing
      Serial.write('\n');  // Add newline for proper parsing on the receiving end
    }
  }
}
