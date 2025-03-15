/*##############################################################################
#   Smart Home Pixel Display                                                   #
#   Last change:   12.03.2025                                                  #
#   Version:       1.0                                                         #
#   Author:        tobo-123                                                    #
#   Github:        tobo-123                                                    # 
################################################################################
  
Copyright (c) 2025 tobo-123

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

#################################################################################

Change log:
V0.1
- First version
V0.2
- With probability of precipitation (pop) scale
- Special functions: buzzer, sonosstop
V0.3
- Display off function
V0.4
- Receive user definded states at start-up
V0.5
- Logging of user state definded changes in files
- Two files log with size limitation
- Web server with html log page
- Code optimization, buzzer integrated in param array
V0.6
- Adjustable number of log files
- Log entry colors
- Memory optimization
V0.7
- Config page
- Safing parameter array in file
- High brightness function
V0.8
- Automatic client registration
- Fixed issue during client registration
- Register can be triggered via HTTP: smarthome.local/register
V0.8b
- Fixed issue with number_entries > 9
- PC command function
V0.9
- Special function parameters can be configurated via smarthome.local/config
- Brightess can be configurated via special function
- Sonos play function
- Local time zone and automatic daylight saving time
- Code optimization
V0.9b
- Restart can be triggered via HTTP: smarthome.local/restart
- Log files can be deleted via HTTP: smarthome.local/deletelog
V1.0
- With start page
- Memory optimization

##################################################################################*/

#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "symbols.h"
#include "html.h"

/*##################################################################################
#####   !!!!!        Adjust the following entries        !!!!!    ##################
##################################################################################*/

const char* WIFI_SSID = "xxxxxxxx";                                        //Your Wifi name
const char* WIFI_PASSWORD = "xxxxxxxxxxxxx";                               //Your Wifi password
const String BSH_IP = "192.168.0.xx";                                      //IP adress of your BSH controller
const String BSH_PASSWORD = "xxxxxxxxxxxxxxxx";                            //Your BSH controller password in base64 coding

const String OWM_KEY = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";                 //Your openweathermap API key
const String OWM_CITY = "xxxxxxx";                                         //Your city name
const String OWM_COUNTRYCODE = "xx";                                       //Country code of your city

const int BUZZER_PIN = 5;                                                  // GPIO pin for buzzer, please check or change
const int LED_MATRIX_PIN = 4;                                              // GPIO pin for display, please check or change

const int INITIAL_BRIGHTNESS = 5;                                          // Brightness adjustment. Be aware that higher values increase power consumption and may damage your USB port.

//Enter your client_cert.pem below

const char CLIENT_CERT[] PROGMEM = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIDcTCCAlmgAwIBAgIUH0Pbupca1qIOwHtCydADDGXs3YEwDQYJKoZIhvcNAQEL
BQAwRzELMAkGA1UEBhMCREUxCzAJBgNVBAgMAkJFMQ8wDQYDVQQHDAZCZXJsaW4x
some lines deleted
vcFfUC1/fuRDlPbpxDfTdnTEepOGrCmWF/dUJoFFtMkB/HvBsKwE+OlcBx5AdeJv
VfDKD7Y1WIuARPFjvp/4QTwQvNoW
-----END CERTIFICATE-----
)string_literal";

//Enter your client_key.pem below

const char CLIENT_KEY[] PROGMEM = R"string_literal(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDtA8MhKS5Q5JED
MNHFjHzANUL7qdnVlM7BFPLk18R4GHXONNAqXjrmGB7DP50AIoLmYo+UC0nALv8X
some lines deleted
9XWH57UgOCpjFmPhg0MN48qCZEMPHKprxWkTtyDLNMpL7JYI1ILbYKudbpZIOAme
EGCBICaGpxbFwcb++//rUbIu
-----END PRIVATE KEY-----
)string_literal";


/*##################################################################################
#####   !!!!!  Nothings needs to be changed after here   !!!!!    ##################
##################################################################################*/


struct param {String name; int square; bool blinking; bool buzzer; String special_function; };
struct square {int pos_x; int pos_y; int r; int g; int b; };

const char BSH_ROOT_CA[] PROGMEM = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIFETCCAvmgAwIBAgIUR8y7kFBqVMZCYZdSQWVuVJgSAqYwDQYJKoZIhvcNAQEL
BQAwYzELMAkGA1UEBhMCREUxITAfBgNVBAoMGEJvc2NoIFRoZXJtb3RlY2huaWsg
R21iSDExMC8GA1UEAwwoU21hcnQgSG9tZSBDb250cm9sbGVyIFByb2R1Y3RpdmUg
Um9vdCBDQTAeFw0xNTA4MTgwNzI0MjFaFw0yNTA4MTYwNzI0MjFaMFsxCzAJBgNV
BAYTAkRFMSEwHwYDVQQKDBhCb3NjaCBUaGVybW90ZWNobmlrIEdtYkgxKTAnBgNV
BAMMIFNtYXJ0IEhvbWUgQ29udHJvbGxlciBJc3N1aW5nIENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsBNK3PPd/E9jbf3YkZIDtfIl2Vo0Nx7oeOsh
F0L9tZwqC3+85ymB5LgFBOoHpr7tTFRb4elyPsfyv/GfXuJmDIxVAWBn/pxFzODa
J3DGJ2kvwipvMNp7IxXHhK10YsG8AaT0QaeaYGq1GRp5uNZafwAOOkrrQfwtG+za
Qn9qUxLYBrB++RN/5mk4Z7gyrq7fi84T23yMOtVkdb+mlb9qStQ3mllglqrRlJQo
MKdQxe24Farg6N3y7h5bxLJEEXGqGExDNwR46ep+4Ys7W2QeD/2LXwYvKQ+wO70+
BNxnikkq8kPcq8694HMsfzUTBrxuHQGi6td9o+3CW01AOEvV0wIDAQABo4HEMIHB
MBIGA1UdEwEB/wQIMAYBAf8CAQEwHQYDVR0OBBYEFHy1ci5zZEQaHLDAaYFYez8R
FHsXMB8GA1UdIwQYMBaAFOFQaxE4w2eoyE+f6oXGTxH1V1Y+MA4GA1UdDwEB/wQE
AwIBBjBbBgNVHR8EVDBSMFCgTqBMhkpodHRwczovLzI5Lm1jZy5lc2NyeXB0LmNv
bS9jcmw/aWQ9ZTE1MDZiMTEzOGMzNjdhOGM4NGY5ZmVhODVjNjRmMTFmNTU3NTYz
ZTANBgkqhkiG9w0BAQsFAAOCAgEAZpp9kE7Qy6tcQrfW4DJAqEcUhzg4zncJYxpb
dTn/o5TvH/uPVOfoxJgtsTFtsY/ytcPJReLcgmqrRN1gTNefdXylJr688hFyhf1Z
xGDoZG8MuzM9QXaHC6UNFzaeZj46ZYfdJiUtDXsYN82opGE6GhBju5JOLoFd2vYK
qUnVKWqdrN0KkihClry6NcfiLEA70m00pNtsVZyVGyk7DP4ErVF5K3j40T5v4ZJl
Q9ri/V97zyqXeIti8kZdla7kzJBFbGEumlUyVPRpoxdpnvWM7AgTOXXsh2sCFAA1
0hUHVOwBZCylaNUXjKMtnA938ykhNCx+OCd2NpZBf3qB6+w2MS7dQuRvMsDJcnLq
X80QHJzXpmDsXEiwKyvmZnZbiAgoOiUSe2O6OaGsDRW8UBzi+wm42pxgbDnAcGUu
r9Cf5y0+SFS0aQkqcWbJYwPy+LQi2MJGkv34FxTOCqygluzZt+w5xZyq5PcpPNm5
1s4Ps2+psvNhcAG3EHRF9vBnlr1MCVU04XYig54HeNGFIQQAFWFFR/9DgnH/cFLf
gPoJEZV/VZtsOjy/EsqYZMFJBzJEtKOiTCKDe+pVirDB9zrcVsJG8LGiLd7266e9
1Eg5GjNiavG7ninMOWSJLfW4xPD6S3zxDAYjsPDJbMFqEFIF2ZvyYC1mVeflB/WM
xnZ+67w=
-----END CERTIFICATE-----
)string_literal";

// Constants

const char* TIMEZONE = "CET-1CEST,M3.5.0/02,M10.5.0/03";                   //Timezone of your destination including daylight saving time

const square SQUARE_ARRAY[] = {{0,6,255,0,0},                              // This array defines the 2x2 square pixel areas to show the userdefinedstates. Scheme: x-pos, y-pos, red, blue, green color value (0-255)
                               {2,6,0,0,255},
                               {4,6,255,100,10},
                               {6,6,200,0,200}};

const char* DNS_NAME = "smartdisplay";
const int NUMBER_OF_FILES = 10;                                            //Number of log files. Many files will slow down first start-up
const int MAX_FILE_SIZE = 2000;                                            //Size of each log file. Increasing this value may bring up memory problems 
const int NUMBER_OF_ENTRIES = 15;                                          //Number of entries in the parameter table. Please change if you want to have more/less entries. Up to 99 entries are allowed 
const int MAX_BRIGHTNESS = 20;                                             //Maxmimum allowed brightness of display. Do not exceed 25!

const String STARTUP_COLOR = "#008CBA"; 
const String ALARM_COLOR = "red";
const String WARNING_COLOR = "yellow";
 
//Variables and objects

String json = "";
String polling_id;
String message = "";
char c;

bool subscribe_poll = true;
bool poll = true;
bool get_weather = true;
bool display_on = true;
bool read = false;

param parameter[NUMBER_OF_ENTRIES] = {};
bool state_array[NUMBER_OF_ENTRIES] = {false};

int lastfile = 0;
int open_brackets = 0;

unsigned long current_time = 0;
unsigned long prev_time = 0;
unsigned long current_time2 = 0;
unsigned long prev_time2 = 0;

struct tm timeinfo;

IPAddress IP;
Adafruit_NeoPixel pixels(64, LED_MATRIX_PIN, NEO_GRB + NEO_KHZ800);
WiFiClientSecure client;
ESP8266WebServer server(80);


// -----------------------------------------------------------------------------------------------------
// Setup -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

void setup() {

  // Setup serial connection 

  Serial.begin(115200);
  while(!Serial) {
     delay(500);
  };
  delay(2000);
  Serial.println();
  Serial.println(F("Smart home pixel display is starting..."));

  // Setup matrix led display 

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(INITIAL_BRIGHTNESS);
 
  // Pin assignment  

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  // Check all squares at start-up

  showSquare(1, true);
  showSquare(2, true);
  showSquare(3, true);
  showSquare(4, true);
  delay(1000);
  pixels.clear();
 
  // Setup wifi connection 

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Connecting to WiFi.."));

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    showSquare(1, true);
    delay(500);
    showSquare(1, false);
    delay(500);
  }

  Serial.println(F(" WiFi connected!"));
  showSquare(1, true);
  IP.fromString(BSH_IP);

  // Start DNS

  if (MDNS.begin(DNS_NAME)) {
    Serial.println(F("DNS started"));
  }

  // Receive time 

  configTime(TIMEZONE, "pool.ntp.org", "time.nist.gov");
  Serial.print(F("Waiting for NTP time sync.."));
  while (!getLocalTime(&timeinfo)){
      Serial.println(".");
  }
  Serial.println(F(" time received!"));
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
  
  // Start file system and check if all files exist. If not, they will be created

  LittleFS.begin();
  checkFiles();
  showSquare(2, true); 
  
  // Setup keys and certificates

  X509List servercert(BSH_ROOT_CA);
  X509List clientcert(CLIENT_CERT);
  PrivateKey clientkey(CLIENT_KEY);
  client.setTrustAnchors(&servercert);
  client.setClientRSACert(&clientcert, &clientkey);
  
  // Register client

  if (readFile("/client_registered") == "false") {
    registerClient();
  }

  // Connect to server at port 8444. This only works after successful client registration
  
  Serial.print(F("Connecting to server at ") + BSH_IP + " ..");

  while (!client.connect(IP, 8444)) {
    Serial.print('.');
    showSquare(3, true);
    delay(500);
    showSquare(3, false);
    delay(500);
  }

  Serial.println(F(" Connected to server!"));
  showSquare(3, true);
  
  // Start web server

  server.onNotFound(handleNotFound); 
  server.on("/", HTTP_GET, handleIndex);
  server.on("/log", HTTP_GET, handleLog);
  server.on("/config", HTTP_GET, handleConfig);
  server.on("/config", HTTP_POST, handleReceiveConfig);
  server.on("/register", HTTP_GET, handleRegister);
  server.on("/restart", HTTP_GET, handleRestart);
  server.on("/deletelog", HTTP_GET, handleDeleteLog);
  server.on("/message", HTTP_GET, handleMessage);  
  server.begin();

  Serial.println(F("Web server started."));
  showSquare(4, true);
   
  // Get the status of all user defined states after start-up and log them

  log(STARTUP_COLOR, F("Smart home pixel display started. Current user defined states:")); 
  getUserDefinedStates();
  log(STARTUP_COLOR, F("Start-up completed"));
  Serial.println(F("Start-up completed.\n"));

  prev_time = millis();
  prev_time2 = millis();
}


// -----------------------------------------------------------------------------------------------------
// Main loop -------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

void loop() {

  // React to client requests and update DNS

  server.handleClient();  
  MDNS.update();

  // Register long-polling for user defined states and start poll when needed

  if (subscribe_poll) {
    polling_id = subscribePolling();
    subscribe_poll = false;
  }

  if (poll) {
    if (polling_id != "no result") {
      startPoll(polling_id);
      poll = false;
    }
  }
  
  // Let square flash, where flashing is enabled and state is on

  current_time = millis();

  if (((current_time - prev_time) > 500) && ((current_time - prev_time) < 699)) {
    for (int i = 0; i < NUMBER_OF_ENTRIES; i++) {
      if (state_array[i]) {
        showSquare(parameter[i].square, true);
        if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }

  if ((current_time - prev_time) > 1000) {
    for (int i = 0; i < NUMBER_OF_ENTRIES; i++) {
      if ((parameter[i].blinking) && (state_array[i])) {
        showSquare(parameter[i].square, false);
        if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, HIGH);
      }
    }
    prev_time = current_time;
  }

  // Reading available data non-blocking

  while (client.available()) {
    c = client.read();
    if (c == '[') { read = true; open_brackets++; }  // "[" marks the beginning of the body message
    if (c == ']') { open_brackets--; }
    if (read) {
      json += c;
      if (open_brackets == 0) break;                 // no open brackets means that we reached the end of body message
    }    
  }
  
  if ((read) && (open_brackets == 0)) {
    
    // Deserialize the json

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      log(WARNING_COLOR, F("Deserialize controller response failed"));
    }

    JsonArray array = doc[0]["result"].as<JsonArray>();
    long error_code = doc[0]["error"]["code"];                  // error code
    const char* error_message = doc[0]["error"]["message"];     // error message
    Serial.print(F("Number of messages received: "));
    Serial.println(String(array.size()));

    for (int i = 0; i < array.size(); i++) {        

      const char* name = doc[0]["result"][i]["name"];          // name of the userdefinedstate
      bool state = doc[0]["result"][i]["state"];               // state of the userdefinedstate: false or true

      // If "name" is not empty, whe have received a message with a new status

      if (String(name) != "") {
          
        // Check, which square needs to bet set and start special functions

        for (int i = 0; i < NUMBER_OF_ENTRIES; i++ ) {
          if (parameter[i].name == String(name)) {

            Serial.println(String(name) + " = " + String(state));
            log("white", String(name) + " = " + String(state));

            if (state) {
              showSquare(parameter[i].square, true);
              executeSpecialFunctions(parameter[i].special_function, true);
              if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, LOW);
              state_array[i] = true;
            }
            else {
              showSquare(parameter[i].square, false);
              executeSpecialFunctions(parameter[i].special_function, false);
              if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, HIGH);
              state_array[i] = false;
            }
          }
        }
      }
    }
  
    // If we receive an error, we re-subscribe the polling

    if (error_code < 0) {
      subscribe_poll = true;
      Serial.println(F("Error code received: ") + String(error_code) +  ", error message : " + String(error_message));
      log(WARNING_COLOR, F("Error code received: ") + String(error_code) +  ", error message : " + String(error_message));
    }

    poll = true;        //since we received a message, we need to send the next poll
    read = false;
    json = "";
  }

  // Receive weather data from OpenWeatherMap every 10 minutes

  current_time2 = millis();
 
  if (((current_time2 - prev_time2) > 600000) || get_weather) {
    getWeather();
    prev_time2 = millis();
    get_weather = false;
  }

  //check, if WIFI and client is still available. If we lost connection, log it and restart the ESP

  if (!client) {                           
    Serial.println(F("Client lost... restart ESP"));
    log(ALARM_COLOR, F("Client lost... restart ESP"));
    ESP.restart();
  }

  if (WiFi.status()!=WL_CONNECTED) {                           
    Serial.println(F("Wifi connection lost... restart ESP"));
    log(ALARM_COLOR, F("Wifi connection lost... restart ESP"));
    ESP.restart();
  }

  delay (100);
}


// -----------------------------------------------------------------------------------------------------
// BSH functions ---------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

//Subscribe polling and return polling_id

String subscribePolling (void) {

  const String output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/subscribe\",\"params\": [\"com/bosch/sh/remote/userdefinedstates\", null]}]";
  String line;
  bool read = false;
  String json = "";

  client.print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_IP + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");


  while (client.connected()) {
    line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  while (client.available()) {
    c = client.read();
    if (c == '[') read = true;
    if (read) {
      json += c;
      if (c == ']') break; 
    }
  }

  // Deserialize the JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  json = "";

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "no result";
  }

  // If result is empty, we didn't got our polling_id

  if (String(doc[0]["result"]) == "") {
    Serial.println(F("Polling couldn't subscribed!"));
    return "no result";
  }

  // Returns the polling_id, it's needed for startPoll

  Serial.print(F("Polling subscribed. Polling_Id: "));
  Serial.println(String(doc[0]["result"]));

  return String(doc[0]["result"]);
}


//Start poll for are already registered long_polling

void startPoll (const String &_polling_id) {

  const String output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/longPoll\",\"params\": [\"" + _polling_id + "\",8000]}]";   // Poll is valid for 8000s, will be renewed automatically

  client.print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_IP + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");
  Serial.println(F("Poll sent... wait for answer"));
}


//Get all user defined states at start-up

void getUserDefinedStates() {

  bool read = false;
  String line;
  String json = "";

  client.print(String("GET ") + "/smarthome/userdefinedstates" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_IP + ":8444\r\n" +
                 "Connection: keep-alive\r\n" +
                 "\r\n");
                 
  Serial.println(F("Get user defined states after start up:"));

  delay(2000);
  pixels.clear();

  // Reads in the data between [ and ] of the controllers response

  while (client.available()) {
    c = client.read();
    if (c == '[') read = true;
    if (read) {
      json += c;
      if (c == ']') break; 
    }
  }

  // Filter rule for json: Since the controllers respronse can be quite big, we just deserialize the data we need

  JsonDocument filter;
  filter[0]["name"] = true;
  filter[0]["state"] = true;

  // Deserialize the json
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));
  json = "";

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  for (JsonObject item : doc.as<JsonArray>()) {

    const char* name = item["name"];
    bool state = item["state"];
    
    if (String(name) != "") {
      
      // Check, which square needs to bet set and start special functions

      for (int i = 0; i < NUMBER_OF_ENTRIES; i++ ) {
        if (parameter[i].name == String(name)) {

          Serial.println(String(name) + " = " + String(state));
          log(STARTUP_COLOR, String(name) + " = " + String(state));

          if (state) {
            showSquare(parameter[i].square, true);
            executeSpecialFunctions(parameter[i].special_function, true);
            state_array[i] = true;
            if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, LOW);
          }
          else {
            showSquare(parameter[i].square, false);
            executeSpecialFunctions(parameter[i].special_function, false);
            state_array[i] = false;
            if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, HIGH);
          }
        }
      }
    }
  }  
}


//Register pixel display at Bosch smart home controller

void registerClient () {

  String cert;
  String line;
  String json;

  Serial.println(F("Connecting to server at port 8443..."));
  Serial.println(F("Press the button on the Bosch smart home controller now!"));

  while (!client.connect(IP, 8443)) { 
    showSquare(3, true);
    delay(250);
    showSquare(3, false);
    delay(250);
    showSquare(3, true);
    delay(250);
    showSquare(3, false);
    delay(250);
    showSquare(3, true);
    delay(250);
    showSquare(3, false);
    delay(250);
    showSquare(3, true);
    delay(250);
    showSquare(3, false);
  }

  Serial.println(F("Connected to server at port 8443!"));
  showSquare(3, true);
  delay(500);

  //formate the client_cert for being sent as JSON to the BSH controller

  cert = String(CLIENT_CERT);
  cert.replace("\n","");
  cert.replace("-----BEGIN CERTIFICATE-----","-----BEGIN CERTIFICATE-----\\r");
  cert.replace("-----END CERTIFICATE-----","\\r-----END CERTIFICATE-----");

  json = "{\"@type\":\"client\",\"id\":\"oss_pixel_display\",\"name\":\"OSS Pixel Display\",\"primaryRole\":\"ROLE_RESTRICTED_CLIENT\",\"certificate\":\"";
  json += cert;
  json += "\"}";
  
  //send header and JSON

  client.print(String("POST ") + "/smarthome/clients" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_IP + ":8443\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Systempassword: " + BSH_PASSWORD + "\r\n" + 
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + json.length() + "\r\n" +
                 "\r\n" +
                 json + "\n");
                 
  Serial.println(F("Client registration posted... wait for answer"));

  while (client.connected()) {
    line = client.readStringUntil('\n');
    
    if(line.indexOf("201") > 0) {                             //code 201 means client registration was successful
         Serial.println(F("Successfully created device. OSS Pixel Display should occur now at Bosch APP at system > mobile devices. ESP restarts in 5 s"));
         showSquare(4, true);
         writeFile("/client_registered", "true");
         log(WARNING_COLOR, F("Smart pixel display registered at Bosch smart home controller"));
         delay(5000);
         ESP.restart();
    }
    if (line == "\r") {
      break;
    }
  }

  Serial.println(F("This didn't work :( Did you use the correct password in base64 coding? ESP restarts in 5 s"));
  writeFile("/client_registered", "false");
  delay(5000);
  ESP.restart();
}


// -----------------------------------------------------------------------------------------------------
// Display functions -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

//Show symbol of 4x4 pixel size

void showSymbol (symbol sym, int pos_x, int pos_y) {

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if ((y+pos_y < 8) && (x+pos_x < 8)) {
         pixels.setPixelColor((y+pos_y)*8+x+pos_x, pixels.Color(sym.color[sym.pixel[y][x]][0], sym.color[sym.pixel[y][x]][1], sym.color[sym.pixel[y][x]][2]));
         pixels.show();
      }
    }
  }
}


//Show temperature (digit + points below) for a given temperature and a given position

void showTemperature (int temp, int pos_x, int pos_y) {

  //overwritting tens digit and minus

  pixels.setPixelColor((4+pos_y)*8+pos_x+1, pixels.Color(0, 0, 0));
  pixels.setPixelColor((4+pos_y)*8+pos_x+2, pixels.Color(0, 0, 0));
  pixels.setPixelColor((4+pos_y)*8+pos_x+3, pixels.Color(0, 0, 0));

  //show points for tens digit and minus for temperatures < 0
  
  if (temp < 0) { pixels.setPixelColor((4+pos_y)*8+pos_x+1, pixels.Color(0, 80, 80)); pixels.setPixelColor((4+pos_y)*8+pos_x+2, pixels.Color(0, 80, 80)); }
  if (temp < -9)  pixels.setPixelColor((4+pos_y)*8+pos_x+3, pixels.Color(0, 0, 150));
  if (temp > 9)   pixels.setPixelColor((4+pos_y)*8+pos_x+1, pixels.Color(100, 50, 0));
  if (temp > 19)  pixels.setPixelColor((4+pos_y)*8+pos_x+2, pixels.Color(150, 50, 0));
  if (temp > 29)  pixels.setPixelColor((4+pos_y)*8+pos_x+3, pixels.Color(200, 0, 0));

  //show digit

  temp = abs(temp);
  temp = temp % 10;

  if (temp == 0) showSymbol(digit_0, pos_x, pos_y);
  if (temp == 1) showSymbol(digit_1, pos_x, pos_y);
  if (temp == 2) showSymbol(digit_2, pos_x, pos_y);
  if (temp == 3) showSymbol(digit_3, pos_x, pos_y);
  if (temp == 4) showSymbol(digit_4, pos_x, pos_y);
  if (temp == 5) showSymbol(digit_5, pos_x, pos_y);
  if (temp == 6) showSymbol(digit_6, pos_x, pos_y);
  if (temp == 7) showSymbol(digit_7, pos_x, pos_y);
  if (temp == 8) showSymbol(digit_8, pos_x, pos_y);
  if (temp == 9) showSymbol(digit_9, pos_x, pos_y);
  
}


// Show propability of precipitation

void showPop (float number, int pos_x, int pos_y) {

  //overwritting existing points

  pixels.setPixelColor((pos_y)*8+pos_x, pixels.Color(0, 0, 0));
  pixels.setPixelColor((pos_y)*8+pos_x+1, pixels.Color(0, 0, 0));
  pixels.setPixelColor((pos_y)*8+pos_x+2, pixels.Color(0, 0, 0));
  pixels.setPixelColor((pos_y)*8+pos_x+3, pixels.Color(0, 0, 0));

   //show points. Each point = 25%
  
  if (number > 0.00) pixels.setPixelColor((pos_y)*8+pos_x, pixels.Color(0, 0, 100));
  if (number > 0.25) pixels.setPixelColor((pos_y)*8+pos_x+1, pixels.Color(0, 0, 140));
  if (number > 0.50) pixels.setPixelColor((pos_y)*8+pos_x+2, pixels.Color(0, 0, 200));
  if (number > 0.75) pixels.setPixelColor((pos_y)*8+pos_x+3, pixels.Color(0, 0, 255));

  pixels.show();

}


//Show weather symbol according to weather icon name received from openweathermap

void showWeather (String icon, int pos_x, int pos_y) {

  if (icon == "01d") showSymbol(sun, pos_x, pos_y);
  if (icon == "01n") showSymbol(moon, pos_x, pos_y);
  if (icon == "02d") showSymbol(partlysun, pos_x, pos_y);
  if (icon == "02n") showSymbol(partlymoon, pos_x, pos_y);
  if (icon == "03d") showSymbol(cloudy, pos_x, pos_y);
  if (icon == "03n") showSymbol(cloudy, pos_x, pos_y);
  if (icon == "04d") showSymbol(cloudy, pos_x, pos_y);
  if (icon == "04n") showSymbol(cloudy, pos_x, pos_y);
  if (icon == "09d") showSymbol(rain, pos_x, pos_y);
  if (icon == "09n") showSymbol(rain, pos_x, pos_y);
  if (icon == "10d") showSymbol(partlyrainsun, pos_x, pos_y);
  if (icon == "10n") showSymbol(partlyrainmoon, pos_x, pos_y);
  if (icon == "11d") showSymbol(thunderstorm, pos_x, pos_y);
  if (icon == "11n") showSymbol(thunderstorm, pos_x, pos_y);
  if (icon == "13d") showSymbol(snow, pos_x, pos_y);
  if (icon == "13n") showSymbol(snow, pos_x, pos_y);
  if (icon == "50d") showSymbol(mist, pos_x, pos_y);
  if (icon == "50n") showSymbol(mist, pos_x, pos_y);
}


//Show a 2x2 pixel square according to the entries in square_array

void showSquare (int number, bool show) {
  
  uint32 color;

  if (number == 0) return;  // number 0 = show no square

  square current_square = SQUARE_ARRAY[(number-1)];

  if (show) color = pixels.Color(current_square.r, current_square.g, current_square.b);
  else color = pixels.Color(0,0,0);

  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+1, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+8, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+9, color);
  pixels.show();
}


//Receive current weather from openweathermap by HTTP get request

void getWeather() {
  
  WiFiClient client_owm;
  HTTPClient http_owm;
  String json_weather = "{}";
  JsonDocument doc_weather; 
  JsonDocument filter;

  String serverPath = "http://api.openweathermap.org/data/2.5/forecast?q=" + OWM_CITY + "," + OWM_COUNTRYCODE + "&cnt=1&APPID=" + OWM_KEY;

  http_owm.begin(client_owm, serverPath);
  int httpResponseCode = http_owm.GET();
    
  if (httpResponseCode == 200) {
    Serial.println(F("Weather data received!"));
    json_weather = http_owm.getString();
  }
  else {
    Serial.print(F("Error code: "));
    Serial.println(httpResponseCode);
  }

  http_owm.end();

  //define Json Filter document:

  filter["list"][0]["main"]["temp"] = true;
  filter["list"][0]["weather"][0]["icon"] = true;
  filter["list"][0]["pop"] = true;

  // Deserialize the json

  DeserializationError error = deserializeJson(doc_weather, json_weather, DeserializationOption::Filter(filter));
 
  if (error) {
    Serial.print(F("deserialize of json_weather failed: "));
    Serial.println(error.c_str());
    log(WARNING_COLOR, F("Deserialize openweathermap response failed"));
  }

  float main_temp = doc_weather["list"][0]["main"]["temp"];
  const char* weather_icon = doc_weather["list"][0]["weather"][0]["icon"];
  int pop = doc_weather["list"][0]["pop"];

  Serial.println("Temperature: " + String(main_temp));
  Serial.println("Weather Icon: " + String(weather_icon));
  Serial.println("Pop: " + String(pop));

  if (display_on) {                                 //show weather, if display is set on
    showTemperature((main_temp-273.15), 4, 0);
    showWeather(weather_icon, 0, 0);
    showPop(float(pop), 0, 5);
  }
}


// -----------------------------------------------------------------------------------------------------
// File system functions -------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

//Create a file and write first data

void writeFile(const String &path, const String &message) {
 
  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println(F("Failed to open file for writing"));
    return;
  }
  if (!file.print(message)) {
    Serial.println(F("Write failed"));
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}


//Read all data from a file

String readFile(const String &path) {

  String log_text;

  File file = LittleFS.open(path, "r");
  if (file) {
    while (file.available()) {
      log_text += char(file.read());
    }
    file.close();
  }
  else {
    Serial.println(F("Failed to open file for reading"));
  }
  return log_text;
}


//Adds a message to the log files. 

void appendFile(const String &message) {
 
  for (int i = 0; i < NUMBER_OF_FILES; i++) {                     //checks in every file, if max_file_size is not exceeded. Then append message to the first file with space

    File file = LittleFS.open("/log_file" + String(i), "a");

    if (!file) {
      Serial.println(F("Failed to open file for append data"));
      return;
    }
    
    if (file.size() < MAX_FILE_SIZE) {
      if (!file.print(message)) {
        Serial.println("Append to file " + String(i) + "failed");
      }
      lastfile = i;  
      file.close();
      return;
    }
    else {
      file.close();
    }
  }

  lastfile++;                                              //When all files are full: Overwrite the file after the last file where a message was added to. If this was the last file -> jump back to the first one
  if (lastfile == NUMBER_OF_FILES) lastfile = 0;
  writeFile("/log_file" + String(lastfile), message);
  Serial.println("File /log_file" + String(lastfile) +" overwritten");
}


//Adds to message a timestap and set HTML color tags

void log(const String &color, String newentry) {

  String time;
  getLocalTime(&timeinfo);

  time = String(asctime(&timeinfo));
  time = time.substring(0, time.length()-1);

  if (color == "white") {                                     //white is the standard color. font color tags are not set to save memory
    newentry = time + "  " + newentry + "<br>";
  }
  else {
    newentry = "<font color=\"" + color + "\"> " + time + " " + newentry + "<font color=\"white\"><br>";
  }
 
  appendFile(newentry); 
}


//Check for log_files, parameter and client_registered file at start-up. If not found, create them

void checkFiles() {

  for (int i = 0; i < NUMBER_OF_FILES; i++) {
    bool fileexists = LittleFS.exists("/log_file" + String(i));
    if(!fileexists) {
      Serial.println("Log file " + String(i) + " does not exist. Creating file...");
      writeFile("/log_file" + String(i), " ");
    }
    else {
      Serial.println("Log file " + String(i) + " found");
    }
  }

  bool fileexists = LittleFS.exists("/parameter");
  if(!fileexists) {
    Serial.println(F("Parameter file does not exist."));
  }
  else {
    Serial.println(F("Parameter file found"));
    stringToParameterArray(readFile("/parameter"));    //when parameter file found, read in data and convert into parameter_array
  }

  fileexists = LittleFS.exists("/client_registered");
  if(!fileexists) {
    Serial.println(F("Client_registered file does not exist. Creating it"));
    writeFile("/client_registered", "false");             //If file doesn't exist, create them with entry "false" -> this activates client registration at start_up
  }
  else {
    Serial.print(F("Client_registered = "));
    Serial.println(readFile("/client_registered"));
  }
}


// -----------------------------------------------------------------------------------------------------
// Web server functions---------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}


//Index Page

void handleIndex(){
  server.send(200, "text/html", FPSTR(INDEX));
}

//Message

void handleMessage(){
  server.send(200, "text/plain", message);
}


//Send HTML config page

void handleConfig(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(FPSTR(CONFIG_HEADER));
  
  for (int i = 0; i < sizeof(SQUARE_ARRAY)/sizeof(square); i++) {  
    server.sendContent("<div style=\"width:50px;height:50px;background-color:rgb(" + String(SQUARE_ARRAY[i].r) + ", " + String(SQUARE_ARRAY[i].g) + ", " + String(SQUARE_ARRAY[i].b) + "); float: left;\">" + String(i+1) + "</div>");
  }

  server.sendContent(FPSTR(CONFIG_MID));

  for (int i = 0; i < NUMBER_OF_ENTRIES; i++) {
    
    String number;
    String text;

    if (i < 10) number = "0" + String(i);   //adds a leading "0" when i is only one digit
    else number = String(i);

    text = "<input type=\"text\" id=\"name\" name=\"name" + number + "\" value=\"" + parameter[i].name + "\" maxlength=\"30\" style=\"width: 200px;\">";
    text += "<input type=\"text\" id=\"square\" name=\"square" + number + "\" value=\"" + String(parameter[i].square) + "\" maxlength=\"1\" style=\"width: 15px;\">";
    text += "<input type=\"checkbox\" id=\"blinking\" name=\"blinking" + number + "\"";
    if (parameter[i].blinking) text += "checked";
    text += "><input type=\"checkbox\" id=\"buzzer\" name=\"buzzer" + number + "\"";
    if (parameter[i].buzzer) text += "checked";
    text += "><input type=\"text\" id=\"function\" name=\"function" + number + "\" value=\"" + parameter[i].special_function + "\" maxlength=\"100\" style=\"width: 500px;\"><br>";
    server.sendContent(text);
  }
  
  server.sendContent(FPSTR(CONFIG_END));
  server.client().stop();
  message = "";
}


//Handle POST message from HTML form

void handleReceiveConfig(){
  String text = server.arg("plain");            // read in POST body message
  writeFile("/parameter", text);                // write message into file
  stringToParameterArray(text);                 //convert message into parameter_array
  handleConfig();
}


//Restart register process. That's usefull if you delete the pixel display from your list of mobile devices at the Bosch APP and dont want to upload and erase the flash memory

void handleRegister(){
  handleIndex();
  log(WARNING_COLOR, F("Register process restarted via HTTP /register")); 
  writeFile("/client_registered", "false");  
  Serial.println(F("Register process restarted via HTTP /register"));                  
  ESP.restart();
}


//Restart the display... just a nice to have function ;)

void handleRestart(){
  handleIndex();
  log(WARNING_COLOR, F("Restart via HTTP /restart"));
  Serial.println(F("Restart via HTTP /restart"));                
  ESP.restart();
}


//Delete all log files

void handleDeleteLog(){
  handleIndex();
  for (int i = 0; i < NUMBER_OF_FILES; i++) {
    writeFile("/log_file" + String(i), " ");
  }
  log(ALARM_COLOR, F("All log files deleted!"));
  Serial.println(F("All log files deleted!"));
  message = F("All log files deleted!");
}


//Send HTML log page

void handleLog(){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(FPSTR(LOG_HEADER));
  
    for (int i = 0; i < NUMBER_OF_FILES; i++) {
      File file = LittleFS.open("/log_file" + String(i), "r");
      if (file) server.sendContent(file);
      else server.sendContent(F("Log file not found!<br>"));
      file.close();
    }

    server.sendContent("<br>-End of log-<br><br>Free RAM: " + String(ESP.getFreeHeap()) + " byte</p></body></html>");
    server.client().stop();
    message = "";
  }


// Converts the body of POST message from the HTML form into parameter_array

void stringToParameterArray (const String &text) {
  int pos_end = 0;
  int pos_end_new = 0;
  int pos_mid = 0;
  String variable;
  String value;
  String position;
  int pos;

  for (int i = 0; i < NUMBER_OF_ENTRIES; i++) {      //HTML form don't send unchecked boxes. Therefore, all entries are set to false
    parameter[i].blinking = false;
    parameter[i].buzzer = false;
  }

  for (;;) {

    pos_end_new = text.indexOf("\n", pos_end);
    pos_mid = text.indexOf("=", pos_end);
    if (pos_end_new < 0) break;                   //when no new \n is found, we are at the end of message

    variable = text.substring(pos_end, pos_mid-2);
    position = text.substring(pos_mid-2, pos_mid);
    value = text.substring(pos_mid+1, pos_end_new-1);
    pos_end = pos_end_new+1;

    pos = position.toInt();

    if (variable == "name") { parameter[pos].name = value; }
    if (variable == "square") { parameter[pos].square = value.toInt(); }
    if (variable == "blinking") { if (value == "on") { parameter[pos].blinking = true; }}
    if (variable == "buzzer") { if (value == "on") { parameter[pos].buzzer = true; }}
    if (variable == "function") { parameter[pos].special_function = value; }
  }
}


// -----------------------------------------------------------------------------------------------------
// Special functions -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

void executeSpecialFunctions (const String &text, bool execute) {

  int pos_end = 0;
  int pos_end_new = 0;
  int pos_mid = 0;
  String function_name;
  String function_parameter;

  for (;;) {

    pos_end_new = text.indexOf(";", pos_end);                   //every special function call needs to be followed by ";". If no ";" found, end this function
    if (pos_end_new < 0) break;

    pos_mid = text.indexOf("(", pos_end); 	                    //looking for parameters between ()
    if (pos_mid < 0) {                                          //no parameters
      function_name = text.substring(pos_end, pos_end_new);
      function_name.trim();    
    }
    else {                                                      //with parameters
      function_name = text.substring(pos_end, pos_mid); 
      function_name.trim();
      function_parameter = text.substring(pos_mid+1, pos_end_new-1);
      function_parameter.trim();
    }
    pos_end = pos_end_new+1;


    //all special functions come here:

    if (function_name == "sonos_stop") {
      if (execute) {
        sonosCommand("Stop", function_parameter);
      }
    }

    if (function_name == "sonos_play") {
      if (execute) {
        sonosCommand("Play", function_parameter);
      }
    }

    if (function_name == "display_off") {
      if (execute) {
        for (int i = 0; i < 48; i++ ) {
          pixels.setPixelColor(i, pixels.Color(0,0,0));
          pixels.show();
        }
        display_on = false;   
      }
      else {
        get_weather = true;
        display_on = true;
      }
    }

    if (function_name == "brightness_high") {
      if (execute) {
        int brightness = function_parameter.toInt();
        constrain(brightness, 0, MAX_BRIGHTNESS);
        pixels.setBrightness(brightness);
        pixels.show();
      }
      else {
        pixels.setBrightness(INITIAL_BRIGHTNESS);
        pixels.show();
      }
    }

    if (function_name == "pc_execute") {
      if (execute) {
        pcCommand(function_parameter);
      }
    }
  }
}


// Send command via UDP to a PC. You need to install and run UDPRun software on that PC

void pcCommand(const String &command) {

  WiFiUDP Udp;
  IPAddress IP;

  IP.fromString("192.168.0.255");       //broadcast in local subnet
  Udp.begin(4210);
  Udp.beginPacket(IP, 9);               //connect to port 9 at PC. UDPRun needs to listen on the same port
  Udp.write(command.c_str());
  Udp.endPacket();
  Serial.println(F("PC command sent: ") + command);
}


//Stop a sonos speaker under a given IP address by HTTP post 

void sonosCommand(const String &action, const String &IP) {

  String arguments;

  if (action == "Stop") arguments = "<InstanceID>0</InstanceID>";
  if (action == "Play") arguments = "<InstanceID>0</InstanceID><Speed>1</Speed>";

  const String url = F("/AVTransport/Control");
  const String service = F("AVTransport:1");
  const String output = "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:" + action + " xmlns:u=\"urn:schemas-upnp-org:service:" + service + "\">" + arguments + "</u:" + action + "></s:Body></s:Envelope>";
  WiFiClient client_sonos;
 
  client_sonos.connect(IP, 1400);

  client_sonos.print(String("POST ") + "/MediaRenderer/AVTransport/Control HTTP/1.1\r\n" +
                 "Host: " + IP + "\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "Content-Type: text/xml; charset=utf-8\r\n" +
                 "Soapaction: urn:schemas-upnp-org:service:" + service + "#" + action + "\r\n" +
                 "Connection: close\r\n" +                 
                 "\r\n" +
                 output + "\n");
   
   delay(100);

   Serial.println(F("Sonos stop sent to IP: ") + IP);

   client_sonos.stop();
}