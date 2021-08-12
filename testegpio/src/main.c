#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

const gpio_num_t led = GPIO_NUM_2;

void setup(void);
void loop(void);

void app_main(void) {
    setup();
    loop();
}

void setup(){
    printf("Exemplo GPIO\n");
    gpio_pad_select_gpio(led);
    gpio_set_direction(led, GPIO_MODE_OUTPUT);
}

void loop(){
    bool state = 1;

    while (1) {
        state = !state;

        printf("Piscando\n");
        gpio_set_level(led, state);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
