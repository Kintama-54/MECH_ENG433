#include <stdio.h>
#include "pico/stdlib.h"
#include "cam.h"
#include <stdlib.h>

// Motor A pins (LEFT)
#define IN1 16  // PWM
#define IN2 17  // Direction

// Motor B pins (RIGHT)
#define IN3 18  // Direction
#define IN4 19  // PWM

uint16_t right_speed;
uint16_t left_speed;
uint16_t speed = 30000;  // Start at 50% duty cycle

static int tight_turn = 4;

static int old_com = 40;
static int new_com;
void setup_pwm(uint gpio_pin, uint16_t duty) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin); // Get PWM slice number
    float div = 50; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = 60000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
}

void update_speed(uint gpio_pwm, uint16_t new_speed) {
    uint slice = pwm_gpio_to_slice_num(gpio_pwm);
    uint channel = pwm_gpio_to_channel(gpio_pwm);
    pwm_set_chan_level(slice, channel, new_speed);
}

int main()
{
    stdio_init_all();

    // Setup direction pins
    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_put(IN2, 0);  // Forward

    gpio_init(IN3);
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_put(IN3, 0);  // Forward

    init_camera_pins();
    setup_pwm(IN1, speed);
    setup_pwm(IN4, speed);
    while (true) {
        // uncomment these and printImage() when testing with python 
        //char m[10];
        //scanf("%s",m);

        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        int com = findLine(50); // calculate the position of the center of the ine
        // setPixel(IMAGESIZEY/2,com,0,255,0); // draw the center so you can see it in python
        // printImage();
        // printf("%d\r\n",com); // comment this when testing with python

        //if (abs(new_com-old_com) > 2) {
            int error = com - 40;
            int turn_amount = error * 1;

            int left_speed = (25 + turn_amount) * 600;
            int right_speed = (25 - turn_amount) * 600;

            update_speed(IN1, left_speed);
            update_speed(IN4, right_speed);

            sleep_ms(150);
            update_speed(IN1, 0);
            update_speed(IN4, 0);
        //}
        /*
        else if (abs(new_com-old_com) < 2) {
            int error = new_com - 40;
            int turn_amount = error * 0.4;

            int left_speed = (25 + turn_amount) * 600;
            int right_speed = (25 - turn_amount) * 600;

            update_speed(IN1, left_speed);
            update_speed(IN4, right_speed);

            sleep_ms(300);
            update_speed(IN1, 0);
            update_speed(IN4, 0);
        }
        
        old_com = new_com;
        */
    }
}

