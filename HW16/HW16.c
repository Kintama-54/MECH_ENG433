#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

// Motor A pins
#define IN1 16  // PWM
#define IN2 17  // Direction

// Motor B pins
#define IN3 18  // Direction
#define IN4 19  // PWM

#define SPEED_STEP 5000
#define MAX_SPEED  65535
#define MIN_SPEED  0

uint16_t speed = 50000;  // Start at 50% duty cycle

void setup_pwm(uint gpio_pin, uint16_t duty) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio_pin);
    uint channel = pwm_gpio_to_channel(gpio_pin);
    pwm_set_wrap(slice, 65535);
    pwm_set_chan_level(slice, channel, duty);
    pwm_set_enabled(slice, true);
}

void update_speed(uint gpio_pwm, uint16_t new_speed) {
    uint slice = pwm_gpio_to_slice_num(gpio_pwm);
    uint channel = pwm_gpio_to_channel(gpio_pwm);
    pwm_set_chan_level(slice, channel, new_speed);
}

int main() {
    stdio_init_all();

    // Setup direction pins
    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_put(IN2, 0);  // Forward

    gpio_init(IN3);
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_put(IN3, 0);  // Forward

    // Setup PWM on IN1 and IN4
    setup_pwm(IN1, speed);
    setup_pwm(IN4, speed-1637);

    printf("Use '+' to increase speed, '-' to decrease.\n");

    while (true) {
        int c = getchar_timeout_us(0);  // Non-blocking input
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '+') {
                if (speed + SPEED_STEP <= MAX_SPEED)
                    speed += SPEED_STEP;
            } else if (c == '-') {
                if (speed >= SPEED_STEP)
                    speed -= SPEED_STEP;
            }

            // Apply updated speed to both motors
            update_speed(IN1, speed);
            update_speed(IN4, speed);

            printf("Speed: %u\n", speed);
        }

        sleep_ms(10);  // Prevent CPU hogging
    }

    return 0;
}
