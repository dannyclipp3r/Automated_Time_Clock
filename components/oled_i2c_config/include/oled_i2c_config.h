#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

// Function to initialize the OLED display
esp_err_t display_init(i2c_master_bus_handle_t bus);

// Function to clear the OLED display
void display_clear(void);

esp_err_t display_show_status(const char *status);

esp_err_t display_render_main_screen(const char *status, const char *time_str);

esp_err_t display_render_hours_screen(const char *status, const char *time_str,const char *hours_str, size_t hours_str_len);
