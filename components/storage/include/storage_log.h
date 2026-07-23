// storage_log.h
#pragma once
#include "esp_err.h"
#include <time.h>

esp_err_t storage_log_init(void);
esp_err_t storage_log_append_shift(time_t in_time, time_t out_time, double hours_worked);
esp_err_t storage_log_read_all(char *buffer, size_t max_len);
esp_err_t storage_log_get_daily_hours(double *total_hours);
esp_err_t storage_log_get_weekly_hours(double *total_hours);