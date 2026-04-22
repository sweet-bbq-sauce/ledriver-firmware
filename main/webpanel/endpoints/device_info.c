#include <inttypes.h>
#include <stdio.h>

#include <esp_err.h>
#include <esp_http_server.h>

#include <sdkconfig.h>
#include <utils/device.h>

static esp_err_t device_version_handler_get(httpd_req_t* req) {
    char response[128];
    const int length =
        snprintf(response, sizeof(response), "{\"firmware\":\"%s\", \"idf\":\"%s\"}",
                 ledriver_device_get_full_firmware_version(), CONFIG_IDF_INIT_VERSION);

    if (length < 0 || length >= sizeof(response))
        return ESP_FAIL;

    const esp_err_t result = httpd_resp_set_type(req, "application/json");
    if (result != ESP_OK)
        return result;

    return httpd_resp_send(req, response, length);
}

static esp_err_t device_uptime_handler_get(httpd_req_t* req) {
    char response[32];
    const int length = snprintf(response, sizeof(response), "{\"seconds\":%" PRIu64 "}",
                                ledriver_device_get_uptime_ms() / 1000 / 1000);

    if (length < 0 || length >= sizeof(response))
        return ESP_FAIL;

    const esp_err_t result = httpd_resp_set_type(req, "application/json");
    if (result != ESP_OK)
        return result;

    return httpd_resp_send(req, response, length);
}

esp_err_t register_endpoint_device_info(httpd_handle_t server) {
    static const httpd_uri_t uri1 = {.uri = "/device/version",
                                     .method = HTTP_GET,
                                     .handler = device_version_handler_get,
                                     .user_ctx = NULL};
    static const httpd_uri_t uri2 = {.uri = "/device/uptime",
                                     .method = HTTP_GET,
                                     .handler = device_uptime_handler_get,
                                     .user_ctx = NULL};

    const esp_err_t result = httpd_register_uri_handler(server, &uri1);
    if (result != ESP_OK)
        return result;

    return httpd_register_uri_handler(server, &uri2);
}