/**
 *  ESP Bitcoin Meter
 *
 *  Shows the current Bitcoin Exchange Rate
 *  works on Wemos LoLin ESP32 board with OLED display
 *  can be updated over the air (ArduinoOTA)
 *   
 *  Author: Frank Hellmann, www.vfx.to, Dez. 2017
 *
 *  Libraries needed:
 *  -  ESP32 dev environment, includes 
 *      - WiFi.h
 *      - WiFiClient.h
 *      - ESPmDNS.h
 *      - ArduinoOTA.h
 *      - Wire.h
 *  - ArduinoJson.h  ( from std libs )
 *  - SSD1306.h      ( from https://github.com/squix78/esp8266-oled-ssd1306 )
 *
 * Configuration:
 * - Enter your WiFi SSID und Password below
 * - Select the currency you would like to get displayed
 * 
*/
// Version Info
const char *vers = "Version 0.1";

// WiFi Includes and Configuration
#include <WiFi.h>
WiFiClient client;
const char *ssid         = "...";   // your Wifi SSID
const char *password     = "...";   // your WiFi Password
int wifi_reconnects      = 0;

// Arduino OTA Update Includes
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

// OLED Display Includes, uses SSD1306 128x64 
#include <Wire.h>
#include "SSD1306.h"
SSD1306  display(0x3c, 5, 4);         // I2C Addr, Data, Clock for LoLin Board

// BTC related stuff
#define BTCHOST "api.coindesk.com"    // Coindesk provides a nice BTC rate api
#define BTCURL "/v1/bpi/currentprice.json"
// Uncomment one of the currency combinations below
#define BTCCURRENCY "EUR"             // Valid: EUR, USD, GBP
#define BTCSTRING "Bitcoin in Euro"   // pretty currency string for OLED
//#define BTCCURRENCY "USD"             // Valid: EUR, USD, GBP
//#define BTCSTRING "Bitcoin in Dollar" // pretty currency string for OLED
//#define BTCCURRENCY "GBP"             // Valid: EUR, USD, GBP
//#define BTCSTRING "Bitcoin in Pound"  // pretty currency string for OLED
#define BTCDELAY 60000                // 60 seconds delay between updates
long lastmillis = 0;
float btcvalue = 0.0;

// Parse JSON String
#include <ArduinoJson.h>
float parseJson( String json ) {
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + 410;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject( json ); 
  JsonObject& btc = root["bpi"][BTCCURRENCY];   
  float btc_rate  = btc["rate_float"];       // returns float exchange rate
  return btc_rate; 
}

// Update Bitcoin Display
void updateBTC() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(65, 3, BTCSTRING);
  display.setFont(ArialMT_Plain_24);
  display.drawString(64, 23, String(btcvalue,2));
}

// fetch Bitcoin Exchange Rate
void fetchBTC() {
  Serial.print("connecting to ");
  Serial.println(BTCHOST);
  if (!client.connect(BTCHOST, 80)) {
    // Reconnect WiFi  (below is workaround for non-functioning WiFi.reconnect())
    Serial.println("reconnecting");
    WiFi.disconnect(true);
    WiFi.begin ( ssid, password );
    // Wait for connection
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 10 );
    }
    delay(100);
    if (!client.connect(BTCHOST, 80)) {
      Serial.println("connection failed");
      wifi_reconnects++;
      return;
    }
  }
               
  Serial.print("requesting BTC data from: ");
  Serial.println(BTCHOST);

  client.print(String("GET ") + BTCURL + " HTTP/1.1\r\n" +
               "Host: " + BTCHOST + "\r\n" +
               "User-Agent: BitcoinMeter\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String returnedjson = client.readStringUntil('\n');
 
  //  Serial.println(returnedjson);
 
  if (returnedjson.length()>10) {
    btcvalue = parseJson(returnedjson);
    Serial.print("BTC Value: ");
    Serial.println(btcvalue);
    wifi_reconnects=0;
  } else {
    Serial.println("invalid json reply received");
    wifi_reconnects++;
  }
}

void setup() {
  // open Serial for Debug
  Serial.begin(115200);
  Serial.printf("*** BitCoin Meter ***\n");
  Serial.printf("Version: ");
  Serial.printf(vers);
  Serial.printf("\nAuthor: Frank Hellmann\n\n");
 
  // Initialize OLED
  display.init();
  display.flipScreenVertically();
  display.setContrast(128);
  display.clear();

  // Startup Message
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(DISPLAY_WIDTH/2,  8, "Bitcoin Meter");
  display.drawString(DISPLAY_WIDTH/2, 20, vers);
  display.display();

  // Start WiFi  
  WiFi.begin ( ssid, password );
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 10 );
  }
  // OTA setup
  ArduinoOTA.setHostname("Bitcoin");
  ArduinoOTA.setPassword("btc123");
  ArduinoOTA.begin();

  // OTA Routinen
  ArduinoOTA.onStart([]() {       // Start of OTA Update
    // Show OTA Update Message on OLED
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "OTA Update");
    display.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Update OTA Progress on OLED
    display.drawProgressBar(4, 32, 120, 8, progress / (total / 100) );
    display.display();
  });

  ArduinoOTA.onEnd([]() {         // End of OTA Update
    display.drawString(DISPLAY_WIDTH/2, 58, "Restart");
    display.display();
    ESP.restart();
  });

  ArduinoOTA.onError([](unsigned int error) {         // Error during OTA Update
    display.clear();
    display.drawString(DISPLAY_WIDTH/2, 58, "OTA Error");
    display.display();
    delay(500);
    ESP.restart();
  });

  // Show OTA IP Address
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_10);
  display.drawString(DISPLAY_WIDTH/2, 60, "OTA IP: " + WiFi.localIP().toString());
  display.display();

  // Display BTC
  fetchBTC();
  updateBTC();
}

void loop() {
  // Update OLED with counter
  display.setColor(BLACK);
  display.fillRect(0,50,128,14);
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(16, 52, "Next Update in: ");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(110, 52, String(((lastmillis-millis()+BTCDELAY)/1000)%1000));
  display.display();

  // BTC Update
  if (millis() > lastmillis+BTCDELAY ) {
    display.setColor(BLACK);
    display.fillRect(0,50,128,14);
    display.setColor(WHITE);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 52, "Update: ");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(120, 52, String(BTCHOST));
    display.display();
    lastmillis=millis();
    fetchBTC();
    updateBTC();
  }

  // Check if WiFi connection hangs, if so reset ourself as a last measure
  if (wifi_reconnects > 6) {
    display.setColor(BLACK);
    display.fillRect(0,50,128,14);
    display.setColor(WHITE);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 52, "WiFi failed. Reset now.");
    display.display();
    ESP.restart();
  }
    
  // Over the air update 
  ArduinoOTA.handle();
}
