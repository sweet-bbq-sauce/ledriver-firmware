#pragma once

#include <stdbool.h>

#include <esp_err.h>

typedef struct {
    int fd;
    const char* mime;
    bool compressed;
} ledriver_httpd_resource_t;

#define LEDRIVER_RESOURCE_INIT {.fd = -1, .mime = NULL, .compressed = false}

esp_err_t ledriver_httpd_resource_get_from_uri(ledriver_httpd_resource_t* resource,
                                               const char* uri);
void ledriver_httpd_resource_free(ledriver_httpd_resource_t* resource);