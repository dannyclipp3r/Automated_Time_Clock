// time_sync_ntp.h
// Daniel Clipper
#pragma once

#include "esp_err.h"

esp_err_t time_sync_ntp_init(void);
void time_sync_ntp_sync(void);
void time_sync_ntp_get_time(char *time_str, size_t max_len);