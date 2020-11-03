#include "pwm1.h"

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

/*================================Initialize PWM1(Single Edges)=========================================
*@brief:    Initialize Clock Frequency + Turn ON Peripheral PWM
*@para:     Frequency (Hertz)
*@return:   No Return
*@Note:     Using Match Control Register(MRC) + MR0 to set The Frequency
            * set MCR[1] = 1 --> Reset PWM1 when Match MR0
            * set TCR[0] = 1 + TCR[3] = set Counter Range
            * set PCR[9] = 0x3F to turn on Ouput
========================================================================================================*/
void pwm1__init_single_edge(uint32_t frequency_in_hertz) {
  const uint32_t pwm_channel_output_enable_mask = 0x3F;
  const uint32_t default_frequency_in_hertz = 50;
  uint32_t valid_frequency_in_hertz = frequency_in_hertz;

  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__PWM1);

  if (0 == frequency_in_hertz) {
    valid_frequency_in_hertz = default_frequency_in_hertz;
  }
  const uint32_t match_reg_value = (clock__get_peripheral_clock_hz() / valid_frequency_in_hertz);

  /*
        MR0 holds the value that Timer Counter should count upto
        This will get us the desired PWM pulses per second
        Ex: If CPU freq = 10Hz, desired frequency = 2Hz
        MR0 = 10/2 = 5. This means, TC will count upto 5 per pulse, generating 2 pulse per second
  */
  if (match_reg_value > 0) {
    LPC_PWM1->MR0 = (match_reg_value - 1);
  }
  /* Enable PWM reset when it matches MR0 */
  LPC_PWM1->MCR |= (1 << 1);
  /* Enable PWM counter */
  LPC_PWM1->TCR = (1 << 0) | (1 << 3);
  /* Enable the PWM (bits 9-14) */
  LPC_PWM1->PCR |= (pwm_channel_output_enable_mask << 9);
}

/*=================================Set Duty Cycle PWM1(Single Edges)=====================================
*@brief:    Set Duty Cycle for PWM1
*@para:     PWM1 Channel
            Duty Cycle (percent 1--100%)
*@return:   No Return
*@Note:     Using Match Control Register(MRC) + MR0 to set The Frequency
            * set MR1 = P2_0; MR2 = P2_1; MR3 = P2_2 ; MR5 = P2_4; MR6 = P2_5;
            * set LER = set Latch Register (Enable MR0-->MR6)
========================================================================================================*/
void pwm1__set_duty_cycle(pwm1_channel_e pwm1_channel, float duty_cycle_in_percent) {
  const uint32_t mr0_reg_val = LPC_PWM1->MR0;
  const uint32_t match_reg_value = (mr0_reg_val * duty_cycle_in_percent) / 100;

  switch (pwm1_channel) {
  case PWM1__2_0:
    LPC_PWM1->MR1 = match_reg_value;
    break;
  case PWM1__2_1:
    LPC_PWM1->MR2 = match_reg_value;
    break;
  case PWM1__2_2:
    LPC_PWM1->MR3 = match_reg_value;
    break;
  case PWM1__2_4:
    LPC_PWM1->MR5 = match_reg_value;
    break;
  case PWM1__2_5:
    LPC_PWM1->MR6 = match_reg_value;
    break;
  default:
    break;
  }

  LPC_PWM1->LER |= (1 << (pwm1_channel + 1)); ///< Enable Latch Register
}

/*=============================Set PWM Function  IO PIN PWM1(Single Edges)===============================
*@brief:    Set PWM Function for IO pin
*@para:     Port
            Pin
*@return:   No Return { P2_0, P2_1, P2_2, P2_4,P2_5 }
*@Note:     Using The  gpio__construct_with_function() in "gpio.h"
            Only Supported:
========================================================================================================*/
void pin_configure_pwm_channel_as_io_pin(uint8_t port, uint8_t pin) {
  /* MODE 1: 001 ; { P2_0, P2_1, P2_2, P2_4,P2_5 } */
  gpio__construct_with_function(port, pin, 1);
}
