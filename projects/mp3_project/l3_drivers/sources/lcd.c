#include "gpio.h"

gpio_s LCD_EN, LCD_RS, LCD_RW, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6,
    LCD_D7;

void lcd_init() {
  LCD_EN = gpio__construct_as_output(GPIO__PORT_0, 6); // E
  gpio__construct_with_function(GPIO__PORT_0, 6, GPIO__FUNCITON_0_IO_PIN);
  
  LCD_RS = gpio__construct_as_output(GPIO__PORT_1, 4); // RS
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCITON_0_IO_PIN);

  LCD_RW = gpio__construct_as_output(GPIO__PORT_4, 28); // RW
  gpio__construct_with_function(GPIO__PORT_4, 28, GPIO__FUNCITON_0_IO_PIN);

  LCD_D0 = gpio__construct_as_output(GPIO__PORT_1, 1); // D0
  LCD_D1 = gpio__construct_as_output(GPIO__PORT_1, 0); // D1
  LCD_D2 = gpio__construct_as_output(GPIO__PORT_2, 2); // D2
  LCD_D3 = gpio__construct_as_output(GPIO__PORT_2, 3); // D3
  LCD_D4 = gpio__construct_as_output(GPIO__PORT_1, 20); // D4
  LCD_D5 = gpio__construct_as_output(GPIO__PORT_1, 23); // D5
  LCD_D6 = gpio__construct_as_output(GPIO__PORT_1, 28); // D6
  LCD_D7 = gpio__construct_as_output(GPIO__PORT_1, 29); // D7

  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_2, 2, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_2, 3, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 20, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 23, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 28, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 29, GPIO__FUNCITON_0_IO_PIN);
  
  write_to_data_bus(0x32, 0); // Display init
  write_to_data_bus(0x38, 0); // 8-bit data bus, 3 line display (0x30)?
  write_to_data_bus(0x06, 0); // Entry mode
  write_to_data_bus(0x0C, 0); // Display setup

  write_to_data_bus(0x01, 0); // Clear display
}

void write_to_data_bus(uint8_t instr, uint8_t rs) {

  gpio__reset(LCD_RW);

  if (rs) {
    gpio__set(LCD_RS); // Write to Data Register
  } else {
    gpio__reset(LCD_RS); // Write to Instruction Register
  }

  if (instr >> 0 & 1) {
    gpio__set(LCD_D0);
  } else {
    gpio__reset(LCD_D0);
  }

  if (instr >> 1 & 1) {
    gpio__set(LCD_D1);
  } else {
    gpio__reset(LCD_D1);
  }

  if (instr >> 2 & 1) {
    gpio__set(LCD_D2);
  } else {
    gpio__reset(LCD_D2);
  }

  if (instr >> 3 & 1) {
    gpio__set(LCD_D3);
  } else {
    gpio__reset(LCD_D3);
  }

  if (instr >> 4 & 1) {
    gpio__set(LCD_D4);
  } else {
    gpio__reset(LCD_D4);
  }

  if (instr >> 5 & 1) {
    gpio__set(LCD_D5);
  } else {
    gpio__reset(LCD_D5);
  }

  if (instr >> 6 & 1) {
    gpio__set(LCD_D6);
  } else {
    gpio__reset(LCD_D6);
  }

  if (instr >> 7 & 1) {
    gpio__set(LCD_D7);
  } else {
    gpio__reset(LCD_D7);
  }
}

void LCD_print_char(char c) { write_to_data_bus((uint8_t)c, 1); }

void LCD_print_string(char *s) {
  int len = strlen(s);
  for (int i = 0; i < len; i++) {
    LCD_print_char(s[i]);
  }
}