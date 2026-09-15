#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>  // DHT sensor library
#include <OneButton.h>
#include <Wire.h>

// Prototypes
int Gen_snpp(int type, char* destination, int size, float reading);
void printCentered(char* text, int y, unsigned int size);
int update_Toggle(int num);
void display012(void);
void startDeepSleep(void);
void startAnimation(const char* label, int duration);
void shutdownAnimation();

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
#define BUTTON_PIN1 7
#define BUTTON_PIN2 2

DHT dht(DHTPIN, DHTTYPE);  // Object for DHT22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Object for OLED
// Sleep objects
OneButton button(BUTTON_PIN2, true);

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
  // Secure Serial connection
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
  startAnimation("LOADING...", 2500);

  // Start stuff up
  dht.begin();
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  // Settings
  display.setTextColor(SSD1306_WHITE);
  button.setPressMs(3000);
  button.attachLongPressStart(startDeepSleep);
  
  delay(1500); // You Ready?
}


// Measure running time for counting when the DHT22 would take the reading
const unsigned long run = millis();
const unsigned long reading_pause = 3000;
unsigned long current_time;
unsigned long last_time = 0;

void loop()
{
  button.tick();

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

  // Button1 code
  int reading = digitalRead(BUTTON_PIN1);
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
  last_Button_State = reading;
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

void startDeepSleep(void)
{
  // Trun off OLED
  display.clearDisplay();
  shutdownAnimation();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  while (digitalRead(BUTTON_PIN2) == LOW)
  {
    delay(50);
  }
  delay(1000);

  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN2, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
}

void startAnimation(const char* label, int duration)
{
  // Dimensions for the loading bar outline
  int barWidth = 100;
  int barHeight = 12;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = 36;
  
  int padding = 2; // Space between outer border and inner fill
  int maxFillWidth = barWidth - (padding * 2);
  int steps = 50; // Smoothness of the fill
  int delayPerStep = duration / steps;

  for (int i = 0; i <= steps; i++) {
    display.clearDisplay();
    display.setTextSize(1);
    
    // Center the header text dynamically
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 16);
    display.print(label);

    // 2. Draw Outer Rounded Frame
    display.drawRoundRect(barX, barY, barWidth, barHeight, 3, SSD1306_WHITE);

    // 3. Calculate and Draw Inner Progress Fill
    int currentFillWidth = map(i, 0, steps, 0, maxFillWidth);
    if (currentFillWidth > 0) {
      display.fillRoundRect(barX + padding, barY + padding, currentFillWidth, barHeight - (padding * 2), 2, SSD1306_WHITE);
    }

    display.display();
    delay(delayPerStep);
  }
  delay(200);
}

void shutdownAnimation()
{
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  // Screen squashes down into horizontal line
  for (int h = SCREEN_HEIGHT / 2; h > 1; h -= 2) {
    display.clearDisplay();
    display.fillRect(0, centerY - h, SCREEN_WIDTH, h * 2, SSD1306_WHITE);
    display.display();
    delay(15);
  }

  // Line shrinks into dot)
  for (int w = SCREEN_WIDTH / 2; w > 2; w -= 4) {
    display.clearDisplay();
    display.fillRect(centerX - w, centerY - 1, w * 2, 2, SSD1306_WHITE);
    display.display();
    delay(10);
  }

  // Flash and fade dot
  display.clearDisplay();
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  display.display();
  delay(100);
}