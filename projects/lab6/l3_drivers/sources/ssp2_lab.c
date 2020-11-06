#include "ssp2_lab.h"

/*======================================SPI2 Setup(max_clock_mhz)=====================================
*@brief:  Initialize SPI (Power ON + Config controll Reg + Adjust SPI clock)
*@para:   max_clock_mhz (in)
*@return: No return
*@Note:   Power On            ( LPC_SC->PCONP )
          Setup Crontrol Reg  ( LPC_SSP2->CR0 or CR1 )
          Setup Prescalar     ( LPC_SSP2->CPSR )
======================================================================================================*/
void ssp2__setup(uint32_t max_clock_mhz) {
  max_clock_mhz = max_clock_mhz * 1000 * 1000;
  /* a) Power on Peripheral */
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP2);

  /* b) Setup control registers CR0 and CR1 */
  LPC_SSP2->CR0 = 7;        // 8-Bit transfer
  LPC_SSP2->CR1 = (1 << 1); // SSP Control Enable

  /* c) Setup prescalar register to be <= max_clock_mhz-(Input) */
  const uint32_t CPU_CLK = clock__get_core_clock_hz(); // 96-MHz
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_CLK / divider) <= max_clock_mhz) {
      fprintf(stderr, "Pre_Scale: %d \n", divider);
      break;
    }
    /* Setup PreScale Control[7:0] */
    LPC_SSP2->CPSR = divider;
  }
}

/*==========================================SPI2 Pin Config()========================================
*@brief:  Config IO PIN SSP2 function
*@para:   No Parameter
*@return: No Return
*@Note:   Initialize Pin mode from IOControl Register (SPI2)
=====================================================================================================*/
void ssp2_PIN_config() {
  gpio__construct_with_function(1, 1, GPIO__FUNCTION_4); // MISO
  gpio__construct_with_function(1, 4, GPIO__FUNCTION_4); // MOSI
  gpio__construct_with_function(1, 0, GPIO__FUNCTION_4); // CLK
}

/*==================================SPI2 Transfer Byte(data exchange)================================
*@brief:    Exchange byte (READ/WRITE)
*@para:     Data_transfer (in)
*@return:   Data from reg (out)
*@Note:     Send byte to read byte from Data Register
            Check Status Reg [4]
=====================================================================================================*/
uint8_t ssp2__transfer_byte(uint8_t data_transfer) {
  /* 16-Bits Data Register [15:0] */
  LPC_SSP2->DR = data_transfer;

  /* Status Register-BUSY[4] */
  while (LPC_SSP2->SR & (1 << 4)) {
    ; /* Wait while it is busy(1), else(0) BREAK */
  }
  /* READ 16-Bits Data Register [15:0] */
  return (uint8_t)(LPC_SSP2->DR & 0xFF);
}

/*==================================SPI2 Chip_select (data exchange)==================================
*@brief:  Enable Specific Slave (CS PIN)
*@para:   Port, Pin, ON/OFF ( in )
*@return: No Return
*@Note:
=====================================================================================================*/

void Chip_Select(uint8_t port, uint8_t pin, ON_OFF ONE_or_OFF) {
  if (ONE_or_OFF == ON) {
    /* Set Output + LOW/On */
    gpio__construct_as_output(port, pin);
    gpio1__set(port, pin, false);
  } else if (ONE_or_OFF == OFF) {
    /* Set Output + HIGH/Off */
    gpio__construct_as_output(port, pin);
    gpio1__set(port, pin, true);
  }
}

/*====================================SPI2 READ Byte(data exchange)===================================
*@brief:    Read Data (Single/Multi)
*@para:     OP_Code, num_of_Byte  (in)
            receive_buffer        (out)
*@return:   No return
*@Note:     Return function pointer->Array address
=====================================================================================================*/
void ssp2_read(uint8_t OP_Code, uint8_t num_of_Byte, uint8_t *receive_buffer) {
  /* READ 1 Byte by default */
  num_of_Byte = 1;
  Chip_Select(1, 10, ON);
  // delay__us(1);
  /*  Send OP Code */
  ssp2__transfer_byte(OP_Code);
  for (int i = 0; i <= num_of_Byte; i++) {
    /* Send Dummy after OP Code */
    receive_buffer[i] = ssp2__transfer_byte(0xff);
  }
  Chip_Select(1, 10, OFF);
  /* Return Array Address */
}

/*===================================SPI2 write page (extra credit)==================================
*@brief:     Extra Credit write page
*@para:      address (in)
*@return:    no return
*@Note:      write 250 blocks test data
=====================================================================================================*/
void ssp2__write_page(uint32_t address) {
  /* Step-1: Write enable */
  write_Enable();
  Chip_Select(1, 10, ON);

  /* Step-2: OP-Code + 3-byte Address */
  ssp2__transfer_byte(0x02); // OP-Code
  adesto_flash_send_address(address);

  /* Step-3: Send Data */
  for (uint8_t data = 250; data >= 0; data--) {
    ssp2__transfer_byte(data);
  }
  write_Disable();
  Chip_Select(1, 10, OFF);
}

/*====================================SPI2 read page (extra credit)==================================
*@brief:     Extra Credit read page
*@para:      address (in)
             *data   (out)
*@return:    no return
*@Note:      read 250 blocks test data
             Using pointer to get the data out
======================================================================================================*/
void ssp2__read_page(uint32_t address, uint8_t *data) {
  Chip_Select(1, 10, ON);
  ssp2__transfer_byte(0x03); // OP-Code
  adesto_flash_send_address(address);
  // uint8_t temp[256];
  for (int i = 0; i <= 250; i++) {
    /* Send Dummy after OP Code */
    data[i] = ssp2__transfer_byte(0xff);
  }
  Chip_Select(1, 10, OFF);
}

void write_Enable() {
  Chip_Select(1, 10, ON);
  ssp2__transfer_byte(0x06); // OP-Code
  Chip_Select(1, 10, OFF);
}

/* ================================Extra Credit Supplement===================================
 *===========================================================================================*/

void adesto_flash_send_address(uint32_t address) {
  (void)ssp2__transfer_byte((address >> 16) & 0xFF);
  (void)ssp2__transfer_byte((address >> 8) & 0xFF);
  (void)ssp2__transfer_byte((address >> 0) & 0xFF);
}

void write_Disable() {
  Chip_Select(1, 10, ON);
  ssp2__transfer_byte(0x04); // OP-Code
  Chip_Select(1, 10, OFF);
}

// Underconstruction :)
void erase_data(uint8_t Page_num) {
  Chip_Select(1, 10, ON);
  write_Enable();
  Chip_Select(1, 10, OFF);

  Chip_Select(1, 10, ON);
  ssp2__transfer_byte(0x81); // OP-Code
  /* Step-3: Send Data */
  ssp2__transfer_byte(0xc1);
  ssp2__transfer_byte(Page_num); // pages number 00
  ssp2__transfer_byte(0xc1);
  // write_Disable();
  Chip_Select(1, 10, OFF);
  delay__ms(100);
}