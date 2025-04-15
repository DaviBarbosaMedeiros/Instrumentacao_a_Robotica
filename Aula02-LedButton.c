#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h" // Acessar GPIO
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "esp_log.h"

//Definições de entrada e saída :P

#define LED_PIN_1 2 
#define LED_PIN_2 18
#define BUTTON_PIN_1 13
#define BUTTON_PIN_2 12

#define DEBOUNC_TIME_MS 50

//Criando a Tag B)

static const char *TAG = "BUTTON TEST" ;

int debounce(int pin, int *last_state) {
	int current_state = gpio_get_level(pin);
	if(current_state != *last_state){
		vTaskDelay(DEBOUNC_TIME_MS/portTICK_PERIOD_MS);
		if(current_state == gpio_get_level(pin)) {
			*last_state = current_state;
			return current_state;
		}
	}
	return -1;
}

void app_main(void) {
	
    int button_state_1 = 1;
    int button_state_2 = 1;
    int led_state_1 = 0;
    int led_state_2 = 0;
    
    gpio_reset_pin(LED_PIN_1);
    gpio_set_direction(LED_PIN_1, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(LED_PIN_2);
    gpio_set_direction(LED_PIN_2, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(BUTTON_PIN_1);
    gpio_set_direction(BUTTON_PIN_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_1, GPIO_PULLUP_ONLY);
    
    gpio_reset_pin(BUTTON_PIN_2);
    gpio_set_direction(BUTTON_PIN_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_2, GPIO_PULLUP_ONLY);
    
    while (1) {
		
		int new_state_1 = debounce(BUTTON_PIN_1,&button_state_1);
		if(new_state_1 ==0){
			led_state_1 ^=1;
			gpio_set_level(LED_PIN_1, led_state_1);
			ESP_LOGI(TAG,"Button_01 TOGGlED LED_01 TO %d", led_state_1);
		}
		
		int new_state_2 = debounce(BUTTON_PIN_2,&button_state_2);
		if(new_state_2 ==0){
			led_state_2 ^=1;
			gpio_set_level(LED_PIN_2, led_state_2);
			ESP_LOGI(TAG,"Button_02 TOGGlED LED_02 TO %d", led_state_2);
		}

	}	

}
