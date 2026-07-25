idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        driver
        board
        clock_service
        network
        storage
        app_config
)