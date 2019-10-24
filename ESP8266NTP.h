/**
 * ESP8266NTP
 * Receives the time from an ntp-time server on an ESP8266.
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
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <WiFiUdp.h>

const int ESP8266NTP_BUFFER_SIZE = 48;

class ESP8266NTP
{
  public:
    /**
     * Initializes a ESP8266NTP object.
     */
    ESP8266NTP(void);

    /**
     * Starts the updates at a given interval.
     * This function requires a running WIFI connection!
     * @serverURL:
     *      URL to a valid NTP server at port 123.
     * @updateInterval:
     *      Interval in seconds when the time should be updated over NTP.
     */
    void begin(char* serverURL, uint32_t updateInterval);

    /**
     * Updates the time over NTP. Should be called in each loop.
     */
    void update(void);

    /**
     * Returns the current UNIX time.
     */
    uint32_t now(void);

  private:
    char* serverURL;
    uint32_t updateInterval;    // in ms
    WiFiUDP udp;
    IPAddress timeServerIP;

    unsigned char ntpBuffer[ESP8266NTP_BUFFER_SIZE];
    unsigned long lastRequestMillis;
    unsigned long lastCalculatedMillis;
    unsigned long lastReceivedMillis;
    uint32_t ntpTime;
    uint32_t unixTime;

    uint32_t parseNTP(void);
    void sendUDPRequest(void);
};
