/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define FLAG_VALUE 123
#define ADC_FLAG 0
#define LED_ON_FLAG 1
#define LED_OFF_FLAG 2
 
#define LED_PIN 25
float voltage;

void pico_led_init(void) {
    // Initialise LED Pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void core1_entry() {
    pico_led_init();
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0
    while (1) {
        uint32_t flag = multicore_fifo_pop_blocking(); //Input flag from core 0 
         if (flag  == ADC_FLAG){
            voltage = (adc_read()/4096.0)*3.3;
            multicore_fifo_push_blocking(FLAG_VALUE);
         }
         else if (flag == LED_ON_FLAG){
            gpio_put(LED_PIN, true);
            multicore_fifo_push_blocking(FLAG_VALUE);
         }
         else if (flag == LED_OFF_FLAG){
            gpio_put(LED_PIN, false);
            multicore_fifo_push_blocking(FLAG_VALUE);
         }
    }
}

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, multicore!\n");

    /// \tag::setup_multicore[]

    multicore_launch_core1(core1_entry);

    // Wait for it to start up

    int input;
    char message[50];

    while (1){
        printf("Enter a command \n\r 0: ADC read \n\r 1: LED On \n\r 2: LED Off \n\r");
        scanf("%s", &message);
        printf("Input: %s\r\n\n", message);

        input = atoi(message);
        if (input == 0){
            multicore_fifo_push_blocking(ADC_FLAG);
            uint32_t flag = multicore_fifo_pop_blocking();

            if (flag != FLAG_VALUE){
                printf("Hmm, that's not right on core 1!\n");
                break;
            }

            printf("The voltage is %f\r\n\n", voltage);
        }
        else if (input == 1) {
            multicore_fifo_push_blocking(LED_ON_FLAG);
            uint32_t flag = multicore_fifo_pop_blocking();
            if (flag != FLAG_VALUE){
                printf("Hmm, that's not right on core 1!\n");
                break;
            }
        }
        else if (input == 2) {
            multicore_fifo_push_blocking(LED_OFF_FLAG);
            uint32_t flag = multicore_fifo_pop_blocking();
            if (flag != FLAG_VALUE){
                printf("Hmm, that's not right on core 1!\n");
                break;
            }
        }
    }
    /// \end::setup_multicore[]
}
