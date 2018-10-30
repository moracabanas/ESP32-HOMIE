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


#define DHTPIN 14     // what pin the DHT is connected to

// Initialize the OLED display using Wire library
SSD1306  display(0x3C, 5, 4); // (I2C Address, SCL Pin, SDA Pin)

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


const int TEMPERATURE_INTERVAL = 1;
unsigned long last_temperature_sent = 0;

const int HUMIDITY_INTERVAL = 1;
unsigned long last_humidity_sent = 0;
String display_temp;
String display_humid;

HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");

DHT dht(DHTPIN, DHTTYPE);

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

void displayData() {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, "Office Temp/Humidty");   
    display.setFont(ArialMT_Plain_24);
    display.drawString(66, 13, display_temp + "°C");
    display.drawString(66, 40, display_humid + "%");
    display.display();
}

void loopHandler() {
  getSendTemperature();
  getSendHumid(); 
  displayData();  
}

void setup() {
  Homie.setLedPin(16, LOW); //Onboard built-in LED on pin 16 --- Slow blink connecting WiFi / Fast blink connecting MQTT server / LOW (off when connected)
  Serial.begin(115200);
  Serial << endl << endl;


  Homie_setFirmware("awesome-temperature", "1.0.0");
  
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