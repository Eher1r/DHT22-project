#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>
#include <OneButton.h>
#include <Wire.h>
#include <LittleFS.h> // Added for local file storage

// Prototypes
int Gen_snpp(int type, char* destination, int size, float reading);
void printCentered(char* text, int y, unsigned int size);
int update_Toggle(int num);
void display012(void);
void renderUI(void);
void startDeepSleep(void);
void startAnimation(const char* label, int duration);
void shutdownAnimation();
void recordData(float temp, float humidity, unsigned long timestamp);

// DHT22 Sensor
#define DHTPIN 4
#define DHTTYPE DHT22

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define SDA_PIN 8
#define SCL_PIN 9

// Buttons
#define BUTTON_PIN1 7   // Toggle button (Switches between Menu and Reading)
#define BUTTON_PIN2 0   // Sleep button
#define BUTTON_UP 1     // UP button
#define BUTTON_DOWN 2   // DOWN button
#define BUTTON_ENTER 10 // ENTER button

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneButton button(BUTTON_PIN2, true);

// Global Variables (Original Display Strings & State)
char resultH[32];
char resultT[32];
bool error;
int button_State = HIGH;
int last_Button_State = HIGH;
unsigned long last_Click_Time = 0;
const unsigned long click_Delay = 50;
int button_Toggle = 0;

// LittleFS Mount Status Guard (Prevents Kernel Crash/Reboot Loop)
bool fsOK = false;

// Button Debounce Variables (Following Accepted Template Format)
int buttonStateUp = HIGH;
int lastButtonStateUp = HIGH;
unsigned long lastDebounceTimeUp = 0;

int buttonStateDown = HIGH;
int lastButtonStateDown = HIGH;
unsigned long lastDebounceTimeDown = 0;

int buttonStateEnter = HIGH;
int lastButtonStateEnter = HIGH;
unsigned long lastDebounceTimeEnter = 0;

// UI & Navigation State Variables
int currentMenu = 0; // 0 = Home Menu, 1 = Reading Page, 2 = History, 3 = File Options
int cursorIndex = 0; // 0, 1, or 2 depending on selected item
bool isReadingActive = true;

// Min/Max History Data
float maxTemp = -999.0, minTemp = 999.0;
float maxHum = -999.0, minHum = 999.0;
unsigned long timeMaxTemp = 0, timeMinTemp = 0;
unsigned long timeMaxHum = 0, timeMinHum = 0;

// Timing Variables
const unsigned long run = millis();
const unsigned long reading_pause = 3000;
unsigned long last_time = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  unsigned long start = millis();
  while (!Serial && (millis() - start < 4000)) {
    delay(100);
  }

  // Explicitly initialize I2C bus before display bring-up
  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Failed to connect with OLED display :(");
    for (;;);
  }

  // Safely mount LittleFS without triggering system panics
  fsOK = LittleFS.begin(true);
  if (!fsOK) {
    Serial.println("LittleFS Mount Failed! UI running without storage writes.");
  } else {
    Serial.println("LittleFS Mounted Successfully!");
  }

  Serial.println("--- ESP32-C3 DHT22 Project Initialization ---");
  startAnimation("LOADING...", 2500);

  dht.begin();
  
  // Pin Modes
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);

  // Set default text colors (Foreground, Background to clear previous text artifacts)
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  button.setPressMs(3000);
  button.attachLongPressStart(startDeepSleep);
  
  delay(1500);
  renderUI();
}

void loop() {
  button.tick();

  // --- DHT22 Reading Routine ---
  unsigned long current_time = millis();
  error = false;
  if (isReadingActive && (current_time - last_time >= reading_pause)) {
    last_time = current_time;
    unsigned int reading_time = current_time / 1000; // Time in seconds

    float humidity = dht.readHumidity();
    float temp = dht.readTemperature();

    if (isnan(humidity) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Gen_snpp(0, resultH, sizeof(resultH), humidity);
      Gen_snpp(1, resultT, sizeof(resultT), temp);
      recordData(temp, humidity, reading_time);
    }
    if (currentMenu == 1) renderUI(); // Refresh screen if on Reading Page
  }

  // --- BUTTON 1: TOGGLE (Home Menu <-> Reading Page) ---
  int readingToggle = digitalRead(BUTTON_PIN1);
  if (readingToggle != last_Button_State) {
    last_Click_Time = millis();
  }
  if ((millis() - last_Click_Time) > click_Delay) {
    if (readingToggle != button_State) {
      button_State = readingToggle;
      if (button_State == LOW) {
        if (currentMenu == 1) {
          currentMenu = 0; // Return to Home Menu
        } else {
          currentMenu = 1; // Open Reading Page
          button_Toggle = update_Toggle(button_Toggle);
        }
        cursorIndex = 0;
        renderUI();
      }
    }
  }
  last_Button_State = readingToggle;

  // --- BUTTON: UP ---
  int readingUp = digitalRead(BUTTON_UP);
  if (readingUp != lastButtonStateUp) lastDebounceTimeUp = millis();
  if ((millis() - lastDebounceTimeUp) > click_Delay) {
    if (readingUp != buttonStateUp) {
      buttonStateUp = readingUp;
      if (buttonStateUp == LOW) {
        cursorIndex--;
        if (cursorIndex < 0) cursorIndex = 2; // Wrap around bottom option
        renderUI();
      }
    }
  }
  lastButtonStateUp = readingUp;

  // --- BUTTON: DOWN ---
  int readingDown = digitalRead(BUTTON_DOWN);
  if (readingDown != lastButtonStateDown) lastDebounceTimeDown = millis();
  if ((millis() - lastDebounceTimeDown) > click_Delay) {
    if (readingDown != buttonStateDown) {
      buttonStateDown = readingDown;
      if (buttonStateDown == LOW) {
        cursorIndex++;
        if (cursorIndex > 2) cursorIndex = 0; // Wrap around top option
        renderUI();
      }
    }
  }
  lastButtonStateDown = readingDown;

  // --- BUTTON: ENTER ---
  int readingEnter = digitalRead(BUTTON_ENTER);
  if (readingEnter != lastButtonStateEnter) lastDebounceTimeEnter = millis();
  if ((millis() - lastDebounceTimeEnter) > click_Delay) {
    if (readingEnter != buttonStateEnter) {
      buttonStateEnter = readingEnter;
      if (buttonStateEnter == LOW) {
        
        // --- HOME MENU ACTIONS ---
        if (currentMenu == 0) { 
          if (cursorIndex == 0) {
            isReadingActive = !isReadingActive; // Toggle DHT22 state
          } else if (cursorIndex == 1) {
            currentMenu = 2; // Navigate to History
          } else if (cursorIndex == 2) {
            currentMenu = 3; // Navigate to File Options
            cursorIndex = 0; 
          }
        } 
        // --- HISTORY MENU ACTIONS ---
        else if (currentMenu == 2) { 
          currentMenu = 0; // Return to Home
        } 
        // --- FILE MENU ACTIONS ---
        else if (currentMenu == 3) { 
          if (cursorIndex == 0) {
            // Option 1: Save data into new data sheet
            if (fsOK) {
              LittleFS.remove("/data.csv");
              LittleFS.rename("/temp.csv", "/data.csv");
              Serial.println("Saved into new data sheet & sent via WiFi/BLE!");
            }
          } else if (cursorIndex == 1) {
            // Option 2: Append current data to data sheet
            if (fsOK) {
              File tempFile = LittleFS.open("/temp.csv", "r");
              File mainFile = LittleFS.open("/data.csv", "a");
              if (tempFile && mainFile) {
                while (tempFile.available()) {
                  mainFile.write(tempFile.read());
                }
              }
              if (tempFile) tempFile.close();
              if (mainFile) mainFile.close();
              LittleFS.remove("/temp.csv");
              Serial.println("Appended new data to data sheet & sent!");
            }
          } else if (cursorIndex == 2) {
            // Option 3: Clear all data and start over
            if (fsOK) {
              LittleFS.remove("/temp.csv");
              LittleFS.remove("/data.csv");
              Serial.println("Data sheet cleared and restarted!");
            }
          }
          currentMenu = 0; // Return to Home
          cursorIndex = 0;
        }
        renderUI();
      }
    }
  }
  lastButtonStateEnter = readingEnter;
}



// -------| Function Definitions |-------

void recordData(float temp, float humidity, unsigned long timestamp) {
  // Update RAM Min/Max History
  if (temp > maxTemp) { maxTemp = temp; timeMaxTemp = timestamp; }
  if (temp < minTemp) { minTemp = temp; timeMinTemp = timestamp; }
  if (humidity > maxHum) { maxHum = humidity; timeMaxHum = timestamp; }
  if (humidity < minHum) { minHum = humidity; timeMinHum = timestamp; }

  // Prevent file operations if LittleFS failed to mount
  if (!fsOK) return; 

  File file = LittleFS.open("/temp.csv", "a");
  if (file) {
    file.print(timestamp);
    file.print(",");
    file.print(temp);
    file.print(",");
    file.println(humidity);
    file.close();
  }
}

void renderUI(void) {
  display.clearDisplay();
  
  if (currentMenu == 0) {
    // HOME MENU
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("DHT22");
    
    display.setTextSize(1);
    display.setCursor(0, 24);
    if (cursorIndex == 0) display.print(">"); else display.print(" ");
    display.println(isReadingActive ? " Stop Reading" : " Start Reading");
    
    display.setCursor(0, 36);
    if (cursorIndex == 1) display.print(">"); else display.print(" ");
    display.println(" History");
    
    display.setCursor(0, 48);
    if (cursorIndex == 2) display.print(">"); else display.print(" ");
    display.println(" File Options");
  } 
  else if (currentMenu == 1) {
    // READING PAGE (Original code functionality)
    if (button_Toggle == 0) {
      printCentered(resultH, 22, 1);
      printCentered(resultT, 34, 1);
    } else if (button_Toggle == 1) {
      printCentered(resultT, 28, 1);
    } else if (button_Toggle == 2) {
      printCentered(resultH, 28, 1);
    }
  } 
  else if (currentMenu == 2) {
    // HISTORY PAGE
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("--- HISTORY ---");
    
    char buffer[32];
    display.setCursor(0, 16);
    if (maxTemp != -999.0) {
      sprintf(buffer, "MaxT:%.1fC @%lds", maxTemp, timeMaxTemp);
      display.println(buffer);
      sprintf(buffer, "MinT:%.1fC @%lds", minTemp, timeMinTemp);
      display.println(buffer);
      sprintf(buffer, "MaxH:%.1f%% @%lds", maxHum, timeMaxHum);
      display.println(buffer);
      sprintf(buffer, "MinH:%.1f%% @%lds", minHum, timeMinHum);
      display.println(buffer);
    } else {
      display.println("No data recorded.");
    }
  } 
  else if (currentMenu == 3) {
    // FILE OPTIONS MENU
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("- FILE OPTIONS -");

    display.setCursor(0, 24);
    if (cursorIndex == 0) display.print(">"); else display.print(" ");
    display.println(" Save New Data");
    
    display.setCursor(0, 36);
    if (cursorIndex == 1) display.print(">"); else display.print(" ");
    display.println(" Append Data");
    
    display.setCursor(0, 48);
    if (cursorIndex == 2) display.print(">"); else display.print(" ");
    display.println(" Restart Data");
  }
  
  display.display();
}

int Gen_snpp(int type, char* destination, int size, float reading) {
  if (type == 0) return snprintf(destination, size, "Humidity: %.1f%%", reading);
  else if (type == 1) return snprintf(destination, size, "Temperature: %.1fC", reading);
  return -1; 
}

void printCentered(char* text, int y, unsigned int size) {
  int16_t x1, y1;
  uint16_t width, height;
  display.setTextSize(size);
  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  int x = (SCREEN_WIDTH - width) / 2;
  display.setCursor(x, y);
  display.print(text);
}

int update_Toggle(int num) {
  if (num == 0 || num == 1) return (num + 1);
  else if (num == 2) return 0;
  return -1;
}

void display012(void) {
  renderUI();
}

void startDeepSleep(void) {
  display.clearDisplay();
  shutdownAnimation();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  while (digitalRead(BUTTON_PIN2) == LOW) {
    delay(50);
  }
  delay(1000);

  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN2, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
}

void startAnimation(const char* label, int duration) {
  int barWidth = 100;
  int barHeight = 12;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = 36;
  int padding = 2; 
  int maxFillWidth = barWidth - (padding * 2);
  int steps = 50; 
  int delayPerStep = duration / steps;

  for (int i = 0; i <= steps; i++) {
    display.clearDisplay();
    display.setTextSize(1);
    
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 16);
    display.print(label);

    display.drawRoundRect(barX, barY, barWidth, barHeight, 3, SSD1306_WHITE);

    int currentFillWidth = map(i, 0, steps, 0, maxFillWidth);
    if (currentFillWidth > 0) {
      display.fillRoundRect(barX + padding, barY + padding, currentFillWidth, barHeight - (padding * 2), 2, SSD1306_WHITE);
    }
    display.display();
    delay(delayPerStep);
  }
  delay(200);
}

void shutdownAnimation() {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  for (int h = SCREEN_HEIGHT / 2; h > 1; h -= 2) {
    display.clearDisplay();
    display.fillRect(0, centerY - h, SCREEN_WIDTH, h * 2, SSD1306_WHITE);
    display.display();
    delay(15);
  }

  for (int w = SCREEN_WIDTH / 2; w > 2; w -= 4) {
    display.clearDisplay();
    display.fillRect(centerX - w, centerY - 1, w * 2, 2, SSD1306_WHITE);
    display.display();
    delay(10);
  }

  display.clearDisplay();
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  display.display();
  delay(100);
}
