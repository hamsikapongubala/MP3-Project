#include "FreeRTOS.h"
#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"

#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"

#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "ssp2_lab.h"
#include "task.h"
#include <stdio.h>

#include "string.h"
#include "uart_lab.h"

/***************************************UART Task (Part 1)***************************************
 *@brief: Checking UART basic driver UART (Tx-P4_28; Rx-P4_29)
 *@Note:  Require Jumper cable form P4_28 to P4-29
 ************************************************************************************************/

/* ---------------------------------- SEND ---------------------------------- */

void uart_write_task(void *p) {
  char temp0 = 'H';
  char temp1 = 'E';
  char temp2 = 'L';
  char temp3 = 'P';

  while (1) {
    /* send a value */
    uart_lab__transmit(UART_3, temp0);
    // uart_lab__transmit(UART_3, temp1);
    // uart_lab__transmit(UART_3, temp2);
    // uart_lab__transmit(UART_3, temp3);

    vTaskDelay(500);
  }
}

/* --------------------------------- RECEIVE -------------------------------- */

void uart_read_task(void *p) {

  while (1) {
    char *receive;
    uart_lab__receive(UART_3, &receive);
    fprintf(stderr, "Task Receive:  %c\n", receive);
    vTaskDelay(500);
  }
}

/***************************************UART Task (Part 2)***************************************
 *@brief: Using Interupt at RX buffer + Queue to transmit data
 *@Note:  Require enable INTR UART in main() + Check the UART for more info
 ************************************************************************************************/

/* ----------------------------------- RECEIVE ---------------------------------- */

void uart_read_fromISR(void *p) {
  while (1) {
    char *receive;
    // printf("Before checking into ISR\n");
    if (uart_lab__get_char_from_queue(&receive, portMAX_DELAY)) {
      printf("Queue received: %c \n ", receive);
    }
  }
}

/* ---------------------------------- SEND ---------------------------------- */

void uart_write_toISR(void *p) {
  while (1) {
    uart_lab__transmit(UART_3, 'B');
    uart_lab__transmit(UART_3, 'A');
    uart_lab__transmit(UART_3, 'N');
    uart_lab__transmit(UART_3, 'G');
    uart_lab__transmit(UART_3, '#');
    vTaskDelay(500);
  }
}

/*********************************UART Task  Extra credit(Part 3)*********************************
 *@brief: Convert Number to Char Array[] sent them (UART2-UART LoopBack)
 *@Note:  Generate random 3 digit number
          Convert each Digit to Char Array[]
          Send 1 char / per time
 ************************************************************************************************/

/* ------------------------------- UART2 SEND ------------------------------- */

void Extra_sender_task(void *p) {
  char number_as_string[16] = {0};

  while (true) {
    const int number = rand() % 1000; // 0-999
    fprintf(stderr, "Random num: %d \n", number);

    /* Convert number to char array */
    sprintf(number_as_string, "%i", number);

    /* UART2 SEND 1_char/1_time + TERMINATE NULL char */
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__transmit(UART_2, number_as_string[i]);
    }
    fprintf(stderr, "UART2 Sent: %i \n", number);
    vTaskDelay(3500);
  }
}

/* ------------------------------ UART3 RECEIVE ----------------------------- */

void Extra_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    /* Iteration 1-by-1 'char' */
    fprintf(stderr, "Received: %c\n", byte);

    /* End of String (last char)  */
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("UART3 Received: %s \n\n\n", number_as_string);
    }
    /* Continuously, buffer the data */
    else {
      number_as_string[counter] = byte;
      printf("Loading Arr[%i]: %s \n", counter, number_as_string);
      counter++;
    }
  }
}

/***************************************** MAIN LOOP *********************************************
**************************************************************************************************/
int main(void) {

  /* ----------------------------- Initialization ----------------------------- */
  puts("Starting RTOS");

  /* --------------------------------- Part 1 --------------------------------- */
  uart_lab__init(UART_3, 96, 115200);
  gpio__construct_with_function(4, 28, GPIO__FUNCTION_2);
  gpio__construct_with_function(4, 29, GPIO__FUNCTION_2);
  xTaskCreate(uart_write_task, "Write", 1024, NULL, 2, NULL);
  xTaskCreate(uart_read_task, "Read", 1024, NULL, 1, NULL);

  /* --------------------------------- Part 2 --------------------------------- */
  // uart_lab__init(UART_3, 96, 115200);
  // uart__enable_RX_INTR(UART_3, 5);
  // xTaskCreate(uart_read_fromISR, "Read ISR", 1024, NULL, 1, NULL);
  // xTaskCreate(uart_write_toISR, "Write ISR", 1024, NULL, 1, NULL);

  /* --------------------------------- Part 3 --------------------------------- */
  // uart_lab__init(UART_3, 96, 115200);
  // uart_lab__init(UART_2, 96, 115200);
  // uart__enable_RX_INTR(UART_3, 5);
  // xTaskCreate(Extra_receiver_task, "Read UART2", 1024, NULL, 1, NULL);
  // xTaskCreate(Extra_sender_task, "Write UART3", 1024, NULL, 2, NULL);

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
