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
#include "http_client.h"

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

static void email_task(void *pvParameters)
{
    char *email_body = (char *)pvParameters;

    ESP_LOGI("EMAIL_TASK", "Starting email task");
    ESP_LOGI("EMAIL_TASK", "Stack high water mark before send: %u bytes",
             (unsigned)uxTaskGetStackHighWaterMark(NULL));

    send_email(email_body);

    ESP_LOGI("EMAIL_TASK", "Stack high water mark after send: %u bytes",
             (unsigned)uxTaskGetStackHighWaterMark(NULL));

    free(email_body);
    vTaskDelete(NULL);
}

static bool email_sent_this_week = false;

void check_and_send_email(void)
{
    ESP_LOGI(TAG, "Checking whether weekly email should be sent");

    char email_body[512];
    char date_str[32];
    double weekly_hours = 0.0;
    struct tm timeinfo;

    if (storage_log_get_weekly_hours(&weekly_hours) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get weekly hours for email check");
        return;
    }

    if (time_sync_ntp_get_timeinfo(&timeinfo) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get time info for email check");
        return;
    }

    ESP_LOGI(TAG, "Weekly hours retrieved: %.2f", weekly_hours);
    ESP_LOGI(TAG, "Time info: wday=%d hour=%d min=%d",
             timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min);

    strftime(date_str, sizeof(date_str), "%m-%d-%Y", &timeinfo);
    build_email_body(email_body, sizeof(email_body), date_str, weekly_hours);

    ESP_LOGI(TAG, "Built email body: %s", email_body);

    if (timeinfo.tm_wday == 6 && timeinfo.tm_hour == 21) {
        if (!email_sent_this_week) {
            ESP_LOGI(TAG, "Conditions met for sending email");
            char *email_copy = malloc(strlen(email_body) + 1);
            if (email_copy == NULL) {
                ESP_LOGE(TAG, "Failed to allocate email buffer");
                return;
            }
            strcpy(email_copy, email_body);
            BaseType_t result = xTaskCreate(
                email_task,
                "email_task",
                8192,
                email_copy,
                5,
                NULL
            );
            if (result != pdPASS) {
                ESP_LOGE(TAG, "Failed to create email task");
                free(email_copy);
                return;
            }
            email_sent_this_week = true;
        } else {
            ESP_LOGI(TAG, "Email already sent for this week");
        }
    } else {
        ESP_LOGI(TAG, "Not time to send email yet");
    }

    // Reset flag after the scheduled day/time window has passed
    if (timeinfo.tm_wday != 6) {
        email_sent_this_week = false;
    }
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
        ESP_LOGE(TAG, "===== APP_MAIN BUILD MARKER 12345 =====");
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

        check_and_send_email();

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}