#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>

static const char* TAG = "wifi station";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAILED_BIT BIT1
#define WIFI_CONNECT_TIMEOUT_MS 30000

typedef struct {
    EventGroupHandle_t event_group;
    int retry_limit;
    int retry_count;
} wifi_station_context_t;

static wifi_station_context_t s_wifi = {
    .event_group = NULL,
    .retry_limit = 0,
    .retry_count = 0,
};

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                          void* event_data) {
    wifi_station_context_t* context = (wifi_station_context_t*)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi station started, connecting ...");

        const esp_err_t result = esp_wifi_connect();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_connect failed: %s", esp_err_to_name(result));
            xEventGroupSetBits(context->event_group, WIFI_FAILED_BIT);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t* event =
            (const wifi_event_sta_disconnected_t*)event_data;

        ESP_LOGW(TAG, "Disconnected from AP, reason=%d", event->reason);

        if (context->retry_count < context->retry_limit) {
            context->retry_count++;
            ESP_LOGI(TAG, "Retrying WiFi connection (%d/%d)", context->retry_count,
                     context->retry_limit);

            const esp_err_t result = esp_wifi_connect();
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "esp_wifi_connect failed: %s", esp_err_to_name(result));
                xEventGroupSetBits(context->event_group, WIFI_FAILED_BIT);
            }
        } else {
            xEventGroupSetBits(context->event_group, WIFI_FAILED_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t* event = (const ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        context->retry_count = 0;
        xEventGroupSetBits(context->event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t connect_to_wifi(const char* ssid, const char* password, int retry_num) {
    if (ssid == NULL || ssid[0] == '\0') {
        ESP_LOGE(TAG, "WiFi SSID is empty");
        return ESP_ERR_INVALID_ARG;
    }

    if (s_wifi.event_group == NULL) {
        s_wifi.event_group = xEventGroupCreate();
        if (s_wifi.event_group == NULL) {
            ESP_LOGE(TAG, "Can't create WiFi event group");
            return ESP_ERR_NO_MEM;
        }
    }

    s_wifi.retry_limit = retry_num;
    s_wifi.retry_count = 0;
    xEventGroupClearBits(s_wifi.event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT);

    ESP_ERROR_CHECK(esp_netif_init());

    esp_err_t result = esp_event_loop_create_default();
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE)
        return result;

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &event_handler, &s_wifi, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &event_handler, &s_wifi, &instance_got_ip));

    wifi_country_t country = {
        .cc = "PL",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_MANUAL,
    };
    ESP_ERROR_CHECK(esp_wifi_set_country(&country));

    wifi_config_t wifi_cfg = {
        .sta =
            {
                .scan_method = WIFI_ALL_CHANNEL_SCAN,
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            },
    };
    memset(wifi_cfg.sta.ssid, 0, sizeof(wifi_cfg.sta.ssid));
    strncpy((char*)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid) - 1);

    memset(wifi_cfg.sta.password, 0, sizeof(wifi_cfg.sta.password));
    strncpy((char*)wifi_cfg.sta.password, password, sizeof(wifi_cfg.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    const EventBits_t bits =
        xEventGroupWaitBits(s_wifi.event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT, pdFALSE,
                            pdFALSE, pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID:%s", ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAILED_BIT) {
        ESP_LOGI(TAG, "Can't connect to SSID:%s", ssid);
        return ESP_ERR_WIFI_NOT_CONNECT;
    } else {
        ESP_LOGE(TAG, "WiFi connect timeout after %d ms", WIFI_CONNECT_TIMEOUT_MS);
        return ESP_ERR_TIMEOUT;
    }
}
