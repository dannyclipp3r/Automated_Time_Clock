idf_component_register(
    SRCS "src/time_sync_ntp.c"
    INCLUDE_DIRS "include" "../../include"
    REQUIRES wifi_manager)