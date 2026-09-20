# DHT22 Temperature and Humidity Reader by Kaden Chung
## Brief Device Overview / Introduction
This DHT22 Temperature and Humidity Reader takes in humidity and temperature readings and displays them on the OLED screen. Recorded data will also be stored in a cav file, which can be downloaded onto a connected device. 

## Functions / How to Use
Plug the device in, connected to power. A starting animation would appear on the OLED screen. After it ends, the device's Home Menu would be shown. 

Before starting, connect your mobile phone, computer or iPad to the device's hotspot. The default WiFi name is The_Great_Kaden_Server, with password 67676767. After you are connected, open your default browsing application and go to link http://192.168.4.1 . Opening this link allows you to interact with the device and save data.

After connecting to The_Great_Kaden_Server, we'll direct our attention to the device. The device has 5 buttons: Toggle button (Top left), Sleep button (Top right), Up button (Bottom left), Enter button (Bottom middle), and Down button (Bottom right). These buttons are used to interact with the simple UI on the OLED display.

Once you get to the device's Home Menu, there are 3 options below. You can toggle the options by clicking the Up button (Bottom left) or Down button (Bottom right), and press enter to execute the option's function. You can also press the Toggle button (Top left), where the display displays the Humidity and Temperature readings. Pressing the Toggle button agains brings you back to the Home Menu. You can press the Toggle button anytime, in any part of the UI, but keep in mind that you'll always be sent back to the Home Menu after exiting the Reading Page.

About the Sleep button (Top right), you can send the device to sleep anytime during the device's operation. Sleeping the device sends the internal ESP32C3 Super-mini onto an ultra-lower-power operating mode, dropping power draw and shutdowns components (Except for the wake-up circuit). To activate Sleep, click and hold the Sleep button for at least 3 seconds until the shutdown animation appeared. Pressing the Sleep button again once when the device is in sleep wakes up the device. Be reminded that after waking up device, everything resets and starts over. So ** IMPORTANT REMINDER: Download all data needed before sending device to sleep! **.

Now on the device's Home Menu, the first option is to Start / Stop Reading. After the starting animation, the DHT22 will automatically start to take in readings, storing the data locally. You can use this option to manually toggle if you'd like the DHT22 to take in reading or not.

The second option is History. Pressing on this option (Pressing the Enter button (Bottom middle) when this option is selected) displays the History Page. The History page displays: Highest Temperature recorded (MaxT), Lowest Temperature recorded (MinT), Maximum Humidity recorded (MaxH), and Minimum Humidity recorded (MinH). The time (in seconds where 

## Building process
## Self-reflection
## Specifiations
