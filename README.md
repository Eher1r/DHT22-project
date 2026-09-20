# DHT22 Temperature and Humidity Reader by Kaden Chung
## Brief Device Overview / Introduction
This DHT22 Temperature and Humidity Reader takes in humidity and temperature readings and displays them on the OLED screen. Recorded data will also be stored in a cav file, which can be downloaded onto a connected device. 

## Functions / How to Use
Plug the device in, connected to power. A starting animation would appear on the OLED screen. After it ends, the device's Home Menu would be shown. 

Before starting, connect your mobile phone, computer or iPad to the device's hotspot. The default WiFi name is The_Great_Kaden_Server, with password 67676767. After you are connected, open your default browsing application and go to link  http://192.168.4.1 . After opening this link, you can click on the Download CSV button, where the readings recorded from the start till that moment would be in that spreadsheet. The spreadsheet automatically updates itself. Above the Download button is a file name input box, where you can edit the name of the CSV file downloaded.

After connecting to The_Great_Kaden_Server, we'll direct our attention to the device. The device has 5 buttons: Toggle button (Top left), Sleep button (Top right), Up button (Bottom left), Enter button (Bottom middle), and Down button (Bottom right). These buttons are used to interact with the simple UI on the OLED display.

Once you get to the device's Home Menu, there are 3 options below. You can toggle the options by clicking the Up button (Bottom left) or Down button (Bottom right), and press enter to execute the option's function. You can also press the Toggle button (Top left), where the display displays the Humidity and Temperature readings. Pressing the Toggle button agains brings you back to the Home Menu. You can press the Toggle button anytime, in any part of the UI, but keep in mind that you'll always be sent back to the Home Menu after exiting the Reading Page.

About the Sleep button (Top right), you can send the device to sleep anytime during the device's operation. Sleeping the device sends the internal ESP32C3 Super-mini onto an ultra-lower-power operating mode, dropping power draw and shutdowns components (Except for the wake-up circuit). To activate Sleep, click and hold the Sleep button for at least 3 seconds until the shutdown animation appeared. Pressing the Sleep button again once when the device is in sleep wakes up the device. Be reminded that after waking up device, everything resets and starts over. So **IMPORTANT REMINDER: Download all data needed before sending device to sleep!**.

Now on the device's Home Menu, the first option is to Start / Stop Reading. After the starting animation, the DHT22 will automatically start to take in readings, storing the data locally. You can use this option to manually toggle if you'd like the DHT22 to take in reading or not.

The second option is History. Pressing on this option (Pressing the Enter button (Bottom middle) when this option is selected) displays the History Page. The History page displays: Highest Temperature recorded (MaxT), Lowest Temperature recorded (MinT), Maximum Humidity recorded (MaxH), and Minimum Humidity recorded (MinH). The time (in seconds from the device is turned on) is also provided along with the reading.

The third option is File Option. Selecting this will direct you to the File Page, where there would be 2 more options: Restart Data and Back. Restart Data will delete all the previous data, and generate a brand new spreadsheet, and a blank CSV file, where the new data would be added. The timestamp would also start from 0. The Back option brings the display back to the Home Menu.

## Building process
### Phase 1: Setting up the Environment (2 days)
As this is my first hardware project, I had to set up the coding environment and gather the materials needed to start, while also started the repository used.

For the components, the ESP32C3 Super-Mini was used as the microcontroller, due to its cost and availability to me. Then to record Humidity and Temperature Readings, a DHT22 was used. After soldering the pins of the ESP32C3 Super-mini, I placed both components onto a breadboard and connected them with jumper wires. 

Other components were also gathered for future use. This includes a SSD1306 0.96" OLED Display for displaying the readings and creating the UI later, 5 TPS645-5NC60 Tactile switches to interact with the UI, a 100uf electrolytic capacitor to smooth out electrical noise, and a whole bunch of jumper wires to wire all the components together.

For the coding environment, I'd set up Arduino IDE on my computer, which lets me compile my code and upload it to my ESP32C3 Super-Mini. I also created a new repo (This) to store the code. After Jude's incredible learning video, YouTube tutorials and Gemini's help, I set up my GitHub repository and learned the basic commands for git (add to stage, commit, pull, push, create branch, toggle between branches, merge branch, delete branch). 

### Phase 2: Coding the Basic Functions (3 days)
I first started with sending readings from the DHT22 to the Serial Monitor on Arduino IDE. This first code taught me on the basics of using a Serial Monitor with functions like Serial.begin(115200);, Serial.print("Hello"); and Serial.println("--- New line :) ---");. I also got to experience DHT-native functions, like .readHumidity and .readTemperature. This first step also presented problems outside of the code, like enabling USD CDC On Boot and matching the baud rate to the number in Serial.begin();

After reading the readings on my computer, I added the OLED display to my circuit. Adding this feature is a big step for me, as much more code is needed to be added. This includes more #define, more objects created, more setup code (Setting up text size, text colour, cursor starting point ...).

One major problem that occurred when I had to center the text. To make this possible I was introduced to new functions like snprintf(). I also had to create my own functions.

### Phase 3: Refining the Code and Add Advanced Functions (3 days)
Next I added 5 buttons to the project. These buttons would be used to toggle the OLED display. The display also became more advanced, with a Home Menu. A hotspot was also created, where I can connect my device to receive data as a CSV file.

I coded the initial button logic, which checks what is considered as a click and what doesn't. This required lots of variables and lots of if-statements. It really was a head-scratcher to code it. For the other functions, like OLED display and hotspot, their code were coded with big support from Gemini.

### Phase 4: Creating a product (2 days)
Next a PCB had to be designed, along with a well-drawn CAD model to enclose the device.

## Self-reflection
This project is my first hardware project. Even though it might sound easy to create such a device, inside there are layers of knowledge that need to be uncovered. This project was made possible mainly because of Gemini, who guided me through mistakes and provided solutions. At the start I learned and understood all its code and commands before executing, but at the end due to time constraints, I had no choice but to copy and paste. 

Talking about what I learned, there's really a full list. Let me display it as a list:
* Arduino IDE Setup and How to Run Code
* Different pins on the ESP32C3 and their usage
* Breadboard usage
* C/C++ Code for components like OLED display, buttons, DHT22 and Serial Monitor
