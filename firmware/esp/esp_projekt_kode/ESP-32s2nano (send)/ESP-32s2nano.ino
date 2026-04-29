#include <Arduino.h>

// UART pins from ESP32-S3 to ESP8266
#define ESP32_TX_PIN 17   // connect to ESP8266 RX
#define ESP32_RX_PIN 18   // optional, connect to ESP8266 TX if needed

// Buttons
#define START_BUTTON_PIN 21
#define STOP_BUTTON_PIN 22

HardwareSerial espSerial(1);

String session_uuid = "";
float = distance;
bool session_in_progress = false;


// Timing for distance sending
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 200; // ms

void setup() {
  Serial.begin(115200); // USB serial monitor

  espSerial.begin(
    115200,
    SERIAL_8N1,
    ESP32_RX_PIN,
    ESP32_TX_PIN
  );

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("ESP32-S3 sender ready");

}

void loop() {

  // 🔹 START button
  if (digitalRead(START_BUTTON_PIN) == LOW && !session_in_progress) {
    delay(50); // debounce
    if (digitalRead(START_BUTTON_PIN) == LOW) {
      sendUser(session_uuid);
      sendSessionStart();
    }
  }

  // 🔹 STOP button
  if (digitalRead(STOP_BUTTON_PIN) == LOW && session_in_progress) {
    delay(50); // debounce
    if (digitalRead(STOP_BUTTON_PIN) == LOW) {
      sendSessionStop();
      delay(200);
      sendUser("NONE");
    }
  }

  // 🔹 Send distance periodically during session
  if (sessionActive && millis() - lastSendTime > SEND_INTERVAL) {
    lastSendTime = millis();

    //DEN RIGTIGE DISTANCE-SENSORVÆRDI HER:
    float distance = (float)random(0, 500) / 100.0; // example: 0.00–5.00

    sendDistance(distance);
  }
}

void sendUser(String username) {
  username.trim();
  espSerial.println("USER:" + username);
  Serial.println("Sent: USER:" + username);
}

void sendDistance(float dist) {
  String msg = "DIST:" + String(dist, 2);
  espSerial.println(msg);
  Serial.println("Sent: " + msg);
}

void sendSessionStart() {
  espSerial.println("SESSION:START");
  Serial.println("Sent: SESSION:START");
  sessionActive = true;
}

void sendSessionStop() {
  espSerial.println("SESSION:STOP");
  Serial.println("Sent: SESSION:STOP");
  sessionActive = false;
}