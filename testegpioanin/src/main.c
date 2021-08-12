#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

const gpio_num_t led = GPIO_NUM_25;

void setup(void);
void loop(void);

void app_main(void) {
    setup();
    loop();
}

void setup(){
    printf("Exemplo GPIO PWM\n");

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_12_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0,
        .gpio_num   = led,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_1
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);
}

void loop(){
    while (1) {

        vTaskDelay(2000/ portTICK_PERIOD_MS);
    }
}
