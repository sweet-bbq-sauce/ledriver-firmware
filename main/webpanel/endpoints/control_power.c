#include <stdio.h>

#include <esp_err.h>
#include <esp_http_server.h>

#include <cJSON.h>

#include <ledc/ledc.h>

#define REQUEST_BODY_MAX_LENGTH 24

static esp_err_t control_power_handler_get(httpd_req_t* req) {
    bool power;
    esp_err_t result = ledriver_ledc_get_power(&power);
    if (result != ESP_OK) {
        httpd_resp_send_500(req);
        return result;
    }

    char response[24];
    const int length =
        snprintf(response, sizeof(response), "{\"on\":%s}", power ? "true" : "false");

    if (length < 0 || length >= sizeof(response))
        return ESP_FAIL;

    result = httpd_resp_set_type(req, "application/json");
    if (result != ESP_OK)
        return result;

    return httpd_resp_send(req, response, length);
}

static esp_err_t control_power_handler_put(httpd_req_t* req) {
    if (req->content_len == 0 || req->content_len >= REQUEST_BODY_MAX_LENGTH) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request body");
        return ESP_ERR_INVALID_SIZE;
    }

    char body[REQUEST_BODY_MAX_LENGTH] = {};
    const int n = httpd_req_recv(req, body, req->content_len);
    if (n <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    cJSON* json = cJSON_ParseWithLength(body, n);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_ERR_INVALID_ARG;
    }

    const cJSON* power_json = cJSON_GetObjectItemCaseSensitive(json, "on");
    if (!cJSON_IsBool(power_json)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_ERR_INVALID_ARG;
    }

    const bool power = cJSON_IsTrue(power_json);
    cJSON_Delete(json);

    const esp_err_t result = ledriver_ledc_set_power(power);
    if (result != ESP_OK) {
        httpd_resp_send_500(req);
        return result;
    }

    httpd_resp_set_status(req, "204 No Content");
    return httpd_resp_send(req, NULL, 0);
}

esp_err_t register_endpoint_control_power(httpd_handle_t server) {
    static const httpd_uri_t uri1 = {.uri = "/control/power",
                                     .method = HTTP_GET,
                                     .handler = control_power_handler_get,
                                     .user_ctx = NULL};
    static const httpd_uri_t uri2 = {.uri = "/control/power",
                                     .method = HTTP_PUT,
                                     .handler = control_power_handler_put,
                                     .user_ctx = NULL};

    const esp_err_t result = httpd_register_uri_handler(server, &uri1);
    if (result != ESP_OK)
        return result;

    return httpd_register_uri_handler(server, &uri2);
}
