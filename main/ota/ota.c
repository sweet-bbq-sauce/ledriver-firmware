#include <stdbool.h>
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
#include <ota/webpanel.h>

static const char* TAG = "software update";

static esp_err_t partition_sha256_to_string(const esp_partition_t* partition, char hash[65]) {
    if (!partition)
        return ESP_ERR_INVALID_ARG;

    uint8_t sha256[32];
    const esp_err_t result = esp_partition_get_sha256(partition, sha256);
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

static esp_err_t get_running_partition_sha256(char hash[65]) {
    const esp_partition_t* running = esp_ota_get_running_partition();
    if (!running)
        return ESP_FAIL;

    return partition_sha256_to_string(running, hash);
}

static esp_err_t get_webpanel_partition_sha256(char hash[65]) {
    const esp_partition_t* webpanel = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "webpanel");
    if (!webpanel)
        return ESP_ERR_NOT_FOUND;

    return partition_sha256_to_string(webpanel, hash);
}

esp_err_t ledriver_ota_check_and_update(void) {
    ESP_LOGI(TAG, "Checking firmware update ...");
    char local_firmware_sha256[65];
    esp_err_t result = get_running_partition_sha256(local_firmware_sha256);
    if (result != ESP_OK)
        return result;

    char local_webpanel_sha256[65];
    result = get_webpanel_partition_sha256(local_webpanel_sha256);
    if (result != ESP_OK)
        return result;

    ledriver_ota_manifest_t *firmware, *webpanel;
    result = ledriver_ota_get_manifest(&firmware, &webpanel);
    if (result != ESP_OK)
        return result;

    const bool update_firmware = strcmp(local_firmware_sha256, firmware->sha256) != 0;
    const bool update_webpanel = strcmp(local_webpanel_sha256, webpanel->sha256) != 0;

    if (update_firmware) {
        ESP_LOGI(TAG, "Firmware update available. New version: '%s'.", firmware->version);

        char url[256];
        snprintf(url, sizeof(url), "http://%s:8888%s", CONFIG_APP_OTA_SERVER, firmware->path);

        const esp_http_client_config_t http_config = {.url = url, .timeout_ms = 10000};
        const esp_https_ota_config_t ota_config = {.http_config = &http_config};

        result = esp_https_ota(&ota_config);
        if (result != ESP_OK) {
            ledriver_ota_free_manifest(&firmware);
            ledriver_ota_free_manifest(&webpanel);
            return result;
        }

        if (!update_webpanel) {
            ledriver_ota_free_manifest(&firmware);
            ledriver_ota_free_manifest(&webpanel);
            ESP_LOGI(TAG, "Update done. Rebooting ...");
            esp_restart();
        }
    }

    if (update_webpanel) {
        ESP_LOGI(TAG, "Webpanel update available. New version: '%s'.", webpanel->version);
        result = update_webpanel_partition(webpanel);
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Webpanel update done. Rebooting ...");
            esp_restart();
        }

        ledriver_ota_free_manifest(&firmware);
        ledriver_ota_free_manifest(&webpanel);
        return result;
    }

    ledriver_ota_free_manifest(&firmware);
    ledriver_ota_free_manifest(&webpanel);
    return ESP_OK;
}
