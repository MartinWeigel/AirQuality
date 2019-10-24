/**
 * AirQuality.ino
 *
 * Creates a webserver to display the current data of the BME680.
 *
 * Copyright (C) 2019 Martin Weigel <mail@MartinWeigel.com>
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
#include <Arduino.h>
#include <Wire.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "Adafruit_BME680.h"
#include "ESP8266NTP.h"

#include "static/index.html.h"
#include "static/favicon.png.h"
#include "static/manifest.webmanifest.h"

#include "wifi_credentials.h"

#define SERVER_PORT  (80)
#define SOCKET_PORT  (81)
#define I2C_SDA      (4)
#define I2C_CLK      (0)

#define TEMP_REGRESSION_M (-4.9776)
#define TEMP_REGRESSION_X ( 0.9167)

typedef struct SensorData
{
    uint32_t unixTime;
    float temperature;
    float pressure;
    float humidity;
    float gasResistance;
} SensorData;

static char buffer[1024];       // Buffer for string creation
Adafruit_BME680 bme;
ESP8266WebServer server(SERVER_PORT);
WebSocketsServer websocket(SOCKET_PORT);
ESP8266NTP ntp;
SensorData sensordata;
uint32_t sensorReadingDone = 0; // Time in millis when the BME680 is ready

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght)
{
  switch (type) {
    case WStype_DISCONNECTED:
        Serial.print("[WebSocket] Disconnected client ");
        Serial.println(num);
        break;
    case WStype_CONNECTED: {
            IPAddress ip = websocket.remoteIP(num);
            Serial.print("[WebSocket] Connected client ");
            Serial.print(num);
            Serial.print(" from ");
            Serial.print(ip[0]);
            Serial.print(".");
            Serial.print(ip[1]);
            Serial.print(".");
            Serial.print(ip[2]);
            Serial.print(".");
            Serial.print(ip[3]);
            Serial.println();
        }
        break;
    case WStype_TEXT:
    break;
  }
}

void sensordataToJSON(SensorData* data, char* buffer, size_t bufferSize)
{
    // Make JSON string from sensor data
    snprintf(buffer, bufferSize,
        "{"
        "\"time\":%d,"
        "\"temperature\":%f,"
        "\"humidity\":%f,"
        "\"pressure\":%f,"
        "\"gas\":%f"
        "}",
        data->unixTime,
        data->temperature,
        data->humidity,
        data->pressure,
        data->gasResistance);
}

void serveData()
{
    sensordataToJSON(&sensordata, buffer, sizeof(buffer));
    server.send(200, "text/json", buffer);
}

void dataUpdateOverWebsocket()
{
    sensordataToJSON(&sensordata, buffer, sizeof(buffer));
    websocket.broadcastTXT(buffer, strlen(buffer));
}

void setup()
{
    Serial.begin(115200);

    // Connect to WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    server.begin();
    Serial.print("Server started ");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(SERVER_PORT);

    // SETUP I2C
    Wire.begin(I2C_SDA, I2C_CLK);
    if(!bme.begin()) {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        while (1);
    }
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    // SETUP SERVER ROUTES
    server.on("/", []() {
        server.send_P(200, "text/html", (const char*)index_html, index_html_len);
    });
    server.on("/favicon.png", []() {
        server.send_P(200, "image/png", (const char*)favicon_png, favicon_png_len);
    });
    server.on("/manifest.webmanifest", []() {
        server.send_P(200, "application/manifest+json", (const char*)manifest_webmanifest, manifest_webmanifest_len);
    });
    server.on("/data", serveData);
    server.begin();

    // Start first sensor reading
    sensorReadingDone = bme.beginReading();

    // Start websockets
    websocket.begin();
    websocket.onEvent(webSocketEvent);

    ntp.begin("0.europe.pool.ntp.org", 300);
}

void loop()
{
    uint32_t currentMillis = millis();

    // Update the time
    ntp.update();
    uint32_t unix = ntp.now();

    // Read sensor data
    if(currentMillis >= sensorReadingDone) {
        // Save data for last sensor readings
        bme.endReading();
        sensordata.unixTime = unix;
        sensordata.temperature = TEMP_REGRESSION_X * bme.temperature + TEMP_REGRESSION_M;
        sensordata.humidity = bme.humidity;
        sensordata.pressure = bme.pressure / 100.0;
        sensordata.gasResistance = bme.gas_resistance / 1000.0;

        // Publish data
        dataUpdateOverWebsocket();

        // Start new sensing cycle
        sensorReadingDone = bme.beginReading();
    }

    // Serve data
    websocket.loop();
    server.handleClient();
}
