#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>

/* ------------------------------- Declaration ------------------------------ */
/* ------------------------------------ x ----------------------------------- */
typedef enum {
  OFF = 0,
  ON = 1,
} ON_OFF;

/* ----------------------------- Public Function ---------------------------- */
/* ------------------------------------ F ----------------------------------- */

/* Initialize SPI (Power ON + Config controll Reg + Adjust SPI clock) */
void ssp2__setup(uint32_t max_clock_mhz);

/* Config IO PIN SSP2 function */
void ssp2_PIN_config();

/* Exchange byte (READ/WRITE) */
uint8_t ssp2__transfer_byte(uint8_t data_transfer);

/* Enable Specific Slave */
void Chip_Select(uint8_t port, uint8_t pin, ON_OFF ONE_or_OFF);

/* Read Data (Single/Multi) */
void ssp2_read(uint8_t OP_Code, uint8_t num_of_Byte, uint8_t *receive_buffer);

/* Sending 24-Bits Address */
void adesto_flash_send_address(uint32_t address);

/* Extra Credit write page */
void ssp2__write_page(uint32_t address);

/* Extra Credit read page */
void ssp2__read_page(uint32_t address, uint8_t *data);

/* Write Enable Latch (WEL) */
void write_Enable();

/* Write Disable Latch (WDL)*/
void write_Disable();

/* Erase Page Flash Memory  */
void erase_data(uint8_t Page_num);