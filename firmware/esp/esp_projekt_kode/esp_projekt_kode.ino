#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>


// ___ hardware setup ___
const int START_BUTTON_PIN = 6;
const int STOP_BUTTON_PIN = 7;

const int LIMIT_SWITCH_TOP_PIN = 8;
const int LIMIT_SWITCH_BOTTOM_PIN = 9;

const int POTENTIOMETER_PIN = 5;

const int TOF_SDA_PIN = 11;
const int TOF_SCL_PIN = 12;

const int STEPPER_STEP_PIN = 14;
const int STEPPER_DIR_PIN = 13;
const int STEPPER_ENABLE_PIN = 10;
const int STEP_DELAY_US = 100;


const int TX_PIN = 17;
const int RX_PIN = 16;
HardwareSerial mySerial(2);

VL53L0X tof;

// ___ sensor variable ___
bool MOTOR_UP = false;
bool MOTOR_DOWN = true;

int difficulty = 0; // mapping fra potmeter value

uint16_t tofDistance = 0;

uint16_t tofBottomDistance; // afstanden til tof sensoren når den er i bund.
uint16_t tofTopDistance;

/*
* Sørg for at starte serveren for at modtage og se dataene på localhost:5050.
*/
const char* WIFI_SSID = "<wifi name>";				    // update ssid
const char* WIFI_PASSWORD = "<wifi password>";			// update pw
const char* SERVER_BASE_URL = "http://<...ip...:5050";	// update ip

const int SECOND_IN_MILLIS = 1000;

// ___ data sending consts and variables ___
// Data til server
const unsigned long TCP_MESSAGE_INTERVAL = 5 * SECOND_IN_MILLIS; // Hvor ofte der sendes data til serveren, hvert 20 sekund. TODO: opdater efter behov.
unsigned long last_tcp_message_send_time = millis();
// poller server for at tjekke om der er user logget ind
const unsigned long POLL_INTERVAL = 5 * SECOND_IN_MILLIS;  // poll hvert 5. sekund
unsigned long last_poll_time = 0;
// sender update til display
unsigned long uart_transmit_timer = millis();
unsigned long UART_TRANSMIT_DELAY = 1000; // updaterer display hvert 100. millisekund.

// ___ session variables ___
bool session_in_progress = false;
uint16_t max_distance = 0; // kun til display
String session_uuid = "";

// ___ user consts and variables ___
bool user_logged_in = false;


void updateDisplay(String tekst, int size = 10);

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // UART2


    // tryk knapper og kontakter
    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LIMIT_SWITCH_BOTTOM_PIN, INPUT_PULLUP);
    pinMode(LIMIT_SWITCH_TOP_PIN, INPUT_PULLUP);

    // motor setup
    pinMode(STEPPER_STEP_PIN, OUTPUT);
    pinMode(STEPPER_DIR_PIN, OUTPUT);
    pinMode(STEPPER_ENABLE_PIN, OUTPUT);

    digitalWrite(STEPPER_ENABLE_PIN, LOW);
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_DIR_PIN, LOW);

    // time of flight sensor
    Wire.begin(TOF_SDA_PIN, TOF_SCL_PIN, 400000);
    tof.setTimeout(500);
    tof.init(); // TODO: afgør om der skal være et tjek af om init gik godt.
    tof.startContinuous();

    calibrateTofSensor();

    // connecter til wifi
    Serial.print("Forbinder til WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        updateDisplay("No network \nconnection..");
    }
    Serial.println("\nForbundet! IP: " + WiFi.localIP().toString());
}

void loop() {

    pollUserStatus(); // Checks if a user is logged in.

    difficulty = map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 10);
    tofDistance = readTofSensor() - tofBottomDistance + 15; // tof.readRangeContinuousMillimeters(); // TODO: opdater med fejlen fra kalibrationen!
    if (tofDistance > 1000) {
        tofDistance = 0;
    }

    max_distance = max(tofDistance, max_distance);

    // TIL TEST
    // Serial.print("buttom pin:");
    // Serial.println(digitalRead(LIMIT_SWITCH_BOTTOM_PIN));
    // Serial.print("top pin:");
    // Serial.println(digitalRead(LIMIT_SWITCH_TOP_PIN));
    // Serial.print("distance: ");
    // Serial.println(String(tofDistance));
    // delay(500);

    if (!session_in_progress)
        if (!user_logged_in) {
            updateDisplay("no one logged in");
        } else {
            updateDisplay("user is logged in\n ready to start");
        }

    // Start button
    if (digitalRead(START_BUTTON_PIN) == LOW && not session_in_progress) {
        session_uuid = generateUUID();
        max_distance = 0;  // resetter max distance ved start af ny session.
        if (startSession(session_uuid) == true) {
            session_in_progress = true;
            updateDisplay("Starting session");
        }
        else {
            updateDisplay("Log in first!");
        }
    }

    // Stop button
    if (digitalRead(STOP_BUTTON_PIN) == LOW && session_in_progress) {
        bool result = stopSession(session_uuid);
        session_in_progress = !result;  // opdatere session state ud fra return af stop_session.
        updateDisplay("Session has ended\n\nMax distance for session was: " + String((float)max_distance / 10) + " cm");
    }

    if (session_in_progress == true) {

        runMotor(difficulty);

        if ((millis() - TCP_MESSAGE_INTERVAL) > last_tcp_message_send_time) {

            last_tcp_message_send_time = millis();

            // if (isnan(tofDistance)) { // virker kun til floats eller doubles..
            if (tof.timeoutOccurred()) {
                updateDisplay("Fejl: Kunne ikke læse afstands sensor!");
            } else {
                updateDisplay("Distance: " + String((float)tofDistance / 10) + " cm");
                bool sendDataResult = send_session_data();

                if (sendDataResult == true) {
                    updateDisplay("Session in prograss\nDistance: " + String((float)tofDistance / 10) + " cm" + "\n\nMax distance: " + String((float)max_distance / 10) + " cm" + "\n Difficulty: " + String(difficulty));
                } else {
                    updateDisplay("kunne ikke sende data!");
                }
            }
        }
    }
}


// ___ Helper functions ___

/*
* generates an uuid for the current training session such that we can make sure
* it is the esp's stop button that starts and ends a given training session.
*
* return String: uuid for training session.
*/
String generateUUID() {
  char uuid[37];
  snprintf(uuid, sizeof(uuid),
           "%08x-%04x-4%03x-%04x-%012llx",
           (uint32_t)esp_random(),
           (uint16_t)esp_random(),
           (uint16_t)(esp_random() & 0x0FFF),
           (uint16_t)((esp_random() & 0x3FFF) | 0x8000),
           ((uint64_t)esp_random() << 32) | esp_random());
  return String(uuid);
}

/*
* initiate a new training session. The server checks that a user is logged in for the
* given training session.
*
* return boolean: true if a user is logged in to the webapp otherwise false.
*/
bool startSession(String uuid) {

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClient client;
    HTTPClient http;
    http.begin(client, String(SERVER_BASE_URL) + "/start_session");
    http.addHeader("Content-Type", "application/json");

    String body = "{\"session_uuid\":\"" + uuid + "\"}";

    int svar = http.POST(body);
    http.end();

    if (svar == 200) {
        return true;
    } else {
        return false;
    }
}

/*
* sends data to the server for the current training session. The data includes distance, difficulty and session uuid.
*/
bool send_session_data() {

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClient client;
    HTTPClient http;
    http.begin(client, String(SERVER_BASE_URL) + "/data");
    http.addHeader("Content-Type", "application/json");

    String body = "{\"distance\":" + String((float)tofDistance / 10, 1)
                + ",\"session_uuid\":\"" + session_uuid + "\""
                + ",\"difficulty\":" + String((float)difficulty / 10, 1) + "}";

    int svar = http.POST(body);
    http.end();
    // updateDisplay(svar == 200 ? "Sendt OK" : "Fejl: " + String(svar));
    if (svar == 200) {
        return true;
    } else {
        return false;
    }
}


/*
* Stops an ongoing training session.
*
* return boolean: true if training was not in progress or stopped.
*                 false if there was no connection to wifi or server responds with err.
*/
bool stopSession(String uuid) {

    if (session_in_progress == false) {
        return true; // Hvis der ikke er en træningssession i gang tælle dette som success.
    }

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClient client;
    HTTPClient http;
    http.begin(client, String(SERVER_BASE_URL) + "/end_session");
    http.addHeader("Content-Type", "application/json");

    String body = "{\"session_uuid\":\"" + uuid + "\"}";

    int svar = http.POST(body);
    http.end();

    if (svar == 200) {
        return true;
    } else {
        return false;
    }
}

/*
*  Sends a request to the server at every poll_interval times to check if there is a user that is logged in.
*  The state of user_logged_in then gets updated.
*/
void pollUserStatus() {
    if (millis() - last_poll_time < POLL_INTERVAL) {
        return;
    }
    last_poll_time = millis();

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    WiFiClient client;
    HTTPClient http;
    http.begin(client, String(SERVER_BASE_URL) + "/current_user");
    int httpCode = http.GET();

    if (httpCode == 200) {
        user_logged_in = true;
        Serial.println("Bruger logget ind");
    } else {
        user_logged_in = false;
        Serial.println("Ingen bruger logget ind");
    }
    http.end();
}

void updateDisplay(String tekst, int fontsize) {

    String stringToSend = tekst;
    if ((millis() - UART_TRANSMIT_DELAY) > uart_transmit_timer) {
        // TODO: implementer TX funktionalitet til String tekst
        mySerial.println(stringToSend);
        Serial.println(stringToSend);

        uart_transmit_timer = millis();
    }
}

/*
* Helper funktion to read from time of flight sensor
* Return: distance in millimeters
*/
uint16_t readTofSensor() {
    uint16_t tof_distance = tof.readRangeContinuousMillimeters();

    return tof_distance;
}

/*
* Running the motor to the top and bottom limit switch to calibrate the time of fligth sensor with respect to the
* limit swithces.
*/
void calibrateTofSensor() {
    Serial.println("calibrating ToF");
    digitalWrite(STEPPER_DIR_PIN, MOTOR_UP);
    delayMicroseconds(10);

    while(digitalRead(LIMIT_SWITCH_TOP_PIN) == HIGH) {
        digitalWrite(STEPPER_STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY_US);
        digitalWrite(STEPPER_STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY_US);
    }
    tofTopDistance = readTofSensor();
    Serial.println("top limit switch activated");
    Serial.print("Top pos: ");
    Serial.println(tofTopDistance);

    digitalWrite(STEPPER_DIR_PIN, MOTOR_DOWN);
    delayMicroseconds(10);

    while(digitalRead(LIMIT_SWITCH_BOTTOM_PIN) == HIGH) {
        digitalWrite(STEPPER_STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY_US);
        digitalWrite(STEPPER_STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY_US);
    }
    tofBottomDistance = readTofSensor();
    Serial.println("bottom limit switch activated");
    Serial.print("Bottom pos: ");
    Serial.println(tofBottomDistance);

    digitalWrite(STEPPER_ENABLE_PIN, HIGH);
}

/*
* Run the motor down based on difficulty
*/
void runMotor(int difficulty) {
    return; // TODO: Remove me!
    if (difficulty == 0) {
        return;
    }
    if (digitalRead(LIMIT_SWITCH_BOTTOM_PIN) == LOW) {
        return;
    }

    int stepDelay = map(difficulty, 1, 10, 1000, 100);
    int steps = map(difficulty, 1, 10, 5, 50);

    digitalWrite(STEPPER_ENABLE_PIN, LOW);

    for (int i = 0; i < steps; i++) {
        if (digitalRead(LIMIT_SWITCH_BOTTOM_PIN) == LOW) break;

        digitalWrite(STEPPER_STEP_PIN, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(STEPPER_STEP_PIN, LOW);
        delayMicroseconds(stepDelay);
    }

    digitalWrite(STEPPER_ENABLE_PIN, HIGH);
}
