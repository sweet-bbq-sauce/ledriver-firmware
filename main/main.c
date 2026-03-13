#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <sdkconfig.h>

static const char* TAG = "main";

esp_err_t connect_to_wifi(const char* ssid, const char* password, int retry_num);
esp_err_t ledriver_ota_check_and_update(void);

void app_main(void) {
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    result = connect_to_wifi(CONFIG_APP_WIFI_SSID, CONFIG_APP_WIFI_PASSWORD, 2);
    if (result != ESP_OK)
        ESP_LOGE(TAG, "connect_to_wifi: %s", esp_err_to_name(result));

    if (result == ESP_OK) {
        result = ledriver_ota_check_and_update();
        if (result != ESP_OK)
            ESP_LOGE(TAG, "ledriver_ota_check_and_update: %s", esp_err_to_name(result));
    }
}
