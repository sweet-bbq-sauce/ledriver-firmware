#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <esp_err.h>
#include <esp_netif_sntp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

esp_err_t ledriver_set_timezone(const char* tz) {
    if (!tz)
        return ESP_ERR_INVALID_ARG;

    if (setenv("TZ", tz, 1) != 0)
        return ESP_FAIL;

    tzset();

    return ESP_OK;
}

esp_err_t ledriver_sntp_synchronize_time(void) {
    static bool sntp_inited = false;

    if (!sntp_inited) {
        const esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
        const esp_err_t result = esp_netif_sntp_init(&config);
        if (result != ESP_OK)
            return result;

        sntp_inited = true;
    }

    return esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000));
}