#include <Arduino.h>
#include <Wire.h>
#include "SH1106Wire.h"

// ESP8266 I2C pins:
// SDA = D2 = GPIO4
// SCL = D1 = GPI14
SH1106Wire display(0x3c, 4, 14);

String incoming = "";
String currentText = "";


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

  Wire.begin(4, 5); //TODO: change to (4,14)

  display.init();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  drawScreen("GenStraek", 24);
}

void loop() {
  String msg = receiveMessage();
  msg.replace(";", "\n");

  if (msg.length() > 0) {
    Serial.println(msg);
    currentText = msg;   // store message
    drawScreen(currentText, 13);
    Serial.println("----");
  }
}

void drawScreen(String tekst, int fontsize) {
    // vi kan generere egne fonts her:
    //  https://oleddisplay.squix.ch/
    display.clear();
    if (fontsize >= 24) {
        display.setFont(ArialMT_Plain_24);
    }
    else if (fontsize >= 16) {
        display.setFont(ArialMT_Plain_16);
    }
    else {
        display.setFont(ArialMT_Plain_10);
    }
    display.drawString(0, 0, tekst);
    display.display();
}