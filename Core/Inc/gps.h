
/*
 * gps.h
 *
 *  Created on: Oct 10, 2025
 *      Author: ASUS
 */

#ifndef _GPS_H_
#define _GPS_H_

#include <math.h>
#include <main.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kf.h"
#include "global.h"

//======================== CONFIG =================================//
#define GPS_UART_PORT huart2
extern UART_HandleTypeDef GPS_UART_PORT;

extern UART_HandleTypeDef huart1;

#define GPS_UART_BUFFER_SIZE 256

extern uint8_t gps_uart_idx;
extern uint8_t gps_uart_tranfer_count;
extern uint8_t gps_uart_copy_flag;
extern uint8_t gps_uart_rx_buffer[GPS_UART_BUFFER_SIZE];

//======================== CONTROL  =================================//
//GPS Control register
/**
 * 0: READ
 * 1: WRITE
 * 2: ALT_BUF
 * 3-31: Reverse
 **/

extern volatile uint32_t __gps_ctrl_reg;

// ===== Bit masks =====
#define GPS_READ_BIT     (1U << 0)   // bit 0
#define GPS_WRITE_BIT    (1U << 1)   // bit 1
#define GPS_ALT_BIT      (1U << 2)   // bit 2

// Check
#define gps_ctrl_check(bit)     (__gps_ctrl_reg & (bit))

// Set
#define gps_ctrl_set(bit)       (__gps_ctrl_reg |= (bit))

// Clear
#define gps_ctrl_clear(bit)     (__gps_ctrl_reg &= ~(bit))

#define delay_ms(__ms) HAL_Delay(__ms)

//======================== ERROR =================================//

#define GPS_ALT_BUFFER 1
#define GPS_TIMEOUT 2
#define GPS_BUFFER_OVERFLOW 3

//======================== DATA STRUCT  =================================//

typedef struct {
	double Lat;       // Latitude (decimal degree)
	double Lon;       // Longtiude (decimal degree)
    char Lat_area;    // 'N' or 'S'
    char Lon_area;    // 'E' or 'W'
} COORDINATES_t;

typedef struct {
    char TalkerID[3]; // GP, GN, GL, GA, ...
    int Time_H;
    int Time_M;
    int Time_S;
    char Status;      // 'A' = fix, 'V' = no fix
    double Lat;       // decimal degree
    double Lon;       // decimal degree
    char Lat_area;    // 'N' hoặc 'S'
    char Lon_area;    // 'E' hoặc 'W'
    double SOG;       // knots
    double COG;       // degrees
    int Day;
    int Month;
    int Year;         // 2 chữ số
    char ModeInd;     // GNSS mode (A,D,E,N)
    char NavStatus;   // Navigation status
} RMC_t;

typedef struct {
    double Lat;       // vĩ độ (decimal degree)
    double Lon;       // kinh độ (decimal degree)
    char Lat_area;    // 'N' hoặc 'S'
    char Lon_area;    // 'E' hoặc 'W'
    int FixQuality;   // 0=no fix, 1=GPS fix, 2=DGPS fix
    int NumSV;        // số vệ tinh đang sử dụng
    double HDOP;      // Horizontal dilution of precision
    double Altitude;  // độ cao (m)
    double GeoidSep;  // chênh lệch geoid (m)
} GGA_t;

//======================== VARIABLE  =================================//
#define GPS_BUFFER_SIZE 256
#define MAX_SENTENCE 200


extern GGA_t gga_tmp;
extern RMC_t rmc_tmp;

extern KalmanFilter kf_lon;
extern KalmanFilter kf_lat;
extern KalmanFilter kf_alt;

//======================== FUNCTION  =================================//

uint8_t GPS_CaculateChecksum(const char *sentence);
uint8_t GPS_SendCommand(const char *data);
uint8_t GPS_ReceiveRawData(uint8_t* uart_rx_buffer,uint16_t size);

uint8_t GPS_Init();
uint8_t GPS_Data_Update();
uint8_t GPS_RMC_Get (RMC_t *result);
uint8_t GPS_GGA_Get (GGA_t *result);
uint8_t GPS_Coordinates_Get (COORDINATES_t *result);
uint8_t GPS_Filter();

void GPS_RMC_Print(RMC_t * rmc);
void GPS_GGA_Print(GGA_t * gga);
void GPS_Coordinates_Print(COORDINATES_t * coordinates);

#endif
