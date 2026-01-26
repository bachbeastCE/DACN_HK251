#ifndef __ST7735__H
#define __ST7735__H

#include "global.h"
#include "fonts.h"
#define RST 0x01 //software reset: COMMAND
#define SLEEP 0X10 //BOOSTER OFF: COMMAND
#define WAKE 0X11 //BOOSTER ON: COMMAND

#define NORMAL_MODE_FRAME 0xB1 //FULL COLOR: COMMAND
#define IDLE_MODE_FRAME 0xB2//8 COLOR: COMMAND
#define PARTIAL_MODE_FRAME 0xB3//FULL COLOR PARTIAL: COMMAND
typedef enum{
	RTNA = 0x01,
	FPA = 0x2C,
	BPA = 0x2D,
}frame;
typedef enum{
	COLOR_16BIT = 0X05,
	COLOR_12BIT = 0X03,
	COLOR_18BIT = 0X06,
}color_resolution;
extern SPI_HandleTypeDef ST7735_SPI_PORT;
#define INVERSION_ON 0xB4 //DISPLAY INVERSION CONTROL: COMMAND
#define INVERSION_OFF 0x20 //DISPLAY INVERSION CONTROL: COMMAND
#define DATA_INVERSION //DATA

#define POWER1 0xC0 //COMMAND
#define POWER2 0xC1 //COMMAND
#define POWER3 0xC2 //COMMAND
#define POWER4 0xC3 //COMMAND
#define POWER5 0xC4 //COMMAND
#define VCOM 0xC5 //COMMAND
#define MEMACCESS 0x36 //COMMAND
#define PIXELFORMAT 0X3A //COMMAND
#define COLADDR 0x2A //COMMAND
#define ROWADDR 0X2B //COMMAND
#define GAMMAPOSITIVE 0xE0 //COMMAND
#define GAMMANEGATIVE 0xE1

//TFT PIN
#define ST7735_SPI_PORT hspi1
extern SPI_HandleTypeDef ST7735_SPI_PORT;
#define ST7735_CS_Pin        GPIO_PIN_4
#define ST7735_CS_GPIO_Port  GPIOA
#define ST7735_RES_Pin       GPIO_PIN_0
#define ST7735_RES_GPIO_Port GPIOB
#define ST7735_DC_Pin        GPIO_PIN_1
#define ST7735_DC_GPIO_Port  GPIOB


//COLOR
#define WHITE    0xFFFF
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define MAGENTA  0xF81F
#define GREEN    0x07E0
#define CYAN     0x7FFF
#define YELLOW   0xFFE0
#define BROWN 	 0XBC40
#define GRAY  	 0X8430


void ST7735_init(void);
void fillFullScreen(uint16_t color);
void ST7735_WriteChar(uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bg);
void ST7735_WriteString(uint8_t x, uint8_t y, char* str, FontDef font, uint16_t color, uint16_t bg);
void ST7735_Run();
#endif
