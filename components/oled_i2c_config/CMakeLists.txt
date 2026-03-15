idf_component_register(SRCS "src/oled_i2c_config.c"
                       INCLUDE_DIRS "include" "../../include"
                       REQUIRES i2c_bus ssd1306)