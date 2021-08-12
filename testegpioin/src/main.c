#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

const gpio_num_t led = GPIO_NUM_25;
const gpio_num_t btn = GPIO_NUM_5;
u_int32_t pb = 0;

void setup(void);
void loop(void);
void muda_estado();

void app_main(void) {
    setup();
    loop();
}

void setup(){
    printf("Exemplo GPIO Input\n");
    gpio_pad_select_gpio(led);
    gpio_pad_select_gpio(btn);

    gpio_set_direction(led, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(btn, GPIO_MODE_INPUT);

    gpio_pullup_en(btn);
    gpio_pulldown_dis(btn);

    gpio_set_level(led, 0);
}

void loop(){
    while (1) {
        bool state = gpio_get_level(btn);

        if(state){
            printf("Botão pressionado! %d\n", pb);
            muda_estado();
            pb++;
        }

        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}

void muda_estado() {
    bool state = gpio_get_level(led);

    gpio_set_level(led, !state);

    printf("Estado atual do led: %s\n", !state ? "ligado" : "desligado");
}
