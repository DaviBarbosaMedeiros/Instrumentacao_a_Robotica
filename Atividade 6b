#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Definição dos pinos
#define BUTTON_R_PIN GPIO_NUM_12  // --> Botão para LED Vermelho
#define BUTTON_G_PIN GPIO_NUM_14  // --> Botão para LED Verde
#define BUTTON_B_PIN GPIO_NUM_27  // --> Botão para LED Azul
#define LED_R_PIN    GPIO_NUM_26  // --> Pino do LED Vermelho
#define LED_G_PIN    GPIO_NUM_25  // --> Pino do LED Verde
#define LED_B_PIN    GPIO_NUM_33  // --> Pino do LED Azul
#define DEBOUNCE_MS  50           // --> Tempo de debounce (ms)

static const char *TAG = "RGB_CONTROL";
volatile uint32_t lastRTime = 0, lastGTime = 0, lastBTime = 0;  // --> Tempos dos últimos eventos

// ISR para o botão do LED Vermelho
void IRAM_ATTR button_r_isr(void *arg) {
    uint32_t currentTime = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;  // --> Tempo atual
    if (currentTime - lastRTime > DEBOUNCE_MS) {  // --> Debounce
        gpio_set_level(LED_R_PIN, !gpio_get_level(LED_R_PIN));  // --> Inverte o estado do LED
    }
    lastRTime = currentTime;  // --> Atualiza o tempo
}

// ISR para o botão do LED Verde (mesma lógica)
void IRAM_ATTR button_g_isr(void *arg) {
    uint32_t currentTime = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if (currentTime - lastGTime > DEBOUNCE_MS) {
        gpio_set_level(LED_G_PIN, !gpio_get_level(LED_G_PIN));
    }
    lastGTime = currentTime;
}

// ISR para o botão do LED Azul (mesma lógica)
void IRAM_ATTR button_b_isr(void *arg) {
    uint32_t currentTime = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if (currentTime - lastBTime > DEBOUNCE_MS) {
        gpio_set_level(LED_B_PIN, !gpio_get_level(LED_B_PIN));
    }
    lastBTime = currentTime;
}

void app_main() {
    // Configuração dos botões
    gpio_config_t btn_config = {
        .pin_bit_mask = (1ULL << BUTTON_R_PIN) | (1ULL << BUTTON_G_PIN) | (1ULL << BUTTON_B_PIN),  // --> Múltiplos pinos
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE  // --> Interrupção na borda de descida
    };
    gpio_config(&btn_config);  // --> Aplica a configuração

    // Configuração dos LEDs
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_R_PIN) | (1ULL << LED_G_PIN) | (1ULL << LED_B_PIN),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&led_config);

    // Inicializa LEDs desligados
    gpio_set_level(LED_R_PIN, 0);
    gpio_set_level(LED_G_PIN, 0);
    gpio_set_level(LED_B_PIN, 0);

    // Instala e configura as ISRs
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_R_PIN, button_r_isr, NULL);
    gpio_isr_handler_add(BUTTON_G_PIN, button_g_isr, NULL);
    gpio_isr_handler_add(BUTTON_B_PIN, button_b_isr, NULL);

    ESP_LOGI(TAG, "Sistema RGB com debounce pronto.");  // --> Mensagem inicial
}
