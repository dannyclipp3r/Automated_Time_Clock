// WiFi Manager Header File

#pragma once

#include "esp_err.h"
#include <stdbool.h>

// Initialize NVS + WiFi (Station mode) and connect to your network.
esp_err_t wifi_manager_init(void);

esp_err_t wifi_manager_deinit(void);

// Check if WiFi is connected
bool wifi_manager_is_connected(void);
