// @file gpio_isr.c
#include "gpio_isr.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>
// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];
static function_pointer_t gpio0_callbacks_f[32];

void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {

  if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    gpio0_callbacks[(int)pin] = callback;
    LPC_GPIOINT->IO0IntEnR |= (1U << pin);
  } else {
    gpio0_callbacks_f[(int)pin] = callback;
    LPC_GPIOINT->IO0IntEnF |= (1U << pin);
  }
}

int pin_generate_interrupt(void) {
  for (int i = 0; i < 32; i++) {
    if (LPC_GPIOINT->IO0IntStatF & (1U << i)) {
      return i;
    } else if (LPC_GPIOINT->IO0IntStatR & (1U << i)) {
      return i;
    }
  }
  return -1;
}

void clear_pin_interrupt(int pin) { LPC_GPIOINT->IO0IntClr |= (1U << pin); }

// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  const int pin_that_generated_interrupt = pin_generate_interrupt();
  function_pointer_t attached_user_handler;

  if (gpio0_callbacks[pin_that_generated_interrupt]) {
    attached_user_handler = gpio0_callbacks[pin_that_generated_interrupt];

  } else {
    attached_user_handler = gpio0_callbacks_f[pin_that_generated_interrupt];
  }

  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}
