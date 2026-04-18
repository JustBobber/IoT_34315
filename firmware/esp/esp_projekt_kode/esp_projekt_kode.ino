#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>

// hardware setup

const int START_BUTTON_PIN = 21;
const int STOP_BUTTON_PIN = 22;

bool session_in_progress = false;
String session_uuid = "";

/*
* Sørg for at starte serveren for at modtage og se dataene på localhost:5050.
*/
const char* WIFI_SSID = "<wifi name>";				// update ssid
const char* WIFI_PASSWORD = "<wifi password>";			// update pw
const char* SERVER_URL = "http://<...ip...:5050/data";		// update ip

const int INTERVAL_SEK = 20;  // Hvor ofte der sendes
const int SECOND_IN_MILLIS = 1000;
const unsigned long TCP_MESSAGE_INTERVAL = 20 * SECOND_IN_MILLIS;
unsigned long last_tcp_message_send_time = millis();

const unsigned long POLL_INTERVAL = 5 * SECOND_IN_MILLIS;  // poll hvert 5. sekund
unsigned long last_poll_time = 0;
bool user_logged_in = false;


void setup() {
  Serial.begin(115200);

  Serial.print("Forbinder til WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nForbundet! IP: " + WiFi.localIP().toString());

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

}

void loop() {

    pollUserStatus();

    // Start button
    if (digitalRead(START_BUTTON_PIN) == LOW && not session_in_progress) {
        session_uuid = generateUUID();
        if (startSession(session_uuid) == true) {
            session_in_progress = true;
            Serial.print("Starting session with uuid: ");
            Serial.println(session_uuid);
        }
        else {
            Serial.println("display.write : 'Log ind';");
        }
    }

    // Stop button
    if (digitalRead(STOP_BUTTON_PIN) == LOW && session_in_progress) {
        Serial.println("Stopping session");

        bool result = stopSession(session_uuid);
        session_in_progress = !result;  // updatere session state ud fra return af stop_session.
                                        // TODO: find ud af om det er hensigtsmessigt..

    }

    if (session_in_progress == true && (millis() - TCP_MESSAGE_INTERVAL) > last_tcp_message_send_time) {

        last_tcp_message_send_time = millis();
        float distance = (float)rand() / RAND_MAX * 10.0f;  // tof.readDistance();

        if (isnan(distance)) {
            Serial.println("Fejl: Kunne ikke læse ... sensor!");
        } else {
            Serial.printf("Distance: %.3f (some unit) \n", distance);
            if (WiFi.status() == WL_CONNECTED) {
                WiFiClient client;
                HTTPClient http;
                http.begin(client, String(SERVER_BASE_URL) + "/data");
                http.addHeader("Content-Type", "application/json");

                String body = "{\"distance\":" + String(distance, 3) + ",\"session_uuid\":\"" + session_uuid + "\"}";

                int svar = http.POST(body);
                Serial.println(svar == 200 ? "Sendt OK" : "Fejl: " + String(svar));
                http.end();
            }

        }
    }
}

/*
* generates an uuid for the current training session such that we can make sure
* it is the esp's stop button that starts and ends a given training session.
*
* return: String uuid for training session.
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
* return: boolean, true if a user is logged in to the webapp otherwise false.
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

    int httpCode = http.POST(body);
    http.end();

    if (httpCode == 200) {
        // display.print("Logget ind!");
        return true;
    } else {
        // display.print("Log ind!");  // 401
        return false;
    }
}

/*
* Stops an ongoing training session.
*
* return boolean: true if training was not in progress or stopped.
                  false if there was no connection to wifi or server responds with err.
*/
bool stopSession(String uuid) {

    if (session_in_progress == false) {
        return true; // if session is not in progress it counts as success.
    }

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClient client;
    HTTPClient http;
    http.begin(client, String(SERVER_BASE_URL) + "/end_session");
    http.addHeader("Content-Type", "application/json");

    String body = "{\"session_uuid\":\"" + uuid + "\"}";

    int httpCode = http.POST(body);
    http.end();

    if (httpCode == 200) {
        // display.print("Logget ind!");
        return true;
    } else {
        // display.print("Log ind!");  // 401
        return false;
    }
}

/*
*  Sends a request to the server at every poll_interval times to check if there is a user that is logged in.
*  The state of user_logged_in then gets updated.
*/
void pollUserStatus() {
    if (millis() - last_poll_time < POLL_INTERVAL) return;
    last_poll_time = millis();

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

