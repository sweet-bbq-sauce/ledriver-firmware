#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_system.h>

static void reboot_task(void* arg) {
    vTaskDelay(pdMS_TO_TICKS(5000));

    esp_restart();
    vTaskDelete(NULL);
}

static esp_err_t device_reboot_handler_post(httpd_req_t* req) {
    httpd_resp_set_status(req, "202 Accepted");
    httpd_resp_send(req, NULL, 0);

    xTaskCreate(reboot_task, "reboot_task", 8192, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t register_endpoint_device_power(httpd_handle_t server) {
    static const httpd_uri_t uri = {.uri = "/device/reboot",
                                    .method = HTTP_POST,
                                    .handler = device_reboot_handler_post,
                                    .user_ctx = NULL};

    return httpd_register_uri_handler(server, &uri);
}