#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "acceleration.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "i2c_slave_init.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "string.h"


void led_checker(void) {
  const uint8_t led = (1 << 3);
  LPC_GPIO2->DIR |= led;
  while (1) {
    if (slave_memory[0] == 1) {
      LPC_GPIO2->CLR = led; // turn on LED
      fprintf(stderr, "LED ON\n");
    } else {
      LPC_GPIO2->SET = led; // turn off LED
      fprintf(stderr, "LED OFF\n");
    }
    vTaskDelay(100);
  }
}

int main(void) {

  LPC_IOCON->P0_1 |= (1 << 10); // I2C2_SDA
  LPC_IOCON->P0_0 |= (1 << 10); // I2C2_SCL

  gpio__construct_with_function(0, 0, 3);
  gpio__construct_with_function(0, 1, 3);

  i2c__initialize(I2C__1, 9600, 96000000);

  i2c2__slave_init(I2C__1, 0x86);

  // xTaskCreate(led_checker, "led_checker", 1024 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  xTaskCreate(sj2_cli__init, "sj2_cli", 1024 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
