idf_component_register(
    SRCS "src/led.c"
    INCLUDE_DIRS "include" "../../include"
    REQUIRES esp_driver_gpio
)