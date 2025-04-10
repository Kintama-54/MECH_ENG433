
/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

 // Initialise pins
#define LED_PIN 16
#define GPIO_WATCH_PIN 2
 
static int count;

void pico_led_init(void) {
     #ifdef LED_PIN
        // Initialise LED Pin
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
     #endif
     }
 
// Function to turn the LED on and off
void pico_set_led(bool led_on) {
#if defined(LED_PIN)
    // Just set the GPIO on or off
    gpio_put(LED_PIN, led_on);
#endif
}
 
 // ISR
 void gpio_callback(uint gpio, uint32_t events) {
    // Button Debouncing
    sleep_ms(30);
    if(!gpio_get(GPIO_WATCH_PIN))
    {
        return;
    }
    if (count % 2 == 0) // Toggles LED on and off
    {
        pico_set_led(false);
    } 
    else {
        pico_set_led(true);
    }
    printf("Count: %d\n", count); // Prints to screen
    count += 1; // appends count
 }
 
 int main() {
    pico_led_init();
    stdio_init_all();
    
    count = 0;

    // configure button
    gpio_init(GPIO_WATCH_PIN);
    gpio_set_dir(GPIO_WATCH_PIN,GPIO_IN);
    gpio_set_irq_enabled_with_callback(GPIO_WATCH_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    while(1);
 }
 
