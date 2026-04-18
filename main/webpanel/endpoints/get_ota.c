#include <esp_err.h>
#include <esp_http_server.h>

#include <ota/ota.h>

static void ota_task(void* arg) {
    ledriver_ota_check_and_update();
    vTaskDelete(NULL);
}

esp_err_t httpd_get_ota_handler(httpd_req_t* req) {
    httpd_resp_set_status(req, "202 Accepted");
    httpd_resp_sendstr(req, "OTA started");
    xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
    return ESP_OK;
}