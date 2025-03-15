# smart-home-pixel-display
An ESP8266-based pixel display for the Bosch Smart Home System (BSH) showing user defined states and the current weather. The weather feature uses the openweathermap API. All state changes are recorded in a log file, which can be retrieved via a web page on your browser.

![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/front.jpg)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/weather.jpg)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/weather_and_states.jpg)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/assembly.jpg)
![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/menu.jpg)

### Features:

- 4 pixel areas which can be used independently in the BSH for indicating user defined states, e.g., for indicating open windows, high humidity or as a reminder for the alarm system
- Shows weather forecast for the next hours with current weather symbol, temperature and propability of precipitation indicator. Temperature range -19 to +39 degree celcius
- Optional buzzer which can be activated by user defined states
- Log file and configuration via web page: http://smartdisplay.local
- Special functions: states of your BSH can set the display off (e.g. at night), increase display brigthness (e.g. at day), can stop/play Sonos speakers at hour home and send commands to your computer (e.g. for shutdown). You need to run the software UDPrun at your PC for this.
- Power via USB
- Low power consumption (0.5 W)

### What you need:

- ESP8266 microcontroller (D1 Mini with 16 pins)
- 8x8 Matrix with WS2812B LEDs (5V, size: 65 mm x 65 mm )
- optional: a buzzer module
- 3D printed case (.stl files in this repository)
- some wires
- micro USB cable
- soldering iron
- Arduino IDE on your pc
- OpenSLL on your pc
- A free OpenWeatherMap account
- Arduino program code (in this repository)

### How to set it up:

1. Print the case. I use standard white PLA material. Both parts don't need supports. The orientation is with the front / back plate facing downwards. 15 % infill.
2. Connect the display (and the buzzer, if used) to the ESP8266. See wiring diagram below. Insert everything into the case.
3. Open a free account at openweathermap and note your API key.
4. Create a self-signed certificate and key with OpenSSL by using the command: openssl req -x509 -nodes -days 9999 -newkey rsa:2048 -keyout client-key.pem -out client-cert.pem
5. Open the progam code with Arduino IDE. Open the client-key.pem and client-cert.pem files with notepad and copy the text in the corresponding sections of the program code. Also put in your WIFI name, WIFI password, BSH system password and BSH controller IP as well as the your OpenWeatherMap API key, city and country code. The BSH system password needs to be coded in Base64, there are many encoders online to do this for you (e.g., https://www.base64encode.org)
6. Check if all necessary libraries are installed in Arduino IDE. Connect the ESP with your computer via USB cable and upload and run the program code.
7. During first start-up, the display starts in register mode: Just press the button on your Bosch controller, when you see the yellow pixel area of the display flashing 4 times. Check in BSH app if "OSS Pixel Display" occurs at system -> mobil devices. If yes, the display is registered now. If not, check your Bosch system password and Bosch controller IP.
8. Wait until the ESP reboots automatically. If everything works, it shows the current weather. Open the webpage http://smartdisplay.local in a browser and click on config. Put in the names of the user defined states of your BSH app, the corresponding pixel area number to indicate the state, flashing and buzzer mode. You can also add a "special function". Available at the moment: display_off, brightness_high(value), sonos_off(IP), sonos_play(IP) and PC_execute(Name_of_PC|command|parameters)
9. Remove the display from your PC and use it where you want. WIFI reception is needed ;)

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

