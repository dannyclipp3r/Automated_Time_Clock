# ESP32-S3 Automated Time Clock

A button-operated shift tracker built with ESP-IDF in C on a Seeed Studio XIAO ESP32S3. The device synchronizes its clock over Wi-Fi, records completed shifts to flash, and displays the last shift, daily total, and weekly total on an OLED. It can also submit a scheduled weekly report through Brevo.

## How it works

| Component | Role |
| --- | --- |
| [Main loop](main/app_main.c) | Polls the button for a press and toggles between clocked in and clocked out. The LED is on during a shift. |
| [Time sync](components/time_sync_ntp/src/time_sync_ntp.c) | Uses SNTP at startup and displays local time in US Central Time with daylight saving. |
| [Shift tracking](components/clock_in/src/clock_in.c) and [storage](components/storage_log/src/storage_log.c) | Calculates elapsed hours on clock-out and appends the clock-in date, local start and end times, and decimal hours to `/spiffs/shifts.csv`. Daily and Monday–Sunday totals group each completed shift by its clock-in date. |
| [Weekly report](components/http_client/src/http_client.c) | Posts the weekly total to Brevo over HTTPS from a FreeRTOS task. The scheduler attempts one send per boot during the Saturday 9–10 p.m. Central Time window. |

Completed shifts are stored in flash. An in-progress shift and the weekly send flag live in RAM, so a reset clears them. A failed email attempt does not trigger an automatic retry during the same boot.

## Hardware

| Peripheral | Connection |
| --- | --- |
| SSD1306 128×64 I²C OLED, address `0x3C` | SDA to GPIO5, SCL to GPIO6, and common ground. Use a module compatible with 3.3 V logic. |
| Momentary pushbutton | GPIO8 to ground when pressed. The firmware enables an internal pull-up. |
| Status LED | GPIO9 through a series resistor to an LED wired to ground. The output is active high. |

Pin numbers are ESP32-S3 GPIO numbers as defined in [app_config.h](include/app_config.h).

## Build and flash

1. Install PlatformIO and open the repository root. The checked-in [PlatformIO environment](platformio.ini) targets `seeed_xiao_esp32s3` with ESP-IDF and a custom SPIFFS partition. The checked-in SDK configuration was generated with ESP-IDF 5.5.0.
2. Configure `WIFI_SSID` and `WIFI_PASS` locally in [wifi_manager.c](components/wifi_manager/src/wifi_manager.c). Startup requires Wi-Fi and a successful SNTP sync. Keep credentials out of commits.
3. To test email delivery, set the Brevo API key, verified sender, and test recipient locally in [http_client.c](components/http_client/src/http_client.c).
4. Connect the XIAO over USB and run:

   ```sh
   pio run -e seeed_xiao_esp32s3
   pio run -e seeed_xiao_esp32s3 -t upload
   pio device monitor -b 115200
   ```

## Bench test

1. Boot with the OLED attached and confirm the serial log reports SNTP sync and the display shows the local time.
2. Press and release the button. Confirm the OLED shows `CLOCKED IN` and the LED turns on. Wait a few minutes, then press again and confirm `CLOCKED OUT`, LED off, and the expected increase in the day and week totals. The display rounds to two decimal places in hours.
3. Reset after clocking out and confirm the stored day and week totals reload.
4. With Brevo configured, leave the device running during the Saturday reporting window. Check the logged HTTP status and verify the report arrives in the test inbox.

The repository contains no hardware photos, wiring diagram, automated test cases, or recorded timing and delivery measurements.

## Resume bullet

- Built an ESP32-S3 time clock in C with ESP-IDF, integrating GPIO controls, an I²C OLED, SNTP time sync, SPIFFS shift logs, and scheduled email reporting through the Brevo HTTPS API.
