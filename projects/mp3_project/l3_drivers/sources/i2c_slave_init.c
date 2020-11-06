#include "i2c.h"
#include "lpc40xx.h"

void i2c2__slave_init(i2c_e i2c_number, uint8_t slave_address_to_respond_to) {

  if (i2c_number == I2C__2) {
    LPC_I2C2->ADR2 = slave_address_to_respond_to;
    LPC_I2C2->CONSET = 0x44;
  } else if (i2c_number == I2C__1) {
    LPC_I2C1->ADR1 = slave_address_to_respond_to;
    LPC_I2C1->CONSET = 0x44;
  } else {
    LPC_I2C0->ADR0 = slave_address_to_respond_to;
    LPC_I2C0->CONSET = 0x44;
  }
}

static volatile uint8_t slave_memory[256];

bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {

  if (memory_index > 256) {
    return false;
  }

  *memory = slave_memory[memory_index];
  return true;
}

bool i2c_slave_callback__write_memory(uint8_t memory_index,
                                      uint8_t memory_value) {

  if (memory_index > 256) {
    return false;
  }

  slave_memory[memory_index] = memory_value;

  return true;
}
