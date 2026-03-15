#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t time_clock_init(void);
esp_err_t time_clock_clock_in(void);
esp_err_t time_clock_clock_out(void);
bool time_clock_is_clocked_in(void);
double time_clock_get_last_hours(void);