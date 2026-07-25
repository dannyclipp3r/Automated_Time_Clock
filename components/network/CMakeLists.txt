idf_component_register(
    SRCS "wifi_manager.c"
    SRCS "http_client.c"
    SRCS "time_sync_ntp.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_event esp_wifi esp_netif nvs_flash esp_http_client
    PRIV_REQUIRES app_config mbedtls
)