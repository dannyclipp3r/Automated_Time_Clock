#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "app_config.h"

typedef struct {
    gpio_num_t gpio_num;
    uint8_t active_level;
} led_t;

esp_err_t led_init(void);

void led_on(void);

void led_off(void);