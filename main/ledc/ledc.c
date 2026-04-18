#include <stdint.h>

#include <driver/ledc.h>
#include <esp_err.h>

#include <ledc/ledc.h>

#define LEDC_TIMER LEDC_TIMER_0

#define LEDC_CHANNEL_RED LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE LEDC_CHANNEL_2

static ledriver_ledc_brightness_state_t current_state = {};
static bool power_on = false;

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

static esp_err_t ledc_set_brightness(ledc_channel_t channel, uint16_t brightness, bool save,
                                     bool force) {
    if (channel != LEDC_CHANNEL_RED && channel != LEDC_CHANNEL_GREEN &&
        channel != LEDC_CHANNEL_BLUE)
        return ESP_ERR_INVALID_ARG;

    if (power_on || force) {
        esp_err_t result = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, brightness);
        if (result != ESP_OK)
            return result;

        result = ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
        if (result != ESP_OK)
            return result;
    }

    if (save) {
        switch (channel) {
        case LEDC_CHANNEL_RED: {
            current_state.r = brightness;
            break;
        }

        case LEDC_CHANNEL_GREEN: {
            current_state.g = brightness;
            break;
        }

        case LEDC_CHANNEL_BLUE: {
            current_state.b = brightness;
            break;
        }

        default:
            return ESP_ERR_INVALID_ARG;
        }
    }
    return ESP_OK;
}

static esp_err_t ledc_set_red(uint16_t brightness, bool save, bool force) {
    return ledc_set_brightness(LEDC_CHANNEL_RED, brightness, save, force);
}

static esp_err_t ledc_set_green(uint16_t brightness, bool save, bool force) {
    return ledc_set_brightness(LEDC_CHANNEL_GREEN, brightness, save, force);
}

static esp_err_t ledc_set_blue(uint16_t brightness, bool save, bool force) {
    return ledc_set_brightness(LEDC_CHANNEL_BLUE, brightness, save, force);
}

static esp_err_t ledc_set_rgb(const ledriver_ledc_brightness_state_t* state, bool save,
                              bool force) {
    if (state == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t result = ledc_set_red(state->r, save, force);
    if (result != ESP_OK)
        return result;

    result = ledc_set_green(state->g, save, force);
    if (result != ESP_OK)
        return result;

    result = ledc_set_blue(state->b, save, force);
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

esp_err_t ledriver_ledc_set_power(bool power) {
    if (power) {
        const esp_err_t result = ledc_set_rgb(&current_state, false, true);
        if (result != ESP_OK)
            return result;
    } else {
        static const ledriver_ledc_brightness_state_t power_off_state = {0};

        const esp_err_t result = ledc_set_rgb(&power_off_state, false, true);
        if (result != ESP_OK)
            return result;
    }
    power_on = power;
    return ESP_OK;
}

esp_err_t ledriver_ledc_get_power(bool* power) {
    if (power == NULL)
        return ESP_ERR_INVALID_ARG;

    *power = power_on;
    return ESP_OK;
}

esp_err_t ledriver_ledc_set_red(uint16_t brightness) {
    return ledc_set_brightness(LEDC_CHANNEL_RED, brightness, true, false);
}

esp_err_t ledriver_ledc_set_green(uint16_t brightness) {
    return ledc_set_brightness(LEDC_CHANNEL_GREEN, brightness, true, false);
}

esp_err_t ledriver_ledc_set_blue(uint16_t brightness) {
    return ledc_set_brightness(LEDC_CHANNEL_BLUE, brightness, true, false);
}

esp_err_t ledriver_ledc_set_rgb(const ledriver_ledc_brightness_state_t* state) {
    return ledc_set_rgb(state, true, false);
}
