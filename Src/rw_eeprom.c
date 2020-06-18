#include "rw_eeprom.h"

/*
 *	@param Type FLASH_TYPEPROGRAMDATA_BYTE, FLASH_TYPEPROGRAMDATA_HALFWORD, FLASH_TYPEPROGRAMDATA_WORD
 *	
*/
void EEPROM_Write(uint32_t Flash_Address, uint16_t Data)
{
	HAL_FLASHEx_DATAEEPROM_Unlock();
	HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, Flash_Address, Data);
	HAL_FLASHEx_DATAEEPROM_Lock ();
	HAL_Delay(50);
}

uint16_t EEPROM_Read(uint32_t Flass_Address)
{
	return *(__IO char *)Flass_Address;
}
