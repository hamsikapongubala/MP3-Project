#include "lpc40xx.h"
#include <stdbool.h>
#include <stdint.h>

/// Should alter the hardware registers to set the pin as input
void gpio0__set_as_input(uint8_t pin_num) { LPC_GPIO1->DIR &= ~(1U << pin_num); }

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(uint8_t pin_num) { LPC_GPIO1->DIR |= (1U << pin_num); }

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(uint8_t pin_num) {
  gpio0__set_as_output(pin_num);
  LPC_GPIO1->SET = (1U << pin_num);
}

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(uint8_t pin_num) {
  gpio0__set_as_output(pin_num);
  LPC_GPIO1->CLR = (1U << pin_num);
}

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio0__set(uint8_t pin_num, bool high) {
  gpio0__set_as_output(pin_num);
  if (high) {
    gpio0__set_high(pin_num);
  } else {
    gpio0__set_low(pin_num);
  }
}

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio0__get_level(uint8_t pin_num) {
  gpio0__set_as_input(pin_num);
  if (LPC_GPIO1->PIN & (1U << pin_num))
    return true;
  return false;
}