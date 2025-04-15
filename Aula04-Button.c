#include <stdio.h>
#include "freertos/FreeRTOS.h"  // Biblioteca do FreeRTOS para multitarefa
#include "freertos/task.h"      // Biblioteca para criação de tarefas
#include "driver/gpio.h"        // Biblioteca para controle dos pinos GPIO
#include "esp_log.h"            // Biblioteca para logs (ESP_LOGI)

#define BUTTON_PIN GPIO_NUM_13  // Define o pino do botão como GPIO 13

// TAG usada para exibição de logs no terminal
static const char *TAG = "BUTTON_COUNT";  

// Variáveis globais voláteis para uso na interrupção
volatile int buttonCount = 0;  // Contador do número de vezes que o botão foi pressionado
volatile bool counting = true; // Flag que indica se a contagem está ativa

// Função chamada pela interrupção quando o botão é pressionado
static void IRAM_ATTR button_isr_handler(void *arg) {
    if (counting) {  // Se a contagem estiver ativa
        buttonCount++;  // Incrementa o contador
    }
}

// Task do FreeRTOS que exibe e reseta a contagem a cada 10 segundos
void count_task(void *pvParameter) {
    while (1) {  
        vTaskDelay(pdMS_TO_TICKS(10000)); // Aguarda 10 segundos (10.000 ms)

        counting = false;  // Pausa a contagem enquanto exibe o valor
        ESP_LOGI(TAG, "Botao pressionado %d vezes nos ultimos 10 segundos", buttonCount); // Exibe o número de pressões do botão
        buttonCount = 0;   // Reseta o contador
        counting = true;   // Reativa a contagem
    }
}

// Função principal do programa
void app_main(void) {
    // Configuração do GPIO para o botão
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN), // Define o pino que será configurado
        .mode = GPIO_MODE_INPUT,              // Define o pino como entrada
        .pull_up_en = GPIO_PULLUP_ENABLE,     // Habilita o resistor de pull-up interno
        .pull_down_en = GPIO_PULLDOWN_DISABLE,// Desativa o resistor de pull-down
        .intr_type = GPIO_INTR_NEGEDGE        // Configura a interrupção na borda de descida (quando o botão é pressionado)
    };
    gpio_config(&io_conf); // Aplica as configurações ao GPIO

    // Instala o serviço de interrupções do ESP32
    gpio_install_isr_service(0);
    
    // Adiciona o handler da interrupção para o botão
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    // Cria a task do FreeRTOS para exibir e resetar a contagem a cada 10 segundos
    xTaskCreate(count_task, "count_task", 2048, NULL, 1, NULL);
}
