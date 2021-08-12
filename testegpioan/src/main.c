#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"


void setup(void);
void loop(void);

void app_main(void) {
    setup();
    loop();
}

void setup(){
    printf("Exemplo GPIO Analogico\n");

    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop(){
    while (1) {
        u_int16_t valor = adc1_get_raw(ADC1_CHANNEL_5);

        printf("Sensor: %d\n", valor);

        u_int8_t num = map(valor, 0, 4095, 0, 100);

        printf("MAP: %d\n", num);

        vTaskDelay(2000/ portTICK_PERIOD_MS);
    }
}
