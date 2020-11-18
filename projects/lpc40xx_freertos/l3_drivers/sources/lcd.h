#include <stdio.h>

void LCD_print_string(char *s);
void LCD_print_char(char c);
void write_to_data_bus(uint8_t instr, uint8_t rs);
void lcd_init();
void lcd_set_cursor(int row, int col);