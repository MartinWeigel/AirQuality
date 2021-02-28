# AirQuality

Send your air quality data to a MQTT broker with an [ESP8266](https://www.espressif.com) and a [BME680](https://www.bosch-sensortec.com/bst/products/all_products/bme680). The BME680 sensor measures temperature, humitidy, air pressure, and gas concentration.

## Requirements
### Hardware
- ESP8266
- BME680

### Software
- [Arduino IDE](https://www.arduino.cc/) or [Arduino CLI](https://github.com/arduino/arduino-cli)
- Arduino library: [PubSubClient](https://github.com/knolleary/pubsubclient) for MQTT support

## Usage
1. Connect the ESP8266 with the BME680 over I2C. You can set SDA and SCL pins in `AirQuality.ino` with the defines `I2C_SDA` and `I2C_CLK`.
2. Set your WIFI credentials in `configuration.h`.
3. Set the IP and port of your MQTT broker in `configuration.h`.
4. Flash the firmware to the ESP8266.

## AirQLogger
If `UDP_ENABLE` in `AirQuality.ino` is enabled, the sensor data is sent in a specified interval to a server.
The data is serialized using [MessagePack](https://msgpack.org/) and sent as a UDP packet.
The folder `AirQLogger` contains a Python 3 example implementing a server that logs this data to a file.
This can be useful to record the BME8266 data over a longer time span for offline analysis.

## Licenses
- `AirQuality.ino` is licensed under the [ISC license](LICENSE-ISC.md).
- `bme680_defs.h` and `bme680.[c/h]` from [Bosch Sensortec GmbH](https://www.bosch-sensortec.com) include their license in their header.
- `Adafruit_BME680.[h/cpp]` from [Adafruit Industries](https://github.com/adafruit/Adafruit_BME680) are licensed under the [BSD license](https://en.wikipedia.org/wiki/BSD_licenses).
- `Adafruit_Sensor.h` from [Adafruit Industries](https://github.com/adafruit/Adafruit_Sensor) is licensed under the [Apache license 2.0](http://www.apache.org/licenses/LICENSE-2.0).
