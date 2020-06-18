#ifndef __LCD_5110__
#define __LCD_5110__

#include <stdbool.h>
#include "Font.h"
#include "stm32l0xx_hal.h"

#define LCD_COMMAND 0
#define LCD_DATA 1

#define LCD_SetYADDR 0x40
#define LCD_SetXADDR 0x80
#define LCD_DISPLAY_BLANK 0x08
#define LCD_DISPLAY_NORMAL 0x0C
#define LCD_DISPLAY_ALL_ON 0x09
#define LCD_DISPLAY_INVERTED 0x0D

#define LCD_WIDTH 84
#define LCD_HEIGHT 48
#define LCD_SIZE LCD_WIDTH * LCD_HEIGHT / 8

/*
 * @brief LCD parameters
 */
struct LCD_att{
	uint8_t Buff[LCD_SIZE];
	bool InvertText;
};

/*
 * @brief GPIO ports used
 */
typedef struct {
	GPIO_TypeDef* RST_Port;
	uint16_t RST_Pin;

	GPIO_TypeDef* CE_Port;
	uint16_t CE_Pin;

	GPIO_TypeDef* DC_Port;
	uint16_t DC_Pin;

	GPIO_TypeDef* DIN_Port;
	uint16_t DIN_Pin;

	GPIO_TypeDef* CLK_Port;
	uint16_t CLK_Pin;
} LCD_Pin_t;

/*----- GPIO Pins -----*/
void LCD_SetRST(GPIO_TypeDef* PORT, uint16_t PIN);
void LCD_SetCE(GPIO_TypeDef* PORT, uint16_t PIN);
void LCD_SetDC(GPIO_TypeDef* PORT, uint16_t PIN);
void LCD_SetDIN(GPIO_TypeDef* PORT, uint16_t PIN);
void LCD_SetCLK(GPIO_TypeDef* PORT, uint16_t PIN);

/*----- Library Functions -----*/
void LCD_Send(uint8_t val);
void LCD_Write(uint8_t data, uint8_t mode);
void LCD_Init(void);
void LCD_Invert(bool mode);
void LCD_InvertText(bool mode);
void LCD_putChar(char c);
void LCD_Print(char *str, uint8_t x, uint8_t y);
void LCD_clrScr(void);
void LCD_GotoXY(uint8_t x, uint8_t y);

/*----- Draw Functions -----*/
/*
 * These functions draw in a Buff variable. It's necessary to use LCD_RefreshScr() or LCD_RefreshArea()
 * in order to send data to the LCD.
 */
void LCD_RefreshScr(void);
void LCD_RefreshArea(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax);
void LCD_SetPixel(uint8_t x, uint8_t y, bool pixel);
void LCD_DrawHLine(int x, int y, int l);
void LCD_DrawVLine(int x, int y, int l);
void LCD_DrawLine(int x1, int y1, int x2, int y2);
void LCD_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#endif
