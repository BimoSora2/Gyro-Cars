#include <Arduino.h>

const int IN1 = 2, IN2 = 3, IN3 = 4, IN4 = 7;
const int ENA = 6, ENB = 5;
const int LIGHT_L = 8, LIGHT_R = 10, SEIN_L = 9, SEIN_R = 11;

void setup() {
  Serial.begin(115200);  // Increased baud rate
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(LIGHT_L, OUTPUT); pinMode(LIGHT_R, OUTPUT); pinMode(SEIN_L, OUTPUT); pinMode(SEIN_R, OUTPUT);
  digitalWrite(LIGHT_L, HIGH);
  digitalWrite(LIGHT_R, HIGH);
  digitalWrite(SEIN_L, HIGH);
  digitalWrite(SEIN_R, HIGH);
}

void loop() {
  static unsigned long lastBlinkTime = 0;
  static bool blinkState = false;
  static bool policeMode = false;

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    
    if (command.startsWith("up,")) {
      int speed = command.substring(3).toInt();
      moveMotors(speed, speed, HIGH, LOW, HIGH, LOW);
    } else if (command.startsWith("down,")) {
      int speed = command.substring(5).toInt();
      moveMotors(speed, speed, LOW, HIGH, LOW, HIGH);
    } else if (command.startsWith("left,")) {
      int speed = command.substring(5).toInt();
      moveMotors(speed, speed, HIGH, LOW, LOW, HIGH);
    } else if (command.startsWith("right,")) {
      int speed = command.substring(6).toInt();
      moveMotors(speed, speed, LOW, HIGH, HIGH, LOW);
    } else if (command == "stop") {
      moveMotors(0, 0, LOW, LOW, LOW, LOW);
    } else if (command == "light") {
      digitalWrite(LIGHT_L, !digitalRead(LIGHT_L));
      digitalWrite(LIGHT_R, !digitalRead(LIGHT_R));
    } else if (command == "police") {
      policeMode = !policeMode;
    }
  }

  // Handle police lights
  if (policeMode && millis() - lastBlinkTime > 250) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
    digitalWrite(SEIN_L, blinkState);
    digitalWrite(SEIN_R, !blinkState);
  } else if (!policeMode) {
    digitalWrite(SEIN_L, HIGH);
    digitalWrite(SEIN_R, HIGH);
  }
}

void moveMotors(int speedA, int speedB, int in1State, int in2State, int in3State, int in4State) {
  analogWrite(ENA, speedA);
  analogWrite(ENB, speedB);
  digitalWrite(IN1, in1State);
  digitalWrite(IN2, in2State);
  digitalWrite(IN3, in3State);
  digitalWrite(IN4, in4State);
}
