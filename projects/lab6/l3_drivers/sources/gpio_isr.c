// @file gpio_isr.c
#include "gpio_isr.h"
#include "delay.h"
#include "gpio_lab.h"
#include "lpc_peripherals.h"
//-----> Note: You may want another separate array for falling vs. rising edge callbacks
// I can do it but, I wont because, the Instruction said " You might want to......"--> not requirement
// So I try to keep my code simple.
//( Please let me know if it is a requirement, I can total modify code in live stream meeting)

/*-----Interupt vector table Port 1 + Port 2-------------*/
static function_pointer_t gpio0_callbacks[64] = {

    lpc_peripheral__halt_handler, // Pin 0
    lpc_peripheral__halt_handler, // Pin 1
    lpc_peripheral__halt_handler, // Pin 2
    lpc_peripheral__halt_handler, // Pin 3
    lpc_peripheral__halt_handler, // Pin 4
    lpc_peripheral__halt_handler, // Pin 5
    lpc_peripheral__halt_handler, // Pin 6
    lpc_peripheral__halt_handler, // Pin 7

    lpc_peripheral__halt_handler, // Pin 8
    lpc_peripheral__halt_handler, // Pin 9
    lpc_peripheral__halt_handler, // Pin 10
    lpc_peripheral__halt_handler, // Pin 11
    lpc_peripheral__halt_handler, // Pin 12
    lpc_peripheral__halt_handler, // Pin 13
    lpc_peripheral__halt_handler, // Pin 14
    lpc_peripheral__halt_handler, // pin 15

    lpc_peripheral__halt_handler, // Pin 16
    lpc_peripheral__halt_handler, // Pin 17
    lpc_peripheral__halt_handler, // Pin 18
    lpc_peripheral__halt_handler, // Pin 19
    lpc_peripheral__halt_handler, // Pin 20
    lpc_peripheral__halt_handler, // Pin 21
    lpc_peripheral__halt_handler, // Pin 22
    lpc_peripheral__halt_handler, // Pin 23

    lpc_peripheral__halt_handler, // pin 24
    lpc_peripheral__halt_handler, // Pin 25
    lpc_peripheral__halt_handler, // Pin 26
    lpc_peripheral__halt_handler, // Pin 27
    lpc_peripheral__halt_handler, // Pin 28
    lpc_peripheral__halt_handler, // Pin 29 switch 2
    lpc_peripheral__halt_handler, // Pin 30 Switch 3
    lpc_peripheral__halt_handler, // Pin 31

    /*========================= Port 2: Interupt Array ======================*/

    lpc_peripheral__halt_handler, // Pin 0; Port 2 start at index[32]
    lpc_peripheral__halt_handler, // Pin 1
    lpc_peripheral__halt_handler, // Pin 2
    lpc_peripheral__halt_handler, // Pin 3
    lpc_peripheral__halt_handler, // Pin 4
    lpc_peripheral__halt_handler, // Pin 5
    lpc_peripheral__halt_handler, // Pin 6
    lpc_peripheral__halt_handler, // Pin 7

    lpc_peripheral__halt_handler, // Pin 8
    lpc_peripheral__halt_handler, // Pin 9
    lpc_peripheral__halt_handler, // Pin 10
    lpc_peripheral__halt_handler, // Pin 11
    lpc_peripheral__halt_handler, // Pin 12
    lpc_peripheral__halt_handler, // Pin 13
    lpc_peripheral__halt_handler, // Pin 14
    lpc_peripheral__halt_handler, // pin 15

    lpc_peripheral__halt_handler, // Pin 16
    lpc_peripheral__halt_handler, // Pin 17
    lpc_peripheral__halt_handler, // Pin 18
    lpc_peripheral__halt_handler, // Pin 19
    lpc_peripheral__halt_handler, // Pin 20
    lpc_peripheral__halt_handler, // Pin 21
    lpc_peripheral__halt_handler, // Pin 22
    lpc_peripheral__halt_handler, // Pin 23

    lpc_peripheral__halt_handler, // pin 24
    lpc_peripheral__halt_handler, // Pin 25
    lpc_peripheral__halt_handler, // Pin 26
    lpc_peripheral__halt_handler, // Pin 27
    lpc_peripheral__halt_handler, // Pin 28
    lpc_peripheral__halt_handler, // Pin 29
    lpc_peripheral__halt_handler, // Pin 30
    lpc_peripheral__halt_handler, // Pin 31; Port 2 End at index[63]
};

/*===============================gpio0__attach_Interrupt (Port 0)========================================
*@brief:    Save the CallBack_function add to INTR table + Config INTR for port 0
*@para:     Pin_num
            Interrupt_type ( Rising or Falling)
            Callback (function address)
*@return:   No Return
*@Note:     Set Pin as ( Input + PullDown + Rising or Falling + NVIC_EnableIQR )
            LPC_GPIOINT ( IO0IntEnR or IO0IntEnF )
            Require PullDown -->but It's been config as external resistor base on schematic
========================================================================================================*/
void gpio0__attach_interrupt(uint8_t pin_num, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  if (interrupt_type) {

    /*Set Pin as Input*/
    gpio1__set_as_input(0, pin_num);
    /*Enable Rising INTR + NVIC enable*/
    LPC_GPIOINT->IO0IntEnR |= (1 << pin_num);
    NVIC_EnableIRQ(GPIO_IRQn);
    /*Store the callback based on the pin at gpio0_callbacks*/
    gpio0_callbacks[pin_num] = callback;

  } else {

    /*Set Pin as Input*/
    gpio1__set_as_input(0, pin_num);
    /*Enable Falling INTR + NVIC enable*/
    LPC_GPIOINT->IO0IntEnF |= (1 << pin_num);
    NVIC_EnableIRQ(GPIO_IRQn);
    /*Store the callback based on the pin at gpio0_callbacks*/
    gpio0_callbacks[pin_num] = callback;
  }
}

/*===============================gpio2__attach_Interrupt (Port 2)==========================================
*@brief:    Save the CallBack_function add to INTR table + Config INTR for port 2
*@para:     Pin_num
            Interrupt_type ( Rising or Falling)
            Callback (function address)
*@return:   No Return
*@Note:     Set Pin as ( Input + PullDown + Rising or Falling + NVIC_EnableIQR )
            LPC_GPIOINT ( IO2IntEnR or IO2IntEnF )
            Table for Port 2: START at index[32] -- END at index[63]
===========================================================================================================*/
void gpio2__attach_interrupt(uint8_t pin_num, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  if (interrupt_type) {

    /*Set Pin as Input*/
    gpio1__set_as_input(2, pin_num);
    /*Enable Rising INTR + NVIC enable*/
    LPC_GPIOINT->IO2IntEnR |= (1 << pin_num);
    NVIC_EnableIRQ(GPIO_IRQn);
    /*Store the callback based on the pin at gpio0_callbacks*/
    gpio0_callbacks[pin_num + 32] = callback;

  } else {

    /*Set Pin as Input*/
    gpio1__set_as_input(2, pin_num);
    /*Enable Falling INTR + NVIC enable*/
    LPC_GPIOINT->IO2IntEnF |= (1 << pin_num);
    NVIC_EnableIRQ(GPIO_IRQn);
    /*Store the callback based on the pin at gpio0_callbacks*/
    gpio0_callbacks[pin_num + 32] = callback; // START at index[32]
  }
}

/*===============================gpio0__interrupt_dispatcher (Port 0)======================================
*@brief:    Dispatch the specific function in Interrupt Service Routine (ISR)
*@para:     No parameter
*@return:   No Return
*@Note:     Two main things ( check the interrupt Pin + Map it to interrupt vector table in ISR, then clear)
            LPC_GPIOINT->status ( IO0IntStatR or IO0IntStatF )
            LPC_GPIOINT->clear ( IO0IntClr )
===========================================================================================================*/
void gpio0__interrupt_dispatcher(void) {
  /* Check Pin interrupt in Port 0 */
  // fprintf(stderr, "Dispatch \n");
  int pin_that_generated_interrupt = 0;
  for (int i = 0; i <= 31; i++) {
    if ((LPC_GPIOINT->IO0IntStatF & (1 << i)) || (LPC_GPIOINT->IO0IntStatR & (1 << i))) {
      pin_that_generated_interrupt = i;
      break;
    };
  }
  /* Assign Function Address from CallBack Table base on Pin Number */
  function_pointer_t attached_user_function = gpio0_callbacks[pin_that_generated_interrupt];
  /* Invoke or CallBack function */
  attached_user_function();
  /* Clear the interrupt after done ISR */
  LPC_GPIOINT->IO0IntClr |= (1 << pin_that_generated_interrupt);
  delay__ms(1);
}

/*===============================gpio2__interrupt_dispatcher (Port 2)======================================
*@brief:    Dispatch the specific function in Interrupt Service Routine (ISR)
*@para:     No parameter
*@return:   No Return
*@Note:     Two main things ( check the interrupt Pin + Map it to interrupt vector table in ISR, then clear)
            LPC_GPIOINT->status ( IO2IntStatR or IO2IntStatF )
            LPC_GPIOINT->clear ( IO2IntClr )
            Table for Port 2: START at index[32] -- END at index[63]
===========================================================================================================*/
void gpio2__interrupt_dispatcher(void) {
  /* Check Pin interrupt in Port 0 */
  // fprintf(stderr, "Dispatch \n");
  int pin_that_generated_interrupt = 0;
  for (int i = 0; i <= 31; i++) {
    if ((LPC_GPIOINT->IO2IntStatF & (1 << i)) || (LPC_GPIOINT->IO2IntStatR & (1 << i))) {
      pin_that_generated_interrupt = i;
      break;
    };
  }
  /* Assign Function Address from CallBack Table base on Pin Number */
  /* START at index[32] */
  function_pointer_t attached_user_function = gpio0_callbacks[pin_that_generated_interrupt + 32];
  /* Invoke or CallBack function */
  attached_user_function();
  /* Clear the interrupt after done ISR */
  LPC_GPIOINT->IO2IntClr |= (1 << pin_that_generated_interrupt);
  delay__ms(1);
}