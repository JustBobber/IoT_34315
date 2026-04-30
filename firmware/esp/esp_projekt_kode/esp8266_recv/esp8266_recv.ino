
// Var for the msg received from esp32
String incoming = "";

// RECEIVE FUNCTION  -- If you want to use it on the esp32: Use same, but change Serial to mySeral (in the function only)
String receiveMessage() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      Serial.print("Received: ");
      Serial.println(incoming);
      incoming = "";
      return incoming;
    } else {
      incoming += c;
    }
  }
  return "";
}

void setup() {
  Serial.begin(9600);  // Same UART for RX/TX
  Serial.println("ESP8266 Ready (Send + Receive)");
}

void loop() {

  // Store message in var
  String msg = receiveMessage(); // Always listen
  msg.trim();

  if (msg.length() > 0) {
    Serial.println(msg);
  }

  delay(1000);
}









