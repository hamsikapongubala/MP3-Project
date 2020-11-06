#include "uart_lab.h"
#include "lpc40xx.h"

/*======================================UART Setup(Baud Rate)======================================
*@brief:    Initialize ( UART Port + CLOCK + Baud Rate )
*@para:     Uart_number_e
            Peripheral_clock_mhz
            Baud_rate
*@return:   @No_return

*@Note:     Require DLAB[] enable
            LPC_SC->PCONP (Power)
            LPC_UART2->LCR (Line Control)
            LPC_UART2->FCR ( FiFo Control Rx + Tx)
            LPC_UART2->DLM (MSB) divider
            LPC_UART2->DLL (LSB) divider
===================================================================================================*/
void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock_mhz, uint32_t baud_rate) {
  const float fraction = 0.5;
  peripheral_clock_mhz = peripheral_clock_mhz * 1000 * 1000;
  uint16_t dlm_dll = (peripheral_clock_mhz / (16 * baud_rate) + fraction);

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

/*========================================UART Receive (RX)========================================
*@brief:    Receive Input data from RX line
*@para:     uart_number_e [in]
            *input_byte   [out]
*@return:   Buffer not empty -->True, Else -->False

*@Note:     DLAB must be Disable
            Using pointer to get result out
==================================================================================================*/
bool uart_lab__receive(uart_number_e uart, char *input_byte) {

  if (uart == UART_2) {
    /* Set DLAB = 0 --> Access RBR */
    LPC_UART2->LCR &= ~(1 << 7);

    /* Wait !!! */
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ; /* Line Status Register RDR[0]=0 */
    }
    /* Not Empty --> Receive  */
    *input_byte = LPC_UART2->RBR & 0xFF;
    return (*input_byte != 0) ? true : false;
  } else if (uart == UART_3) {
    /* Set DLAB = 0 --> Access RBR */
    LPC_UART3->LCR &= ~(1 << 7);

    /* Wait !!! */
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ; /* Line Status Register RDR[0]=0 */
    }
    /* Not Empty --> Receive  */
    *input_byte = LPC_UART3->RBR & 0xFF;
    return (*input_byte != 0) ? true : false;
  }
}

/*=======================================UART Transmit (TX)========================================
*@brief:    Transmit Output data from TX line
*@para:     uart_number_e [in]
            output_byte   [in]
*@return:   Buffer not empty -->True, Else -->False

*@Note:     DLAB must be Disable
==================================================================================================*/
bool uart_lab__transmit(uart_number_e uart, char output_byte) {

  if (uart == UART_2) {
    /* Set DLAB = 0 --> Access RBR */
    LPC_UART2->LCR &= ~(1 << 7);

    /* Wait !!! */
    while (!(LPC_UART2->LSR & (1 << 5))) {
      ; /* Line Status Register RDR[5]=0 */
    }
    /* Empty --> SEND */
    LPC_UART2->THR = output_byte;
    return (output_byte != 0) ? true : false;
  } else if (uart == UART_3) {
    /* Set DLAB = 0 --> Access RBR */
    LPC_UART3->LCR &= ~(1 << 7);

    /* Wait !!! */
    while (!(LPC_UART3->LSR & (1 << 5))) {
      ; /* Line Status Register RDR[5]=0 */
    }
    /* Empty --> SEND */
    LPC_UART3->THR = output_byte;
    return (output_byte != 0) ? true : false;
  }
}

/*====================================== UART RX Interrupt ========================================
*@brief:  Callback function to get data (RX buffer) while entering ISR
*@para:   No para
*@return: No return

*@Note:   Checking INTR ID + RX Buffer status
          Using Queue.h (RTOS)--> Copy Data to Queue
===================================================================================================*/

/* --------------------------- UART3_ISR_callback --------------------------- */
static void uart3_RX_INTR(void) {
  /* Check the Interrupt ID Register */
  if (((LPC_UART3->IIR >> 1) & 0xF) == 0x2) {
    /* Status Register-RDR Bits */
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }
  } // End Check
  /* READ the RBR register --> RX Queue */
  const char byte = LPC_UART3->RBR & 0xFF;
  xQueueSendFromISR(uart_rx_queue, &byte, NULL);
}
/* --------------------------- UART2_ISR_callback --------------------------- */
static void uart2_RX_INTR(void) {
  /* Check the Interrupt ID Register */
  if (((LPC_UART2->IIR >> 1) & 0xF) == 0x2) {
    /* Status Register-RDR Bits */
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }
  } // End Check
  /* READ the RBR register --> RX Queue */
  const char byte = LPC_UART2->RBR & 0xFF;
  xQueueSendFromISR(uart_rx_queue, &byte, NULL);
}

/*====================================UART enable INTR (RX)======================================
*@brief:  Enable INTR for RX Buffer
*@para:   uart_number_e
          queue size
*@return: No return

*@Note:   Using Queue.h (RTOS)--> Create Queue object (desire Size)
          require NVIC_enableIRQ from LPC_40xx.h
          require dispatcher API from lpc_peripheral.h
          DLAB must be Disable
          LPC_UART2->IER (enable)
================================================================================================*/
void uart__enable_RX_INTR(uart_number_e uart, int queue_size) {
  /* Queue Object*/
  uart_rx_queue = xQueueCreate(queue_size, sizeof(char));
  if (uart == UART_2) {
    /* Nest INTR + Dispatcher(...,uart2_RX_INTR,...) */
    NVIC_EnableIRQ(UART2_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, uart2_RX_INTR, NULL);

    /* DLAB[OFF] + IER register[0]=1 */
    LPC_UART2->LCR &= ~(1 << 7);
    LPC_UART2->IER |= (1 << 0);
    fprintf(stderr, "UART 2 INTR_RX \n");
  } else if (uart == UART_3) {
    /* Nest INTR + Dispatcher(...,uart3_RX_INTR,...) */
    NVIC_EnableIRQ(UART3_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, uart3_RX_INTR, NULL);

    /* DLAB[OFF] + IER register[0]=1 */
    LPC_UART3->LCR &= ~(1 << 7);
    LPC_UART3->IER |= (1 << 0);
    fprintf(stderr, "UART 3 INTR_RX \n");
  }
}

/*=====================================UART get from Queue =======================================
*@brief:  Get data from Queue
*@para:   *input_byte [out]
          timeout     [in]
*@return: TRUE If successfully received from the queue, Else FALSE.

*@Note:   Using pointer to get result out
=================================================================================================*/
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(uart_rx_queue, input_byte, timeout);
}
