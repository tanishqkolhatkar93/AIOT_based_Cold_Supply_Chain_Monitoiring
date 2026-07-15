#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define WIFI_SSID "Oplus"
#define WIFI_PASSWORD "12345678"
#define THINGSPEAK_WRITE_API_KEY "HTOUR5TXHL52L0L8"

// ThingSpeak URL
String server = "http://api.thingspeak.com/update?api_key=" + String(THINGSPEAK_WRITE_API_KEY);

Adafruit_BMP280 bmp;

// Ultrasonic Pins
#define TRIG_PIN 13
#define ECHO_PIN 12

// Soil Moisture ADC Pin
#define SOIL_PIN 36

unsigned long lastUpdate = 0;
const long updateInterval = 20000;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

int getMoisture() {
  int val = analogRead(SOIL_PIN);
  return map(val, 4095, 0, 0, 100);  // Convert to %
}

void setup() {
  Serial.begin(115200);
  WiFi.disconnect();
  connectWiFi();

  Wire.begin();
  if (!bmp.begin(0x76)) {
    Serial.println("BMP not found!");
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(SOIL_PIN, INPUT);
}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    float temp = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;
    float distance = getDistanceCM();
    int moisture = getMoisture();

    int spoilageRisk = (temp > 8 || moisture > 50) ? 90 : 20;

    Serial.println("Sending Data to ThingSpeak...");
    String url = server +
      "&field1=" + String(temp) +
      "&field2=" + String(pressure) +
      "&field3=" + String(distance) +
      "&field4=" + String(moisture) +
      "&field5=" + String(spoilageRisk);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    http.end();

    Serial.print("Update Status: ");
    Serial.println(httpCode);
  }
}

