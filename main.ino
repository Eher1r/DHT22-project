#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>  // DHT sensor library
#include <Wire.h>

// DHT22 Sensor
#define DHTPIN 4  // Pin on ESP connected to DATA pin on DHT22
#define DHTTYPE DHT22  // Type of DHT sensor defined

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define SDA_PIN 8
#define SCL_PIN 9

DHT dht(DHTPIN, DHTTYPE);  // Object for DHT22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Object for OLED

void setup()
{
  // Start stuff up
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(1000);

  unsigned long start = millis();
  while (!Serial && (millis() - start < 4000))
  {
    delay(100);
  }
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("Failed to connect with OLED display :()");
    for (;;);
  }

  Serial.println("--- ESP32-C3 DHT22 Project Initialization ---");

  // OLED screen settings
  display.setTextColor(SSD1306_WHITE);

  delay(1500);
  dht.begin();
}

const unsigned int run = millis();  // Time initialization begun

void loop()
{
  delay(3000);  // 3 seconds for the DHT22 to take in the reading
  
  // CLears display and define variable
  display.clearDisplay();
  display.setCursor(32, 0);
  bool error = false;
  int needed;
  int length;

  unsigned int time = millis() - run;  // Time after initializing, the reading was taken

  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  // Error checking
  if (isnan(humidity) || isnan(temp))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  for (int i = 0; i < 2; i++)
  {
    // Get size of message
    if (i == 0)
    {
      needed = Gen_snpp(i, NULL, 0, humidity);
    }
    else if (i == 1)
    {
      needed = Gen_snpp(i, NULL, 0, temp);
    }
    else
    {
      needed = -1;
    }

    // Error checking
    if (needed <= 0 || needed > 168)
    {
      error = true;
    }

    // Create corresponding result variable
    if (i == 0)
    {
      char result0[(needed + 1)];  // Include null terminator
      length = Gen_snpp(i, result0, sizeof(result0), humidity);
    }
    else if (i == 1)
    {
      char result1[(needed + 1)];
      length = Gen_snpp(i, result1, sizeof(result1), temp);
    }
    else
    {
      length = -1;
    }
    
    // Error checking
    if (length <= 0 || length > 168)
    {
      error = true;
    }
  }
  
  // Print reading out
  if (!error)
  {  
    printCentered(result0, 22, 1);
    printCentered(result1, 34, 1);
    display.display();
    Serial.printf("%.1fs : %s  |  %s\n", (time / 1000.0), result0, result1);
  }
  else
  {
    Serial.println("Error displaying text :(");
  }
}

// snprintf for Humidity and Temperature respectively
int Gen_snpp(int type, char* destination, int size, float reading)
{
  if (type == 0)
  {
    return snprintf(destination, size, "Humidity: %.3f%%", reading);
  }
  else if (type == 1)
  {
    return snprintf(destination, size, "Temperature: %.3fC", reading);
  }
  return -1;  // Both conditions above failed to run
}