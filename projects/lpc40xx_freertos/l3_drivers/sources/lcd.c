#include "delay.h"
#include "gpio.h"
#include "stdio.h"
#include "string.h"

gpio_s LCD_EN, LCD_RS, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6,
    LCD_D7;

void lcd_init() {
  delay_ms(2);
  lcd_write_instr(0x06); // Incrment mode for the cursor
  lcd_write_instr(0x0C); // The display on, the cursor off
  lcd_write_instr(0x38); // 8-bit data bus, two line display
}

void lcd_pin_init() {
  // LCD_EN = gpio__construct_as_output(GPIO__FUNCTION_4, 28);
  // LCD_RS = gpio__construct_as_output(GPIO__FUNCTION_4, 29);

  // LCD_D1 = gpio__construct_as_output(GPIO__PORT_2, 5);
  // LCD_D2 = gpio__construct_as_output(GPIO__PORT_2, 7);
}

void lcd_write_instr() {}
