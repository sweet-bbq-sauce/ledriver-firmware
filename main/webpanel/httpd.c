#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <http_parser.h>

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static const char* TAG = "httpd";
static httpd_handle_t server = NULL;

#define WEBPANEL_ROOT "/webpanel"

static bool has_ext_before_gz(const char* name, const char* ext) {
    const size_t name_len = strlen(name);
    const size_t ext_len = strlen(ext);
    const size_t base_len = name_len - 3;

    if (name_len < 3 || strcmp(name + base_len, ".gz") != 0 || base_len < ext_len)
        return false;

    return memcmp(name + base_len - ext_len, ext, ext_len) == 0;
}

static const char* set_content_type_from_path(const char* filename) {
    if (has_ext_before_gz(filename, ".html"))
        return "text/html; charset=utf-8";
    else if (has_ext_before_gz(filename, ".css"))
        return "text/css";
    else if (has_ext_before_gz(filename, ".js"))
        return "application/javascript";
    else if (has_ext_before_gz(filename, ".json"))
        return "application/json";
    else if (has_ext_before_gz(filename, ".png"))
        return "image/png";
    else if (has_ext_before_gz(filename, ".jpg") || has_ext_before_gz(filename, ".jpeg"))
        return "image/jpeg";
    else if (has_ext_before_gz(filename, ".svg"))
        return "image/svg+xml";
    else if (has_ext_before_gz(filename, ".txt"))
        return "text/plain; charset=utf-8";

    return "application/octet-stream";
}

typedef struct {
    FILE* fd;
    const char* mime;
    bool gzipped;
} file_source_t;

static void free_resource(file_source_t* resource) {
    if (resource->fd) {
        fclose(resource->fd);
        resource->fd = NULL;
    }
    resource->gzipped = false;
}

static esp_err_t get_resource(const char* resource, file_source_t* result) {
    assert(resource);
    assert(result);

    if (!resource)
        return ESP_ERR_INVALID_ARG;
    if (!result)
        return ESP_ERR_INVALID_ARG;

    result->fd = NULL;
    result->mime = NULL;
    result->gzipped = false;

    if (strcmp(resource, "/") == 0)
        resource = "/index.html";

    const size_t len = strlen(WEBPANEL_ROOT) + strlen(resource) + 4; // "/webpanel/<PATH>[.gz]\0"
    char* path = malloc(len);
    if (!path)
        return ESP_ERR_NO_MEM;

    snprintf(path, len, "%s%s.gz", WEBPANEL_ROOT, resource);
    const char* mime = set_content_type_from_path(path);

    FILE* fd = fopen(path, "rb");
    if (fd) {
        free(path);
        result->fd = fd;
        result->mime = mime;
        result->gzipped = true;
        return ESP_OK;
    }

    path[len - 4] = '\0';
    fd = fopen(path, "rb");
    if (fd) {
        free(path);
        result->fd = fd;
        result->mime = mime;
        result->gzipped = false;
        return ESP_OK;
    }

    free(path);
    return ESP_ERR_NOT_FOUND;
}

static esp_err_t httpd_get_handler(httpd_req_t* req) {
    assert(req);

    ESP_LOGI(TAG, "URI: %s", req->uri);

    file_source_t src;
    esp_err_t result = get_resource(req->uri, &src);
    if (result != ESP_OK)
        ESP_LOGE(TAG, "error: %s", esp_err_to_name(result));
    if (result == ESP_ERR_NOT_FOUND) {
        httpd_resp_send_404(req);
        return ESP_OK;
    } else if (result != ESP_OK) {
        httpd_resp_send_500(req);
        return result;
    }

    ESP_LOGI(TAG, "mime: %s", src.mime);
    httpd_resp_set_type(req, src.mime);

    if (src.gzipped)
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

    char buffer[512];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), src.fd)) > 0) {
        esp_err_t err = httpd_resp_send_chunk(req, buffer, n);
        if (err != ESP_OK) {
            free_resource(&src);
            return err;
        }
    }

    free_resource(&src);
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