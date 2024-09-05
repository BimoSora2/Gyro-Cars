#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050.h>

const char* ssid = "RC_Car_AP";
const char* password = "12345678";
const char* serverIP = "192.168.4.1";
unsigned int serverPort = 4210;
const char* expectedMAC = "6A:C6:3A:F3:1B:E6";  // MAC address yang diharapkan dari AP

WiFiUDP udp;
MPU6050 mpu;

byte mac[6];

// Define button pins
const int buttonLight = D6;
const int buttonPolice = D7;

// Accelerometer sensitivity thresholds
const int16_t TILT_THRESHOLD = 2000;  // Reduced from 8000
const int16_t MAX_TILT = 15000;       // Reduced from 32767

void setup() {
  Serial.begin(9600);
  Serial.println();

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Timeout after 30 seconds if unable to connect
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
  } else {
    Serial.println("Failed to connect to WiFi. Restarting...");
    ESP.restart();  // Restart the ESP8266 to retry connection
  }

  String actualMAC = WiFi.BSSIDstr();
  Serial.print("Connected MAC Address: ");
  Serial.println(actualMAC);
  
  if (actualMAC.equals(expectedMAC)) {
    Serial.println("Connected to the correct AP with MAC address: " + actualMAC);
  } else {
    Serial.println("MAC address mismatch! Disconnecting...");
    WiFi.disconnect();
    while (true) {
      delay(1000); // Halt if MAC address does not match
    }
  }

  Wire.begin(D1, D2);
  mpu.initialize();

  pinMode(buttonLight, INPUT_PULLUP);
  pinMode(buttonPolice, INPUT_PULLUP);
}
int16_t mapAndConstrain(int16_t value, int16_t fromLow, int16_t fromHigh, int16_t toLow, int16_t toHigh) {
  int32_t mappedValue = map(constrain(value, fromLow, fromHigh), fromLow, fromHigh, toLow, toHigh);
  return constrain(mappedValue, toLow, toHigh);
}

void loop() {
  static unsigned long lastSendTime = 0;
  const unsigned long sendInterval = 20;  // Send commands every 20ms (50Hz)

  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    lastSendTime = currentTime;

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    char command[16];
    if (abs(ax) > abs(ay)) {  // Prioritize forward/backward over left/right
      if (ax > TILT_THRESHOLD) {
        int speed = mapAndConstrain(ax, TILT_THRESHOLD, MAX_TILT, 0, 255);
        snprintf(command, sizeof(command), "up,%d", speed);
      } else if (ax < -TILT_THRESHOLD) {
        int speed = mapAndConstrain(-ax, TILT_THRESHOLD, MAX_TILT, 0, 255);
        snprintf(command, sizeof(command), "down,%d", speed);
      } else {
        strcpy(command, "stop");
      }
    } else {
      if (ay > TILT_THRESHOLD) {
        int speed = mapAndConstrain(ay, TILT_THRESHOLD, MAX_TILT, 0, 255);
        snprintf(command, sizeof(command), "left,%d", speed);
      } else if (ay < -TILT_THRESHOLD) {
        int speed = mapAndConstrain(-ay, TILT_THRESHOLD, MAX_TILT, 0, 255);
        snprintf(command, sizeof(command), "right,%d", speed);
      } else {
        strcpy(command, "stop");
      }
    }

    sendUDPPacket(command);
  }

  // Handle button presses
  static bool lastLightState = true, lastPoliceState = true;
  bool currentLightState = digitalRead(buttonLight);
  bool currentPoliceState = digitalRead(buttonPolice);

  if (currentLightState != lastLightState && currentLightState == LOW) {
    sendUDPPacket("light");
  }
  if (currentPoliceState != lastPoliceState && currentPoliceState == LOW) {
    sendUDPPacket("police");
  }

  lastLightState = currentLightState;
  lastPoliceState = currentPoliceState;
}

void sendUDPPacket(const char* command) {
  udp.beginPacket(serverIP, serverPort);
  udp.write(command);
  udp.endPacket();
}
