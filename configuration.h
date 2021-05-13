#pragma once

// Approximate publishing delay in ns (1min = 60000)
#define DELAY      (60e6)

// Delay to wait for wifi sending MQTT data (in ms)
#define WIFI_DELAY (100)

// WIFI configuration
#define WIFI_SSID  ("YOUR_SSID")
#define WIFI_PWD   ("YOUR_PASSWORD")

// MQTT configuration
#define MQTT_BROKER_IP            ("192.168.178.28")
#define MQTT_BROKER_PORT          (1883)
#define MQTT_CLIENT_NAME          ("AirQ-01")
#define MQTT_CHANNEL_TEMPERATURE  ("AirQ-01/Temperature")
#define MQTT_CHANNEL_HUMIDITY     ("AirQ-01/Humidity")
#define MQTT_CHANNEL_PRESSURE     ("AirQ-01/Pressure")
#define MQTT_CHANNEL_GAS          ("AirQ-01/GasResistance")

// BME680 pins
#define I2C_SDA      (4)
#define I2C_CLK      (0)
