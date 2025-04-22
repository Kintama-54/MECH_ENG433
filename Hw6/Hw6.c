#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPI14 (SDA) and GPI15 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13
#define HEARTBEAT_LED 25
#define ADDR 0b0100000 // since A2,1,0 are 0

void setPin(uint8_t chip_adr, uint8_t reg, uint8_t value);
uint8_t readPin(uint8_t register);
void heartbeat();

void heartbeat(){
    gpio_put(HEARTBEAT_LED, true);
    sleep_ms(300);
    gpio_put(HEARTBEAT_LED, false);
    sleep_ms(700);
}

void setPin(uint8_t chip_adr, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    i2c_write_blocking(I2C_PORT, chip_adr, data, 2, false);
}

uint8_t readPin(uint8_t reg) {
    uint8_t result;

    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);  // true to keep master control of bus
    i2c_read_blocking(I2C_PORT, ADDR, &result, 1, false);  // false - finished with bus
    return result;
}

int main()
{
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\r\n");

    // Initialise Heartbeat
    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);

    printf("Heartbeat Initialised\r\n");

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    printf("Pins Initialised\r\n");

    // Initialise Pins on MCP
   setPin(ADDR, 0x00, 0b00000001);
   printf("MCP Initialised");

while (true) {
        heartbeat(); // heartbeat GP25 testing

        uint8_t results = readPin(0x09);
        int push = results & 0b1;

        if (push) {
            setPin(ADDR, 0x0A, 0b10000000);
        }
        else {
            setPin(ADDR, 0x0A, 0b00000000);
        }
    }
}
