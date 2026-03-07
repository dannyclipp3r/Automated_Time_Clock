#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app_config.h"
#include "i2c_bus.h"
#include "oled_i2c_config.h"
#include "wifi_manager.h"
#include "time_sync_ntp.h"
#include "clock_in.h"

// Tag for logging
static const char *TAG = "APP";

void app_main(void)
{
    // Initialize I2C bus and error check
    i2c_bus_t bus;
    ESP_ERROR_CHECK(i2c_bus_init(&bus));

    // Initialize OLED display and error check
    ESP_ERROR_CHECK(display_init(bus.bus));

    // Initialize WiFi and error check
    ESP_ERROR_CHECK(wifi_manager_init());

    // Initialize NTP time synchronization and error check
    ESP_ERROR_CHECK(time_sync_ntp_init());

    // Initialize clock in functionality and error check
    ESP_ERROR_CHECK(clock_in_init());

    // Main loop
    while (1) {

    }


}
