#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>

#include <sdkconfig.h>

#include <ota/manifest.h>

static const char* TAG = "software update";

static esp_err_t get_running_partition_sha256(char hash[65]) {
    const esp_partition_t* running = esp_ota_get_running_partition();
    if (!running)
        return ESP_FAIL;

    uint8_t sha256[32];
    const esp_err_t result = esp_partition_get_sha256(running, sha256);
    if (result != ESP_OK)
        return result;

    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        hash[i * 2] = hex[(sha256[i] >> 4) & 0xF];
        hash[i * 2 + 1] = hex[sha256[i] & 0xF];
    }
    hash[64] = '\0';

    return ESP_OK;
}

esp_err_t ledriver_ota_check_and_update(void) {
    ESP_LOGI(TAG, "Checking firmware update ...");
    char local_firmware_sha256[65];
    esp_err_t result = get_running_partition_sha256(local_firmware_sha256);
    if (result != ESP_OK)
        return result;

    ledriver_ota_manifest_t *firmware, *webpanel;
    result = ledriver_ota_get_manifest(&firmware, &webpanel);
    if (result != ESP_OK)
        return result;

    // Not used yet.
    ledriver_ota_free_manifest(&webpanel);

    if (strcmp(local_firmware_sha256, firmware->sha256) != 0) {
        ESP_LOGI(TAG, "Firmware update available. New version: '%s'.", firmware->version);

        char url[256];
        snprintf(url, sizeof(url), "http://%s:8888%s", CONFIG_APP_OTA_SERVER, firmware->path);

        ledriver_ota_free_manifest(&firmware);

        const esp_http_client_config_t http_config = {.url = url, .timeout_ms = 10000};
        const esp_https_ota_config_t ota_config = {.http_config = &http_config};

        result = esp_https_ota(&ota_config);
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Update done. Rebooting ...");
            esp_restart();
        }

        return result;
    }

    ledriver_ota_free_manifest(&firmware);
    return ESP_OK;
}