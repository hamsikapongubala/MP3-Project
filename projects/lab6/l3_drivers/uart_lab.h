#pragma once
#include "FreeRTOS.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* ------------------------------- Declaration ------------------------------ */
/* ------------------------------------ x ----------------------------------- */

typedef enum {
  UART_2,
  UART_3,
} uart_number_e;

static QueueHandle_t uart_rx_queue;

/* ----------------------------- Public Function ---------------------------- */
/* ------------------------------------ F ----------------------------------- */

/* Initialize ( UART Port + CLOCK + Baud Rate ) */
void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock_mhz, uint32_t baud_rate);

/* Receive Input data from RX line  */
bool uart_lab__receive(uart_number_e uart, char *input_byte);

/* Transmit Output data from TX line */
bool uart_lab__transmit(uart_number_e uart, char output_byte);

/* Checking UART3 INTR ID --> Copy to Queue from ISR */
static void uart3_RX_INTR(void);

/* Checking UART2 INTR ID --> Copy to Queue from ISR */
static void uart2_RX_INTR(void);

/* Enable INTR for RX Buffer */
void uart__enable_RX_INTR(uart_number_e uart, int queue_size);

/* Get data from Queue */
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout);