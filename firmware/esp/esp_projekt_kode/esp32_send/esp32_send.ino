#define RXD2 16
#define TXD2 17

HardwareSerial mySerial(2);


String incoming = "";

void receiveMessage() {
  while (mySerial.available()) {
    char c = mySerial.read();

    if (c == '\n') {
      Serial.print("Received: ");
      //String msg = buffer;
      Serial.println(incoming);
      incoming = "";
      //return msg;   // return complete message
    } else {
      incoming += c;
    }
  }
  //return ""; // no full message yet
}

// SEND FUNCTION
void sendMessage(String msg) {
  mySerial.println(msg);
}


void setup() {
  Serial.begin(115200);                         // Debug monitor
  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2); // UART2
  Serial.println("ESP32 Ready (Send + Receive)");
}



void loop() {

  // ----------------------------------------//

  /* Fuck this, remove it - only for demo. Just use "sendMessage" */
  // Example sending every 2 seconds
  static unsigned long lastSend = 0;

  if (millis() - lastSend > 2000) {
    sendMessage("Hello Liv");
    lastSend = millis();
  }
  // ----------------------------------------//



}



