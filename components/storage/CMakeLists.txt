idf_component_register(
    SRCS "storage_log.c"
    INCLUDE_DIRS "include"
    REQUIRES spiffs
    PRIV_REQUIRES app_config)