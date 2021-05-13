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

void sendUpdate(const char* channel, const char* value)
{
    Serial.print(channel);
    Serial.print(": ");
    Serial.print(value);
    Serial.println();
    mqtt.publish(channel, value);
}

void setup()
{
    Adafruit_BME680 bme;
    uint32_t nextUpdate = 0;

    Serial.begin(115200);

    // Setup I2C
    Wire.begin(I2C_SDA, I2C_CLK);
    if(!bme.begin()) {
        Serial.println("ERROR: Could not find a valid BME680 sensor, check wiring!");
        ESP.deepSleep(DELAY);
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    Serial.println("BME started");

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

    if(mqtt.connect(MQTT_CLIENT_NAME)) {
        Serial.println("MQTT connected");
    } else {
        Serial.println("ERROR: MQTT is not connected.");
        ESP.deepSleep(DELAY);
    }
    mqtt.loop();

    // Start and wait for first BME reading (skipped)
    nextUpdate = bme.beginReading();
    while (millis() <= nextUpdate)
        delay(10);
    bme.endReading();
    // Second reading
    nextUpdate = bme.beginReading();
    while (millis() <= nextUpdate)
        delay(10);

    // Save data for last sensor readings
    if(bme.endReading()) {
        Serial.println("Finished BME reading");

        float temperature = bme.temperature;
        float humidity = bme.humidity;
        float pressure = bme.pressure / 100.0;
        float gasResistance = bme.gas_resistance / 1000.0;

        // Send update of values
        sendUpdate(MQTT_CHANNEL_TEMPERATURE, String(temperature).c_str());
        sendUpdate(MQTT_CHANNEL_HUMIDITY, String(humidity).c_str());
        sendUpdate(MQTT_CHANNEL_PRESSURE, String(pressure).c_str());
        sendUpdate(MQTT_CHANNEL_GAS, String(gasResistance).c_str());
    } else {
        Serial.println("ERROR: BME reading not finished.");
    }

    delay(WIFI_DELAY);
    Serial.println("Entering sleep mode");
    ESP.deepSleep(DELAY);
}

void loop() {}
