#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <gpio.h>
#include <stdint.h>
#include <stdio.h>

void ssp2__init(uint32_t max_clock_mhz) {

  LPC_SC->PCONP |= (1 << 20); // PCONP set to SSP2

  LPC_SSP2->CR0 = (7 << 0); // set control register CR0

  LPC_SSP2->CR1 = (1 << 1); // set control register CR1

  uint8_t divider = 2;
  const uint32_t cpu_clock_mhz = clock__get_core_clock_hz();

  while (max_clock_mhz < (cpu_clock_mhz / divider) && divider <= 254) {
    divider += 2;
  }

  LPC_SSP2->CPSR = divider;
}

uint8_t ssp2__exchange_byte_lab(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP2->DR = data_out;

  while (LPC_SSP2->SR & (1 << 4))
    ;

  return LPC_SSP2->DR;
}