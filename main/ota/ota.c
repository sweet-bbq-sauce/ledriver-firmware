#include <esp_err.h>
#include <esp_log.h>

#include <cJSON.h>

#include <ota/manifest.h>

static const char* TAG = "software update";

esp_err_t ledriver_ota_check_and_update(void) {
    ledriver_ota_manifest_t *firmware, *webpanel;
    const esp_err_t result = ledriver_ota_get_manifest(&firmware, &webpanel);

    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Can't download manifest: %s", esp_err_to_name(result));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Firmware version: %s sha256:%s %s", firmware->version, firmware->sha256,
             firmware->path);
    ESP_LOGI(TAG, "Webpanel version: %s sha256:%s %s", webpanel->version, webpanel->sha256,
             webpanel->path);

    ledriver_ota_free_manifest(firmware);
    ledriver_ota_free_manifest(webpanel);

    return ESP_OK;
}