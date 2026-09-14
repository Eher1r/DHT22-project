#include <Arduino.h>
#include <DHT.h>  // DHT sensor library

#define DHTPIN 4  // Pin on ESP connected to DATA pin on DHT22
#define DHTTYPE DHT22  // Type of DHT sensor defined

DHT dht(DHTPIN, DHTTYPE);  // Creates DHT object for future use

void setup()
{
  Serial.begin(115200);
  
  // Wait for connection
  while (!Serial)
  {
    delay(100);
  }
  Serial.println("Connected!");

  delay(1500);
  dht.begin();
}

void loop()
{
  delay(2000);  // 2 seconds for the DHT22 to take in the reading

  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  // Error checking
  if (isnan(humidity) || isnan(temp))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print reading out
  Serial.printf("Humidity: %f%  |  Temperature: %f°C\n", humidity, temp);
}
