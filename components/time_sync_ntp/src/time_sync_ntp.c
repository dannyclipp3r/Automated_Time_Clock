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

static void time_sync_notification_cb(struct timeval *tv)
{
    s_time_synced = true;
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

esp_err_t time_sync_ntp_init(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");

    s_time_synced = false;

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, NTP_SERVER);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    // Central Time with DST
    setenv("TZ", "CST6CDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();

    return ESP_OK;
}

esp_err_t time_sync_ntp_sync(void)
{
    const int retry_count = 10;

    for (int retry = 0; retry < retry_count; retry++) {
        if (s_time_synced) {
            time_t now = 0;
            struct tm timeinfo = {0};

            time(&now);
            localtime_r(&now, &timeinfo);

            if (timeinfo.tm_year >= (2024 - 1900)) {
                ESP_LOGI(TAG, "Time synchronized");
                return ESP_OK;
            }
        }

        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry + 1, retry_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGE(TAG, "SNTP time sync failed");
    return ESP_FAIL;
}

esp_err_t time_sync_ntp_get_time(char *time_str, size_t max_len)
{
    if (time_str == NULL || max_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    time_t now = 0;
    struct tm timeinfo = {0};

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2024 - 1900)) {
        return ESP_FAIL;
    }

    if (strftime(time_str, max_len, "%m-%d %H:%M", &timeinfo) == 0) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t time_sync_ntp_get_timeinfo(struct tm *timeinfo)
{
    if (timeinfo == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    time_t now = 0;
    time(&now);
    localtime_r(&now, timeinfo);

    if (timeinfo->tm_year < (2024 - 1900)) {
        return ESP_FAIL;
    }

    return ESP_OK;
}