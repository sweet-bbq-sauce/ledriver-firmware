#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <esp_err.h>

typedef struct {
    uint16_t r, g, b;
} ledriver_ledc_brightness_state_t;

esp_err_t ledriver_ledc_init(int r_gpio, int g_gpio, int b_gpio);

esp_err_t ledriver_ledc_set_red(uint16_t brightness);
esp_err_t ledriver_ledc_set_green(uint16_t brightness);
esp_err_t ledriver_ledc_set_blue(uint16_t brightness);

esp_err_t ledriver_ledc_set_rgb(const ledriver_ledc_brightness_state_t* state);
esp_err_t ledriver_ledc_get_rgb(ledriver_ledc_brightness_state_t* state);

esp_err_t ledriver_ledc_set_power(bool power);
esp_err_t ledriver_ledc_get_power(bool* power);
