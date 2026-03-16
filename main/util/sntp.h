#pragma once

#include <esp_err.h>

esp_err_t ledriver_set_timezone(const char* tz);
esp_err_t ledriver_sntp_synchronize_time(void);