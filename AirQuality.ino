/**
 * AirQuality.ino
 *
 * Send the data of a BME680 over MQTT.
 *
 * Copyright (C) 2021 Martin Weigel <mail@MartinWeigel.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_BME680.h"

#include "configuration.h"

WiFiClient wifi;
PubSubClient mqtt(wifi);

Adafruit_BME680 bme;
bool isSensorRunning = false;
uint32_t nextUpdate = 0;

void setup()
{
    Serial.begin(115200);

    // WIFI setup
    Serial.print("Connecting to wifi (");
    Serial.print(WIFI_SSID);
    Serial.print(")...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.print(" OK! IP: ");
    Serial.println(WiFi.localIP());

    // MQTT setup
    mqtt.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);

    // Setup I2C
    Wire.begin(I2C_SDA, I2C_CLK);
    if(!bme.begin()) {
        Serial.println("ERROR: Could not find a valid BME680 sensor, check wiring!");
        while (1);
    }
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    // Start first sensor reading
    nextUpdate = bme.beginReading();
    isSensorRunning = true;
}

void loop()
{
    uint32_t currentMillis = millis();

    // Ensure connection over mqtt
    while (!mqtt.connected()) {
        Serial.print("Connecting to mqtt... ");
        if (mqtt.connect(MQTT_CLIENT_NAME)) {
            Serial.println("OK");
        } else {
            Serial.print("FAILED (state=");
            Serial.print(mqtt.state());
            Serial.println(") try again in 5 seconds");
            delay(5000);
        }
    }
    mqtt.loop();

    // Only update after delay
    if(currentMillis >= nextUpdate) {
        if(isSensorRunning) {
            // Save data for last sensor readings
            bme.endReading();
            float temperature = bme.temperature;
            float humidity = bme.humidity;
            float pressure = bme.pressure / 100.0;
            float gasResistance = bme.gas_resistance / 1000.0;

            // Publish sensor data over mqtt
            mqtt.publish(MQTT_CHANNEL_TEMPERATURE, String(temperature).c_str());
            mqtt.publish(MQTT_CHANNEL_HUMIDITY, String(humidity).c_str());
            mqtt.publish(MQTT_CHANNEL_PRESSURE, String(pressure).c_str());
            mqtt.publish(MQTT_CHANNEL_GAS, String(gasResistance).c_str());

            nextUpdate = currentMillis + DELAY;
            isSensorRunning = false;
        } else {
            // Time for new sensor reading
            nextUpdate = bme.beginReading();
            isSensorRunning = true;
        }
    }
}
