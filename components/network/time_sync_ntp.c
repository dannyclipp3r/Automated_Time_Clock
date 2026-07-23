#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_sntp.h"

#include "app_config.h"
#include "time_sync_ntp.h"

static const char *TAG = "TIME_SYNC_NTP";
static bool s_time_synced = false;

// Callback function to be called when SNTP time is synchronized
static void time_sync_notification_cb(struct timeval *tv)
{
    s_time_synced = true;
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

// Function to initialize SNTP and set up time synchronization
esp_err_t time_sync_ntp_init(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");

    s_time_synced = false;

    // Configure SNTP to use the specified NTP server and set the time sync notification callback
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, NTP_SERVER);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    // Set the timezone to CST (Central Standard Time) with daylight saving time rules
    setenv("TZ", "CST6CDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();

    return ESP_OK;
}

// Function to wait for time synchronization and check if the time is valid
esp_err_t time_sync_ntp_sync(void)
{
    const int retry_count = 50;

    // Wait for the time to be synchronized, checking every second for a maximum number of retries
    for (int retry = 0; retry < retry_count; retry++) {
        // Check if the time has been synchronized
        if (s_time_synced) {
            time_t now = 0;
            struct tm timeinfo = {0};

            time(&now);
            localtime_r(&now, &timeinfo);

            if (timeinfo.tm_year >= (2026 - 1900)) {
                ESP_LOGI(TAG, "Time synchronized");
                return ESP_OK; // Time is synchronized and valid
            }
        }

        // Time not yet synchronized, wait and retry
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry + 1, retry_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    // Time synchronization failed after retries
    ESP_LOGE(TAG, "SNTP time sync failed");
    return ESP_FAIL;
}

// Function to get the current time as a formatted string
esp_err_t time_sync_ntp_get_time(char *time_str, size_t max_len)
{
    // Validate input parameters
    if (time_str == NULL || max_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get the current time and format it as a string
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);

    // Check if the time is valid (year should be at least 2026)
    if (timeinfo.tm_year < (2026 - 1900)) {
        return ESP_FAIL;
    }

    // Format the time as a string in the format "MM-DD HH:MM"
    if (strftime(time_str, max_len, "%m-%d %H:%M", &timeinfo) == 0) {
        return ESP_FAIL;
    }
    // Time string successfully formatted
    return ESP_OK;
}

// Function to get the current time information in a tm structure
esp_err_t time_sync_ntp_get_timeinfo(struct tm *timeinfo)
{
    // Validate input parameter
    if (timeinfo == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get the current time and fill the provided tm structure
    time_t now = 0;
    time(&now);
    localtime_r(&now, timeinfo);

    // Check if the time is valid (year should be at least 2026)
    if (timeinfo->tm_year < (2026 - 1900)) {
        return ESP_FAIL;
    }
    // Time information successfully retrieved
    return ESP_OK;
}