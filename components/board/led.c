#include "led.h"
#include "driver/gpio.h"
#include "app_config.h"

esp_err_t led_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CLOCK_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);

    return ESP_OK;
}

void led_on(void)
{
    gpio_set_level(CLOCK_LED_GPIO, 1);
}

void led_off(void)
{
    gpio_set_level(CLOCK_LED_GPIO, 0);
}