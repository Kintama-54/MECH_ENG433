#include <stdio.h>
#include "pico/stdlib.h"


int main()
{
    stdio_init_all();
    volatile float f1, f2;
    printf("Enter two floats to use: ");
    scanf("%f %f", &f1, &f2);
    volatile float f_add, f_sub, f_mult, f_div;
    
    absolute_time_t t1 = get_absolute_time();
    uint64_t t_1 = to_us_since_boot(t1);
    for (int i = 0; i < 1000; i++){
        f_add = f1 + f2;
    }
    absolute_time_t t2 = get_absolute_time();
    uint64_t t_2 = to_us_since_boot(t2);
    for (int i = 0; i < 1000; i++){
        f_sub = f1 - f2;
    }
    absolute_time_t t3 = get_absolute_time();
    uint64_t t_3 = to_us_since_boot(t3);
    for (int i = 0; i < 1000; i++){
        f_mult = f1 * f2;
    }
    absolute_time_t t4 = get_absolute_time();
    uint64_t t_4 = to_us_since_boot(t4);
    for (int i = 0; i < 1000; i++){
        f_div = f1 / f2;
    }
    absolute_time_t t5 = get_absolute_time();
    uint64_t t_5 = to_us_since_boot(t5);

    uint64_t t_add_cycles = ((t_2 - t_1) / 1000.0) / (1.0 / 150.0);
    uint64_t t_sub_cycles = ((t_3 - t_2) / 1000.0) / (1.0 / 150.0);
    uint64_t t_mult_cycles= ((t_4 - t_3) / 1000.0) / (1.0 / 150.0);
    uint64_t t_div_cycles = ((t_5 - t_4) / 1000.0) / (1.0 / 150.0);

    printf("Addition Clock Cycles per operation: %llu\n", t_add_cycles);
    printf("Subtraction Clock Cycles per operation: %llu\n", t_sub_cycles);
    printf("Multiplication Clock Cycles per operation: %llu\n", t_mult_cycles);
    printf("Division Clock Cycles per operation: %llu\n", t_div_cycles);
    
}
