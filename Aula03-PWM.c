// includes do FreeRTOS
#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"
// include GPIO
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

void app_main(void)
{
     uint16_t duty = 0;    //duty cycle value
    
    // Configuração do timer
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE, 
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 2000,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_config);
    
    // Configuração do canal
    ledc_channel_config_t channel_config = {
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_LOW_SPEED_MODE, // Corrigido aqui
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = GPIO_NUM_5,
        .duty = 0 // Duty inicial configurado
    };
    ledc_channel_config(&channel_config);
    
    // Configuração do duty cycle
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    // Criando a função de fading
    while (1) {
        
        
        if(duty<1023){
			
			 ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
             ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
             duty++;
			
		}else{
			duty =0;
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
