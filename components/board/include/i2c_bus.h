#pragma once
#include "esp_err.h"
#include "driver/i2c_master.h"

// I2C bus structure encapsulating the master bus handle
typedef struct {
    i2c_master_bus_handle_t bus;
} i2c_bus_t;

// Function to initialize the I2C bus
esp_err_t i2c_bus_init(i2c_bus_t *out);
