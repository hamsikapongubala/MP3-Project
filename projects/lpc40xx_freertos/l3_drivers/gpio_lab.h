// file gpio_lab.h
#pragma once

#include <stdbool.h>
#include <stdint.h>

// NOTE: The IOCON is not part of this driver

/// Should alter the hardware registers to set the pin as input
void gpio0__set_as_input(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(uint8_t pin_num);

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio0__set(uint8_t pin_num, bool high);

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio0__get_level(uint8_t pin_num);