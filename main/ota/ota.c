#include <esp_err.h>
#include <esp_log.h>

#include <cJSON.h>

static const char* TAG = "software update";

esp_err_t ledriver_ota_get_manifest(cJSON** manifest);

esp_err_t ledriver_ota_check_and_update(void) {
    cJSON* manifest;
    const esp_err_t result = ledriver_ota_get_manifest(&manifest);

    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Can't download manifest: %s", esp_err_to_name(result));
        return ESP_FAIL;
    }

    if (manifest) {
        cJSON* value = cJSON_GetObjectItemCaseSensitive(manifest, "test");
        if (cJSON_IsNumber(value))
            ESP_LOGI(TAG, "\"test\" value: %i", value->valueint);
        cJSON_Delete(manifest);
    }

    return ESP_OK;
}