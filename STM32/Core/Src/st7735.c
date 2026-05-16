#include "st7735.h"

static char buffer[32];

static inline void ST7735_sendCMD(uint8_t cmd){
	 osSemaphoreWait(spiDmaSemHandle, osWaitForever);
	 HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
	 HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, &cmd, sizeof(cmd));
}

static inline void ST7735_sendData(uint8_t* buff, size_t buff_size) {
	osSemaphoreWait(spiDmaSemHandle, osWaitForever);
	HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, buff, buff_size);
}

static inline void ST7735_sendCMDCPU(uint8_t cmd){
	 HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
	 HAL_SPI_Transmit(&ST7735_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
	 HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}

static inline void ST7735_sendDataCPU(uint8_t* buff, size_t buff_size) {
	HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&ST7735_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}


static inline void setPos(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1){
	ST7735_sendCMDCPU(COLADDR); //Column address set 0->160 XS[15:8] XS[7:0] XE[15:8] XE[7:0]
	ST7735_sendDataCPU((uint8_t[]){0x00, x0, 0x00, x1}, 4);

	ST7735_sendCMDCPU(ROWADDR); //Row address set 0->128 YS[15:8] YS[7:0] YE[15:8] YE[7:0]
	ST7735_sendDataCPU((uint8_t[]){0x00, y0, 0x00, y1}, 4);
}
uint16_t framebuf[160 * 128];
static inline void fb_setPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= 160 || y >= 128) return;
    framebuf[y * 160 + x] = color;
}
static inline void fb_clear(uint16_t color)
{
    uint16_t c = (color << 8) | (color >> 8);
    for (int i = 0; i < 160 * 128; i++) framebuf[i] = c;
}
static inline void drawPixel(uint8_t x, uint8_t y, uint16_t color){
	if (x >= 160 || y >= 128) return;
	setPos(x, y, x + 1, y + 1);;
	ST7735_sendCMDCPU(0x2C);
	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;
	uint8_t colour[] = {hi, lo};
	ST7735_sendData(colour, 2);

}

void ST7735_WriteChar(uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bg){
	for (int i = 0; i < font.height; i++){
        uint16_t line = font.data[(ch - 32) * font.height + i];

        for (int j = 0; j < font.width; j++)
        {
            if (line & (1 << (15 - j)))
                fb_setPixel(x + j, y + i, color);
            else
                fb_setPixel(x + j, y + i, bg);
        }
    }
}

void ST7735_WriteString(uint8_t x, uint8_t y, char* str, FontDef font, uint16_t color, uint16_t bg){
	int i = 0;
	while (str[i] != '\0'){
		ST7735_WriteChar(x, y, str[i], font, color, bg);
        x += font.width;
        ++i;
    }

}



void ST7735_Flush(void)
{
	osSemaphoreWait(spiDmaSemHandle, osWaitForever);
    setPos(0, 0, 159, 127);

    ST7735_sendCMDCPU(0x2C);

    HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);

//    ST7735_sendData((uint8_t*)framebuf, sizeof(framebuf));
    HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, (uint8_t*)framebuf, sizeof(framebuf));
}


int check = 0;
void ST7735_Run()
{
    uint16_t y = 0;

    // ==== DRAW ====

    snprintf(buffer, sizeof(buffer),
             "Battery:%d%% HDOP:%03.2f",
			 0, gps_hdop);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, YELLOW, BLACK);

    char lon_dir = (loc_gps_lon >= 0) ? 'E' : 'W';
    snprintf(buffer, sizeof(buffer),
             "Loc_Lon:%c %.6f",
             lon_dir, fabsf(loc_gps_lon));
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, CYAN, BLACK);

    char lat_dir = (loc_gps_lat >= 0) ? 'N' : 'S';
    snprintf(buffer, sizeof(buffer),
             "Loc_Lat:%c %.6f",
             lat_dir, fabsf(loc_gps_lat));
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, CYAN, BLACK);

    snprintf(buffer, sizeof(buffer),
             "Loc_Alt:%.2f",
             loc_gps_alt);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, CYAN, BLACK);

    snprintf(buffer, sizeof(buffer),
             "Loc_Azi:%.2f",
             loc_azi);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, WHITE, BLACK);

    snprintf(buffer, sizeof(buffer),
             "Loc_Pit:%.2f",
             loc_pitch);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, WHITE, BLACK);

    snprintf(buffer, sizeof(buffer),
             "Loc_Rol:%.2f",
             loc_roll);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, WHITE, BLACK);

    // ==== TAG ====

    char tag_lon_dir = (tag_gps_lon >= 0) ? 'E' : 'W';
    snprintf(buffer, sizeof(buffer),
             "Tag_lon:%c %.6f",
             tag_lon_dir, fabsf(tag_gps_lon));
    ST7735_WriteString(0, y += 12, buffer, Font_7x10, GREEN, BLACK);

    char tag_lat_dir = (tag_gps_lat >= 0) ? 'N' : 'S';
    snprintf(buffer, sizeof(buffer),
             "Tag_lat:%c %.6f",
             tag_lat_dir, fabsf(tag_gps_lat));
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, GREEN, BLACK);

    snprintf(buffer, sizeof(buffer),
             "Tag_Alt:%.2f",
             tag_gps_alt);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, GREEN, BLACK);

    snprintf(buffer, sizeof(buffer),
             "Tag_Dis:%.2f",
             tag_distance);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, GREEN, BLACK);

    ST7735_Flush();
}

void ST7735_init(){
	HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	//SW RESET
	ST7735_sendCMDCPU(RST);
	HAL_Delay(150);
	ST7735_sendCMDCPU(WAKE);//sleep out
	HAL_Delay(150);
	uint8_t framerate[] = { RTNA, FPA, BPA };

	//FRAME
	ST7735_sendCMDCPU(NORMAL_MODE_FRAME);
	ST7735_sendDataCPU(framerate, sizeof(framerate));

	ST7735_sendCMDCPU(IDLE_MODE_FRAME);
	ST7735_sendDataCPU(framerate, sizeof(framerate));

	ST7735_sendCMDCPU(PARTIAL_MODE_FRAME);
	ST7735_sendDataCPU(framerate, sizeof(framerate));
	ST7735_sendDataCPU(framerate, sizeof(framerate));

	//Inverse of
	ST7735_sendCMDCPU(INVERSION_ON);
	ST7735_sendDataCPU((uint8_t[]){0x07}, 1);

	//POWER
	ST7735_sendCMDCPU(POWER1);  //Power Control 1
	ST7735_sendDataCPU((uint8_t[]){0xA2, 0x02, 0x84}, 3);

	ST7735_sendCMDCPU(POWER2);  //Power Control 2
	ST7735_sendDataCPU((uint8_t[]){0xC5}, 1);

	ST7735_sendCMDCPU(POWER3); //Power Control 3 (in Normal mode/ Full colors)
	ST7735_sendDataCPU((uint8_t[]){0x0A, 0x00}, 2);


	ST7735_sendCMDCPU(POWER4);  //Power Control 4 (in Idle mode/ 8-colors)
	ST7735_sendDataCPU((uint8_t[]){0x8A, 0x2A}, 2);

	ST7735_sendCMDCPU(POWER5);  //Power Control 5 (in Partial mode/ full-colors)
	ST7735_sendDataCPU((uint8_t[]){0x8A, 0xEE}, 2);

	ST7735_sendCMDCPU(VCOM);  //VCOM Control 1
	ST7735_sendDataCPU((uint8_t[]){0x0E}, 1);

	//Inverse off
	ST7735_sendCMDCPU(INVERSION_OFF);  //Display Inversion Off

	//Mem
	ST7735_sendCMDCPU(MEMACCESS);  //Memory Data Access Control
	ST7735_sendDataCPU((uint8_t[]){0x60}, 1); //RGB mode + mode screen ngang

	ST7735_sendCMDCPU(0x3A); //Interface Pixel Format
	ST7735_sendDataCPU((uint8_t[]){0x05}, 1);

	ST7735_sendCMDCPU(COLADDR); //Column address set 0->160 XS[15:8] XS[7:0] XE[15:8] XE[7:0]
	ST7735_sendDataCPU((uint8_t[]){0x00, 0, 0x00, 159}, 4);

	ST7735_sendCMDCPU(ROWADDR); //Row address set 0->128 YS[15:8] YS[7:0] YE[15:8] YE[7:0]
	ST7735_sendDataCPU((uint8_t[]){0x00, 0, 0x00, 127}, 4);

	uint8_t gamma_pos[] = {
	    0x02,0x1C,0x07,0x12,0x37,0x32,0x29,0x2D,
	    0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10
	};

	uint8_t gamma_neg[] = {
	    0x03,0x1D,0x07,0x06,0x2E,0x2C,0x29,0x2D,
	    0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10
	};
	ST7735_sendCMDCPU(GAMMAPOSITIVE);
	ST7735_sendDataCPU(gamma_pos, sizeof(gamma_pos));

	ST7735_sendCMDCPU(GAMMANEGATIVE);
	ST7735_sendDataCPU(gamma_neg, sizeof(gamma_neg));

	ST7735_sendCMDCPU(0x13); //Partial off (Normal)

	ST7735_sendCMDCPU(0x29); //Display on

	ST7735_sendCMDCPU(0x2C); // RAMWR

	fillFullScreen(BLACK);

}

void fillFullScreen(uint16_t color){
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    ST7735_sendCMDCPU(0x2C); // RAMWR

    HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);

    for (int i = 0; i < 160 * 128; i++) {
        uint8_t colour[2] = {hi, lo};
        HAL_SPI_Transmit(&ST7735_SPI_PORT, colour, 2, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}
