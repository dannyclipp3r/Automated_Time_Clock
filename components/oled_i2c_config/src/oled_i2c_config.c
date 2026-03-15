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
    ssd1306_config_t cfg = SSD1306_128x64_CONFIG_DEFAULT;

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

esp_err_t display_show_status(const char *status)
{
    if (!g_oled) return ESP_FAIL; // Ensure the display is initialized

    ssd1306_clear_display(g_oled, false); // Clear the display before rendering
    ssd1306_display_text(g_oled, 0, status, false); // Display status on the first page

    return ESP_OK;
}

esp_err_t display_render_main_screen(const char *status, const char *time_str)
{
    if (!g_oled) return ESP_FAIL; // Ensure the display is initialized

    ssd1306_clear_display(g_oled, false); // Clear the display before rendering

    // Display status on the first page
    ssd1306_display_text(g_oled, 0, status, false);

    // Display time on the second page
    ssd1306_display_text(g_oled, 2, time_str, false);

    return ESP_OK;
}

esp_err_t display_render_hours_screen(const char *status, const char *time_str,const char *hours_str, size_t hours_str_len)
{
    if (!g_oled) return ESP_FAIL; // Ensure the display is initialized

    ssd1306_clear_display(g_oled, false); // Clear the display before rendering

        // Display status on the first page
    ssd1306_display_text(g_oled, 0, status, false);

    // Display time on the second page
    ssd1306_display_text(g_oled, 2, time_str, false);

    // Display hours on the third page
    ssd1306_display_text(g_oled, 4, hours_str, false);

    return ESP_OK;
}