# ESP32_BitcoinMeter
Bitcoin Display for ESP32 LoLin Module

![BitcoinMeter](BitcoinMeter.jpg?raw=true "Bitcoin Meter Display")

Displays the current Bitcoin Exchange Rate from coindesk.com

Works on Wemos LoLin ESP32 board with OLED display
and can be updated over the air (ArduinoOTA)

Author: Frank Hellmann, www.vfx.to, Dec 2017

Libraries needed:
-  ESP32 dev environment, includes 
     - WiFi.h
     - WiFiClient.h
     - ESPmDNS.h
     - ArduinoOTA.h
     - Wire.h
 - ArduinoJson.h  ( from std libs )
 - SSD1306.h      ( from https://github.com/squix78/esp8266-oled-ssd1306 )

Configuration:
 - Enter your WiFi SSID und Password in sketch
 - Select the currency you would like to get displayed

I got the idea from:

https://steemit.com/making/@makerhacks/programming-a-bitcoin-price-display-bot-with-arduino-esp8266-and-tft-screen
