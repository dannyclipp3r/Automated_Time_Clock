#include "i2c_bus.h"
#include "app_config.h"


// Initialize the I2C bus with specified configuration
esp_err_t i2c_bus_init(i2c_bus_t *out)
{
    // Validate output pointer
    if (!out) return ESP_ERR_INVALID_ARG;

    // Configure I2C master bus settings
    i2c_master_bus_config_t cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .glitch_ignore_cnt = I2C_GLITCH_CNT,
        .flags.enable_internal_pullup = true,
    };

    // Create new I2C master bus and return the result
    return i2c_new_master_bus(&cfg, &out->bus);
}
