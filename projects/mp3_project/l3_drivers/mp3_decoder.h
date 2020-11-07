#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <gpio.h>
#include <stdint.h>
#include <stdio.h>

// init pins
void spi__init(uint32_t max_clock_mhz);

// write function to decoder
uint8_t spi__write(uint8_t data_out);
