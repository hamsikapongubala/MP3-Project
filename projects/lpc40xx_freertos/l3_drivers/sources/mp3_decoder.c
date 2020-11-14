#include "mp3_decoder.h"
#include "delay.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>

void ssp0__init(uint32_t max_clock_khz) {

  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  LPC_SSP0->CR0 = (7 << 0); // set control register CR0

  LPC_SSP0->CR1 = (1 << 1); // set control register CR1

  ssp0__set_max_clock(max_clock_khz);
}

static void ssp0__set_max_clock(uint32_t max_clock_khz) {

  uint8_t divider = 2;

  const uint32_t cpu_clock_khz = clock__get_core_clock_hz() / 1000UL;

  while (max_clock_khz < (cpu_clock_khz / divider) && divider <= 254) {
    divider += 2;
  }

  LPC_SSP0->CPSR = divider;
}

uint8_t ssp0_transferByte(uint8_t data_out) {
  LPC_SSP0->DR = data_out;

  while (LPC_SSP0->SR & (1 << 4)) {
    ; // Wait while busy flag is high
  }
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}

void pin__init() {
  gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2); // SSP0_SCK
  gpio__construct_with_function(GPIO__PORT_0, 17,
                                GPIO__FUNCTION_2); // SSP0_MISO
  gpio__construct_with_function(GPIO__PORT_0, 18,
                                GPIO__FUNCTION_2); // SSP0_MOSI

  gpio__construct_with_function(GPIO__PORT_2, 7,
                                GPIO__FUNCITON_0_IO_PIN); // DREQ
  gpio__construct_as_input(GPIO__PORT_2, 7);

  gpio__construct_with_function(GPIO__PORT_2, 8,
                                GPIO__FUNCITON_0_IO_PIN); // CS - active low
  gpio__construct_as_output(GPIO__PORT_2, 8);

  gpio__construct_with_function(GPIO__PORT_2, 9,
                                GPIO__FUNCITON_0_IO_PIN); // XDCS - active low
  gpio__construct_as_output(GPIO__PORT_2, 9);

  gpio__construct_with_function(GPIO__PORT_0, 16,
                                GPIO__FUNCITON_0_IO_PIN); // RST - active low
  gpio__construct_as_output(GPIO__PORT_0, 16);
}

/* XDCS - active low signal */
void set__XDCS() { LPC_GPIO2->CLR |= 1 << 9; }

void reset__XDCS() { LPC_GPIO2->SET |= 1 << 9; }

/* CS - active low signal */
void set__CS() { LPC_GPIO2->CLR |= 1 << 8; }

void reset__CS() { LPC_GPIO2->SET |= 1 << 8; }

/*RST - active low signal */
void set__RST() { LPC_GPIO0->CLR |= 1 << 16; }

void reset__RST() { LPC_GPIO0->SET |= 1 << 16; }

bool DREQ_status() {
  if (LPC_GPIO2->PIN & (1 << 7)) {
    return true;
  }
  return false;
}

void mp3_decoder_setup() {

  pin__init();

  reset__RST();

  ssp0__init(1000);

  ssp0_transferByte(0xFF);

  delay__ms(50);

  reset__CS();
  reset__XDCS();

  write_register(0x00, 0x88, 0x00); // OUTPUT MODE

  write_register(0x02, 0x06, 0x06); // SCI_BASS REGISTER

  uint16_t status = read_register(0x01);
  int version = (status >> 4) & 0x000F;
  printf("Version: %d\n", version);

  uint16_t mode = read_register(0x00);
  printf("SCI_MODE = 0x%x\n", mode);

  write_register(0x00, 0x48, 0x10); // MP3 Mode

  mode = read_register(0x00);
  printf("SCI_MODE = 0x%x\n", mode);

  write_register(0x0B, 24, 24); // VOLUME

  write_register(0x03, 0x60, 0x00); // Increase SPI clock

  ssp0__set_max_clock(6000); // 6 Khz

  uint16_t clk = read_register(0x03);
  printf("SCI_CLOCKF = 0x%x", clk);

  delay__ms(50);
}

uint16_t read_register(uint16_t address) {
  while (!DREQ_status()) {
    ;
  }
  set__CS();

  ssp0_transferByte(0x03); //  OPCODE for READ
  ssp0_transferByte(address);

  uint8_t first_byte = ssp0_transferByte(0xFF);
  while (!DREQ_status()) {
    ;
  }
  uint8_t second_byte = ssp0_transferByte(0xFE);
  while (!DREQ_status()) {
    ;
  }
  reset__CS();

  uint16_t output = 0;
  output |= ((first_byte << 8) | (second_byte << 0));
  return output;
}

void write_register(uint16_t register_address, uint8_t first_byte,
                    uint8_t second_byte) {
  while (!DREQ_status()) {
    ;
  }
  set__CS();
  ;

  ssp0_transferByte(0x02); //  OPCODE for WRITE
  ssp0_transferByte(register_address);

  ssp0_transferByte(first_byte);
  ssp0_transferByte(second_byte);
  while (!DREQ_status()) {
    ; // Wait
  }
  reset__CS();
}

void send_mp3Data(uint8_t data) {
  set__XDCS();             // Select
  ssp0_transferByte(data); // Send Data
  reset__XDCS();           // Deselect
}