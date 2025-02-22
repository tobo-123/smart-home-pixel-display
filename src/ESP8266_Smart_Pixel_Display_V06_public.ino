/*##############################################################################
#   Smart Home Pixel Display                                                   #
#   Last change:   20.02.2025                                                  #
#   Version:       0.6                                                         #
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

##################################################################################*/

#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "symbols.h"
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

struct param {String name; int square; bool blinking; bool buzzer; String special_function; };
struct square {int pos_x; int pos_y; int r; int g; int b; };


/*##################################################################################
#####   !!!!!        Adjust the following entries        !!!!!    ##################
##################################################################################*/

bool register_client = true;                                               //Keep true for the first run. When client is registered, change to false and re-upload the program to your ESP

const char* WIFI_ssid = "xxxx";                                            //Your Wifi name
const char* WIFI_password = "xxxx";                                        //Your Wifi password
const String BSH_ip = "192.168.xxx.xxx";                                   //IP adress of your BSH controller
const String BSH_password = "xxxxxxxxx";                                   //Your BSH controller password in base64 coding

const String OWM_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";                 //Your openweathermap API key
const String OWM_city = "xxxxxx";                                          //Your city name
const String OWM_countrycode = "xx";                                       //Country code of your city

const int BUZZER_PIN = 5;                                                  // GPIO pin for buzzer, please check or change
const int LED_MATRIX_PIN = 4;                                              // GPIO pin for display, please check or change

const int BRIGHTNESS = 5;                                                  // Brightness adjustment. Be aware that higher values increase power consumption

square square_array[] = { {0,6,255,0,0},                                   // This array defines the 2x2 square pixel areas to show the userdefinedstates. Scheme: x-pos, y-pos, red, blue, green color value (0-255)
                          {2,6,0,0,255},
                          {4,6,255,100,10},
                          {6,6,200,0,200}};

const int number_entries = 7;                                              //Number of entries in the following parameter table. Please change if you have more/less entries

const param parameter[] = {{"window open", 0, false, false, "none"},       //This table connects the userdefinedstates of your BSH with your square areas. The entries are just examples, you need to change them
                           {"bathroom wet", 1, false, false, "none"},      //Scheme: "name of your userdefined state", square number (0-3, 9 means no square), flashing (true/false), buzzer (true/false), name of special function ("none" if no special function is needed)
                           {"alarm reminder", 2, true, false, "none"},           
                           {"post box", 3, false, false, "none"},
                           {"bathroom very wet", 1, true, false, "none"},
                           {"Sonos off", 9, false, false, "sonos"},
                           {"not at home", 9, false, false, "display_off"}};

//Enter your client_cert.pem below

const char* client_cert PROGMEM = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIDcTCCAlmgAwIBAgIUH0Pbupca1qIOwHtCydADDGXs3YEwDQYJKoZIhvcNAQEL
BQAwRzELMAkGA1UEBhMCREUxCzAJBgNVBAgMAkJFMQ8wDQYDVQQHDAZCZXJsaW4x
some lines were removed
DcjLQmyvWrQWtd9CDBkzP5PLG5KM2AJ0Z+cd1nXeO+nNw5l1lWmRBmDTcr1Loa6u
vcFfUC1/fuRDlPbpxDfTdnTEepOGrCmWF/dUJoFFtMkB/HvBsKwE+OlcBx5AdeJv
VfDKD7Y1WIuARPFjvp/4QTwQvNoW
-----END CERTIFICATE-----
)string_literal";

//Enter your client_key.pem below

const char* client_key PROGMEM = R"string_literal(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDtA8MhKS5Q5JED
MNHFjHzANUL7qdnVlM7BFPLk18R4GHXONNAqXjrmGB7DP50AIoLmYo+UC0nALv8X
some lines were removed
i90SiabAh3FxACqZyxR7u3TNnxgn6LHHjT/srIcpUPok+3rmZT2Bdymq75HvTmA7
9XWH57UgOCpjFmPhg0MN48qCZEMPHKprxWkTtyDLNMpL7JYI1ILbYKudbpZIOAme
EGCBICaGpxbFwcb++//rUbIu
-----END PRIVATE KEY-----
)string_literal";


/*##################################################################################
#####   !!!!!  Nothings needs to be changed after here   !!!!!    ##################
##################################################################################*/


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

// constants

const char* dns_name = "smartdisplay";
const String file_path = "/log_file";
const int number_of_files = 5;
const int max_file_size = 2000;


const String basic_color = "white";
const String startup_color = "green"; 
const String alarm_color = "red";
const String background_color = "black";   

//Variables and objects

String header = "<h1><font color=\"" + basic_color + "\">Smart Home Pixel Display</h1>Log of user defined state changes<br>All times are GMT<br><br>";

int lastfile = 0;

JsonDocument doc;
JsonDocument doc2;

String json = "";
String json2 = "";
String output;
String polling_id;
String line;

char c;

bool subscribe_poll = true;
bool poll = true;
bool get_weather = true;
bool display_on = true;

int firstindex;
int lastindex;
unsigned long current_time = 0;
unsigned long prev_time = 0;
unsigned long current_time2 = 0;
unsigned long prev_time2 = 0;
unsigned long current_time3 = 0;
unsigned long prev_time3 = 0;

struct tm timeinfo;
time_t now;

IPAddress IP;

bool state_array[number_entries] = {false};

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

  Serial.println();
  Serial.println();

  // Setup matrix led display 

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);
 
  // pin assignment  

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  // check all squares at start-up for checking

  show_square(0, true);
  show_square(1, true);
  show_square(2, true);
  show_square(3, true);
  delay(1000);
  pixels.clear();
 
  // Setup wifi connection 

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ssid, WIFI_password);
  Serial.print("Connecting to WiFi..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    show_square(0, true);
    delay(500);
    show_square(0, false);
    delay(500);
  }

  Serial.println(" WiFi connected!");
  show_square(0, true);
  IP.fromString(BSH_ip);

  // Start DNS

  if (MDNS.begin(dns_name)) {
    Serial.print("DNS started, available at: ");
    Serial.println("http://" + String(dns_name) + ".local/");
  }

  // Receive time 

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" time received!");
  gmtime_r(&now, &timeinfo);

  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  show_square(1, true); 
  
  // Setup keys and certificates

  X509List servercert(BSH_root_CA);
  X509List clientcert(client_cert);
  PrivateKey clientkey(client_key);
  client->setTrustAnchors(&servercert);
  client->setClientRSACert(&clientcert, &clientkey);

  // Register client

  if (register_client) {
    registerclient();
  }

  // Connect to server at port 8444. This only works after successful client registration
  
  Serial.println("Connecting to server " + BSH_ip + " ..");

  while (!client->connect(IP, 8444)) {
    Serial.print('.');
    show_square(2, true);
    delay(500);
    show_square(2, false);
    delay(500);
  }

  Serial.println("Connected to server!");
  show_square(2, true);
  
  // Start file system and check if all log files exist. If not, they are created

  LittleFS.begin();

  for (int i = 0; i < number_of_files; i++) {
    bool fileexists = LittleFS.exists(file_path + String(i));
    if(!fileexists) {
      Serial.println("File " + file_path + String(i) + " does not exist. Creating file...");
      writeFile(file_path + String(i), " ");
    }
    else {
      Serial.println("File " + file_path + String(i) + " found");
    }
  }

  // Start web server

  server.onNotFound([](){
    server.send(404, "text/plain", "Link not found!"); 
  });

  server.on("/", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Smart Home Pixel Display</title></head><body style=\"background-color:" + background_color + ";\"><p>" + header);

    for (int i = 0; i < number_of_files; i++) {
      server.sendContent(readFile(file_path + String(i)));
    }

    server.sendContent("<br>End of log</p></body></html>"); 
    server.client().stop();
  });

  server.begin();
  Serial.println("Web server started.");
  show_square(3, true);
  delay(1000);
  pixels.clear();
   
  // Get the status of all user defined states after start-up and log them

  log(startup_color, "Smart pixel display started. Current user defined states:"); 
    
  get_user_defined_states();

  Serial.println("Start-up completed.\n");
  log(startup_color, "Start-up completed"); 

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

  // Register long-polling for unserdefinedstates and start poll when needed

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
      Serial.print("deserializeJson() failed: ");
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
            log(basic_color, String(name) + " = " + String(state));

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

  if ((WiFi.status()!=WL_CONNECTED) || (!client)) {                           
    Serial.println("Wifi connenction or client lost... restart ESP");
    log(alarm_color, "Wifi connenction or client lost... restart ESP");
    ESP.restart();
  }

  delay (100);

}


// -----------------------------------------------------------------------------------------------------
// BSH functions ---------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

String subscribePolling (void) {

  // Register long-polling for unserdefinedstates

  output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/subscribe\",\"params\": [\"com/bosch/sh/remote/userdefinedstates\", null]}]";

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
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "no result";
  }

  // If result is empty, we didn't got our polling_id

  if (String(doc[0]["result"]) == "") {
    Serial.println("Polling couldn't subscribed!");
    return "no result";
  }

  // Returns the polling_id, it's needed for startPoll

  Serial.println("Polling subscribed. Polling_Id: " + String(doc[0]["result"]));

  return String(doc[0]["result"]);
}


void startPoll (String _polling_id) {

  output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/longPoll\",\"params\": [\"" + _polling_id + "\",8000]}]";   // Poll is valid for 8000s, will be renewed automatically

  client->print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");
  Serial.println("Poll sent... wait for answer");
}


void get_user_defined_states() {

  client->print(String("GET ") + "/smarthome/userdefinedstates" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Connection: keep-alive\r\n" +
                 "\r\n");
                 
  Serial.println("Get user defined states after start up:");

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
    Serial.print("deserializeJson() failed: ");
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


void registerclient () {

  String cert;

  Serial.println("Connecting to server " + BSH_ip + " ...");

  if (!client->connect(IP, 8443)) {
    Serial.println("Connection to server at port 8443 failed! Did you press the button on the smart home controller? Program is stopped");
    show_square(3, true);
    while(true) {
      delay(1000);
    }
  }
  else {
    Serial.println("Connected to server at port 8443!");
    show_square(2, true);
    delay(500);
  }

  //formate the client_cert for being sent as JSON to the BSH controller

  cert = String(client_cert);
  cert.replace("\n","");
  cert.replace("-----BEGIN CERTIFICATE-----","-----BEGIN CERTIFICATE-----\r");
  cert.replace("-----END CERTIFICATE-----","\r-----END CERTIFICATE-----");
  
  //setup the JSON

  doc["@type"] = "client";
  doc["id"] = "oss_pixel_display";
  doc["name"] = "OSS Pixel Display";
  doc["primaryRole"] = "ROLE_RESTRICTED_CLIENT";
  doc["certificate"] = cert;

  serializeJson(doc, json);

  //send header and JSON

  client->print(String("POST ") + "/smarthome/clients" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8443\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Systempassword: " + BSH_password + "\r\n" + 
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + json.length() + "\r\n" +
                 "\r\n" +
                 json + "\n");
                 
  Serial.println("Client registration posted... wait for answer");

  while (client->connected()) {
    line = client->readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  while (client->available()) {
    c = client->read();
    Serial.print(c);
  }

  Serial.println("\nCheck in BSH app if the OSS Pixel Display occurs at mobil devices. If not, check BSH_password. If yes, set register_client = false and re-upload. Program is stopped.");
  while(true) {
    delay(1000);
  }

}


// -----------------------------------------------------------------------------------------------------
// Display functions -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

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


void show_temperature (int temp, int pos_x, int pos_y) {

  pixels.setPixelColor((4+pos_y)*8+pos_x+1, pixels.Color(0, 0, 0));
  pixels.setPixelColor((4+pos_y)*8+pos_x+2, pixels.Color(0, 0, 0));
  pixels.setPixelColor((4+pos_y)*8+pos_x+3, pixels.Color(0, 0, 0));
  
  if (temp < 0) { pixels.setPixelColor((4+pos_y)*8+pos_x+1, pixels.Color(0, 80, 80)); pixels.setPixelColor((4+pos_y)*8+pos_x+2, pixels.Color(0, 80, 80)); }
  if (temp < -9)  pixels.setPixelColor((4+pos_y)*8+pos_x+3, pixels.Color(0, 0, 150));
  if (temp > 9)   pixels.setPixelColor((4+pos_y)*8+pos_x+1, pixels.Color(100, 50, 0));
  if (temp > 19)  pixels.setPixelColor((4+pos_y)*8+pos_x+2, pixels.Color(150, 50, 0));
  if (temp > 29)  pixels.setPixelColor((4+pos_y)*8+pos_x+3, pixels.Color(200, 0, 0));

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


void show_pop (float number, int pos_x, int pos_y) {

  pixels.setPixelColor((pos_y)*8+pos_x, pixels.Color(0, 0, 0));
  pixels.setPixelColor((pos_y)*8+pos_x+1, pixels.Color(0, 0, 0));
  pixels.setPixelColor((pos_y)*8+pos_x+2, pixels.Color(0, 0, 0));
  pixels.setPixelColor((pos_y)*8+pos_x+3, pixels.Color(0, 0, 0));
  
  if (number > 0.00) pixels.setPixelColor((pos_y)*8+pos_x, pixels.Color(0, 0, 100));
  if (number > 0.25) pixels.setPixelColor((pos_y)*8+pos_x+1, pixels.Color(0, 0, 140));
  if (number > 0.50) pixels.setPixelColor((pos_y)*8+pos_x+2, pixels.Color(0, 0, 200));
  if (number > 0.75) pixels.setPixelColor((pos_y)*8+pos_x+3, pixels.Color(0, 0, 255));

  pixels.show();

}


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


void show_square (int number, bool show) {
  
  uint32 color;

  if (number == 9) return;

  square current_square = square_array[number];

  if (show) color = pixels.Color(current_square.r, current_square.g, current_square.b);
  else color = pixels.Color(0,0,0);

  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+1, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+8, color);
  pixels.setPixelColor(current_square.pos_y*8+current_square.pos_x+9, color);
  pixels.show();
}


void http_get_weather() {
  
  WiFiClient client_owm;
  HTTPClient http_owm;
  String json2 = "{}"; 
  String serverPath;
  int httpResponseCode;

  serverPath = "http://api.openweathermap.org/data/2.5/forecast?q=" + OWM_city + "," + OWM_countrycode + "&cnt=1&APPID=" + OWM_key;

  http_owm.begin(client_owm, serverPath);
  httpResponseCode = http_owm.GET();
    
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code from OpenWeatherMap server: ");
    Serial.println(httpResponseCode);
    json2 = http_owm.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http_owm.end();

  DeserializationError error = deserializeJson(doc2, json2);
    
  if (error) {
    Serial.print("deserializeJson2() failed: ");
    Serial.println(error.c_str());
  }

  JsonObject list_0 = doc2["list"][0];
  JsonObject list_0_main = list_0["main"];
  JsonObject list_0_weather_0 = list_0["weather"][0];
  float main_temp = list_0_main["temp"];
  const char* weather_icon = list_0_weather_0["icon"];
  int pop = list_0["pop"];

  Serial.println("Temperature: " + String(main_temp));
  Serial.println("Weather Icon: " + String(weather_icon));
  Serial.println("Pop: " + String(pop));

  if (display_on) {
    show_temperature((main_temp-273.15), 4, 0);
    show_weather(weather_icon, 0, 0);
    show_pop(float(pop), 0, 5);
  }
}


// -----------------------------------------------------------------------------------------------------
// File system functions -------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

void writeFile(String path, String message) {
 
  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (!file.print(message)) {
    Serial.println("Write failed");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}


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
    Serial.println("Failed to open file for reading");
  }
  return log_text;
}


void appendFile(String message) {
 
  for (int i = 0; i < number_of_files; i++) {

    File file = LittleFS.open(file_path + String(i), "a");

    if (!file) {
      Serial.println("Failed to open file for append data");
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

  lastfile++;

  if (lastfile == number_of_files) lastfile = 0;
  writeFile(file_path + String(lastfile), message);
  Serial.println("File " + file_path + String(lastfile) +" overwritten");
}


void log(String color, String newentry) {
  String time;

  gmtime_r(&now, &timeinfo);

  time = String(asctime(&timeinfo));
  time = time.substring(0, time.length()-1);

  if (color == "basic_color") {
    newentry = time + "  " + newentry + "<br>";
  }
  else {
    newentry = "<font color=\"" + color + "\"> " + time + " " + newentry + "<font color=\"" + basic_color + "\"><br>";
  }
 
  appendFile(newentry); 
}


// -----------------------------------------------------------------------------------------------------
// Special functions -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------


void execute_special_functions (String name_of_function, bool execute) {

  if (name_of_function == "none") {
    // do nothing
  }

  if (name_of_function == "sonos") {
    if (execute) {
      sonos_stop("192.168.xxx.xxx");      // This function will stop a sonos speaker connected to the IP. You can copy this entry if you have more sonos speakers
    }
  }

  if (name_of_function == "display_off") {
    if (execute) {
      for (int i = 0; i < 40; i++ ) {
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
}


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
