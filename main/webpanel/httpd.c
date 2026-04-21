#include <stdbool.h>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include <unistd.h>

#include <sdkconfig.h>

#include <webpanel/resource.h>

static const char* TAG = "httpd";
static httpd_handle_t server = NULL;

esp_err_t register_endpoint_config_power(httpd_handle_t server);
esp_err_t register_endpoint_config_update(httpd_handle_t server);
esp_err_t register_endpoint_control_color(httpd_handle_t server);
esp_err_t register_endpoint_control_power(httpd_handle_t server);
esp_err_t register_endpoint_static(httpd_handle_t server);

esp_err_t ledriver_start_webpanel_server(void) {
    if (server)
        return ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 8;

    esp_err_t result = httpd_start(&server, &config);
    if (result != ESP_OK)
        return result;

    result = register_endpoint_config_power(server);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    result = register_endpoint_config_update(server);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    result = register_endpoint_control_color(server);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    result = register_endpoint_control_power(server);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    result = register_endpoint_static(server);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    ESP_LOGI(TAG, "Webpanel HTTP server started");

    return ESP_OK;
}
