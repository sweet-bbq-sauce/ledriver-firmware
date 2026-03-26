#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <http_parser.h>

#include <unistd.h>

#include <sdkconfig.h>

#include <webpanel/resource.h>

static const char* TAG = "httpd";
static httpd_handle_t server = NULL;

static esp_err_t httpd_get_handler(httpd_req_t* req) {
    assert(req);

    ledriver_httpd_resource_t resource;
    esp_err_t result = ledriver_httpd_resource_get_from_uri(&resource, req->uri);
    if (result != ESP_OK)
        ESP_LOGE(TAG, "error: %s", esp_err_to_name(result));
    if (result == ESP_ERR_NOT_FOUND) {
        httpd_resp_send_404(req);
        return ESP_OK;
    } else if (result != ESP_OK) {
        httpd_resp_send_500(req);
        return result;
    }

    result = httpd_resp_set_type(req, resource.mime);
    if (result != ESP_OK) {
        ledriver_httpd_resource_free(&resource);
        return result;
    }

    if (resource.compressed) {
        result = httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        if (result != ESP_OK) {
            ledriver_httpd_resource_free(&resource);
            return result;
        }
    }

    result = httpd_resp_set_hdr(req, "Server", "LEDriver " CONFIG_APP_PROJECT_VER);
    if (result != ESP_OK) {
        ledriver_httpd_resource_free(&resource);
        return result;
    }

    char buffer[512];
    for (;;) {
        const ssize_t n = read(resource.fd, buffer, sizeof(buffer));

        if (n > 0) {
            result = httpd_resp_send_chunk(req, buffer, n);
            if (result != ESP_OK) {
                ledriver_httpd_resource_free(&resource);
                return result;
            }
        } else if (n == 0)
            break;
        else {
            ESP_LOGE(TAG, "resource read: %s", strerror(errno));
            ledriver_httpd_resource_free(&resource);
            return ESP_FAIL;
        }
    }

    ledriver_httpd_resource_free(&resource);
    return httpd_resp_send_chunk(req, NULL, 0);
}

esp_err_t ledriver_start_webpanel_server(void) {
    if (server)
        return ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 8;

    esp_err_t result = httpd_start(&server, &config);
    if (result != ESP_OK)
        return result;

    const httpd_uri_t uri = {
        .uri = "/*", .method = HTTP_GET, .handler = httpd_get_handler, .user_ctx = NULL};

    result = httpd_register_uri_handler(server, &uri);
    if (result != ESP_OK) {
        httpd_stop(server);
        server = NULL;
        return result;
    }

    ESP_LOGI(TAG, "Webpanel HTTP server started");

    return ESP_OK;
}