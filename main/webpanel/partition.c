#include <stdbool.h>

#include <esp_err.h>
#include <esp_partition.h>
#include <esp_spiffs.h>

static bool is_mounted = false;

esp_err_t ledriver_mount_webpanel_partition(void) {
    if (is_mounted)
        return ESP_OK;

    const esp_vfs_spiffs_conf_t conf = {
        .base_path = "/webpanel",
        .partition_label = "webpanel",
        .max_files = 8,
        .format_if_mount_failed = false
    };

    const esp_err_t result = esp_vfs_spiffs_register(&conf);
    if (result != ESP_OK)
        return result;

    is_mounted = true;
    return ESP_OK;
}

