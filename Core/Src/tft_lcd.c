/*
 * tft_lcd.c
 *
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
 */

#include "tft_lcd.h"
#include "math.h"

static uint16_t deg = 0;
static uint8_t min = 0;
static double sec;
static char buffer[32];

static inline void tft_lcd_test() {
    // Check border
    ST7735_FillScreen(ST7735_BLACK);

    for(int x = 0; x < ST7735_WIDTH; x++) {
        ST7735_DrawPixel(x, 0, ST7735_RED);
        ST7735_DrawPixel(x, ST7735_HEIGHT-1, ST7735_RED);
    }

    for(int y = 0; y < ST7735_HEIGHT; y++) {
        ST7735_DrawPixel(0, y, ST7735_RED);
        ST7735_DrawPixel(ST7735_WIDTH-1, y, ST7735_RED);
    }

    HAL_Delay(3000);

    // Check fonts
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_WriteString(0, 0, "Font_7x10, red on black, lorem ipsum dolor sit amet", Font_7x10, ST7735_RED, ST7735_BLACK);
    ST7735_WriteString(0, 3*10, "Font_11x18, green, lorem ipsum", Font_11x18, ST7735_GREEN, ST7735_BLACK);
    ST7735_WriteString(0, 3*10+3*18, "Font_16x26", Font_16x26, ST7735_BLUE, ST7735_BLACK);
    HAL_Delay(2000);

    // Check colors
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_WriteString(0, 0, "BLACK", Font_11x18, ST7735_WHITE, ST7735_BLACK);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_BLUE);
    ST7735_WriteString(0, 0, "BLUE", Font_11x18, ST7735_BLACK, ST7735_BLUE);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_RED);
    ST7735_WriteString(0, 0, "RED", Font_11x18, ST7735_BLACK, ST7735_RED);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_GREEN);
    ST7735_WriteString(0, 0, "GREEN", Font_11x18, ST7735_BLACK, ST7735_GREEN);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_CYAN);
    ST7735_WriteString(0, 0, "CYAN", Font_11x18, ST7735_BLACK, ST7735_CYAN);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_MAGENTA);
    ST7735_WriteString(0, 0, "MAGENTA", Font_11x18, ST7735_BLACK, ST7735_MAGENTA);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_YELLOW);
    ST7735_WriteString(0, 0, "YELLOW", Font_11x18, ST7735_BLACK, ST7735_YELLOW);
    HAL_Delay(500);

    ST7735_FillScreen(ST7735_WHITE);
    ST7735_WriteString(0, 0, "WHITE", Font_11x18, ST7735_BLACK, ST7735_WHITE);
    HAL_Delay(500);

#ifdef ST7735_IS_128X128
    // Display test image 128x128
    ST7735_DrawImage(0, 0, ST7735_WIDTH, ST7735_HEIGHT, (uint16_t*)test_img_128x128);

/*
    // Display test image 128x128 pixel by pixel
    for(int x = 0; x < ST7735_WIDTH; x++) {
        for(int y = 0; y < ST7735_HEIGHT; y++) {
            uint16_t color565 = test_img_128x128[y][x];
            // fix endiness
            color565 = ((color565 & 0xFF00) >> 8) | ((color565 & 0xFF) << 8);
            ST7735_DrawPixel(x, y, color565);
        }
    }
*/
    HAL_Delay(15000);
#endif // ST7735_IS_128X128
}


#if TFT_ENABLE_CONVERT_DMS
void decimal_to_dms(double decimal, uint16_t *deg, uint8_t *min, double *sec)
{
    double abs_val = fabs(decimal);

    *deg = (int)abs_val;
    double minutes = (abs_val - *deg) * 60.0;
    *min = (int)minutes;
    *sec = (minutes - *min) * 60.0;
}
#endif

static inline void tft_lcd_print_gps_imu_info(){
	uint16_t y = 0;
	snprintf(buffer, sizeof(buffer), "Battery:%d%%", battery_percent);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_CYAN, ST7735_BLACK);

#if TFT_ENABLE_CONVERT_DMS
	char lon_dir = (loc_gps_lon >= 0) ? 'E' : 'W';
	decimal_to_dms(loc_gps_lon, &deg, &min, &sec);
	snprintf(buffer, sizeof(buffer),
	         "LocLon:%c %03d*%02d'%05.2f\"",
			 lon_dir, deg, min, sec);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_CYAN, ST7735_BLACK);
	char lat_dir = (loc_gps_lat >= 0) ? 'N' : 'S';
	decimal_to_dms(loc_gps_lat, &deg, &min, &sec);
	snprintf(buffer, sizeof(buffer),
	         "LocLat:%c %03d*%02d'%05.2f\"",
			 lat_dir, deg, min, sec);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_CYAN, ST7735_BLACK);


#else
	char lon_dir = (loc_gps_lon >= 0) ? 'E' : 'W';
	snprintf(buffer, sizeof(buffer), "Loc_Lon:%c%.5f",lon_dir,fabs(loc_gps_lon));
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_CYAN, ST7735_BLACK);

	char lat_dir = (loc_gps_lat >= 0) ? 'N' : 'S';
	snprintf(buffer, sizeof(buffer), "Loc_Lat:%c%.5f", lat_dir,fabsf(loc_gps_lat) );
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_CYAN, ST7735_BLACK);
#endif

	snprintf(buffer, sizeof(buffer), "Loc_Alt:%.2f", loc_gps_alt);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_CYAN, ST7735_BLACK);

	snprintf(buffer, sizeof(buffer), "Loc_Azi:%.2f", loc_azi);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);

	snprintf(buffer, sizeof(buffer), "Loc_Pit:%.2f", loc_pitch);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);

	snprintf(buffer, sizeof(buffer), "Loc_Rol:%.2f", loc_roll);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);

	//TAG
#if TFT_ENABLE_CONVERT_DMS
    char tag_lon_dir = (tag_gps_lon >= 0) ? 'E' : 'W';
    decimal_to_dms(tag_gps_lon, &deg, &min, &sec);
    snprintf(buffer, sizeof(buffer),
             "TagLon:%c %03d*%02d'%05.2f\"",
			 tag_lon_dir,deg, min, sec);
    ST7735_WriteString(0, y += 12, buffer, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    char tag_lat_dir = (tag_gps_lat >= 0) ? 'N' : 'S';
    decimal_to_dms(tag_gps_lat, &deg, &min, &sec);
    snprintf(buffer, sizeof(buffer),
             "TagLat:%c %03d*%02d'%05.2f\"",
			 tag_lat_dir,deg, min, sec);
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_GREEN, ST7735_BLACK);
#else
    char tag_lon_dir = (tag_gps_lon >= 0) ? 'E' : 'W';
    snprintf(buffer, sizeof(buffer),
             "Tag_lon:%c%.5f",
			 tag_lon_dir, fabs(tag_gps_lon));

    ST7735_WriteString(0, y += 12, buffer, Font_7x10, ST7735_GREEN, ST7735_BLACK);
    char tag_lat_dir = (tag_gps_lat >= 0) ? 'N' : 'S';
    snprintf(buffer, sizeof(buffer),
             "Tag_lat:%c%.5f",
			 tag_lat_dir, fabs(tag_gps_lat));
    ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_GREEN, ST7735_BLACK);
#endif

	snprintf(buffer, sizeof(buffer), "Tag_Alt:%.2f", tag_gps_alt);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_GREEN, ST7735_BLACK);

	snprintf(buffer, sizeof(buffer), "Tag_Dis:%.2f", tag_distance);
	ST7735_WriteString(0, y += 10, buffer, Font_7x10, ST7735_GREEN, ST7735_BLACK);
}

void TFT_LCD_Init(){
	 ST7735_Init();
	 ST7735_FillScreen(ST7735_BLACK);
}

void TFT_LCD_Run(){
#if TFT_ENABLE_SPI_TEST
	tft_lcd_test();
#else
	tft_lcd_print_gps_imu_info();
#endif

}

