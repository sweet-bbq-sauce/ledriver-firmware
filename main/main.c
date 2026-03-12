#include <stdio.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <nvs_flash.h>

#include <sdkconfig.h>

static const char* TAG = "main";

esp_err_t connect_to_wifi(const char* ssid, const char* password, int retry_num);

static void mount_spiffs(void) {
    const esp_vfs_spiffs_conf_t conf = {
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

    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    mount_spiffs();

    result = connect_to_wifi(CONFIG_APP_WIFI_SSID, CONFIG_APP_WIFI_PASSWORD, 2);
    if (result != ESP_OK)
        ESP_LOGE("main", "connect_to_wifi: %s", esp_err_to_name(result));
}
