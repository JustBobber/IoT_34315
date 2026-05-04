#include <Arduino.h>
#include <Wire.h>
#include "SH1106Wire.h"

// ESP8266 I2C pins:
// SDA = D2 = GPIO4
// SCL = D1 = GPIO5
SH1106Wire display(0x3c, 4, 5);

String incoming = "";

String userName = "Torben (Impingement)";
String userStatus = "User OK";
String distanceText = "Waiting...";
bool sessionActive = true;

String receiveMessage() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      String msg = incoming;
      incoming = "";
      msg.trim();
      return msg;
    } else {
      incoming += c;
    }
  }

  return "";
}

void setup() {
  Serial.begin(9600);      // Must match ESP32 UART baud rate
  Serial.setTimeout(50);

  Wire.begin(4, 5);

  display.init();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  drawScreen();
}

void loop() {
  String msg = receiveMessage();

  if (msg.length() > 0) {
    distanceText = msg;
    drawScreen();
  }
}

// Draw UI on screen
void drawScreen() {
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, userName);

  if (sessionActive && userStatus == "User OK") {
    display.setFont(ArialMT_Plain_24);

    String text = distanceText;

    // Add " cm" only if the message is a number
    bool isNumber = true;
    for (int i = 0; i < distanceText.length(); i++) {
      if (!isDigit(distanceText[i])) {
        isNumber = false;
      }
    }

    if (isNumber) {
      text += " cm";
    }

    int16_t x = (128 - display.getStringWidth(text)) / 2;
    int16_t y = (64 - 24) / 2;

    display.drawString(x, y, text);
  }

  display.display();
}