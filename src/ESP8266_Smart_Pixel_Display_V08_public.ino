/*##############################################################################
#   Smart Home Pixel Display                                                   #
#   Last change:   28.02.2025                                                  #
#   Version:       0.8                                                         #
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
#include "symbols.h"

/*##################################################################################
#####   !!!!!        Adjust the following entries        !!!!!    ##################
##################################################################################*/

const char* WIFI_ssid = "xxxxxxxx";                                        //Your Wifi name
const char* WIFI_password = "xxxxxxxxxxxxx";                               //Your Wifi password
const String BSH_ip = "192.168.xxx.xxx";                                   //IP adress of your BSH controller
const String BSH_password = "xxxxxxxxxxxxxxxx";                            //Your BSH controller password in base64 coding

const String OWM_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";                 //Your openweathermap API key
const String OWM_city = "xxxxxx";                                          //Your city name, e.g., Hamburg
const String OWM_countrycode = "xx";                                       //Country code of your city, e.g., DE

const int BUZZER_PIN = 5;                                                  // GPIO pin for buzzer, please check or change
const int LED_MATRIX_PIN = 4;                                              // GPIO pin for display, please check or change

const int BRIGHTNESS = 5;                                                  // Brightness adjustment. Be aware that higher values increase power consumption and may damage your USB port                    

//Enter your client_cert.pem below

const char* client_cert PROGMEM = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIDcTCCAlmgAwIBAgIUH0Pbupca1qIOwHtCydADDGXs3YEwDQYJKoZIhvcNAQEL
BQAwRzELMAkGA1UEBhMCREUxCzAJBgNVBAgMAkJFMQ8wDQYDVQQHDAZCZXJsaW4x
some more lines here
vcFfUC1/fuRDlPbpxDfTdnTEepOGrCmWF/dUJoFFtMkB/HvBsKwE+OlcBx5AdeJv
VfDKD7Y1WIuARPFjvp/4QTwQvNoW
-----END CERTIFICATE-----
)string_literal";

//Enter your client_key.pem below

const char* client_key PROGMEM = R"string_literal(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDtA8MhKS5Q5JED
MNHFjHzANUL7qdnVlM7BFPLk18R4GHXONNAqXjrmGB7DP50AIoLmYo+UC0nALv8X
some more lines here
9XWH57UgOCpjFmPhg0MN48qCZEMPHKprxWkTtyDLNMpL7JYI1ILbYKudbpZIOAme
EGCBICaGpxbFwcb++//rUbIu
-----END PRIVATE KEY-----
)string_literal";


/*##################################################################################
#####   !!!!!  Nothings needs to be changed after here   !!!!!    ##################
##################################################################################*/

struct param {String name; int square; bool blinking; bool buzzer; String special_function; };
struct square {int pos_x; int pos_y; int r; int g; int b; };

const char* BSH_root_CA PROGMEM = R"string_literal(
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

const square square_array[] = {{0,6,255,0,0},                              // This array defines the 2x2 square pixel areas to show the userdefinedstates. Scheme: x-pos, y-pos, red, blue, green color value (0-255)
                               {2,6,0,0,255},
                               {4,6,255,100,10},
                               {6,6,200,0,200}};

const char* dns_name = "smartdisplay";
const int number_of_files = 10;                                            //Number of log files. Many files will slow down first start-up
const int max_file_size = 2000;                                            //Size of each log file. Increasing this value may bring up memory problems 
const int number_entries = 12;                                             //Number of entries in the parameter table. Please change if you want to have more/less entries 

const String startup_color = "royalblue"; 
const String alarm_color = "red";
const String warning_color = "yellow";
 
//Variables and objects

JsonDocument doc;
String json = "";
String polling_id;
char c;

bool subscribe_poll = true;
bool poll = true;
bool get_weather = true;
bool display_on = true;

param parameter[number_entries] = {};
bool state_array[number_entries] = {false};

int firstindex;
int lastindex;
int lastfile = 0;
unsigned long current_time = 0;
unsigned long prev_time = 0;
unsigned long current_time2 = 0;
unsigned long prev_time2 = 0;
unsigned long current_time3 = 0;
unsigned long prev_time3 = 0;

struct tm timeinfo;
time_t now;

IPAddress IP;
Adafruit_NeoPixel pixels(64, LED_MATRIX_PIN, NEO_GRB + NEO_KHZ800);
WiFiClientSecure *client = new WiFiClientSecure;
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
  Serial.println();

  // Setup matrix led display 

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);
 
  // Pin assignment  

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  // Check all squares at start-up for checking

  show_square(1, true);
  show_square(2, true);
  show_square(3, true);
  show_square(4, true);
  delay(1000);
  pixels.clear();
 
  // Setup wifi connection 

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ssid, WIFI_password);
  Serial.print(F("Connecting to WiFi.."));

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    show_square(1, true);
    delay(500);
    show_square(1, false);
    delay(500);
  }

  Serial.println(F(" WiFi connected!"));
  show_square(1, true);
  IP.fromString(BSH_ip);

  // Start DNS

  if (MDNS.begin(dns_name)) {
    Serial.print(F("DNS started, available at: "));
    Serial.println("http://" + String(dns_name) + ".local/");
  }

  // Receive time 

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print(F("Waiting for NTP time sync: "));
  now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(F(" time received!"));
  gmtime_r(&now, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
  
  // Start file system and check if all files exist. If not, they will be created

  LittleFS.begin();
  checkfiles();
  show_square(2, true); 
  
  // Setup keys and certificates

  X509List servercert(BSH_root_CA);
  X509List clientcert(client_cert);
  PrivateKey clientkey(client_key);
  client->setTrustAnchors(&servercert);
  client->setClientRSACert(&clientcert, &clientkey);
  
  // Register client

  if (readFile("/client_registered") == "false") {
    registerclient();
  }

  // Connect to server at port 8444. This only works after successful client registration
  
  Serial.print("Connecting to server " + BSH_ip + " ..");

  while (!client->connect(IP, 8444)) {
    Serial.print('.');
    show_square(3, true);
    delay(500);
    show_square(3, false);
    delay(500);
  }

  Serial.println(F("Connected to server!"));
  show_square(3, true);
  
  // Start web server

  server.onNotFound(handle_not_found); 
  server.on("/", HTTP_GET, handle_log);
  server.on("/config", HTTP_GET, handle_config);
  server.on("/config", HTTP_POST, handle_receive_config);
  server.on("/register", HTTP_GET, handle_register);  
  server.begin();
  Serial.println(F("Web server started."));
  show_square(4, true);
  delay(1000);
  pixels.clear();
   
  // Get the status of all user defined states after start-up and log them

  log(startup_color, "Smart pixel display started. Current user defined states:"); 
  get_user_defined_states();
  log(startup_color, "Start-up completed");
  Serial.println(F("Start-up completed.\n"));

  prev_time = millis();
  prev_time2 = millis();
  prev_time3 = millis();
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
    for (int i = 0; i < number_entries; i++) {
      if (state_array[i]) {
        show_square(parameter[i].square, true);
        if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }

  if ((current_time - prev_time) > 1000) {
    for (int i = 0; i < number_entries; i++) {
      if ((parameter[i].blinking) && (state_array[i])) {
        show_square(parameter[i].square, false);
        if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, HIGH);
      }
    }
    prev_time = current_time;
  }

  // Reading available data non-blocking

  while (client->available()) {
    c = client->read();
    json += c;
  }

  // If we received something > 580 chars long (header + body), start analyze the data

  if (json.length() > 580) {

    // Find the json

    firstindex = json.indexOf("[");
    lastindex = json.lastIndexOf("]");
    json = json.substring(firstindex, lastindex+1);
    
    // Deserialize the json

    DeserializationError error = deserializeJson(doc, json);
    json = "";

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }

    JsonArray array = doc[0]["result"].as<JsonArray>();
    Serial.println("Number of messages received: " + String(array.size()));
    long error_code = doc[0]["error"]["code"];                  // error code
    const char* error_message = doc[0]["error"]["message"];     // error message

    for (int i = 0; i < array.size(); i++) {        

      const char* name = doc[0]["result"][i]["name"];          // name of the userdefinedstate
      bool state = doc[0]["result"][i]["state"];               // state of the userdefinedstate: false or true

      // If "name" is not empty, whe have received a message with a new status

      if (String(name) != "") {
          
        // Check, which square needs to bet set and start special functions

        for (int i = 0; i < number_entries; i++ ) {
          if (parameter[i].name == String(name)) {

            Serial.println(String(name) + " = " + String(state));
            log("white", String(name) + " = " + String(state));

            if (state) {
              show_square(parameter[i].square, true);
              execute_special_functions(parameter[i].special_function, true);
              if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, LOW);
              state_array[i] = true;
            }
            else {
              show_square(parameter[i].square, false);
              execute_special_functions(parameter[i].special_function, false);
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
      Serial.println("Error code received: " + String(error_code) +  ", error message : " + String(error_message));
    }

    poll = true;

  }
 
  // Receive weather data from OpenWeatherMap every 10 minutes

  current_time2 = millis();
 
  if (((current_time2 - prev_time2) > 600000) || get_weather) {
    http_get_weather();
    prev_time2 = millis();
    get_weather = false;
  }

  // Update epoch time

  current_time3 = millis();

  if ((current_time3 - prev_time3) > 1000) {

    prev_time3 = millis() - (current_time3 - prev_time3 - 1000);
    now = now + 1;

  }

  //check, if WIFI and client is still available. If we lost connection, log it and restart the ESP

  if (!client) {                           
    Serial.println(F("Client lost... restart ESP"));
    log(alarm_color, "Client lost... restart ESP");
    ESP.restart();
  }

  if (WiFi.status()!=WL_CONNECTED) {                           
    Serial.println(F("Wifi connection lost... restart ESP"));
    log(alarm_color, "Wifi connection lost... restart ESP");
    ESP.restart();
  }

  delay (100);
}


// -----------------------------------------------------------------------------------------------------
// BSH functions ---------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

//Subscribe polling and return polling_id

String subscribePolling (void) {

  String line;

  String output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/subscribe\",\"params\": [\"com/bosch/sh/remote/userdefinedstates\", null]}]";

  client->print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");

  while (client->connected()) {
    line = client->readStringUntil('\n');
    
    if (line == "\r") {
      break;
    }
  }

  while (client->available()) {
    c = client->read();
    json += c;
  }

  // Find the json

  firstindex = json.indexOf("[");
  lastindex = json.lastIndexOf("]");
  json = json.substring(firstindex, lastindex+1);

  // Deserialize the JSON

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

void startPoll (String _polling_id) {

  String output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/longPoll\",\"params\": [\"" + _polling_id + "\",8000]}]";   // Poll is valid for 8000s, will be renewed automatically

  client->print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");
  Serial.println(F("Poll sent... wait for answer"));
}

//Get all user defined states at start-up

void get_user_defined_states() {

  client->print(String("GET ") + "/smarthome/userdefinedstates" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Connection: keep-alive\r\n" +
                 "\r\n");
                 
  Serial.println(F("Get user defined states after start up:"));

  delay(1000);

  while (client->available()) {
    c = client->read();
    json += c;
  }

  firstindex = json.indexOf("[");
  lastindex = json.lastIndexOf("]");
  json = json.substring(firstindex, lastindex+1);
    
  // Deserialize the json

  DeserializationError error = deserializeJson(doc, json);
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

      for (int i = 0; i < number_entries; i++ ) {
        if (parameter[i].name == String(name)) {

          Serial.println(String(name) + " = " + String(state));
          log(startup_color, String(name) + " = " + String(state));

          if (state) {
            show_square(parameter[i].square, true);
            execute_special_functions(parameter[i].special_function, true);
            state_array[i] = true;
            if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, LOW);
          }
          else {
            show_square(parameter[i].square, false);
            execute_special_functions(parameter[i].special_function, false);
            state_array[i] = false;
            if (parameter[i].buzzer) digitalWrite(BUZZER_PIN, HIGH);
          }
        }
      }
    }
  }  
}

//Register pixel display at Bosch smart home controller

void registerclient () {
  
  String cert;
  String line;
  String json;

  Serial.println(F("Connecting to server at port 8443..."));
  Serial.println(F("Press the button on the Bosch smart home controller now!"));

  while (!client->connect(IP, 8443)) { 
    show_square(3, true);
    delay(250);
    show_square(3, false);
    delay(250);
    show_square(3, true);
    delay(250);
    show_square(3, false);
    delay(250);
    show_square(3, true);
    delay(250);
    show_square(3, false);
    delay(250);
    show_square(3, true);
    delay(250);
    show_square(3, false);
  }

  Serial.println(F("Connected to server at port 8443!"));
  show_square(3, true);
  delay(500);

  //formate the client_cert for being sent as JSON to the BSH controller

  cert = String(client_cert);
  cert.replace("\n","");
  cert.replace("-----BEGIN CERTIFICATE-----","-----BEGIN CERTIFICATE-----\\r");
  cert.replace("-----END CERTIFICATE-----","\\r-----END CERTIFICATE-----");

  json = "{\"@type\":\"client\",\"id\":\"oss_pixel_display\",\"name\":\"OSS Pixel Display\",\"primaryRole\":\"ROLE_RESTRICTED_CLIENT\",\"certificate\":\"";
  json += cert;
  json += "\"}";
  
  //send header and JSON

  client->print(String("POST ") + "/smarthome/clients" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8443\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Systempassword: " + BSH_password + "\r\n" + 
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + json.length() + "\r\n" +
                 "\r\n" +
                 json + "\n");
                 
  Serial.println(F("Client registration posted... wait for answer"));

  while (client->connected()) {
    line = client->readStringUntil('\n');
    
    if(line.indexOf("201") > 0) {                             //code 201 means client registration was successful
         Serial.println(F("Successfully created device. OSS Pixel Display should occur now at Bosch APP at system > mobile devices. ESP restarts in 5 s"));
         show_square(4, true);
         writeFile("/client_registered", "true");
         log(warning_color, "Smart pixel display registered at Bosch smart home controller");
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

void show_symbol (symbol sym, int pos_x, int pos_y) {

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

void show_temperature (int temp, int pos_x, int pos_y) {

  //overwritting points under the digit

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

  if (temp == 0) show_symbol(digit_0, pos_x, pos_y);
  if (temp == 1) show_symbol(digit_1, pos_x, pos_y);
  if (temp == 2) show_symbol(digit_2, pos_x, pos_y);
  if (temp == 3) show_symbol(digit_3, pos_x, pos_y);
  if (temp == 4) show_symbol(digit_4, pos_x, pos_y);
  if (temp == 5) show_symbol(digit_5, pos_x, pos_y);
  if (temp == 6) show_symbol(digit_6, pos_x, pos_y);
  if (temp == 7) show_symbol(digit_7, pos_x, pos_y);
  if (temp == 8) show_symbol(digit_8, pos_x, pos_y);
  if (temp == 9) show_symbol(digit_9, pos_x, pos_y);
  
}

// Show propability of precipitation

void show_pop (float number, int pos_x, int pos_y) {

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

void show_weather (String icon, int pos_x, int pos_y) {

  if (icon == "01d") show_symbol(sun, pos_x, pos_y);
  if (icon == "01n") show_symbol(moon, pos_x, pos_y);
  if (icon == "02d") show_symbol(partlysun, pos_x, pos_y);
  if (icon == "02n") show_symbol(partlymoon, pos_x, pos_y);
  if (icon == "03d") show_symbol(cloudy, pos_x, pos_y);
  if (icon == "03n") show_symbol(cloudy, pos_x, pos_y);
  if (icon == "04d") show_symbol(cloudy, pos_x, pos_y);
  if (icon == "04n") show_symbol(cloudy, pos_x, pos_y);
  if (icon == "09d") show_symbol(rain, pos_x, pos_y);
  if (icon == "09n") show_symbol(rain, pos_x, pos_y);
  if (icon == "10d") show_symbol(partlyrainsun, pos_x, pos_y);
  if (icon == "10n") show_symbol(partlyrainmoon, pos_x, pos_y);
  if (icon == "11d") show_symbol(thunderstorm, pos_x, pos_y);
  if (icon == "11n") show_symbol(thunderstorm, pos_x, pos_y);
  if (icon == "13d") show_symbol(snow, pos_x, pos_y);
  if (icon == "13n") show_symbol(snow, pos_x, pos_y);
  if (icon == "50d") show_symbol(mist, pos_x, pos_y);
  if (icon == "50n") show_symbol(mist, pos_x, pos_y);
}

//Show a 2x2 pixel square according to the entries in square_array

void show_square (int number, bool show) {
  
  uint32 color;

  if (number == 0) return;  // number 0 = show no square

  square current_square = square_array[(number-1)];

  if (show) color = pixels.Color(current_square.r, current_square.g, current_square.b);
  else color = pixels.Color(0,0,0);

  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+1, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+8, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+9, color);
  pixels.show();
}

//Receive current weather from openweathermap by HTTP get request

void http_get_weather() {
  
  WiFiClient client_owm;
  HTTPClient http_owm;
  String json_weather = "{}";
  JsonDocument doc_weather; 
  String serverPath;
  int httpResponseCode;

  serverPath = "http://api.openweathermap.org/data/2.5/forecast?q=" + OWM_city + "," + OWM_countrycode + "&cnt=1&APPID=" + OWM_key;

  http_owm.begin(client_owm, serverPath);
  httpResponseCode = http_owm.GET();
    
  if (httpResponseCode > 0) {
    Serial.print(F("HTTP Response code from OpenWeatherMap server: "));
    Serial.println(httpResponseCode);
    json_weather = http_owm.getString();
  }
  else {
    Serial.print(F("Error code: "));
    Serial.println(httpResponseCode);
  }

  http_owm.end();

  DeserializationError error = deserializeJson(doc_weather, json_weather);
    
  if (error) {
    Serial.print(F("deserialize of json_weather failed: "));
    Serial.println(error.c_str());
  }

  JsonObject list_0 = doc_weather["list"][0];
  JsonObject list_0_main = list_0["main"];
  JsonObject list_0_weather_0 = list_0["weather"][0];
  float main_temp = list_0_main["temp"];
  const char* weather_icon = list_0_weather_0["icon"];
  int pop = list_0["pop"];

  Serial.println("Temperature: " + String(main_temp));
  Serial.println("Weather Icon: " + String(weather_icon));
  Serial.println("Pop: " + String(pop));

  if (display_on) {                                 //show weather, if display is set on
    show_temperature((main_temp-273.15), 4, 0);
    show_weather(weather_icon, 0, 0);
    show_pop(float(pop), 0, 5);
  }
}


// -----------------------------------------------------------------------------------------------------
// File system functions -------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

//Create a file and write first data

void writeFile(String path, String message) {
 
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

String readFile(String path) {

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

void appendFile(String message) {
 
  for (int i = 0; i < number_of_files; i++) {                     //checks in every file, if max_file_size is not exceeded. Then append message to the first file with space

    File file = LittleFS.open("/log_file" + String(i), "a");

    if (!file) {
      Serial.println(F("Failed to open file for append data"));
      return;
    }
    
    if (file.size() < max_file_size) {
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
  if (lastfile == number_of_files) lastfile = 0;
  writeFile("/log_file" + String(lastfile), message);
  Serial.println("File /log_file" + String(lastfile) +" overwritten");
}

//Adds to message a timestap and set HTML color tags

void log(String color, String newentry) {
  String time;
  gmtime_r(&now, &timeinfo);
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

void checkfiles() {
  for (int i = 0; i < number_of_files; i++) {
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
    string_to_parameter_array(readFile("/parameter"));    //when parameter file found, read in data and convert into parameter_array
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

void handle_not_found(){
  server.send(404, "text/plain", "404: Not found");
}

//Send HTML config page

void handle_config(){
  String text;
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(F("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Smart Home Pixel Display - Configuration</title></head><body style=\"background-color:black;\"><p><h1><font color=\"white\">Smart Home Pixel Display</h1>Defined pixel squares and their numbers: <br><br>"));
  
  for (int i = 0; i < sizeof(square_array)/sizeof(square); i++) {  
    server.sendContent("<div style=\"width:70px;height:70px;background-color:rgb(" + String(square_array[i].r) + ", " + String(square_array[i].g) + ", " + String(square_array[i].b) + "); float: left;\">" + String(i+1) + "</div>");
  }

  server.sendContent(F("<p>&nbsp</p><p>&nbsp</p><p>&nbsp</p><p>Configuration of user defined states:</p><div style=\"width: 210px; float: left;\">Name</div><div style=\"width: 25px; float: left;\">#</div><div style=\"width: 20px; float: left;\">F</div><div style=\"width: 20px; float: left;\">B</div>Function<form action=\"/config\" method=\"post\" enctype=\"text/plain\">"));

  for (int i = 0; i < number_entries; i++) {
    text = "<input type=\"text\" id=\"name\" name=\"name" + String(i) + "\" value=\"" + parameter[i].name + "\" maxlength=\"30\" style=\"width: 200px;\">";
    text += "<input type=\"text\" id=\"square\" name=\"square" + String(i) + "\" value=\"" + String(parameter[i].square) + "\" maxlength=\"1\" style=\"width: 15px;\">";
    text += "<input type=\"checkbox\" id=\"blinking\" name=\"blinking" + String(i) + "\"";
    if (parameter[i].blinking) text += "checked";
    text += "><input type=\"checkbox\" id=\"buzzer\" name=\"buzzer" + String(i) + "\"";
    if (parameter[i].buzzer) text += "checked";
    text += "><input type=\"text\" id=\"function\" name=\"function" + String(i) + "\" value=\"" + parameter[i].special_function + "\" maxlength=\"30\" style=\"width: 200px;\"><br>";
    server.sendContent(text);
    text ="";
  }
  
  server.sendContent(F("<br><input type=\"submit\" value=\"Submit\"></form><br>Name = Name of your user defined state in the Bosch smart home app<br># = Number of pixel square. If no square is to be activated, use number 0<br>F = Flashing?<br>B = Buzzer?<br>Function = Special function to call upon state change</p></body></html>"));
  server.client().stop();
}

//Handle POST message from HTML form

void handle_receive_config(){
  String text = server.arg("plain");            // read in POST body message
  writeFile("/parameter", text);                // write message into file
  string_to_parameter_array(text);              //convert message into parameter_array
  handle_config();
}

//Restart register process. That's usefull if you delete the pixel display from your list of mobile devices at the Bosch APP and dont want to upload and erase the flash memory

void handle_register(){
  server.send(200, "text/plain", "Register process restarted");
  log(warning_color, "Register process restarted via HTTP /register"); 
  writeFile("/client_registered", "false");                
  ESP.restart();
}

//Send HTML log page

void handle_log(){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(F("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Smart Home Pixel Display</title></head>"));
    server.sendContent(F("<body style=\"background-color:black;\"><p><h1><font color=\"white\">Smart Home Pixel Display</h1>Log of user defined state changes<br>All times are GMT<br><br>"));

    for (int i = 0; i < number_of_files; i++) {
      server.sendContent(readFile("/log_file" + String(i)));
    }

    gmtime_r(&now, &timeinfo);
    server.sendContent("<br>End of log. <br>Current time: " + String(asctime(&timeinfo)) + ", free RAM: " + String(ESP.getFreeHeap()));
    server.sendContent(F("</p></body></html>")); 
    server.client().stop();
  }


// Converts the body of POST message from the HTML form into parameter_array

void string_to_parameter_array (String text) {
  int pos_end = 0;
  int pos_end_new = 0;
  int pos_mid = 0;
  String variable;
  String value;
  String position;
  int pos;

  for (int i = 0; i < number_entries; i++) {      //HTML form don't send unchecked boxes. Therefore, all entries are set to false
    parameter[i].blinking = false;
    parameter[i].buzzer = false;
  }

  for (;;) {

    pos_end_new = text.indexOf("\n", pos_end);
    pos_mid = text.indexOf("=", pos_end);
    if (pos_end_new < 0) break;                   //when no new \n is found, we are at the end of message

    variable = text.substring(pos_end, pos_mid-1);
    position = text.substring(pos_mid-1, pos_mid);
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

void execute_special_functions (String name_of_function, bool execute) {

  if (name_of_function == "sonos_off") {
    if (execute) {
      sonos_stop("192.168.0.105");
      sonos_stop("192.168.0.72");
      sonos_stop("192.168.0.235");
    }
  }

  if (name_of_function == "display_off") {
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

  if (name_of_function == "brightness_high") {
    if (execute) {
      pixels.setBrightness(BRIGHTNESS + 5);
      pixels.show();
    }
    else {
      pixels.setBrightness(BRIGHTNESS);
      pixels.show();
    }
  }
}


//Stop a sonos speaker under a given IP address by HTTP post 

void sonos_stop(String IP) {

  String url = "/AVTransport/Control";
  String service = "AVTransport:1";
  String action = "Stop";
  String arguments = "<InstanceID>0</InstanceID>";

  WiFiClient client_sonos;

  String output = "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:" + action + " xmlns:u=\"urn:schemas-upnp-org:service:" + service + "\">" + arguments + "</u:" + action + "></s:Body></s:Envelope>";
 
  client_sonos.connect(IP, 1400);

  client_sonos.print(String("POST ") + "/MediaRenderer" + url + " HTTP/1.1\r\n" +
                 "Host: " + IP + "\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "Content-Type: text/xml; charset=utf-8\r\n" +
                 "Soapaction: urn:schemas-upnp-org:service:" + service + "#" + action + "\r\n" +
                 "Connection: close\r\n" +                 
                 "\r\n" +
                 output + "\n");
   
   delay(100);

   Serial.println("Sonos stop sent to IP: " + IP);

   client_sonos.stop();
}