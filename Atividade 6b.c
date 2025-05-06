#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <inttypes.h>

#define BUTTON_R  GPIO_NUM_12
#define BUTTON_G  GPIO_NUM_14
#define BUTTON_B  GPIO_NUM_27
#define LED_R     GPIO_NUM_26
#define LED_G     GPIO_NUM_25
#define LED_B     GPIO_NUM_33
#define DEBOUNCE_MS 50

static const char *TAG = "RGB Control";

typedef struct {
    gpio_num_t button_pin;
    gpio_num_t led_pin;
    uint32_t last_press;
} button_led_t;

button_led_t buttons[] = {
    {BUTTON_R, LED_R, 0},
    {BUTTON_G, LED_G, 0},
    {BUTTON_B, LED_B, 0}
};

void IRAM_ATTR button_isr_handler(void* arg) {
    uint32_t idx = (uint32_t)arg;
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    
    if (now - buttons[idx].last_press > DEBOUNCE_MS) {
        gpio_set_level(buttons[idx].led_pin, !gpio_get_level(buttons[idx].led_pin));
    }
    buttons[idx].last_press = now;
}

void app_main() {
    // Configura LEDs
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_R) | (1ULL << LED_G) | (1ULL << LED_B),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&led_conf);

    // Configura botões
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_R) | (1ULL << BUTTON_G) | (1ULL << BUTTON_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&btn_conf);

    // Instala ISRs
    gpio_install_isr_service(0);
    for (int i = 0; i < 3; i++) {
        gpio_isr_handler_add(buttons[i].button_pin, button_isr_handler, (void*)i);
    }

    ESP_LOGI(TAG, "Controle RGB inicializado. Botões configurados em GPIO %" PRIu32 ", %" PRIu32 ", %" PRIu32, 
             BUTTON_R, BUTTON_G, BUTTON_B);
}
