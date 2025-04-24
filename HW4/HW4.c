#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// Initialise Sine wave and triangle wave 
static float sine_wave[100];
static float triangle_wave[100];

void writeDac(int channel, float voltage);

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void writeDAC(int channel, float voltage) { //channel = 1 is OutB and 0 is outA
    // Ensures no negative or overly large numbers.
    if (voltage < 0) {
        voltage = 0;
    }
    if (voltage > 3.3) {
        voltage = 3.3;
    }
    
    uint16_t voltage_bits = (uint16_t) ((voltage/3.3) * 1023.0); // Makes voltage a number between 0 and 1023
    uint8_t data[2] ; // Initialise data bits to writeDAC
    int len = 2;

    //bitshifting
    data[0] = ((channel & 0x01) << 7) | (0b111 << 4) | ((voltage_bits >> 6) & 0x0F);
    data[1] = ((voltage_bits & 0x3F) << 2);
    
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS);
}

void makeSineWave() {
    float radian = 0;
    for (int i=0;i<100;i++){
        sine_wave[i] = 1.65 * sin(radian) + 1.65; // Centers the sine wave on 1.65 with amplitude 1.65, making it always positive
        radian += (2*3.14159265359) / 100; // Makes two periods in one cycle
    }
}

void makeTriangleWave() { // Creates triangle wave
    int i = 1;
    triangle_wave[0] = 1.65;
    for (int i = 1; i < 100; i++)
    {
        if (i < 25) {
            triangle_wave[i] = triangle_wave[i-1] + (1.65/25);
        }
        else if (i > 24 && i < 75) {
            triangle_wave[i] = triangle_wave[i-1] - (3.3/50);
        }
        else if (i > 74 && i < 100) {
            triangle_wave[i] = triangle_wave[i-1] + (1.65/25);
        }
    }
}

int main()
{
    stdio_init_all();
    makeSineWave();
    makeTriangleWave();
    
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    while (true) {
        for (int i = 0; i < 100; i++){ // One loop per second
            writeDAC(0,sine_wave[i]); // Writes the sine wave
            printf("Sine Value at %d: %.3f\r\n",i,sine_wave[i]); //debugging
            writeDAC(1,triangle_wave[i]); // Writes the triangle wave 
            printf("Triangle Value at %d: %.3f\r\n",i,triangle_wave[i]); // debugging
            sleep_ms(10);
        }
    }
}
