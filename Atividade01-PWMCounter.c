#include <stdio.h>
#include "freertos/FreeRTOS.h"  // Biblioteca do FreeRTOS para multitarefa
#include "freertos/task.h"      // Biblioteca para criação de tarefas
#include "driver/gpio.h"        // Biblioteca para controle dos pinos GPIO
#include "driver/ledc.h"        // Biblioteca para controle de PWM
#include "esp_log.h"            // Biblioteca para logs (ESP_LOGI)


#define PWM_PIN_1 GPIO_NUM_5        // Pino 1 do PWM  
#define COUNTER_PIN_1 GPIO_NUM_13   // Pino 1 do contador (ler o PWM)
#define PWM_PIN_2 GPIO_NUM_18       // Pino 2 do PWM
#define COUNTER_PIN_2 GPIO_NUM_19   // Pino 2 do contador (ler o PWM)
#define PWM_FREQUENCY 60           // Frequência do PWM em Hz
#define DUTY_CYCLE_RESOLUTION 10   // Resolução do Duty Cycle (10-bit)

static const char *TAG = "PWM_COUNTER";  // TAG para logs

volatile int pwmEdgeCount1 = 0;  // Contador de bordas do PWM para o pino 1
volatile int pwmEdgeCount2 = 0;  // Contador de bordas do PWM para o pino 2
volatile bool counting1 = true;  // Flag de contagem para pino 1
volatile bool counting2 = true;  // Flag de contagem para pino 2

// Função de interrupção chamada nas transições de borda do PWM
static void IRAM_ATTR pwm_isr_handler_1(void *arg) {
    if (counting1) {
        pwmEdgeCount1++;  // Incrementa o contador para o pino 1
    }
}

static void IRAM_ATTR pwm_isr_handler_2(void *arg) {
    if (counting2) {
        pwmEdgeCount2++;  // Incrementa o contador para o pino 2
    }
}

// Função que configura o PWM
void setup_pwm() {
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQUENCY,  // Frequência de 60 Hz
        .duty_resolution = LEDC_TIMER_10_BIT,  // Resolução de 10 bits
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_config);

    // Configura o primeiro canal de PWM
    ledc_channel_config_t channel_config_1 = {
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM_PIN_1,  // Pino do PWM 1
        .duty = 1023,  // Duty Cycle inicial (50%)
    };
    ledc_channel_config(&channel_config_1);

    // Configura o segundo canal de PWM
    ledc_channel_config_t channel_config_2 = {
        .channel = LEDC_CHANNEL_1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM_PIN_2,  // Pino do PWM 2
        .duty = 1023,  // Duty Cycle inicial (50%)
    };
    ledc_channel_config(&channel_config_2);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1023);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);  // Atualiza o duty cycle do primeiro PWM

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 1023);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);  // Atualiza o duty cycle do segundo PWM
}

// Função de contagem, exibe o número de bordas do PWM a cada 1 segundo
void count_task(void *pvParameter) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Aguarda 1 segundo

        counting1 = false;  // Pausa a contagem do primeiro PWM
        counting2 = false;  // Pausa a contagem do segundo PWM
        ESP_LOGI(TAG, "Bordas do PWM 1 contadas: %d", pwmEdgeCount1);
        ESP_LOGI(TAG, "Bordas do PWM 2 contadas: %d", pwmEdgeCount2);
        pwmEdgeCount1 = 0;  // Reseta o contador do primeiro PWM
        pwmEdgeCount2 = 0;  // Reseta o contador do segundo PWM
        counting1 = true;   // Reativa a contagem do primeiro PWM
        counting2 = true;   // Reativa a contagem do segundo PWM
    }
}

void app_main(void) {
    // Configura o pino para leitura do PWM (GPIO 13 para PWM 1)
    gpio_config_t io_conf_1 = {
        .pin_bit_mask = (1ULL << COUNTER_PIN_1),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,  // Resistor Pull-up
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,    // Interrupção na borda de descida
    };
    gpio_config(&io_conf_1);  // Aplica as configurações

    // Configura o pino para leitura do PWM (GPIO 19 para PWM 2)
    gpio_config_t io_conf_2 = {
        .pin_bit_mask = (1ULL << COUNTER_PIN_2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,  // Resistor Pull-up
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,    // Interrupção na borda de descida
    };
    gpio_config(&io_conf_2);  // Aplica as configurações

    // Instala o serviço de interrupção do GPIO
    gpio_install_isr_service(0);

    // Adiciona o handler para as interrupções nos pinos dos PWM
    gpio_isr_handler_add(COUNTER_PIN_1, pwm_isr_handler_1, NULL);
    gpio_isr_handler_add(COUNTER_PIN_2, pwm_isr_handler_2, NULL);

    // Cria a tarefa do FreeRTOS para contagem
    xTaskCreate(count_task, "count_task", 2048, NULL, 1, NULL);

    // Configura os dois pinos PWM
    setup_pwm();
}
