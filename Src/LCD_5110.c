#include "LCD_5110.h"

struct LCD_att lcd;
LCD_Pin_t LCD_Pin;


/*----- GPIO Functions -----*/
/*
 * @brief Set functions for GPIO pins used
 * @param Port: Port of the pin used
 * @param PIN: pin of the pin used
 */
void LCD_SetRST(GPIO_TypeDef* Port, uint16_t PIN){
	LCD_Pin.RST_Port = Port;
	LCD_Pin.RST_Pin = PIN;
}

void LCD_SetCE(GPIO_TypeDef* Port, uint16_t PIN){
	LCD_Pin.CE_Port = Port;
	LCD_Pin.CE_Pin = PIN;
}

void LCD_SetDC(GPIO_TypeDef* Port, uint16_t PIN){
	LCD_Pin.DC_Port = Port;
	LCD_Pin.DC_Pin = PIN;
}
void LCD_SetDIN(GPIO_TypeDef* Port, uint16_t PIN){
	LCD_Pin.DIN_Port = Port;
	LCD_Pin.DIN_Pin = PIN;
}

void LCD_SetCLK(GPIO_TypeDef* Port, uint16_t PIN){
	LCD_Pin.CLK_Port = Port;
	LCD_Pin.CLK_Pin = PIN;
}

/*----- Library Functions -----*/

/*
 * @brief Send information to the LCD using configured GPIOs
 * @param val: value to be sent
 */
void LCD_Send(uint8_t val){
  uint8_t i;

  for(i = 0; i < 8; i++){
    HAL_GPIO_WritePin(LCD_Pin.DIN_Port, LCD_Pin.DIN_Pin, (GPIO_PinState)(!!(val & (1 << (7 - i)))));
    HAL_GPIO_WritePin(LCD_Pin.CLK_Port, LCD_Pin.CLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_Pin.CLK_Port, LCD_Pin.CLK_Pin, GPIO_PIN_RESET);
  }
}

/*
 * @brief Writes some data into the LCD
 * @param data: data to be written
 * @param mode: command or data
 */
void LCD_Write(uint8_t data, uint8_t mode){
  if(mode == LCD_COMMAND){
    HAL_GPIO_WritePin(LCD_Pin.DC_Port, LCD_Pin.DC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_Pin.CE_Port, LCD_Pin.CE_Pin, GPIO_PIN_RESET);
    LCD_Send(data);
    HAL_GPIO_WritePin(LCD_Pin.CE_Port, LCD_Pin.CE_Pin, GPIO_PIN_SET);
  }
  else{
    HAL_GPIO_WritePin(LCD_Pin.DC_Port, LCD_Pin.DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_Pin.CE_Port, LCD_Pin.CE_Pin, GPIO_PIN_RESET);
    LCD_Send(data);
    HAL_GPIO_WritePin(LCD_Pin.CE_Port, LCD_Pin.CE_Pin, GPIO_PIN_SET);
  }
}

/*
 * @brief Initialize the LCD using predetermined values
 */
void LCD_Init(){
  HAL_GPIO_WritePin(LCD_Pin.RST_Port, LCD_Pin.RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_Pin.RST_Port, LCD_Pin.RST_Pin, GPIO_PIN_SET);
  LCD_Write(0x21, LCD_COMMAND); //LCD extended commands.
  LCD_Write(0xA0, LCD_COMMAND); //set LCD Vop(Contrast).
  LCD_Write(0x04, LCD_COMMAND); //set temp coefficent.
  LCD_Write(0x14, LCD_COMMAND); //LCD bias mode 1:40.
  LCD_Write(0x20, LCD_COMMAND); //LCD basic commands.
  LCD_Write(LCD_DISPLAY_NORMAL, LCD_COMMAND); //LCD normal.
  LCD_clrScr();
  lcd.InvertText = false;
}

/*
 * @brief Invert the color shown on the display
 * @param mode: true = inverted / false = normal
 */
void LCD_Invert(bool mode){
  if(mode == true){
    LCD_Write(LCD_DISPLAY_INVERTED, LCD_COMMAND);
  }
  else{
    LCD_Write(LCD_DISPLAY_NORMAL, LCD_COMMAND);
  }
}

/*
 * @brief Invert the colour of any text sent to the display
 * @param mode: true = inverted / false = normal
 */
void LCD_InvertText(bool mode){
  if(mode == true){
    lcd.InvertText = true;
  }
  else{
    lcd.InvertText = false;
  }
}

/*
 * @brief Puts one char on the current position of LCD's cursor
 * @param c: char to be printed
 */
void LCD_putChar(char c){
  for(int i = 0; i < 6; i++){
    if(lcd.InvertText != true)
      LCD_Write(ASCII[c - 0x20][i], LCD_DATA);
    else
      LCD_Write(~(ASCII[c - 0x20][i]), LCD_DATA);
  }
}

/*
 * @brief Print a string on the LCD
 * @param x: starting point on the x-axis (column)
 * @param y: starting point on the y-axis (line)
 */
void LCD_Print(char *str, uint8_t x, uint8_t y){
  LCD_GotoXY(x, y);
  while(*str){
    LCD_putChar(*str++);
  }
}

/*
 * @brief Clear the screen
 */
void LCD_clrScr(){
  for(int i = 0; i < 504; i++){
    LCD_Write(0x00, LCD_DATA);
    lcd.Buff[i] = 0;
  }
}

/*
 * @brief Set LCD's cursor to position X,Y
 * @param x: position on the x-axis (column)
 * @param y: position on the y-axis (line)
 */
void LCD_GotoXY(uint8_t x, uint8_t y){
  LCD_Write(0x80 | x, LCD_COMMAND); //Column.
  LCD_Write(0x40 | y, LCD_COMMAND); //Row.
}

/*
 * @brief Updates the entire screen according to lcd.Buff
 */
void LCD_RefreshScr(){
  LCD_GotoXY(LCD_SetXADDR, LCD_SetYADDR);
  for(int i = 0; i < 6; i++){
    for(int j = 0; j < LCD_WIDTH; j++){
      LCD_Write(lcd.Buff[(i * LCD_WIDTH) + j], LCD_DATA);
    }
  }
}

/*
 * @brief Updates a square of the screen according to given values
 * @param xmin: starting point on the x-axis
 * @param xmax: ending point on the x-axis
 * @param ymin: starting point on the y-axis
 * @param ymax: ending point on the y-axis
 */
void LCD_RefreshArea(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax){
  for(int i = 0; i < 6; i++){
    if(i * 8 > ymax){
      break;
    }
    LCD_Write(LCD_SetYADDR | i, LCD_COMMAND);
    LCD_Write(LCD_SetXADDR | xmin, LCD_COMMAND);
    for(int j = xmin; j <= xmax; j++){
      LCD_Write(lcd.Buff[(i * LCD_WIDTH) + j], LCD_DATA);
    }
  }
}

/*
 * @brief Sets a pixel on the screen
 */
void LCD_SetPixel(uint8_t x, uint8_t y, bool pixel){
  if(x >= LCD_WIDTH)
    x = LCD_WIDTH - 1;
  if(y >= LCD_HEIGHT)
    y = LCD_HEIGHT - 1;
    
  if(pixel != false){
    lcd.Buff[x + (y / 8) * LCD_WIDTH] |= 1 << (y % 8);
  }
  else{
    lcd.Buff[x + (y / 8) * LCD_WIDTH] &= ~(1 << (y % 8));
  }
}

/*
 * @brief Draws a horizontal line
 * @param x: starting point on the x-axis
 * @param y: starting point on the y-axis
 * @param l: length of the line
 */
void LCD_DrawHLine(int x, int y, int l){
  int by, bi;

  if ((x>=0) && (x<LCD_WIDTH) && (y>=0) && (y<LCD_HEIGHT)){
    for (int cx=0; cx<l; cx++){
      by=((y/8)*84)+x;
      bi=y % 8;
      lcd.Buff[by+cx] |= (1<<bi);
    }
  }
}

/*
 * @brief Draws a vertical line
 * @param x: starting point on the x-axis
 * @param y: starting point on the y-axis
 * @param l: length of the line
 */
void LCD_DrawVLine(int x, int y, int l){

  if ((x>=0) && (x<84) && (y>=0) && (y<48)){
    for (int cy=0; cy<= l; cy++){
      LCD_SetPixel(x, y+cy, true);
    }
  }
}

/*
 * @brief ABS function used in LCD_DrawLine
 * @param x: any integer
 * @return ABSolute value of x
 */
int ABS(int x){
	if(x < 0){
		return x*-1;
	}
	return x;
}

/*
 * @brief Draws any line
 * @param x1: starting point on the x-axis
 * @param y1: starting point on the y-axis
 * @param x2: ending point on the x-axis
 * @param y2: ending point on the y-axis
 */
void LCD_DrawLine(int x1, int y1, int x2, int y2){
  int tmp;
  double delta, tx, ty;
  
  if (((x2-x1)<0)){
    tmp=x1;
    x1=x2;
    x2=tmp;
    tmp=y1;
    y1=y2;
    y2=tmp;
  }
    if (((y2-y1)<0)){
    tmp=x1;
    x1=x2;
    x2=tmp;
    tmp=y1;
    y1=y2;
    y2=tmp;
  }
  
  if (y1==y2){
    if (x1>x2){
      tmp=x1;
      x1=x2;
      x2=tmp;
    }
    LCD_DrawHLine(x1, y1, x2-x1);
  }
  else if (x1==x2){
    if (y1>y2){
      tmp=y1;
      y1=y2;
      y2=tmp;
    }
    LCD_DrawHLine(x1, y1, y2-y1);
  }
  else if (ABS(x2-x1)>ABS(y2-y1)){
    delta=((double)(y2-y1)/(double)(x2-x1));
    ty=(double) y1;
    if (x1>x2){
      for (int i=x1; i>=x2; i--){
        LCD_SetPixel(i, (int) (ty+0.5), true);
            ty=ty-delta;
      }
    }
    else
    {
      for (int i=x1; i<=x2; i++){
        LCD_SetPixel(i, (int) (ty+0.5), true);
        ty=ty+delta;
      }
    }
  }
  else{
    delta=((float) (x2-x1)/(float) (y2-y1));
    tx=(float) (x1);
        if (y1>y2){
          for (int i=y2+1; i>y1; i--){
            LCD_SetPixel((int) (tx+0.5), i, true);
            tx=tx+delta;
          }
        }
        else{
          for (int i=y1; i<y2+1; i++){
            LCD_SetPixel((int) (tx+0.5), i, true);
            tx=tx+delta;
          }
        }
  }
}

/*
 * @brief Draws a rectangle
 * @param x1: starting point on the x-axis
 * @param y1: starting point on the y-axis
 * @param x2: ending point on the x-axis
 * @param y2: ending point on the y-axis
 */
void LCD_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
  LCD_DrawLine(x1, y1, x2, y1);
  LCD_DrawLine(x1, y1, x1, y2);
  LCD_DrawLine(x2, y1, x2, y2);
  LCD_DrawLine(x1, y2, x2, y2);
}
