#include <stdbool.h>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include <unistd.h>

#include <sdkconfig.h>

#include <webpanel/resource.h>

static const char* TAG = "httpd";
static httpd_handle_t server = NULL;

esp_err_t httpd_ws_rgb_handler(httpd_req_t* req);
esp_err_t httpd_get_ota_handler(httpd_req_t* req);
esp_err_t httpd_get_resource_handler(httpd_req_t* req);

esp_err_t ledriver_start_webpanel_server(void) {
    if (server)
        return ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 8;

    esp_err_t result = httpd_start(&server, &config);
    if (result != ESP_OK)
        return result;

    const httpd_uri_t ws_rgb_uri = {.uri = "/control/rgb",
                                    .method = HTTP_GET,
                                    .handler = httpd_ws_rgb_handler,
                                    .user_ctx = NULL,
                                    .is_websocket = true};

    const httpd_uri_t ota_uri = {
        .uri = "/update", .method = HTTP_GET, .handler = httpd_get_ota_handler, .user_ctx = NULL};

    const httpd_uri_t wildcard = {
        .uri = "/*", .method = HTTP_GET, .handler = httpd_get_resource_handler, .user_ctx = NULL};

    result = httpd_register_uri_handler(server, &ws_rgb_uri);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    result = httpd_register_uri_handler(server, &ota_uri);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    result = httpd_register_uri_handler(server, &wildcard);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    ESP_LOGI(TAG, "Webpanel HTTP server started");

    return ESP_OK;
}