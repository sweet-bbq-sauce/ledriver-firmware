#include <stdint.h>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include <ledc/ledc.h>
#include <util/tools.h>

static const char* TAG = "httpd";

esp_err_t httpd_ws_rgb_handler(httpd_req_t* req) {
    if (req->method == HTTP_GET)
        return ESP_OK;

    httpd_ws_frame_t frame = {};
    esp_err_t result = httpd_ws_recv_frame(req, &frame, 0);
    if (result != ESP_OK)
        return result;

    if (frame.type != HTTPD_WS_TYPE_BINARY || frame.len != 6) {
        ESP_LOGW(TAG, "Invalid ws frame: type=%d len=%u", frame.type, frame.len);
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