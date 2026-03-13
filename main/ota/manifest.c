#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_log.h>

#include <cJSON.h>

#include <ota/manifest.h>
#include <sdkconfig.h>

#define MAX_MANIFEST 512

static ledriver_ota_manifest_t* create_manifest(const cJSON* object) {
    if (!cJSON_IsObject(object))
        return NULL;

    cJSON* version = cJSON_GetObjectItemCaseSensitive(object, "version");
    if (!cJSON_IsString(version))
        return NULL;

    ledriver_ota_manifest_t* manifest = malloc(sizeof(ledriver_ota_manifest_t));
    if (!manifest)
        return NULL;

    manifest->version = malloc(strlen(version->valuestring) + 1);
    if (!manifest->version) {
        free(manifest);
        return NULL;
    }

    strcpy(manifest->version, version->valuestring);

    return manifest;
}

esp_err_t ledriver_ota_get_manifest(ledriver_ota_manifest_t** firmware,
                                    ledriver_ota_manifest_t** webpanel) {
    *firmware = NULL;
    *webpanel = NULL;
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

    cJSON* root_object = cJSON_Parse(content);
    free(content);
    if (!root_object)
        return ESP_FAIL;

    cJSON* firmware_object = cJSON_GetObjectItemCaseSensitive(root_object, "firmware");
    cJSON* webpanel_object = cJSON_GetObjectItemCaseSensitive(root_object, "webpanel");

    *firmware = create_manifest(firmware_object);
    if (!*firmware) {
        cJSON_Delete(root_object);
        return ESP_ERR_INVALID_RESPONSE;
    }

    *webpanel = create_manifest(webpanel_object);
    if (!*webpanel) {
        cJSON_Delete(root_object);
        ledriver_ota_free_manifest(*firmware);
        *firmware = NULL;
        return ESP_ERR_INVALID_RESPONSE;
    }

    cJSON_Delete(root_object);
    return ESP_OK;
}

void ledriver_ota_free_manifest(ledriver_ota_manifest_t* manifest) {
    if (!manifest)
        return;

    if (manifest->version)
        free(manifest->version);

    free(manifest);
}