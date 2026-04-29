#include <Arduino.h>
#include <Wire.h>
#include "SH1106Wire.h" 

// Tanken er at esp32-s3 skal sende info til esp8266 over serial på denne måde:
//Serial.println("USER:Anders");
//Serial.println("SESSION:START");
//Serial.println("DIST:3.42");
//Serial.println("DIST:4.10");
//Serial.println("SESSION:STOP");
//Serial.println("USER:NONE");
//

SH1106Wire display(0x3c, 21, 22); // I2C adresse, SDA, SCL

String userStatus = "No user";
String distanceText = "";
bool sessionActive = false;

void setup() {

  Wire.begin(21, 22);
  display.init();
  display.setFont(ArialMT_Plain_24);
  Serial.begin(115200);  // UART from ESP32
  Serial.setTimeout(50); // optional: faster readString()

  Serial.println("ESP8266 receiver ready");
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg.length() == 0) return;

    Serial.println("Received: " + msg);

    // 🔹 USER message
    if (msg.startsWith("USER:")) {
      String name = msg.substring(5);
      name.trim();

      if (name == "NONE") {
        userName = "No user";
      } else {
        userName = name;
      }
    }

    //Distance
    else if (msg.startsWith("DIST:")) {
      distanceText = msg.substring(5);
    }

    //Session
    else if (msg == "SESSION:START") {
      sessionActive = true;
    }
    else if (msg == "SESSION:STOP") {
      sessionActive = false;
      distanceText = "";
    }

    drawScreen(); // redraw on every update
  }
}

void updateDisplay(String tekst, int fontsize) {
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

//Placerer informationerne på skærmen
void drawScreen() {
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, userStatus);

  if (sessionActive && userStatus == "User OK") {
    display.setFont(ArialMT_Plain_24);

    String text = distanceText + " m";

    int16_t x = (128 - display.getStringWidth(text)) / 2;
    int16_t y = (64 - 24) / 2;

    display.drawString(x, y, text);
  }

  display.display();
}