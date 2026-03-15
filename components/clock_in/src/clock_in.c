// Logic for the clock in and clock out functionality.
// This is used to track the time spent on a task
// Daniel Clipper 03-07-26

#include "clock_in.h"
#include "storage_log.h"

#include "esp_log.h"
#include <time.h>

static bool g_is_clocked_in = false;
static time_t g_in_time = 0;
static time_t g_out_time = 0;
static double g_hours = 0.0;

const char *TAG = "clock_in_status";

esp_err_t time_clock_init(void){
    g_is_clocked_in = false;
    g_in_time = 0;
    g_out_time = 0;
    g_hours = 0.0;
    
    ESP_LOGI(TAG, "Clock in system initialized");
    return ESP_OK;
}

esp_err_t time_clock_clock_in(void){
    if (!g_is_clocked_in) {
        g_in_time = time(NULL);
        g_is_clocked_in = true;
        ESP_LOGI(TAG, "Clock in registered");
    }
    return ESP_OK;
}

esp_err_t time_clock_clock_out(void){
    if (g_is_clocked_in) {
        g_out_time = time(NULL);
        g_is_clocked_in = false;
        ESP_LOGI(TAG, "Clock out registered");
    }
    return ESP_OK;
}

bool time_clock_is_clocked_in(void){
    if(g_is_clocked_in){
        ESP_LOGI(TAG, "Currently clocked in");
    } else {
        ESP_LOGI(TAG, "Currently clocked out");
    }
    return g_is_clocked_in;
}

double time_clock_get_last_hours(void){
    if (g_in_time != 0 && g_out_time != 0) {
        double seconds = difftime(g_out_time, g_in_time);
        g_hours = seconds / 3600.0; // Convert seconds to hours
        ESP_LOGI(TAG, "Hours calculated: %.2f", g_hours);
    } else {
        ESP_LOGW(TAG, "Cannot calculate hours: In time or out time is not set");
    }
    return g_hours;
}