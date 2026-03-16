#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_partition.h>
#include <esp_spiffs.h>

#include <sdkconfig.h>

#include <ota/manifest.h>
#include <ota/webpanel.h>

#define WEBPANEL_DOWNLOAD_BUFFER_SIZE 1024

static const char* TAG = "webpanel update";

static const esp_partition_t* get_webpanel_partition(void) {
    return esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
                                    "webpanel");
}

static esp_err_t partition_sha256_to_string(const esp_partition_t* partition, char hash[65]) {
    if (!partition)
        return ESP_ERR_INVALID_ARG;

    uint8_t sha256[32];
    esp_err_t result = esp_partition_get_sha256(partition, sha256);
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

esp_err_t update_webpanel_partition(ledriver_ota_manifest_t* manifest) {
    if (!manifest || !manifest->path || !manifest->sha256)
        return ESP_ERR_INVALID_ARG;

    const esp_partition_t* partition = get_webpanel_partition();
    if (!partition)
        return ESP_ERR_NOT_FOUND;

    char url[256];
    snprintf(url, sizeof(url), "http://%s:8888%s", CONFIG_APP_OTA_SERVER, manifest->path);

    const esp_http_client_config_t config = {
        .url = url, .method = HTTP_METHOD_GET, .timeout_ms = 10000};
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
        return ESP_FAIL;

    esp_err_t result = esp_http_client_open(client, 0);
    if (result != ESP_OK) {
        esp_http_client_cleanup(client);
        return result;
    }

    int64_t content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_INVALID_RESPONSE;
    }

    const int status = esp_http_client_get_status_code(client);
    if (status != 200) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_INVALID_RESPONSE;
    }

    if ((uint64_t)content_length > partition->size) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_INVALID_SIZE;
    }

    ESP_LOGI(TAG, "Preparing webpanel partition for update (%lld bytes).", content_length);

    esp_vfs_spiffs_unregister("webpanel");

    result = esp_partition_erase_range(partition, 0, partition->size);
    if (result != ESP_OK) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return result;
    }

    uint8_t buffer[WEBPANEL_DOWNLOAD_BUFFER_SIZE];
    size_t offset = 0;
    while (offset < (size_t)content_length) {
        int bytes_to_read = sizeof(buffer);
        if ((size_t)bytes_to_read > (size_t)content_length - offset)
            bytes_to_read = (int)((size_t)content_length - offset);

        const int bytes_read = esp_http_client_read(client, (char*)buffer, bytes_to_read);
        if (bytes_read < 0) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_FAIL;
        }

        if (bytes_read == 0)
            break;

        result = esp_partition_write(partition, offset, buffer, bytes_read);
        if (result != ESP_OK) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return result;
        }

        offset += (size_t)bytes_read;
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (offset != (size_t)content_length)
        return ESP_FAIL;

    char partition_sha256[65];
    result = partition_sha256_to_string(partition, partition_sha256);
    if (result != ESP_OK)
        return result;

    if (strcmp(partition_sha256, manifest->sha256) != 0) {
        ESP_LOGE(TAG, "Webpanel hash mismatch after update.");
        return ESP_ERR_INVALID_CRC;
    }

    ESP_LOGI(TAG, "Webpanel partition updated successfully.");
    return ESP_OK;
}
