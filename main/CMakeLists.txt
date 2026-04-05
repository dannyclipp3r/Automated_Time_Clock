idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES
        wifi_manager
        time_sync_ntp
        i2c_bus
        oled_i2c_config
        clock_in
        led
        storage_log
        http_client
)