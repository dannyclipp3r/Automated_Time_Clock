#include "oled_i2c_config.h"
#include "app_config.h"

#include <stdio.h>
#include <string.h>

#include "ssd1306.h"

// Global handle for the OLED display
static ssd1306_handle_t g_oled = NULL;


esp_err_t display_init(i2c_master_bus_handle_t bus)
{
    // Default config shown in the library readme example
    ssd1306_config_t cfg = I2C_SSD1306_128x64_CONFIG_DEFAULT;

    // Customize the configuration as needed
    ssd1306_init(bus, &cfg, &g_oled);
    if (!g_oled) return ESP_FAIL; // If Initialization failed

    ssd1306_clear_display(g_oled, false); // Clear the display
    ssd1306_set_contrast(g_oled, 0xFF); // Set maximum contrast

    return ESP_OK;
}

void display_clear(void)
{
    if (!g_oled) return; // Ensure the display is initialized

    ssd1306_clear_display(g_oled, true); // Clear the display and refresh immediately
}

void display_show_status(const char *message)
{
    if (!g_oled) return; // Ensure the display is initialized

    ssd1306_clear_display(g_oled, false); // Clear the display before showing new message
    ssd1306_draw_string(g_oled, 0, 0, message, 12, true); // Draw the message at the top-left corner
    ssd1306_refresh(g_oled); // Refresh to show the changes
}

void display_show_time(const char *time_str)
{
    if (!g_oled) return; // Ensure the display is initialized

    ssd1306_clear_display(g_oled, false); // Clear the display before showing new time
    ssd1306_draw_string(g_oled, 0, 0, time_str, 24, true); // Draw the time string in larger font
    ssd1306_refresh(g_oled); // Refresh to show the changes
}

