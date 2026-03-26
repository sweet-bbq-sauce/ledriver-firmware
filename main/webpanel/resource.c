#include <assert.h>
#include <string.h>

#include <esp_err.h>

#include <fcntl.h>
#include <unistd.h>

#include <sdkconfig.h>

#include <webpanel/resource.h>

#define WEBPANEL_ROOT "/webpanel"
#define IS_VALID_FD(fd) ((bool)(fd >= 0))

static bool has_ext_before_gz(const char* name, const char* ext) {
    assert(name);
    assert(ext);

    const size_t name_len = strlen(name);
    const size_t ext_len = strlen(ext);
    const size_t base_len = name_len - 3;

    if (name_len < 3 || strcmp(name + base_len, ".gz") != 0 || base_len < ext_len)
        return false;

    return memcmp(name + base_len - ext_len, ext, ext_len) == 0;
}

static const char* get_mime_type_from_filename(const char* filename) {
    assert(filename);

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

esp_err_t ledriver_httpd_resource_get_from_uri(ledriver_httpd_resource_t* resource,
                                               const char* uri) {
    assert(resource);
    assert(uri);

    if (!resource || !uri)
        return ESP_ERR_INVALID_ARG;

    ledriver_httpd_resource_free(resource);

    if (strcmp(uri, "/") == 0)
        uri = "/index.html";

    const size_t len = strlen(WEBPANEL_ROOT) + strlen(uri) + 4; // "/webpanel/<URI>.gz\0"
    if (len >= CONFIG_SPIFFS_OBJ_NAME_LEN)
        return ESP_ERR_INVALID_SIZE;

    char* filename = malloc(len);
    if (!filename)
        return ESP_ERR_NO_MEM;

    snprintf(filename, len, "%s%s.gz", WEBPANEL_ROOT, uri);
    const char* mime = get_mime_type_from_filename(filename);

    int fd = open(filename, O_RDONLY);
    if (IS_VALID_FD(fd)) {
        resource->fd = fd;
        resource->mime = mime;
        resource->compressed = true;

        free(filename);
        return ESP_OK;
    }

    filename[len - 4] = '\0';
    fd = open(filename, O_RDONLY);
    if (IS_VALID_FD(fd)) {
        resource->fd = fd;
        resource->mime = mime;
        resource->compressed = false;

        free(filename);
        return ESP_OK;
    }

    free(filename);
    return ESP_ERR_NOT_FOUND;
}

void ledriver_httpd_resource_free(ledriver_httpd_resource_t* resource) {
    if (!resource)
        return;

    if (IS_VALID_FD(resource->fd))
        close(resource->fd);

    resource->fd = -1;
    resource->mime = NULL;
    resource->compressed = false;
}