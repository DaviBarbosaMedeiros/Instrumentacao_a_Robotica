#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <inttypes.h>

#define NTC_PIN      ADC1_CHANNEL_0
#define LED_PIN      GPIO_NUM_2
#define THRESHOLD    30.0   // 30°C
#define DEBOUNCE_MS  2000
#define V_REF        1100   // 1.1V

static const char *TAG = "Termostato";
static esp_adc_cal_characteristics_t *adc_chars;

float read_temperature() {
    uint32_t adc_reading = adc1_get_raw(NTC_PIN);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    // Fórmula simplificada - substitua pela equação do seu NTC
    return (voltage * 0.1f); // Exemplo: 300mV = 30°C
}

void temperature_task(void *pvParameter) {
    uint32_t last_trigger = 0;
    
    while(1) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (now - last_trigger > DEBOUNCE_MS) {
            float temp = read_temperature();
            
            if (temp > THRESHOLD) {
                gpio_set_level(LED_PIN, 1);
                ESP_LOGI(TAG, "Temperatura: %.2f°C (ACIMA do limite)", temp);
            } else {
                gpio_set_level(LED_PIN, 0);
                ESP_LOGI(TAG, "Temperatura: %.2f°C", temp);
            }
            last_trigger = now;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main() {
    // Configura ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(NTC_PIN, ADC_ATTEN_DB_11);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, V_REF, adc_chars);

    // Configura LED
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&led_conf);

    // Cria task para monitoramento
    xTaskCreate(temperature_task, "temp_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Termostato inicializado. Limite: %.1f°C", THRESHOLD);
}
