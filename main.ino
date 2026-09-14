#include <Arduino.h>
#include <DHTesp.h>

#define DHTPIN 2  // Connected to physical pin IO4 on ESP32-C3 SuperMini

DHTesp dht;

void setup() {
  Serial.begin(115200);

  // Wait up to 3 seconds for Serial Monitor connection
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }

  Serial.println("\n--- ESP32-C3 DHT22 Initialization ---");

  // Force ESP32 internal pull-up resistor on GPIO 4
  pinMode(DHTPIN, INPUT_PULLUP);
  delay(100);

  // Initialize DHTesp
  dht.setup(DHTPIN, DHTesp::DHT22);
  
  // Allow sensor power rail to stabilize
  delay(2000);
}

void loop() {
  // DHT22 sampling interval must be at least 2000ms
  delay(2000);

  TempAndHumidity data = dht.getTempAndHumidity();

  // Check read status
  if (dht.getStatus() != DHTesp::ERROR_NONE) {
    Serial.print("Sensor Error: ");
    Serial.println(dht.getStatusString());
    return;
  }

  // Print formatted output
  Serial.printf("Humidity: %.1f%%  |  Temperature: %.1f°C\n", data.humidity, data.temperature);
}
