#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define RCPin 15

void servo_init();
void set_angle();

void servo_init(){
    gpio_set_function(RCPin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(RCPin); // Get PWM slice number
    float div = 60; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = 50000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    pwm_set_gpio_level(RCPin, wrap / 8); // set the duty cycle to 50%
}

void set_angle(){
    uint16_t wrap = 50000; // when to rollover, must be less than 65535
    for (int i =0;i<100;i++){
            pwm_set_gpio_level(RCPin, wrap * (0.025 + 0.001*i)); // set the duty cycle to 50%
            sleep_ms(20);
    }
    for (int i =0;i<100;i++){
            pwm_set_gpio_level(RCPin, wrap * (0.125 - 0.001*i)); // set the duty cycle to 50%
            sleep_ms(20);
    }
}

int main()
{
    stdio_init_all();
    servo_init();
    while (1){
        set_angle();
    }
}
