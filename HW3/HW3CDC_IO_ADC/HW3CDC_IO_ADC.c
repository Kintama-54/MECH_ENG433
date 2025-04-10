#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_PIN 16
#define PUSH_BUTTON 2

void pico_led_init(void) {
     #ifdef LED_PIN
        // Initialise LED Pin
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
     #endif
     }

void pico_set_led(bool led_status) {
#if defined(LED_PIN)
    // Just set the GPIO on or off
    gpio_put(LED_PIN, led_status);
#endif
}

// Initialising push button
void pico_pushbutton_init(void){
    gpio_init(PUSH_BUTTON);
    gpio_set_dir(PUSH_BUTTON, GPIO_IN);
    gpio_pull_down(PUSH_BUTTON);

}

int main() {
    // Initialisations
    stdio_init_all();
    pico_led_init();
    pico_pushbutton_init();
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    int num_samples;
    while (!stdio_usb_connected()) { // wait for USB port to open
        sleep_ms(100);
    }
    pico_set_led(true); // Sets LED on
    while (gpio_get(PUSH_BUTTON) == 1) { // Wait until button is pushed
        sleep_ms(10);
    }
    pico_set_led(false); // Sets LED off
    while(1){ //Repeats infinitely
        printf("Enter no. of analog samples to take (0-100): \n"); //Takes user input
        scanf("%d", &num_samples); 
        for (int i = 0;i < num_samples;i++){ // reads ADC num_samples amount of times
            float voltage = (adc_read()/4096.0)*3.3; // converts to volts
            printf("Voltage: %.2fV\r\n",voltage); // prints to screen.
            sleep_ms(10);
        }
    }

}
