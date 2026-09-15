#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>  // DHT sensor library
#include <Wire.h>

int Gen_snpp(int type, char* destination, int size, float reading);
void printCentered(char* text, int y, unsigned int size);
int update_Toggle(int num);
void display012(void);

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

// Buttons
#define BUTTON_PIN 7

DHT dht(DHTPIN, DHTTYPE);  // Object for DHT22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Object for OLED

// Global variables
char resultH[32];
char resultT[32];
bool error;
int button_State = HIGH;
int last_Button_State = HIGH;
unsigned long last_Click_Time = 0;
const unsigned long click_Delay = 50;
int button_Toggle = 0;

void setup()
{
  // Start stuff up
  Serial.begin(115200);
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

  // Start stuff up
  dht.begin();
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Settings
  display.setTextColor(SSD1306_WHITE);

  delay(1500); // You Ready?
}


// Measure running time for counting when the DHT22 would take the reading
const unsigned long run = millis();
const unsigned long reading_pause = 5000;
unsigned long current_time;
unsigned long last_time = 0;

void loop()
{
  // Check if it is time to take the reading
  unsigned long current_time = millis() - run;
  error = false; // For error checking
  if (current_time - last_time >= reading_pause)
  {
    // Define variable
    int lengthH;
    int lengthT;

    unsigned int reading_time = millis() - run;  // Time after initializing, the reading was taken
    last_time = millis() - run;

    // Take readings
    float humidity = dht.readHumidity();
    float temp = dht.readTemperature();

    // Error checking
    if (isnan(humidity) || isnan(temp))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    lengthH = Gen_snpp(0, resultH, sizeof(resultH), humidity);
    lengthT = Gen_snpp(1, resultT, sizeof(resultT), temp);
    
    // Error checking
    if (lengthH <= 0 || lengthT <= 0 || lengthH > 168 || lengthT > 168)
    {
      error = true;
    }

    display012();
  }

  // Button code
  int reading = digitalRead(BUTTON_PIN);
  int c = 0;
  if (reading != last_Button_State)
  {
    last_Click_Time = millis();
  }

  if ((millis() - last_Click_Time) > click_Delay)
  {
    if (reading != button_State)
    {
      button_State = reading;
      if (button_State == LOW)
      {
        button_Toggle = update_Toggle(button_Toggle);
        display012();
      }
    }
  }
}


// -------| Function Definitions |-------


int Gen_snpp(int type, char* destination, int size, float reading)  // snprintf for Humidity and Temperature respectively
{
  if (type == 0)
  {
    return snprintf(destination, size, "Humidity: %.1f%%", reading);
  }
  else if (type == 1)
  {
    return snprintf(destination, size, "Temperature: %.1fC", reading);
  }
  return -1;  // Both conditions above failed to run
}

void printCentered(char* text, int y, unsigned int size)  // Centers the text and prints it out
{
  // Variables used in .getTextBounds
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.setTextSize(size);
  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  int x = (SCREEN_WIDTH - width) / 2;  // Calculate the start point which allow text to be centered
  display.setCursor(x, y);
  display.print(text);
}

int update_Toggle(int num)
{
  if (num == 0 || num == 1)
  {
    return (num + 1);
  }
  else if (num == 2)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

void display012(void)
{
  display.clearDisplay();
  if (button_Toggle == 0)  // Both Humidity and Temperature
  {
    printCentered(resultH, 22, 1);
    printCentered(resultT, 34, 1);
  }
  else if (button_Toggle == 1)  // Just Temperature
  {
    printCentered(resultT, 28, 1);
  }
  else if (button_Toggle == 2)  // Just Humidity
  {
    printCentered(resultH, 28, 1);
  }
  else
  {
    display.println("Error on displaying Temperature/Humidity");
  }
  display.display();
  Serial.printf("Displayed\n");
}