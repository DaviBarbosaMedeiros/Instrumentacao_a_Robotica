#include <stdio.h>
#include "freertos/FreeRTOS.h"  // Biblioteca do FreeRTOS para multitarefa
#include "freertos/task.h"      // Biblioteca para criação de tarefas
#include "driver/gpio.h"        // Biblioteca para controle dos pinos GPIO
#include "driver/ledc.h"        // Biblioteca para controle de PWM
#include "esp_log.h"            // Biblioteca para logs (ESP_LOGI)

#define PWM_PIN GPIO_NUM_5        // Pino do PWM
#define COUNTER_PIN GPIO_NUM_13   // Pino do contador (ler o PWM)
#define PWM_FREQUENCY 120         // Frequência do PWM em Hz
#define DUTY_CYCLE_RESOLUTION 10 // Resolução do Duty Cycle (10-bit)

static const char *TAG = "PWM_COUNTER";  // TAG para logs

volatile int pwmEdgeCount = 0;  // Contador de bordas do PWM
volatile bool counting = true;  // Flag de contagem

// Função de interrupção chamada nas transições de borda do PWM
static void IRAM_ATTR pwm_isr_handler(void *arg) {
    if (counting) {
        pwmEdgeCount++;  // Incrementa o contador a cada borda do PWM
    }
}

// Função que configura o PWM
void setup_pwm() {
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQUENCY,   // Frequência de 60 Hz
        .duty_resolution = LEDC_TIMER_10_BIT, // Resolução de 10 bits
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM_PIN,        // Pino do PWM
        .duty = 1023,                // Duty Cycle inicial (50%)
    };
    ledc_channel_config(&channel_config);

	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1023);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);  // Atualiza o duty cycle
}

// Função de contagem, exibe o número de bordas do PWM a cada 1 segundo
void count_task(void *pvParameter) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo

        counting = false;  // Pausa a contagem durante a exibição do valor
        ESP_LOGI(TAG, "Bordas do PWM contadas: %d", pwmEdgeCount);
        pwmEdgeCount = 0;   // Reseta o contador
        counting = true;    // Reativa a contagem
    }
}

void app_main(void) {
    // Configura o pino para leitura do PWM (GPIO 13)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << COUNTER_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,  // Resistor Pull-up
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,    // Interrupção em qualquer borda (subida ou descida)
    };
    gpio_config(&io_conf);  // Aplica as configurações

    // Instala o serviço de interrupção do GPIO
    gpio_install_isr_service(0);

    // Adiciona o handler para as interrupções no pino do PWM (GPIO 13)
    gpio_isr_handler_add(COUNTER_PIN, pwm_isr_handler, NULL);

    // Cria a tarefa do FreeRTOS para contagem
    xTaskCreate(count_task, "count_task", 2048, NULL, 1, NULL);

    // Configura o PWM no pino GPIO 5
    setup_pwm();
}
