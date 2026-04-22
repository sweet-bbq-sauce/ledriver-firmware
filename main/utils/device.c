#include <stdint.h>

#include <esp_system.h>
#include <esp_timer.h>

#include <sdkconfig.h>
#include <utils/device.h>

const char* ledriver_device_get_firmware_version(void) { return CONFIG_APP_PROJECT_VER; }

const char* ledriver_device_get_full_firmware_version(void) {
    return "LEDriver v" CONFIG_APP_PROJECT_VER;
}

uint64_t ledriver_device_get_uptime_ms(void) { return esp_timer_get_time(); }

void ledriver_device_reboot(void) { esp_restart(); }