#include <Wire.h>
#include <VL53L0X.h>

// -------------------------
// VL53L0X pins
// -------------------------
const int SDA_PIN = 11;
const int SCL_PIN = 12;

VL53L0X sensor;

// -------------------------
// TMC2209 pins
// -------------------------
const int STEP_PIN = 13;
const int DIR_PIN  = 14;
const int EN_PIN   = 7;

// Smaller = faster motor
const int STEP_DELAY_US = 1000;

// -------------------------
// Run motor for a set time
// while also reading sensor
// -------------------------
void runMotorWithSensor(bool dir, unsigned long runTimeMs) {
  digitalWrite(DIR_PIN, dir);

  unsigned long startTime = millis();
  unsigned long lastPrint = 0;

  while (millis() - startTime < runTimeMs) {
    // Step pulse
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY_US);

    // Print sensor every 100 ms
    if (millis() - lastPrint >= 100) {
      lastPrint = millis();

      uint16_t distance_mm = sensor.readRangeContinuousMillimeters();

      if (sensor.timeoutOccurred()) {
        Serial.println("Sensor timeout!");
      } else {
        int distance_cm = round(distance_mm / 10.0);

        Serial.print("Distance: ");
        Serial.print(distance_cm);
        Serial.println(" cm");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Motor setup
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  digitalWrite(EN_PIN, LOW);   // enable TMC2209
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  // I2C + sensor setup
  Wire.begin(SDA_PIN, SCL_PIN, 400000);

  sensor.setTimeout(500);

  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize VL53L0X!");
    while (1) {
      delay(100);
    }
  }

  sensor.startContinuous();

  Serial.println("TMC2209 + VL53L0X ready");
}

void loop() {
  Serial.println("Forward for 3 seconds");
  runMotorWithSensor(HIGH, 3000);
  delay(500);

  Serial.println("Backward for 3 seconds");
  runMotorWithSensor(LOW, 3000);
  delay(500);
}