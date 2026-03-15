#pragma once

// I2C Configuration (Configure as per your hardware setup)
#define I2C_PORT_NUM     0
#define I2C_SDA_GPIO     5
#define I2C_SCL_GPIO     6

#define I2C_GLITCH_CNT   7

// WiFi
#define WIFI_SSID       "your_username"
#define WIFI_PASSWORD   "your_password"

// NTP
#define NTP_SERVER      "pool.ntp.org"

// OLED
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

// GPIO
#define CLOCK_BUTTON_GPIO   8
#define CLOCK_LED_GPIO     9