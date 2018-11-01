/**#include <Homie.h>

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("bare-minimum", "1.0.0"); // The underscore is not a typo! See Magic bytes
  Homie.setup();
}

void loop() {
  Homie.loop();
}*/

/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <Homie.h>
#include <DHT.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#define FW_NAME "Awesome-Temperature_Events-test"
#define FW_VERSION "1.1.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */


#define DHTPIN 14     // what pin the DHT is connected to

// Initialize the OLED display using Wire library
SSD1306  display(0x3C, 5, 4); // (I2C Address, SCL Pin, SDA Pin)

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


const int WIFI_SIGNAL_CHECK_INTERVAL = 5;
unsigned long last_signaQuality_check = 0;

const int TEMPERATURE_INTERVAL = 5;
unsigned long last_temperature_sent = 0;

const int HUMIDITY_INTERVAL = 5;
unsigned long last_humidity_sent = 0;

int8_t display_signal;
String display_temp;
String display_humid;

HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");

DHT dht(DHTPIN, DHTTYPE);


// Event handler system
void onHomieEvent(const HomieEvent& event) {
  switch (event.type) {
    case HomieEventType::STANDALONE_MODE:
      Serial << "Standalone mode started" << endl;
      break;
    case HomieEventType::CONFIGURATION_MODE:
      Serial << "Configuration mode started" << endl;
      break;
    case HomieEventType::NORMAL_MODE:
      Serial << "Normal mode started" << endl;
      break;
    case HomieEventType::OTA_STARTED:
      Serial << "OTA started" << endl;
      break;
    case HomieEventType::OTA_PROGRESS:
      Serial << "OTA progress, " << event.sizeDone << "/" << event.sizeTotal << endl;
      break;
    case HomieEventType::OTA_FAILED:
      Serial << "OTA failed" << endl;
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      Serial << "OTA successful" << endl;
      break;
    case HomieEventType::ABOUT_TO_RESET:
      Serial << "About to reset" << endl;
      break;
    case HomieEventType::WIFI_CONNECTED:
      Serial << "Wi-Fi connected, IP: " << event.ip << ", gateway: " << event.gateway << ", mask: " << event.mask << endl;
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      Serial << "Wi-Fi disconnected"<< endl;
      break;
    case HomieEventType::MQTT_READY:
      Serial << "MQTT connected" << endl;
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      Serial << "MQTT disconnected" << endl;
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      //Serial << "MQTT packet acknowledged, packetId: " << event.packetId << endl;
      break;
    case HomieEventType::READY_TO_SLEEP:
      Serial << "Ready to sleep" << endl;
      break;
  }
}

void setupHandler() {
  // Do what you want to prepare your sensor
  display.clear();
}


void getSendTemperature() {
  if (millis() - last_temperature_sent >= TEMPERATURE_INTERVAL * 1000UL || last_temperature_sent == 0) {
    float temperature = dht.readTemperature(false); // true/false --- Fahrenheit/Celius

    if (isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    display_temp = temperature;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    
    Serial.println(" °C");
    if (temperatureNode.setProperty("temperature").send(String(temperature))) {
      last_temperature_sent = millis();
    } else {
      Serial.println("Sending failed");
    }
  }
}

void getSendHumid() {
  if (millis() - last_humidity_sent >= HUMIDITY_INTERVAL * 1000UL || last_humidity_sent == 0) {
    float humidity = dht.readHumidity();

    if (isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    display_humid = humidity;
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    
    if (humidityNode.setProperty("humidity").send(String(humidity))) {
      last_humidity_sent = millis();
    } else {
      Serial.println("Sending failed");
    }
  }
}

void getWifiQuality() {
  if (millis() - last_signaQuality_check >= WIFI_SIGNAL_CHECK_INTERVAL * 1000UL || last_signaQuality_check == 0) {
    int32_t dbm = WiFi.RSSI();
    
    if(dbm <= -100) {
        display_signal = 0;
    } else if(dbm >= -50) {
        display_signal = 100;
    } else {
        display_signal = 2 * (dbm + 100);
    }
    last_signaQuality_check = millis();
  }
}

void drawWifiQuality() {
  //display.setFont(ArialMT_Plain_10);
  //display.setTextAlignment(TEXT_ALIGN_RIGHT);  
  //display.drawString(91, 2, String(display_signal) + "%");
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (display_signal > i * 25 || j == 0) {
        display.setPixel(121 + 2 * i, 7 - j);
      }
    }
  }
}

void displayData() {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    //display.setFont(ArialMT_Plain_10);
    //display.drawString(64, 0, "Office Temp/Humidty");   
    display.setFont(ArialMT_Plain_24);
    display.drawString(66, 13, display_temp + "°C");
    display.drawString(66, 40, display_humid + "%");
    drawWifiQuality();
    display.display();
}

void loopHandler() {
  getSendTemperature();
  getSendHumid(); 
  getWifiQuality();
  displayData();
}

void setup() {
  Homie.setLedPin(16, LOW); //Onboard built-in LED on pin 16 --- Slow blink connecting WiFi / Fast blink connecting MQTT server / LOW (off when connected)
  Serial.begin(115200);
  Serial << endl << endl;

  Homie.disableLogging(); //ONLY FOR TEST EVENT HANDLER
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.onEvent(onHomieEvent);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
  // Initialising the display.
  display.init();
  display.clear();
  // display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.display();
}

void loop() {
  Homie.loop();
}