#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include <cJSON.h>

#include <ledc/ledc.h>
#include <utils/tools.h>

static const char* TAG = "httpd";

#define REQUEST_BODY_MAX_LENGTH 128

static esp_err_t control_rgb_handler_ws(httpd_req_t* req) {
    if (req->method == HTTP_GET)
        return ESP_OK;

    httpd_ws_frame_t frame = {};
    esp_err_t result = httpd_ws_recv_frame(req, &frame, 0);
    if (result != ESP_OK)
        return result;

    if (frame.type != HTTPD_WS_TYPE_BINARY || frame.len != 6) {
        ESP_LOGW(TAG, "Invalid ws frame: type=%d len=%zu", frame.type, frame.len);
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t payload[6];
    frame.payload = payload;

    result = httpd_ws_recv_frame(req, &frame, sizeof(payload));
    if (result != ESP_OK)
        return result;

    const ledriver_ledc_brightness_state_t state = {.r = BE16(payload[0], payload[1]),
                                                    .g = BE16(payload[2], payload[3]),
                                                    .b = BE16(payload[4], payload[5])};

    return ledriver_ledc_set_rgb(&state);
}

static esp_err_t control_color_handler_get(httpd_req_t* req) {
    ledriver_ledc_brightness_state_t state;
    esp_err_t result = ledriver_ledc_get_rgb(&state);
    if (result != ESP_OK) {
        httpd_resp_send_500(req);
        return result;
    }
    char response[128];
    const int length = snprintf(response, sizeof(response),
                                "{\"red\":%" PRIu16 ",\"green\":%" PRIu16 ",\"blue\":%" PRIu16 "}",
                                state.r, state.g, state.b);

    if (length < 0 || length >= sizeof(response))
        return ESP_FAIL;

    result = httpd_resp_set_type(req, "application/json");
    if (result != ESP_OK)
        return result;

    return httpd_resp_send(req, response, length);
}

static bool check_json_number(double value) {
    if (!isfinite(value))
        return false;

    if (value < 0.0 || value > 65535.0)
        return false;

    if (trunc(value) != value)
        return false;

    return true;
}

static esp_err_t control_color_handler_put(httpd_req_t* req) {
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

    const cJSON* red_json = cJSON_GetObjectItemCaseSensitive(json, "red");
    const cJSON* green_json = cJSON_GetObjectItemCaseSensitive(json, "green");
    const cJSON* blue_json = cJSON_GetObjectItemCaseSensitive(json, "blue");
    if (!cJSON_IsNumber(red_json) || !cJSON_IsNumber(green_json) || !cJSON_IsNumber(blue_json)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_ERR_INVALID_ARG;
    }

    const double red_value = cJSON_GetNumberValue(red_json);
    const double green_value = cJSON_GetNumberValue(green_json);
    const double blue_value = cJSON_GetNumberValue(blue_json);

    cJSON_Delete(json);

    if (!check_json_number(red_value) || !check_json_number(green_value) ||
        !check_json_number(blue_value)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid RGB values");
        return ESP_ERR_INVALID_ARG;
    }

    const ledriver_ledc_brightness_state_t state = {
        .r = (uint16_t)red_value, .g = (uint16_t)green_value, .b = (uint16_t)blue_value};

    const esp_err_t result = ledriver_ledc_set_rgb(&state);
    if (result != ESP_OK) {
        httpd_resp_send_500(req);
        return result;
    }

    httpd_resp_set_status(req, "204 No Content");
    return httpd_resp_send(req, NULL, 0);
}

esp_err_t register_endpoint_control_color(httpd_handle_t server) {
    static const httpd_uri_t uri1 = {.uri = "/control/rgb",
                                     .method = HTTP_GET,
                                     .handler = control_rgb_handler_ws,
                                     .user_ctx = NULL,
                                     .is_websocket = true};
    static const httpd_uri_t uri2 = {.uri = "/control/color",
                                     .method = HTTP_GET,
                                     .handler = control_color_handler_get,
                                     .user_ctx = NULL};
    static const httpd_uri_t uri3 = {.uri = "/control/color",
                                     .method = HTTP_PUT,
                                     .handler = control_color_handler_put,
                                     .user_ctx = NULL};

    esp_err_t result = httpd_register_uri_handler(server, &uri1);
    if (result != ESP_OK)
        return result;

    result = httpd_register_uri_handler(server, &uri2);
    if (result != ESP_OK)
        return result;

    return httpd_register_uri_handler(server, &uri3);
}
