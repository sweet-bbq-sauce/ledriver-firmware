#pragma once

#include <esp_err.h>

#include <ota/manifest.h>

esp_err_t update_webpanel_partition(ledriver_ota_manifest_t* manifest);
