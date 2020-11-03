// @file gpio_isr.h
#pragma once

#include "lpc40xx.h"
#include <stdint.h>
#include <stdio.h>

/*Falling=0 --- Rising = 1*/
typedef enum {
  GPIO_INTR__FALLING_EDGE, // 0
  GPIO_INTR__RISING_EDGE,  // 1
} gpio_interrupt_e;

/*---------Function pointer----*/
typedef void (*function_pointer_t)(void);

/***********************************************************************************************
        Two main things ( Save the CallBack_function to INTR table + Config INTR)
************************************************************************************************/

/* Allow the user to setup + attach their callbacks function Port 0 */
void gpio0__attach_interrupt(uint8_t pin_num, gpio_interrupt_e interrupt_type, function_pointer_t callback);
/* Allow the user to setup + attach their callbacks function Port 2 */
void gpio2__attach_interrupt(uint8_t pin_num, gpio_interrupt_e interrupt_type, function_pointer_t callback);

/***********************************************************************************************
        Two main things ( check the interrupt Pin + Map it to interrupt vector table in ISR)
        Our main() should configure interrupts to invoke this dispatcher
        You can hijack 'interrupt_vector_table.c' or use API at lpc_peripherals.h
************************************************************************************************/

/* Dispatch the specific function in Interrupt Service Routine (ISR)--Port 0 */
void gpio0__interrupt_dispatcher(void);
/* Dispatch the specific function in Interrupt Service Routine (ISR)--Port 2 */
void gpio2__interrupt_dispatcher(void);

/* An unregistered interrupt handler is a forever loop */
static void lpc_peripheral__halt_handler(void) {
  while (1) { /* Nothing*/
  }
}
