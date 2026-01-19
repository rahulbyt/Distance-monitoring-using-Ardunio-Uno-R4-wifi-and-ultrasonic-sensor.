#include <WiFiS3.h>

char ssid[] = "BT-SKA87S";  // mutable char array
char password[] = "UdA3kxgXeMTCgH";

const char* server = "api.thingspeak.com";
const String apiKey = "MR0OTR4ASUKC2PUA";

const int trigPin = 9;
const int echoPin = 10;
const int ledPin = 13;
const int buzzerPin = 11;
const int ALERT_DISTANCE = 20;

long duration;
float distance;

WiFiClient client;

unsigned long lastThingSpeakTime = 0;
const unsigned long thingSpeakInterval = 15000; // 15s
unsigned long lastMeasureTime = 0;
const unsigned long measureInterval = 200; // 0.2s for distance

// ------------------ Distance measurement ------------------
float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return (duration * 0.034) / 2;
}

// ------------------ Setup ------------------
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
}

// ------------------ Loop ------------------
void loop() {
  unsigned long currentMillis = millis();

  // Fast distance measurement every 200ms
  if (currentMillis - lastMeasureTime >= measureInterval) {
    lastMeasureTime = currentMillis;

    distance = measureDistance();
    Serial.print("Distance: ");
    Serial.println(distance);

    // LED & buzzer react immediately
    if (distance > 0 && distance <= ALERT_DISTANCE) {
      digitalWrite(ledPin, HIGH);

      // Buzzer beep speed depends on distance
      int beepDelay = map(distance, 1, ALERT_DISTANCE, 50, 300);
      digitalWrite(buzzerPin, HIGH);
      delay(beepDelay);
      digitalWrite(buzzerPin, LOW);
      delay(beepDelay);
    } else {
      digitalWrite(ledPin, LOW);
      digitalWrite(buzzerPin, LOW);
    }
  }

  // ThingSpeak upload every 15s (non-blocking)
  if (currentMillis - lastThingSpeakTime >= thingSpeakInterval) {
    lastThingSpeakTime = currentMillis;

    if (client.connect(server, 80)) {
      String url = "/update?api_key=" + apiKey + "&field1=" + String(distance);
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" +
                   "Connection: close\r\n\r\n");
      client.stop();
      Serial.println("Data sent to ThingSpeak");
    } else {
      Serial.println("Connection failed");
    }
  }
}
