#include <stdint.h>

#include "adc.h"

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

/*=========================================Initialize ADC()=============================================
*@brief:    Initialize Clock Frequency + Turn ON Peripheral ADC
*@para:     NO Para
*@return:   No Return
*@Note:     Using Control Register(CR)
            * set CR[8]= divider to setup CLK
========================================================================================================*/
void adc__initialize(void) {
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__ADC);
  /* 1 Bit POWER (ON/OFF[21]) CONTROL register */
  const uint32_t enable_adc_mask = (1 << 21);
  LPC_ADC->CR = enable_adc_mask;
  /* 12.4Mhz : max ADC clock in datasheet for lpc40xx */
  const uint32_t max_adc_clock = (12 * 1000UL * 1000UL);
  const uint32_t adc_clock = clock__get_peripheral_clock_hz();

  /* APB clock divicer to support max ADC clock */
  for (uint32_t divider = 2; divider < 255; divider += 2) {
    if ((adc_clock / divider) < max_adc_clock) {
      /* 8 Bit CLKDIV ([16->8]) CONTROL register */
      LPC_ADC->CR |= (divider << 8);
      break;
    }
  }
}

/*=========================================Get ADC Value()============================================
*@brief:    Setup Control Reg + Read from Global Data Register (Software Mode)
*@para:     Channel_num
*@return:   12 Bits value ( GLOBAL DATA REGISTER)
*@Note:     SETUP Control Register (CR)
            READ Global Data Register (GDR) + Check the DONE Bits (GDR)
======================================================================================================*/

uint16_t adc__get_adc_value(adc_channel_e channel_num) {
  uint16_t result = 0;
  /* 12 Bit Result from Data Register */
  const uint16_t twelve_bits = 0x0FFF;
  /* 8 Bit Channel (SEL[7->0]) CONTROL register */
  const uint32_t channel_masks = 0xFF;
  /* 3 Bit Start setup (START[26->24]) CONTROL register */
  const uint32_t start_conversion = (1 << 24);
  const uint32_t start_conversion_mask = (7 << 24); // 3bits - B26:B25:B24
  /* 1 Bit Done (DONE[31]) Data Register */
  const uint32_t adc_conversion_complete = (1 << 31);

  if ((ADC__CHANNEL_2 == channel_num) || (ADC__CHANNEL_4 == channel_num) || (ADC__CHANNEL_5 == channel_num)) {
    LPC_ADC->CR &= ~(channel_masks | start_conversion_mask);
    // Set the channel number and start the conversion now
    LPC_ADC->CR |= (1 << channel_num) | start_conversion;

    while (!(LPC_ADC->GDR & adc_conversion_complete)) {
      /* Wait till conversion is complete - Software */
      /* Espcape the loop when condition is True */;
    }
    /*Get 12 Bit Result and Ignore 4 Bits Reseve  Data Register */
    result = (LPC_ADC->GDR >> 4) & twelve_bits;
  }
  return result;
}

/*=====================================Get ADC Value Burst Mode()=====================================
*@brief:    Burst Mode + Read From Data Register
*@para:     Channel_num
*@return:   12 Bits value ( DATA REGISTER[Channel] )
*@Note:     READ Data Register in specific Channel (LPC_ADC->DR[Channel])
======================================================================================================*/
uint16_t adc__get_adc_value_burst(adc_channel_e channel_num) {
  /* Channel 5 - Data Register [2]<->[4]<->[5] */
  uint16_t result = (LPC_ADC->DR[channel_num] >> 4) & 0x0FFF; // 12bits
  return result;
}

/*=======================================Enable Burst Mode()===========================================
*@brief:    Enable Burst Mode (Hardware Mode)
*@para:     Channel_num
*@return:   No Return
*@Note:     READ Data Register in specific Channel (LPC_ADC->DR[Channel])
======================================================================================================*/
void adc__enable_burst_mode(adc_channel_e channel_num) // Channel 2 4 5
{
  /* Clear B24 B25 B26 */
  LPC_ADC->CR &= ~(7 << 24);
  /* To Enable Burst_mode[16]=1  +  Set Channel SEL[7-0] */
  LPC_ADC->CR |= ((1 << 16) | (1 << channel_num));
}

/*=========================================adc_enable_P1_31()=========================================
*@brief:    Enable Analog Mode + ADC function for I/O Pin P1_31
*@para:     No Para
*@return:   No Return
*@Note:     LPC_IOCON->P1_31[7]     --> enable Analog
            LPC_IOCON->P1_31[2-0]   --> set ADC function
======================================================================================================*/
void adc_enable_P1_31() {
  /* Setup ICON Analog Mode */
  LPC_IOCON->P1_31 &= ~(1 << 7);
  /* Setup ICON ADC Function */
  LPC_IOCON->P1_31 &= ~(0b111); // Set ALL 0
  LPC_IOCON->P1_31 |= 0b011;    // Set OP value
}