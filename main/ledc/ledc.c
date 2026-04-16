#include <stdint.h>

#include <driver/ledc.h>
#include <esp_err.h>

#include <ledc/ledc.h>

#define LEDC_TIMER LEDC_TIMER_0

#define LEDC_CHANNEL_RED LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE LEDC_CHANNEL_2

static ledriver_ledc_brightness_state_t current_state = {};

esp_err_t ledriver_ledc_init(int r_gpio, int g_gpio, int b_gpio) {
    const ledc_timer_config_t timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                       .duty_resolution = LEDC_TIMER_16_BIT,
                                       .timer_num = LEDC_TIMER,
                                       .freq_hz = 1000,
                                       .clk_cfg = LEDC_AUTO_CLK};

    esp_err_t result = ledc_timer_config(&timer);
    if (result != ESP_OK)
        return result;

    const ledc_channel_config_t channels[] = {{
                                                  .gpio_num = r_gpio,
                                                  .speed_mode = LEDC_LOW_SPEED_MODE,
                                                  .channel = LEDC_CHANNEL_RED,
                                                  .intr_type = LEDC_INTR_DISABLE,
                                                  .timer_sel = LEDC_TIMER,
                                                  .duty = 0,
                                                  .hpoint = 0,
                                              },
                                              {
                                                  .gpio_num = g_gpio,
                                                  .speed_mode = LEDC_LOW_SPEED_MODE,
                                                  .channel = LEDC_CHANNEL_GREEN,
                                                  .intr_type = LEDC_INTR_DISABLE,
                                                  .timer_sel = LEDC_TIMER,
                                                  .duty = 0,
                                                  .hpoint = 0,
                                              },
                                              {
                                                  .gpio_num = b_gpio,
                                                  .speed_mode = LEDC_LOW_SPEED_MODE,
                                                  .channel = LEDC_CHANNEL_BLUE,
                                                  .intr_type = LEDC_INTR_DISABLE,
                                                  .timer_sel = LEDC_TIMER,
                                                  .duty = 0,
                                                  .hpoint = 0,
                                              }};

    for (int i = 0; i < 3; i++) {
        result = ledc_channel_config(&channels[i]);
        if (result != ESP_OK)
            return result;
    }

    return ESP_OK;
}

esp_err_t ledriver_ledc_set_red(uint16_t brightness) {
    esp_err_t result =
        ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_RED, brightness, 0);
    if (result != ESP_OK)
        return result;

    current_state.r = brightness;
    return ESP_OK;
}

esp_err_t ledriver_ledc_set_green(uint16_t brightness) {
    esp_err_t result =
        ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_GREEN, brightness, 0);
    if (result != ESP_OK)
        return result;

    current_state.g = brightness;
    return ESP_OK;
}

esp_err_t ledriver_ledc_set_blue(uint16_t brightness) {
    esp_err_t result =
        ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_BLUE, brightness, 0);
    if (result != ESP_OK)
        return result;

    current_state.b = brightness;
    return ESP_OK;
}

esp_err_t ledriver_ledc_set_rgb(const ledriver_ledc_brightness_state_t* state) {
    if (state == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t result = ledriver_ledc_set_red(state->r);
    if (result != ESP_OK)
        return result;

    result = ledriver_ledc_set_green(state->g);
    if (result != ESP_OK)
        return result;

    result = ledriver_ledc_set_blue(state->b);
    if (result != ESP_OK)
        return result;

    return ESP_OK;
}

esp_err_t ledriver_ledc_get_rgb(ledriver_ledc_brightness_state_t* state) {
    if (state == NULL)
        return ESP_ERR_INVALID_ARG;

    *state = current_state;
    return ESP_OK;
}