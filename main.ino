#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>
#include <LittleFS.h>
#include <OneButton.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

// Prototypes
void recordData(float temp, float humidity, unsigned long timestamp);
void renderUI(void);
int Gen_snpp(int type, char* destination, int size, float reading);
void printCentered(const char* text, int y, unsigned int size);
int update_Toggle(int num);
void display012(void);
void handleFileDownload(void);
void handleRoot(void);
void startDeepSleep(void);
void startAnimation(const char* label, int duration);
void shutdownAnimation();

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

// Global Variables
char resultH[32];
char resultT[32];
bool error;
int button_State = HIGH;
int last_Button_State = HIGH;
unsigned long last_Click_Time = 0;
const unsigned long click_Delay = 50;

// Set default display mode to 0 (Both Temp & Humidity displayed)
int button_Toggle = 0; 

// LittleFS Mount Guard Flag
bool fsOK = false;

// Button Debounce Variables
int buttonStateUp = HIGH;
int lastButtonStateUp = HIGH;
unsigned long lastDebounceTimeUp = 0;

int buttonStateDown = HIGH;
int lastButtonStateDown = HIGH;
unsigned long lastDebounceTimeDown = 0;

int buttonStateEnter = HIGH;
int lastButtonStateEnter = HIGH;
unsigned long lastDebounceTimeEnter = 0;

// UI & Navigation State
int currentMenu = 0; // 0 = Home Menu, 1 = Reading Page, 2 = History, 3 = File Options
int cursorIndex = 0; 
bool isReadingActive = true;

// History Data
float maxTemp = -999.0, minTemp = 999.0;
float maxHum = -999.0, minHum = 999.0;
unsigned long timeMaxTemp = 0, timeMinTemp = 0;
unsigned long timeMaxHum = 0, timeMinHum = 0;

// Timing Variables
const unsigned long run = millis();
const unsigned long reading_pause = 3000;
unsigned long last_time = 0;

// Hotspot Credentials
const char* ap_ssid = "The_Great_Kaden_Server";
const char* ap_password = "67676767";

WebServer server(80); // Start HTTP web server on port 80

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0); 
  delay(2000);             

  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Failed to connect with OLED display :(");
    for (;;);
  }

  // LittleFS initialization
  fsOK = LittleFS.begin(true);
  if (!fsOK) {
    Serial.println("LittleFS Mount Failed! UI running without file storage.");
  } else {
    Serial.println("LittleFS Mounted Successfully!");
  }

  Serial.println("\n--- Kaden's Great ESP32-C3 DHT22 Project ---");
  startAnimation("LOADING...", 2500);

  dht.begin();
  
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);

  // --- DIRECT HOTSPOT (ACCESS POINT) MODE ---
  WiFi.persistent(false);              
  WiFi.mode(WIFI_AP);                 
  WiFi.setSleep(false);                
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  
  WiFi.softAP(ap_ssid, ap_password);

  Serial.println("----------------------------------------------");
  Serial.println("ESP32 Hotspot Started Successfully!");
  Serial.println("1. Connect your device to Wi-Fi: The_Great_Kaden_Server");
  Serial.println("2. Password: 67676767");
  Serial.println("3. Open browser and go to: http://192.168.4.1");
  Serial.println("----------------------------------------------");

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/download", handleFileDownload);
  server.begin();
  Serial.println("HTTP Web Server Started!");

  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  button.setPressMs(3000);
  button.attachLongPressStart(startDeepSleep);
  
  delay(500);
  renderUI();
}

void loop()
{
  server.handleClient();
  button.tick();

  // --- DHT22 Reading Routine ---
  unsigned long current_time = millis();
  error = false;
  if (isReadingActive && (current_time - last_time >= reading_pause)) {
    last_time = current_time;
    unsigned int reading_time = current_time / 1000;

    float humidity = dht.readHumidity();
    float temp = dht.readTemperature();

    if (isnan(humidity) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Gen_snpp(0, resultH, sizeof(resultH), humidity);
      Gen_snpp(1, resultT, sizeof(resultT), temp);
      recordData(temp, humidity, reading_time);
    }
    if (currentMenu == 1) renderUI();
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
          currentMenu = 0; 
        } else {
          currentMenu = 1; 
          button_Toggle = 0; 
        }
        cursorIndex = 0;
        renderUI();
      }
    }
  }
  last_Button_State = readingToggle;

  int maxCursor = (currentMenu == 3) ? 3 : 2;

  // --- BUTTON: UP ---
  int readingUp = digitalRead(BUTTON_UP);
  if (readingUp != lastButtonStateUp) lastDebounceTimeUp = millis();
  if ((millis() - lastDebounceTimeUp) > click_Delay) {
    if (readingUp != buttonStateUp) {
      buttonStateUp = readingUp;
      if (buttonStateUp == LOW) {
        cursorIndex--;
        if (cursorIndex < 0) cursorIndex = maxCursor;
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
        if (cursorIndex > maxCursor) cursorIndex = 0;
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
        
        // Home Menu Actions
        if (currentMenu == 0) { 
          if (cursorIndex == 0) {
            isReadingActive = !isReadingActive;
          } else if (cursorIndex == 1) {
            currentMenu = 2; 
          } else if (cursorIndex == 2) {
            currentMenu = 3; 
            cursorIndex = 0; 
          }
        } 
        // History Page Action
        else if (currentMenu == 2) { 
          currentMenu = 0; 
        } 
        // File Menu Actions
        else if (currentMenu == 3) { 
          if (cursorIndex == 0) {
            if (fsOK) {
              LittleFS.remove("/data.csv");
              LittleFS.rename("/temp.csv", "/data.csv");
              Serial.println("Saved into new data sheet!");
            }
            currentMenu = 0;
            cursorIndex = 0;
          } else if (cursorIndex == 1) {
            if (fsOK) {
              File tempFile = LittleFS.open("/temp.csv", "r");
              File mainFile = LittleFS.open("/data.csv", "a");
              if (tempFile && mainFile) {
                if (tempFile.available()) {
                  String firstLine = tempFile.readStringUntil('\n');
                  if (mainFile.size() == 0) {
                    mainFile.println(firstLine);
                  }
                  while (tempFile.available()) {
                    mainFile.write(tempFile.read());
                  }
                }
              }
              if (tempFile) tempFile.close();
              if (mainFile) mainFile.close();
              LittleFS.remove("/temp.csv");
              Serial.println("Appended data to data sheet!");
            }
            currentMenu = 0;
            cursorIndex = 0;
          } else if (cursorIndex == 2) {
            if (fsOK) {
              LittleFS.remove("/temp.csv");
              LittleFS.remove("/data.csv");
              Serial.println("Data sheets cleared!");
            }
            currentMenu = 0;
            cursorIndex = 0;
          } else if (cursorIndex == 3) {
            currentMenu = 0;
            cursorIndex = 0;
          }
        }
        renderUI();
      }
    }
  }
  lastButtonStateEnter = readingEnter;
}



// -------| Function Definitions |-------

void recordData(float temp, float humidity, unsigned long timestamp) {
  if (temp > maxTemp) { maxTemp = temp; timeMaxTemp = timestamp; }
  if (temp < minTemp) { minTemp = temp; timeMinTemp = timestamp; }
  if (humidity > maxHum) { maxHum = humidity; timeMaxHum = timestamp; }
  if (humidity < minHum) { minHum = humidity; timeMinHum = timestamp; }

  if (!fsOK) return; 

  bool exists = LittleFS.exists("/temp.csv");
  File file = LittleFS.open("/temp.csv", "a");
  if (file) {
    if (!exists || file.size() == 0) {
      file.println("Timestamp(s),Temperature(C),Humidity(%)");
    }
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
    if (!isReadingActive) {
      printCentered("Reading stopped", 28, 1);
    } else {
      printCentered(resultH, 22, 1);
      printCentered(resultT, 34, 1);
    }
  } 
  else if (currentMenu == 2) {
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
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("- FILE OPTIONS -");

    display.setCursor(0, 16);
    if (cursorIndex == 0) display.print(">"); else display.print(" ");
    display.println(" Save New Data");
    
    display.setCursor(0, 28);
    if (cursorIndex == 1) display.print(">"); else display.print(" ");
    display.println(" Append Data");
    
    display.setCursor(0, 40);
    if (cursorIndex == 2) display.print(">"); else display.print(" ");
    display.println(" Restart Data");

    display.setCursor(0, 52);
    if (cursorIndex == 3) display.print(">"); else display.print(" ");
    display.println(" Back");
  }
  
  display.display();
}

int Gen_snpp(int type, char* destination, int size, float reading) {
  if (type == 0) return snprintf(destination, size, "Humidity: %.1f%%", reading);
  else if (type == 1) return snprintf(destination, size, "Temperature: %.1fC", reading);
  return -1; 
}

void printCentered(const char* text, int y, unsigned int size) {
  int16_t x1, y1;
  uint16_t width, height;
  display.setTextSize(size);
  display.getTextBounds((char*)text, 0, 0, &x1, &y1, &width, &height);
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

void handleFileDownload(void)
{
  String customFileName = "dht22_data.csv";

  if (server.hasArg("filename") && server.arg("filename").length() > 0) {
    customFileName = server.arg("filename");
    if (!customFileName.endsWith(".csv")) {
      customFileName += ".csv";
    }
  }

  String targetFilePath = "/data.csv";
  if (!LittleFS.exists(targetFilePath)) {
    if (LittleFS.exists("/temp.csv")) {
      targetFilePath = "/temp.csv";
    } else {
      server.send(404, "text/plain", "No CSV data files found!");
      return;
    }
  }

  File downloadFile = LittleFS.open(targetFilePath, "r");
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=\"" + customFileName + "\"");
  server.sendHeader("Connection", "close");
  server.streamFile(downloadFile, "text/csv");
  downloadFile.close();
}

void handleRoot(void)
{
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32-C3 Data Server</title>";
  html += "<style>";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background-color: #f4f6f8; margin: 0; padding: 40px 20px; display: flex; justify-content: center; }";
  html += ".card { background: #ffffff; padding: 30px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.08); max-width: 400px; width: 100%; text-align: center; }";
  html += "h2 { color: #333; margin-top: 0; }";
  html += "label { font-size: 14px; color: #666; display: block; margin-bottom: 8px; text-align: left; }";
  html += "input[type='text'] { width: 100%; padding: 12px; border: 1px solid #ccc; border-radius: 6px; font-size: 16px; box-sizing: border-box; margin-bottom: 20px; }";
  html += "button { background-color: #007aff; color: white; border: none; padding: 12px 20px; font-size: 16px; border-radius: 6px; cursor: pointer; width: 100%; font-weight: 600; }";
  html += "button:hover { background-color: #0056b3; }";
  html += "</style></head><body>";
  html += "<div class='card'>";
  html += "<h2>DHT22 CSV Export</h2>";
  html += "<form action='/download' method='GET'>";
  html += "<label for='filename'>Enter Download File Name:</label>";
  html += "<input type='text' id='filename' name='filename' value='dht22_readings.csv' required>";
  html += "<button type='submit'>Download CSV</button>";
  html += "</form>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
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
    display.getTextBounds((char*)label, 0, 0, &x1, &y1, &w, &h);
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

void shutdownAnimation(void) {
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
