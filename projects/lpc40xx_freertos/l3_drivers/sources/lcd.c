#include "delay.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "stdio.h"

gpio_s LCD_EN, LCD_RS, LCD_RW, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5,
    LCD_D6, LCD_D7;

void lcd_pin_init() {
  LCD_EN = gpio__construct_as_output(GPIO__PORT_0, 6); // E
  gpio__construct_with_function(GPIO__PORT_0, 6, GPIO__FUNCITON_0_IO_PIN);

  LCD_RS = gpio__construct_as_output(GPIO__PORT_1, 4); // RS
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCITON_0_IO_PIN);

  LCD_RW = gpio__construct_as_output(GPIO__PORT_2, 1); // RW
  gpio__construct_with_function(GPIO__PORT_2, 1, GPIO__FUNCITON_0_IO_PIN);

  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCITON_0_IO_PIN);
  LCD_D0 = gpio__construct_as_output(GPIO__PORT_1, 1); // D0

  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCITON_0_IO_PIN);
  LCD_D1 = gpio__construct_as_output(GPIO__PORT_1, 0); // D1

  gpio__construct_with_function(GPIO__PORT_2, 2, GPIO__FUNCITON_0_IO_PIN);
  LCD_D2 = gpio__construct_as_output(GPIO__PORT_2, 2); // D2

  gpio__construct_with_function(GPIO__PORT_2, 4, GPIO__FUNCITON_0_IO_PIN);
  LCD_D3 = gpio__construct_as_output(GPIO__PORT_2, 4); // D3

  gpio__construct_with_function(GPIO__PORT_1, 20, GPIO__FUNCITON_0_IO_PIN);
  LCD_D4 = gpio__construct_as_output(GPIO__PORT_1, 20); // D4

  gpio__construct_with_function(GPIO__PORT_1, 23, GPIO__FUNCITON_0_IO_PIN);
  LCD_D5 = gpio__construct_as_output(GPIO__PORT_1, 23); // D5

  gpio__construct_with_function(GPIO__PORT_1, 28, GPIO__FUNCITON_0_IO_PIN);
  LCD_D6 = gpio__construct_as_output(GPIO__PORT_1, 28); // D6

  gpio__construct_with_function(GPIO__PORT_1, 29, GPIO__FUNCITON_0_IO_PIN);
  LCD_D7 = gpio__construct_as_output(GPIO__PORT_1, 29); // D7
}

void lcd_init() {

  lcd_pin_init();

  delay__ms(16);

  write_to_instruction_register(0x30);

  delay__ms(5);

  // pulse_E();

  write_to_instruction_register(0x30);

  delay__us(160);

  // pulse_E();
  write_to_instruction_register(0x30);

  delay__us(110);

  // pulse_E();

  write_to_instruction_register(0x38); // 8-bit 2 line display
  write_to_instruction_register(0x08);
  write_to_instruction_register(0x01);
  write_to_instruction_register(0x06); // Entry mode

  write_to_instruction_register(0x0C);

  write_to_data_register(0x48);
  write_to_data_register(0x49);

  // printf("INIT LCD\n");
}

void HD44780_SetUpper(unsigned short int data) {
  gpio__reset(LCD_D7);
  gpio__reset(LCD_D6);
  gpio__reset(LCD_D5);
  gpio__reset(LCD_D4);

  if (data & 0x80) {
    gpio__set(LCD_D7);
  }
  if (data & 0x40) {
    gpio__set(LCD_D6);
  }
  if (data & 0x20) {
    gpio__set(LCD_D5);
  }
  if (data & 0x10) {
    gpio__set(LCD_D4);
  }
}

void HD44780_SetLower(unsigned short int data) {
  gpio__reset(LCD_D3);
  gpio__reset(LCD_D2);
  gpio__reset(LCD_D1);
  gpio__reset(LCD_D0);

  if (data & 0x08) {
    gpio__set(LCD_D3);
  }
  if (data & 0x04) {
    gpio__set(LCD_D2);
  }
  if (data & 0x02) {
    gpio__set(LCD_D1);
  }
  if (data & 0x01) {
    gpio__set(LCD_D0);
    // printf("0x01\n");
  }
}

void send_8bit_data_mode(unsigned short int data) {

  HD44780_SetUpper(data);
  HD44780_SetLower(data);

  pulse_E();
}

void write_to_data_register(unsigned short int data) {

  gpio__reset(LCD_RW); // Write to low

  gpio__set(LCD_RS); // Write to Data Register

  send_8bit_data_mode(data);

  check_busy_flag();

  // printf("Writing to data register\n");
}

void write_to_instruction_register(unsigned short int instr) {
  gpio__reset(LCD_RW);
  gpio__reset(LCD_RS);

  send_8bit_data_mode(instr);

  check_busy_flag();

  // printf("Writing to instruction register\n");
}

void LCD_print_char(char c) { write_to_data_register(c); }

void LCD_print_string(char *s) {
  int len = strlen(s);
  for (int i = 0; i < len; i++) {
    LCD_print_char(s[i]);
  }
}

void lcd_set_cursor(int row, int col) {
  uint8_t position = (col + (row * 40)) + 0x80;
  write_to_data_register(position);
}

void pulse_E() {
  delay__us(1);
  gpio__set(LCD_EN);
  delay__us(1);
  gpio__reset(LCD_EN);
  delay__us(1);
}

void check_busy_flag() {

  gpio__construct_as_input(GPIO__PORT_1, 29); // set D7 as input
  gpio__set(LCD_RW);                          // SET RW
  gpio__reset(LCD_RS);                        // CLR RS

  while (1) {
    gpio__set(LCD_EN);

    delay__us(1);
    uint32_t input = LPC_GPIO1->PIN;
    gpio__reset(LCD_EN);

    if (!(input & (1 << 29))) { // D7
      break;
    }
  }

  gpio__reset(LCD_RW);
  gpio__construct_as_output(GPIO__PORT_1, 29);
  // printf("NOT busy\n");
}

void LCD_display_clear(void) {
  // Diplay clear
  write_to_instruction_register(0x01);
}