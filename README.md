# smart-home-pixel-display
An ESP8266-based pixel display for the Bosch Smart Home System (BSH) showing user defined states and the current weather. The weather feature uses the openweathermap API. All state changes are recorded in a log file, which can be retrieved via a URL on your web browser.

![smart display](https://github.com/tobo-123/smart-home-pixel-display/blob/main/pictures/front.jpg)


Features:

- 4 pixel areas which can be used independently in the BSH for indicating user defined states, e.g., for indicating open windows, high humidity or as a reminder for the alarm system
- weather station with weather symbol, temperature and propability of precipitation indicator. Temperature range -19 to +39 degree celcius
- optional: BSH states can activate a buzzer
- log file which can be retrieved via a simple URL in your local network: http://smartdisplay.local
- power via USB
- overall brightness can be adjusted in the program
- low power consumption (0.5 W)

What you need:

- ESP8266 microcontroller (Mini NodeMcu with 16 pins)
- 8x8 Matrix with WS2812B LEDs and 5V
- optional: a buzzer module
- 3D printed case (.stl files in this repository)
- some wires
- micro USB cable
- soldering iron
- Arduino IDE on your pc
- OpenSLL on your pc
- A free OpenWeatherMap account
- Arduino program code (in this repository)

How to set it up:

1. Print the case. I use standard white PLA material. Both parts don't need supports. The orientation is with the front / back plate facing downwards. 15 % infill.
2. Connect the display (and the buzzer, if used) to the ESP8266. See wiring diagram below. Insert everything into the case.
3. Open a free account at openweathermap and note your API key.
4. Create a self-signed certificate and key with OpenSSL by using the command: openssl req -x509 -nodes -days 9999 -newkey rsa:2048 -keyout client-key.pem -out client-cert.pem
5. Open the key and certificate file with notepad and copy the text in the corresponding sections of the program code. Also put in your WIFI name, WIFI password, BSH system password and BSH controller IP and the information on your OpenWeatherMap API key, city and country code. The BSH system password needs to be coded in Base64, there are many encoders online to do this for you.
6. Edit the parameter array in the program code: Put in the names of the user defined states of your BSH app, the corresponding pixel area to indicate the state, flashing and buzzer mode. See comments in the program.
7. Connect the ESP8266 to your PC with the micro USB cable. Press the button on your BSH controller (the middle and right LED of your controller should flash now) and upload and run the program code via the Arduino IDE. You should see each LED of your display flash once. If this does not happen, check whether you have connected the LED correctly and used the correct GPIO pins of the ESP.
8. Check in BSH app if "OSS Pixel Display" occurs at mobil devices -> If yes, the display is registered now. If not, check your BSH system password.
9. Now, set the variable register_client = false in the program code and upload the code again. The display is ready now! Try to change a user defined state and the corresponding pixel area should indicate the state.
10. Remove the display from your PC and use it where you want. WIFI reception is needed ;)

Wiring:
