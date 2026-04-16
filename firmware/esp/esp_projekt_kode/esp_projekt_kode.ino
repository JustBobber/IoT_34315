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

String dummy_uuid = "";

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

  // genererer et dummy uuid til test.
  // dummy_uuid = generateUUID();
  dummy_uuid = "565a9b84-699c-45b0-8458-d738679e63fa";
}

void loop() {


    //TODO: fix så den bruger millis()
        if (digitalRead(START_BUTTON_PIN) == LOW && not session_in_progress) {
            session_uuid = generateUUID();
            if (startSession(session_uuid) == true) {
                session_in_progress = true;
            }
            else {
                Serial.println("display.write : 'Log ind';");
            }
        } 
    
    // TODO: fix så den bruuger millis()
    if (digitalRead(STOP_BUTTON_PIN) == LOW && session_in_progress) {
        session_in_progress = false;
    }

    if (session_in_progress == true) {

        float distance = (float)rand() / RAND_MAX * 10.0f;  // tof.readDistance();

        if (isnan(distance)) {
            Serial.println("Fejl: Kunne ikke læse ... sensor!");
        } else {
            Serial.printf("Distance: %.3f (some unit) \n", distance);
            if (WiFi.status() == WL_CONNECTED) {
                WiFiClient client;
                HTTPClient http;
                http.begin(client, SERVER_URL);
                http.addHeader("Content-Type", "application/json");

                String body = "{\"distance\":" + String(distance, 3) + ",\"session_uuid\":\"" + dummy_uuid + "\"}";

                int svar = http.POST(body);
                Serial.println(svar == 200 ? "Sendt OK" : "Fejl: " + String(svar));
                http.end();
            }
            
        }

        // TODO change me to if millis() > ...
        delay(INTERVAL_SEK * 1000);
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
* return: boolean, true if a user is logged in otherwise false.
*/
bool startSession(String uuid) {
  HTTPClient http;
  WiFiClient client;
  http.begin(client, "http://.../start_session");
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
