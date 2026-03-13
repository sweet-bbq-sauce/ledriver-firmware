#include <stdint.h>
#include <stdlib.h>

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_log.h>

#include <cJSON.h>

#include <sdkconfig.h>

#define MAX_MANIFEST 512

esp_err_t ledriver_ota_get_manifest(cJSON** manifest) {
    *manifest = NULL;
    const esp_http_client_config_t config = {.host = CONFIG_APP_OTA_SERVER,
                                             .path = "/manifest.json",
                                             .port = 8888,
                                             .method = HTTP_METHOD_GET,
                                             .timeout_ms = 5000};

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
        return ESP_FAIL;

    esp_err_t result = esp_http_client_set_header(client, "Accept", "application/json");
    if (result != ESP_OK) {
        esp_http_client_cleanup(client);
        return result;
    }

    result = esp_http_client_open(client, 0);
    if (result != ESP_OK) {
        esp_http_client_cleanup(client);
        return result;
    }

    const int64_t content_length = esp_http_client_fetch_headers(client);
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

    if (content_length > MAX_MANIFEST) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_INVALID_SIZE;
    }

    char* content = calloc(1, content_length + 1);
    if (!content) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }

    int total_read = 0;
    while (total_read < content_length) {
        const int n =
            esp_http_client_read(client, content + total_read, content_length - total_read);
        if (n < 0) {
            free(content);
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_FAIL;
        } else if (n == 0)
            break;

        total_read += n;
    }

    if (total_read != content_length) {
        free(content);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    *manifest = cJSON_Parse(content);
    free(content);
    if (!*manifest)
        return ESP_ERR_INVALID_RESPONSE;

    return ESP_OK;
}