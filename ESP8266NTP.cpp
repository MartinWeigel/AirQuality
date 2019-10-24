/**
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
#include "ESP8266NTP.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#define ESP8266NTP_PORT      (123)

void ESP8266NTP::sendUDPRequest(void)
{
    // Prepare UDP package
    memset(ntpBuffer, 0, ESP8266NTP_BUFFER_SIZE);
    ntpBuffer[0] = 0b11100011;   // LI, Version, Mode

    // Request a timestamp
    udp.beginPacket(timeServerIP, ESP8266NTP_PORT);
    udp.write(ntpBuffer, ESP8266NTP_BUFFER_SIZE);
    udp.endPacket();
}

ESP8266NTP::ESP8266NTP()
{

}

void ESP8266NTP::begin(char* serverURL, uint32_t updateInterval)
{
    updateInterval = updateInterval * 1000;
    udp = WiFiUDP();
    udp.begin(ESP8266NTP_PORT);

    // Get the IP address of the NTP server
    if(!WiFi.hostByName(serverURL, timeServerIP)) {
        Serial.println("[ESP8266NTP] DNS lookup failed.");
        Serial.flush();
        while(true);
    }
    Serial.print("[ESP8266NTP] Time server IP: ");
    Serial.println(timeServerIP);
    sendUDPRequest();
}


uint32_t ESP8266NTP::parseNTP(void)
{
    if (udp.parsePacket() != 0) {
        udp.read(ntpBuffer, ESP8266NTP_BUFFER_SIZE);
        return (ntpBuffer[40] << 24) | (ntpBuffer[41] << 16) | (ntpBuffer[42] << 8) | ntpBuffer[43];
    } else
        return 0;
}

uint32_t ESP8266NTP::now(void)
{
    if(lastReceivedMillis > 0)
        return unixTime + (millis() - lastCalculatedMillis) / 1000;
    else
        return 0;
}

void ESP8266NTP::update(void)
{
    const uint32_t UNIX1970 = 2208988800UL;
    unsigned long currentMillis = millis();

    // Check if new UDP time arrived
    uint32_t t = parseNTP();
    if(t > 0) {
        lastReceivedMillis = currentMillis;
        lastCalculatedMillis = lastRequestMillis / 2 + lastReceivedMillis / 2;
        ntpTime = t;
        unixTime = t - UNIX1970;
    }

    // Autoupdate using NTP server
    if (currentMillis - lastRequestMillis >= updateInterval) {
        lastRequestMillis = currentMillis;
        sendUDPRequest();
    }
}
