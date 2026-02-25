# smart-home-pixel-display
An ESP8266-based pixel display for the Bosch Smart Home System (BSH). It displays the status of your smart home, logs data and events, displays the weather and can trigger actions of your smart home when certain weather conditions occur. The weather information is retrieved from openweathermap.com. The display can be shown and configured via a website in your browser. You can also use the ESP on its owm (without display and case), if you are just interested in the website functions. The program comes with its own upload tool.

Hardware:

![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/front.jpg)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/weather_and_states.jpg)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/assembly.jpg)

Web interface:

![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/dashboard.png)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/chart3.png)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/config.png)

### Features:

- 4 status indicators which can be used independently for showing user defined states of your Bosch Smart Home System, e.g., for indicating open windows, high humidity or as a reminder for the alarm system
- Shows weather forecast for the next hours with current weather symbol, temperature, wind speed and propability of precipitation indicator. Temperature range -19 to +39 degree celcius
- Optional buzzer which can be activated by user defined states
- Special functions: user defined states of your BSH can set the display off (e.g. at night), increase display brigthness (e.g. at day), can stop/play Sonos speakers at hour home and send commands to your computer (e.g. for shutdown). It also can trigger IFTTT commands.
- Changes of user defined states are logged in an event log file
- Weather data and data from smart home devices, e.g. power consumptions, temperatures, valve postions, can be logged in a data log
- Web interface with dashboard function, direct access to event log, graphical data chart and configuration page
- Powered via USB
- Low power consumption (0.5 W)
- Upload-tool for Windows, so you don't need to install Arduino or anything else to upload the firmware to the ESP

- The display shows the following information:

![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/symbols.png)

### What you need:

If you want to use just the webpage functionality, you need:
- ESP8266 microcontroller (D1 Mini with 16 pins)
- micro USB cable
- A free OpenWeatherMap account
- Firmware and upload-tool from this repository

If you want to build the pixel display physically, you also need:
- 8x8 Matrix with WS2812B LEDs (5V, size: 65 mm x 65 mm )
- optional: a buzzer module
- 3D printed case (.stl files in this repository)
- some wires
- soldering iron

### How to set it up:

If you want to use just the webpage functionality, start with step 3.

1. Print the case. I use standard white PLA material. Both parts don't need supports. The orientation is with the front / back plate facing downwards. 15 % infill.
2. Connect the display (and the buzzer, if used) to the ESP8266. See wiring diagram below. Insert everything into the case.
3. Open a free account at openweathermap and note your API key.
4. Download the latest firmware (*.bin), esptool.exe and SmartHomePixelDisplay_Uploader.exe from this repository
5. Connect the ESP with your computer via USB cable and open SmartHomePixelDisplay_Uploader.exe. It will guide you through the setup process. You need your WIFI SSID, WIFI password, BSH system password and BSH controller IP. You can find the BSH IP in your Bosch Smart Home App. During setup, you need to press the button on your Bosch controller.
6. After sucessfully registration of the ESP at the Bosch controller, remove the device from your PC and use it where you want. WIFI reception is needed.

### Configuration:

Open the webpage http://smartdisplay.local in a browser and open config. Go to "Configuration of user defined states". Select the name of the user defined state, the corresponding indicator number to show the state, flashing and buzzer mode.

You can also add a "special function". Available at the moment:

| Special function                        | If user defined state turns on                                              | If user defined state tuns off                           
| --------------------------------------- | --------------------------------------------------------------------------- | -------------------------------------------
|display_off                              | Turns the display off.                                                      | Turns the display on.
|brightness_high(value)                   | Sets the display brightness to a certain level (0-20)                       | Sets the display brightness back to standard level  
|sonos_stop(IP)                           | Send a stop command to a sonos speaker with IP                              | -
|sonos_play(IP)                           | Send a play command to a sonos speaker with IP                              | -
|PC_execute(Name_of_PC|command|parameters)| Sends a windows command to a pc*                                             | -
|IFTTT(event)                             | Sends an IFTTT webhook for a event                                          | -

*For the pc execute function, you need to run [this program]([https://www.beispiel.de](https://www.softpedia.com/get/Internet/Remote-Utils/UDPRun.shtml)) on your pc. For eample, the following special function will shut down your pc: pc_execute(PC_NAME|c:\windows\system32\shutdown.exe|/s /t 0);

Each special function must be followed by a “;” in the input field.

### Wiring:

![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/smart_pixel_display_wiring.png)

### Flashing pixel areas at start-up

After connecting the display to USB, the 4 pixel areas will indicate the boot procedure according to the table. At the end of start-up, all 4 areas should be on. (Directly at the beginning, all 4 areas flash shortly to show that all pixels work)

| Pixel area       | Meaning                                                                     | Troubleshooting                                                                                |
| ---------------- | --------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
|red flashing      | Trying to connect to WIFI                                                   | If flashing doesn't stop, check your WIFI and WIFI password in program code                    |
|red on            | WIFI connected                                                              |                                                                                                |
|blue on           | Time received and file system started                                       | If blue doesn't switch on: Is your WIFI connected to the internet?                             |
|yellow flashing 4x| Register mode: Press the button on the Bosch controller                     | Flashing doesn't stop, although you pressed the button? Check the BSH IP in the program code   |
|yellow flashing 1x| Trying to connect to Bosch controller                                       | If flashing doesn't stop, check if BSH controller is running                                   |
|yellow constant on| Connected to Bosch controller                                               |                                                                                                | 
|purple on         | Register mode: Successfully registered. Normal start up: Web server started | If purple stays off during register process, check your BSH password (base64!) in program code |

