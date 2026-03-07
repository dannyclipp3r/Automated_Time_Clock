#pragma once

#include "esp_err.h"
#include <stdbool.h>

esp_err_t clock_in_init(void);
void clock_in_handle_in(void);
void clock_in_handle_out(void);
bool clock_in_is_active(void);