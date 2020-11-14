#include <gpio.h>
#include <stdint.h>
#include <stdio.h>

void mp3_decoder_setup();

static void ssp0__set_max_clock(uint32_t max_clock_khz);

uint8_t ssp0_transferByte(uint8_t data_out);

void pin__init();
void set__XDCS();
void reset__XDCS();
void set__CS();
void reset__CS();
void set__RST();
void reset__RST();
bool DREQ_status();

void mp3_decoder_setup();

uint16_t read_register(uint16_t address);

void write_register(uint16_t register_address, uint8_t first_byte,
                    uint8_t second_byte);

void send_mp3Data(uint8_t data);