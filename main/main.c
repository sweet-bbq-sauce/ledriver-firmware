#include <stdio.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_spiffs.h>

static const char* TAG = "main";

static void mount_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "webpanel",
        .max_files = 8,
        .format_if_mount_failed = false,
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(err));
        return;
    }

    size_t total = 0;
    size_t used = 0;
    err = esp_spiffs_info(conf.partition_label, &total, &used);
    if (err == ESP_OK)
        ESP_LOGI(TAG, "SPIFFS mounted: /spiffs, used %u / %u bytes", (unsigned)used,
                 (unsigned)total);
    else
        ESP_LOGW(TAG, "Mounted, but failed to read SPIFFS info: %s", esp_err_to_name(err));
}

void app_main(void) {
    mount_spiffs();
    printf("Hello world!\n");
}
