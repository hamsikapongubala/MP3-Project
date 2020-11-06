// file gpio_lab.h
#pragma once

#include <stdbool.h>
#include <stdint.h>

/*===================================CONFIG GPIO PIN as INPUT====================================*/
void gpio1__set_as_input(uint8_t port_num, uint8_t pin_num);

/*===================================CONFIG GPIO PIN as OUTPUT===================================*/
void gpio1__set_as_output(uint8_t port_num, uint8_t pin_num);

/*=================================CONFIG GPIO PIN as HIGH state=================================*/
void gpio1__set_high(uint8_t port_num, uint8_t pin_num);

/*=================================CONFIG GPIO PIN as LOW state==================================*/
void gpio1__set_low(uint8_t port_num, uint8_t pin_num);

/*=============================CONFIG GPIO PIN as HIGH or LOW state==============================*/
void gpio1__set(uint8_t port_num, uint8_t pin_num, bool high);

/*==============================READ GPIO PIN state (HIGH or LOW)================================*/
bool gpio1__get_level(uint8_t port_num, uint8_t pin_num);