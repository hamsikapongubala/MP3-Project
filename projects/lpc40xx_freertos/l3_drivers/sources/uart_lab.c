#include "uart_lab.h"
#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
// Private queue handle of our uart_lab.c
static QueueHandle_t your_uart_rx_queue;

// Private function of our uart_lab.c
static void your_receive_interrupt(void) {

  uint16_t status = LPC_UART3->IIR & 0xE;

  if (status & (4 << 0)) {
    // fprintf(stderr, "STATUS: DATA AVAILABLE IN QUEUE\n");

    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }

    const char byte = LPC_UART3->RBR;

    xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  }
}

void uart__enable_receive_interrupt(uart_number_e uart_number) {

  NVIC_EnableIRQ(UART3_IRQn);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, your_receive_interrupt, "UART_GPIO");

  if (uart_number == UART_3) {
    LPC_UART3->LCR &= ~(1 << 7);
    LPC_UART3->IER = (1 << 0);
  }

  else if (uart_number == UART_2) {
    LPC_UART3->LCR &= ~(1 << 7);
    LPC_UART2->IER = (1 << 0);
  }

  your_uart_rx_queue = xQueueCreate(2, sizeof(char));
}

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  /* Baud = PCLK / (16 * (DMM* 256 + DLL))
   * (DMM* 256 + DLL) = PCLK/ (16* Baud)
   */
  const float fraction = 0.5;
  peripheral_clock = peripheral_clock * 1000 * 1000;
  uint16_t dlm_dll = (peripheral_clock / (16 * baud_rate) + fraction);

  if (uart == UART_2) {
    LPC_SC->PCONP |= (1 << 24); // SET PCUART2

    LPC_UART2->LCR |= (1 << 7) | (3 << 0); // ENABLE DLAB
    LPC_UART2->FCR |= (1 << 0);
    LPC_UART2->DLM = (dlm_dll >> 8) & 0xFF;
    LPC_UART2->DLL = (dlm_dll >> 0) & 0xFF;

    LPC_UART2->LCR &= ~(1 << 7); // RESET DLAB

  } else if (uart == UART_3) {
    LPC_SC->PCONP |= (1 << 25); // SET PCUART3

    LPC_UART3->LCR |= (1 << 7) | (3 << 0); // ENABLE DLAB

    LPC_UART3->FCR = (1 << 0); // ENABLE FIFO
    LPC_UART3->DLM = (dlm_dll >> 8) & 0xFF;
    LPC_UART3->DLL = (dlm_dll >> 0) & 0xFF;

    LPC_UART3->LCR &= ~(1 << 7); // RESET DLAB
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {

  if (uart == UART_2) {

    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }

    *input_byte = LPC_UART2->RBR;

  } else if (uart == UART_3) {

    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }

    *input_byte = LPC_UART3->RBR;
  }
  return *input_byte != 0 ? true : false;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {

  if (uart == UART_2) {

    while (!(LPC_UART2->LSR & (1 << 5))) {
      ;
    }

    LPC_UART2->THR = output_byte;

  } else if (uart == UART_3) {

    while (!(LPC_UART3->LSR & (1 << 5))) {
      ;
    }

    LPC_UART3->THR = output_byte;
  }

  return output_byte != 0 ? true : false;
}