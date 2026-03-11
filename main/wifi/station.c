#include <string.h>
#include <strings.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>

static EventGroupHandle_t s_wifi_event_group;
static const char* TAG = "wifi station";

struct RetryCounter {
    int limit, counter;
};

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                          void* event_data) {

    struct RetryCounter* rc = (struct RetryCounter*)arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        esp_wifi_connect();
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (rc->counter < rc->limit) {
            esp_wifi_connect();
            rc->counter++;
        } else
            xEventGroupSetBits(s_wifi_event_group, BIT1);

        ESP_LOGI(TAG, "Connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, BIT0);
        rc->counter = 0;
    }
}

esp_err_t connect_to_wifi(const char* ssid, const char* password, int retry_num) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    struct RetryCounter rc = {.limit = retry_num, .counter = 0};

    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &event_handler, &rc, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &event_handler, &rc, &instance_got_ip));

    wifi_config_t wifi_cfg = {
        .sta = {.threshold.authmode = 0, .sae_pwe_h2e = 3, .sae_h2e_identifier = ""}};
    bzero(wifi_cfg.sta.ssid, sizeof(wifi_cfg.sta.ssid));
    strncpy((char*)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid) - 1);

    bzero(wifi_cfg.sta.password, sizeof(wifi_cfg.sta.password));
    strncpy((char*)wifi_cfg.sta.password, password, sizeof(wifi_cfg.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits =
        xEventGroupWaitBits(s_wifi_event_group, BIT0 | BIT1, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & BIT0) {
        ESP_LOGI(TAG, "Connected to ap:%s pass:%s", ssid, password);
        return ESP_OK;
    } else if (bits & BIT1) {
        ESP_LOGI(TAG, "Can't connect to ap:%s pass:%s", ssid, password);
        return ESP_ERR_WIFI_NOT_CONNECT;
    } else {
        ESP_LOGI(TAG, "Unexpected event");
        return ESP_FAIL;
    }
}