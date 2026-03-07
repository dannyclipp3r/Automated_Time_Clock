#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

// Function to initialize the OLED display
esp_err_t display_init(i2c_master_bus_handle_t bus);

// Function to clear the OLED display
void display_clear(void);

// Function to show status message on the OLED display
void display_show_status(const char *status);

// Function to show time on the OLED display
void display_show_time(const char *time_str);
