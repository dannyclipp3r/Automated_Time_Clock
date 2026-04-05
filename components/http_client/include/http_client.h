#pragma once

#include <stddef.h>

void send_email(const char *email_body);

void build_email_body(char *buffer, size_t len, const char *date_str, double weekly_hours);