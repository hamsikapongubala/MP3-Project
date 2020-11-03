/*
 * Pulse Width Modulation (PWM) driver
 */
#pragma once

#include "gpio.h"
#include <stdint.h>

/* Only the below PWM1 channels are available on SJ2 board */
typedef enum {
  PWM1__2_0 = 0, ///< Controls P2.0
  PWM1__2_1 = 1, ///< Controls P2.1
  PWM1__2_2 = 2, ///< Controls P2.2
  PWM1__2_4 = 4, ///< Controls P2.4
  PWM1__2_5 = 5, ///< Controls P2.5
} pwm1_channel_e;

/***********************************NOTE*************************************
 *     For simple single edge PWM, all channels share the same frequency
 *****************************************************************************/

/* Initialize Clock Frequency + Turn ON Peripheral PWM   */
void pwm1__init_single_edge(uint32_t frequency_in_hertz);

/* Convert Match Register to Duty Cycle + PWM1 Channel  */
void pwm1__set_duty_cycle(pwm1_channel_e pwm1_channel, float duty_cycle_in_percent);

/* Set I0CON Register to perform PWM on physical PIN  */
void pin_configure_pwm_channel_as_io_pin(uint8_t port, uint8_t pin);
