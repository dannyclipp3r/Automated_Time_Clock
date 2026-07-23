// storage_log.c
// Daniel Clipper

#include "storage_log.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "storage_log";
static const char *LOG_PATH = "/spiffs/shifts.csv";

static bool parse_shift_line(const char *line, int *year, int *month, int *day, double *hours)
{
    char in_buf[16];
    char out_buf[16];

    int matched = sscanf(line, "%d-%d-%d,%15[^,],%15[^,],%lf",
                         year, month, day, in_buf, out_buf, hours);

    return (matched == 6);
}

esp_err_t storage_log_init(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 5,
        .format_if_mount_failed = true // dangerous, good for development but consider false for production to avoid data loss
    };

    // Register and mount SPIFFS
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SPIFFS");
        return err;
    }

    // Logs SPIFFS total and used space for debugging
    size_t total = 0, used = 0;
    err = esp_spiffs_info(NULL, &total, &used);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS total=%u used=%u", (unsigned)total, (unsigned)used);
    }

    // Checks if log file exists
    FILE *f = fopen(LOG_PATH, "r");
    if (f) {
        ESP_LOGI(TAG, "Existing shift log found: %s", LOG_PATH);
        fclose(f);
    } else {
        ESP_LOGW(TAG, "Shift log file not found: %s", LOG_PATH);
    }

    return ESP_OK;
}

esp_err_t storage_log_append_shift(time_t in_time, time_t out_time, double hours_worked)
{
    // Opens the log file for appending
    FILE *f = fopen(LOG_PATH, "a");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open log file for append");
        return ESP_FAIL;
    }

    // Converts timestamps to readable format
    struct tm in_tm = {0};
    struct tm out_tm = {0};
    localtime_r(&in_time, &in_tm);
    localtime_r(&out_time, &out_tm);

    // Buffers for formatted date and time strings
    char date_buf[16];
    char in_buf[16];
    char out_buf[16];

    // Formats the date and time for logging
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &in_tm);
    strftime(in_buf, sizeof(in_buf), "%H:%M:%S", &in_tm);
    strftime(out_buf, sizeof(out_buf), "%H:%M:%S", &out_tm);

    // Writes the shift entry to the log file in CSV format
    fprintf(f, "%s,%s,%s,%.2f\n", date_buf, in_buf, out_buf, hours_worked);

    fclose(f);      // Closes the log file after reading
    return ESP_OK;  // Returns success after appending the shift entry into the log file
}

esp_err_t storage_log_read_all(char *buffer, size_t max_len)
{
    // Validates input parameters
    if (!buffer || max_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Opens the log file for reading
    FILE *f = fopen(LOG_PATH, "r");
    if (!f) {
        return ESP_FAIL;
    }

    // Reads the entire log file into the provided buffer with a null terminator
    size_t used = fread(buffer, 1, max_len - 1, f);
    buffer[used] = '\0';

    fclose(f);      // Closes the log file after reading
    return ESP_OK;  // Returns success after reading the log contents into the buffer
}

esp_err_t storage_log_get_daily_hours(double *total_hours)
{
    // Validates input parameter
    if (!total_hours) {
        return ESP_ERR_INVALID_ARG;
    }

    *total_hours = 0.0;

    // Opens the log file for reading
    FILE *f = fopen(LOG_PATH, "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open log file: %s", LOG_PATH);
        return ESP_FAIL;
    }

    // Gets and formats the current date
    time_t now = time(NULL);
    struct tm today_tm = {0};
    localtime_r(&now, &today_tm);

    // Extracts the current year, month, and day for comparison
    int today_year = today_tm.tm_year + 1900;
    int today_month = today_tm.tm_mon + 1;
    int today_day = today_tm.tm_mday;

    ESP_LOGI(TAG, "Today is %04d-%02d-%02d", today_year, today_month, today_day);

    char line[128];

    // Reads the log file line by line and sums hours for entries matching today's date
    while (fgets(line, sizeof(line), f)) {
        int year, month, day;
        double hours;

        if (parse_shift_line(line, &year, &month, &day, &hours)) {
            ESP_LOGI(TAG, "Daily parsed row: %04d-%02d-%02d -> %.2f",
                     year, month, day, hours);

            if (year == today_year && month == today_month && day == today_day) {
                *total_hours += hours;
                ESP_LOGI(TAG, "Daily match, running total = %.2f", *total_hours);
            }
        }
    }

    fclose(f);      // Closes the log file after reading
    return ESP_OK;  // Returns success after calculating daily hours
}

esp_err_t storage_log_get_weekly_hours(double *total_hours)
{
    // Validates input parameter
    if (!total_hours) {
        return ESP_ERR_INVALID_ARG;
    }

    *total_hours = 0.0;

    // Opens the log file for reading
    FILE *f = fopen(LOG_PATH, "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open log file: %s", LOG_PATH);
        return ESP_FAIL;
    }

    // Gets and formats the current date to determine the start and end of the current week
    time_t now = time(NULL);
    struct tm now_tm = {0};
    localtime_r(&now, &now_tm);

    // Calculates how many days have passed since Monday (tm_wday: Sunday=0, Monday=1, ..., Saturday=6)
    int days_since_monday = (now_tm.tm_wday + 6) % 7;

    // Determines the start of the week (Monday at 00:00:00) and the end of the week (next Monday at 00:00:00)
    struct tm week_start_tm = now_tm;
    week_start_tm.tm_hour = 0;
    week_start_tm.tm_min = 0;
    week_start_tm.tm_sec = 0;
    week_start_tm.tm_mday -= days_since_monday;

    time_t week_start = mktime(&week_start_tm);

    // Calculates the end of the week by adding 7 days to the start of the week
    struct tm week_end_tm = week_start_tm;
    week_end_tm.tm_mday += 7;
    time_t week_end = mktime(&week_end_tm);

    // Logs the calculated week start and end timestamps for debugging
    ESP_LOGI(TAG, "Week start ts=%lld end ts=%lld",
             (long long)week_start, (long long)week_end);

    char line[128];

    // Reads the log file line by line and sums hours that fall within the current week
    while (fgets(line, sizeof(line), f)) {
        int year, month, day;
        double hours;

        if (parse_shift_line(line, &year, &month, &day, &hours)) {
            struct tm entry_tm = {0};
            entry_tm.tm_year = year - 1900;
            entry_tm.tm_mon = month - 1;
            entry_tm.tm_mday = day;
            entry_tm.tm_hour = 12;

            time_t entry_time = mktime(&entry_tm);

            ESP_LOGI(TAG, "Weekly parsed row: %04d-%02d-%02d -> %.2f (ts=%lld)",
                     year, month, day, hours, (long long)entry_time);

            if (entry_time >= week_start && entry_time < week_end) {
                *total_hours += hours;
                ESP_LOGI(TAG, "Weekly match, running total = %.2f", *total_hours);
            }
        }
    }

    fclose(f);      // Closes the log file after reading
    return ESP_OK;  // Returns success after calculating weekly hours
}