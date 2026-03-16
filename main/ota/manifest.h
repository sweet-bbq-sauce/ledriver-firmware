#pragma once

#include <esp_err.h>

typedef struct {
    char* version;
    char* sha256;
    char* path;
} ledriver_ota_manifest_t;

esp_err_t ledriver_ota_get_manifest(ledriver_ota_manifest_t** firmware,
                                    ledriver_ota_manifest_t** webpanel);
void ledriver_ota_free_manifest(ledriver_ota_manifest_t* manifest);