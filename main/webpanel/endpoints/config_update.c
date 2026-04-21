#include <esp_err.h>
#include <esp_http_server.h>

#include <ota/ota.h>

static void ota_task(void* arg) {
    ledriver_ota_check_and_update();
    vTaskDelete(NULL);
}

static esp_err_t config_update_handler_post(httpd_req_t* req) {
    httpd_resp_set_status(req, "202 Accepted");
    httpd_resp_send(req, NULL, 0);
    xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t register_endpoint_config_update(httpd_handle_t server) {
    static const httpd_uri_t uri = {.uri = "/config/update",
                                    .method = HTTP_POST,
                                    .handler = config_update_handler_post,
                                    .user_ctx = NULL};

    return httpd_register_uri_handler(server, &uri);
}