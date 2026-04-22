#pragma once

#include <stdint.h>

const char* ledriver_device_get_firmware_version(void);
const char* ledriver_device_get_full_firmware_version(void);

uint64_t ledriver_device_get_uptime_ms(void);

void ledriver_device_reboot(void);