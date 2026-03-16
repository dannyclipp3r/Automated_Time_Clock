#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "app_config.h"
#include "i2c_bus.h"
#include "oled_i2c_config.h"
#include "wifi_manager.h"
#include "time_sync_ntp.h"
#include "clock_in.h"
#include "led.h"
#include "storage_log.h"

static const char *TAG = "APP";

static gpio_config_t button_conf = {
    .pin_bit_mask = (1ULL << CLOCK_BUTTON_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

static void update_hour_buffers(char *hours_buffer, size_t hours_len,
                                char *daily_buffer, size_t daily_len,
                                char *weekly_buffer, size_t weekly_len,
                                double last_hours)
{
    double daily_hours = 0.0;
    double weekly_hours = 0.0;

    if (storage_log_get_daily_hours(&daily_hours) != ESP_OK) {
        daily_hours = 0.0;
    }

    if (storage_log_get_weekly_hours(&weekly_hours) != ESP_OK) {
        weekly_hours = 0.0;
    }

    snprintf(hours_buffer, hours_len, "Hours: %.2f", last_hours);
    snprintf(daily_buffer, daily_len, "Day: %.2f", daily_hours);
    snprintf(weekly_buffer, weekly_len, "Week: %.2f", weekly_hours);

    ESP_LOGI(TAG, "Updated display values -> last: %.2f day: %.2f week: %.2f",
             last_hours, daily_hours, weekly_hours);
    ESP_LOGI(TAG, "%s | %s | %s", hours_buffer, daily_buffer, weekly_buffer);
}

void app_main(void)
{
    i2c_bus_t bus;
    int last_button_state = 1;
    int current_button_state = 0;

    char time_buffer[32];
    char hours_buffer[32];
    char daily_buffer[32];
    char weekly_buffer[32];

    ESP_LOGI(TAG, "Initializing I2C bus");
    ESP_ERROR_CHECK(i2c_bus_init(&bus));

    ESP_LOGI(TAG, "Initializing OLED display");
    ESP_ERROR_CHECK(display_init(bus.bus));

    ESP_LOGI(TAG, "Initializing WiFi");
    ESP_ERROR_CHECK(wifi_manager_init());

    ESP_LOGI(TAG, "Initializing SNTP");
    ESP_ERROR_CHECK(time_sync_ntp_init());

    ESP_LOGI(TAG, "Waiting for SNTP sync");
    ESP_ERROR_CHECK(time_sync_ntp_sync());

    ESP_LOGI(TAG, "Initializing time clock");
    ESP_ERROR_CHECK(time_clock_init());

    ESP_LOGI(TAG, "Initializing LED");
    ESP_ERROR_CHECK(led_init());

    ESP_LOGI(TAG, "Initializing storage log");
    ESP_ERROR_CHECK(storage_log_init());

    ESP_LOGI(TAG, "Configuring button GPIO");
    ESP_ERROR_CHECK(gpio_config(&button_conf));

    display_show_status("System Ready");

    // Boot-time values from stored log
    update_hour_buffers(hours_buffer, sizeof(hours_buffer),
                        daily_buffer, sizeof(daily_buffer),
                        weekly_buffer, sizeof(weekly_buffer),
                        0.0);

    while (1) {
        current_button_state = gpio_get_level(CLOCK_BUTTON_GPIO);

        if (current_button_state == 0 && last_button_state == 1) {
            if (!time_clock_is_clocked_in()) {
                ESP_LOGI(TAG, "Clocking in");
                ESP_ERROR_CHECK(time_clock_clock_in());
                led_on();
            } else {
                double last_hours = 0.0;

                ESP_LOGI(TAG, "Clocking out");
                ESP_ERROR_CHECK(time_clock_clock_out());
                led_off();

                last_hours = time_clock_get_last_hours();

                update_hour_buffers(hours_buffer, sizeof(hours_buffer),
                                    daily_buffer, sizeof(daily_buffer),
                                    weekly_buffer, sizeof(weekly_buffer),
                                    last_hours);
            }
        }

        last_button_state = current_button_state;

        if (time_sync_ntp_get_time(time_buffer, sizeof(time_buffer)) == ESP_OK) {
            if (time_clock_is_clocked_in()) {
                ESP_ERROR_CHECK(display_render_main_screen("CLOCKED IN", time_buffer));
            } else {
                ESP_ERROR_CHECK(display_render_hours_screen(
                    "CLOCKED OUT",
                    time_buffer,
                    hours_buffer,
                    daily_buffer,
                    weekly_buffer
                ));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}