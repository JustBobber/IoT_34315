#include <Arduino.h>
#include <Wire.h>
#include "SH1106Wire.h" 

// ESP8266 I2C pins: SDA = D2 (GPIO4), SCL = D1 (GPIO5)
SH1106Wire display(0x3c, 4, 14);

String userName = "Torben (Impingement)";
String userStatus = "User OK";
String distanceText = "60";
bool sessionActive = true;

void setup() {

  Wire.begin(4, 5);   // Initialize I2C with correct pins
  display.init();
  display.setFont(ArialMT_Plain_24);

  Serial.begin(115200);
  Serial.setTimeout(50);

  Serial.println("ESP8266 receiver ready");
}

void loop() {
  drawScreen();
}

// Draw UI on screen
void drawScreen() {
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, userName);

  if (sessionActive && userStatus == "User OK") {
    display.setFont(ArialMT_Plain_24);

    String text = distanceText + " cm";

    int16_t x = (128 - display.getStringWidth(text)) / 2;
    int16_t y = (64 - 24) / 2;

    display.drawString(x, y, text);
  }

  display.display();
}