#include <Arduino.h>
#include <DHTesp.h>

#define DHTPIN 4  // DATA pin on DHT22 is connected to Pin 2

DHTesp dht;

void setup() 
{
  Serial.begin(115200);

  // Wait for connection between Serial Monitor and ESP32
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000))  // Makes sure Serial is connected, but will still run after 3 seconds
  {
    delay(10);
  }

  Serial.println("--- ESP32-C3 DHT22 Initialization ---");

  pinMode(DHTPIN, INPUT_PULLUP);  // Suppresses noise
  delay(100);

  // Initialize DHTesp
  dht.setup(DHTPIN, DHTesp::DHT22);
  Serial.println("Setup Complete");
  delay(2000);  // Wait to make sure setup is complete
}

void loop() 
{
  delay(3000);  // Wait for DHT22 to take reading

  TempAndHumidity data = dht.getTempAndHumidity();

  // Check read status
  if (dht.getStatus() != DHTesp::ERROR_NONE) 
  {
    Serial.print("Sensor Error: ");
    Serial.println(dht.getStatusString());
    return;
  }

  // Print formatted output
  Serial.printf("Humidity: %.1f%%  |  Temperature: %.1f°C\n", data.humidity, data.temperature);
}
