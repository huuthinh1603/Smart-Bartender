#ifndef __RW_EEPROM_H__
#define __RW_EEPROM_H__

#include "stm32l0xx_hal.h"

void EEPROM_Write(uint32_t Flash_Address, uint16_t Data);
uint16_t EEPROM_Read(uint32_t Flass_Address);

#endif /* __RW_EEPROM_H__ */
