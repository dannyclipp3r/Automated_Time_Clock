idf_component_register(SRCS "src/clock_in.c"
                       INCLUDE_DIRS "include" "../../include"
                       REQUIRES time_sync_ntp)