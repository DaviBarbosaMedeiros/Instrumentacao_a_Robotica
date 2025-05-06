#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <inttypes.h>

#define BUTTON_GPIO  GPIO_NUM_13
#define DEBOUNCE_MS  50

static const char *TAG = "Button Counter";
volatile uint32_t counter = 0;
volatile uint32_t last_isr_time = 0;

void IRAM_ATTR button_isr_handler(void* arg) {
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if (now - last_isr_time > DEBOUNCE_MS) {
        counter++;
    }
    last_isr_time = now;
}

void app_main() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    while(1) {
        ESP_LOGI(TAG, "Pressionamentos: %" PRIu32, counter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
