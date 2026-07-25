idf_component_register(SRCS "clock_service.c"
                       INCLUDE_DIRS "include"
                       REQUIRES network storage 
                       PRIV_REQUIRES app_config)