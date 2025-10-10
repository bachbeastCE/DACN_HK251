/*
 * gps.h
 *
 *  Created on: Oct 10, 2025
 *      Author: ASUS
 */

#ifndef _GPS_H_
#define _GPS_H_

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "main.h"

//======================== CONFIG =================================//
#define GPS_UART_PORT huart2
extern UART_HandleTypeDef GPS_UART_PORT;

extern UART_HandleTypeDef huart1;

#define GPS_UART_BUFFER_SIZE 256
#define GPS_BUFFER_SIZE 256
#define MAX_SENTENCE 128

extern uint8_t gps_uart_idx;
extern uint8_t gps_uart_tranfer_count;
extern uint8_t gps_uart_copy_flag;
extern uint8_t gps_uart_rx_buffer[GPS_UART_BUFFER_SIZE];

//======================== COMMAND =================================//

//Startup mode
#define HOT_START       "$PMTK101"
#define WARM_START      "$PMTK102"
#define COLD_START      "$PMTK103"
#define FULL_COLD_START "$PMTK104"

//Standby mode -- Exit requires high level trigger
#define SET_PERPETUAL_STANDBY_MODE      "$PMTK161"

#define SET_PERIODIC_MODE               "$PMTK225"
#define SET_NORMAL_MODE                 "$PMTK225,0"
#define SET_PERIODIC_BACKUP_MODE        "$PMTK225,1,1000,2000"
#define SET_PERIODIC_STANDBY_MODE       "$PMTK225,2,1000,2000"
#define SET_PERPETUAL_BACKUP_MODE       "$PMTK225,4"
#define SET_ALWAYSLOCATE_STANDBY_MODE   "$PMTK225,8"
#define SET_ALWAYSLOCATE_BACKUP_MODE    "$PMTK225,9"

//Set the message interval,100ms~10000ms
#define SET_POS_FIX         "$PMTK220"
#define SET_POS_FIX_100MS   "$PMTK220,100"
#define SET_POS_FIX_200MS   "$PMTK220,200"
#define SET_POS_FIX_400MS   "$PMTK220,400"
#define SET_POS_FIX_800MS   "$PMTK220,800"
#define SET_POS_FIX_1S      "$PMTK220,1000"
#define SET_POS_FIX_2S      "$PMTK220,2000"
#define SET_POS_FIX_4S      "$PMTK220,4000"
#define SET_POS_FIX_8S      "$PMTK220,8000"
#define SET_POS_FIX_10S     "$PMTK220,10000"

//Switching time output
#define SET_SYNC_PPS_NMEA_OFF   "$PMTK255,0"
#define SET_SYNC_PPS_NMEA_ON    "$PMTK255,1"

//Baud rate
#define SET_NMEA_BAUDRATE           "$PMTK251"
#define SET_NMEA_BAUDRATE_115200    "$PMTK251,115200"
#define SET_NMEA_BAUDRATE_57600     "$PMTK251,57600"
#define SET_NMEA_BAUDRATE_38400     "$PMTK251,38400"
#define SET_NMEA_BAUDRATE_19200     "$PMTK251,19200"
#define SET_NMEA_BAUDRATE_14400     "$PMTK251,14400"
#define SET_NMEA_BAUDRATE_9600      "$PMTK251,9600"
#define SET_NMEA_BAUDRATE_4800      "$PMTK251,4800"

//To restore the system default setting
#define SET_REDUCTION               "$PMTK314,-1"

//Set NMEA sentence output frequencies
#define SET_NMEA_OUTPUT   			"$PMTK314,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
#define SET_NMEA_OUTPUT_RMC_ONLY  	"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"

//======================== CONTROL  =================================//
//GPS Control register
/**
 * 0: READ
 * 1: WRITE
 * 2: ALT_BUF
 * 3-4: IN_MODE : INPUT MODE: ALL(0), RMC_ONLY(1),GGA_ONLY(2), RMC & GGA(3)
 * 5-31: Reverse
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

//======================== FUNCTION  =================================//

uint8_t GPS_CaculateChecksum(const char *sentence);
uint8_t GPS_SendCommand(const char *data);
uint8_t GPS_ReceiveRawData(uint8_t* uart_rx_buffer,uint16_t size);

uint8_t GPS_Init();
uint8_t GPS_Data_Update();
uint8_t GPS_RMC_Get (RMC_t *result);
uint8_t GPS_GGA_Get (GGA_t *result);
uint8_t GPS_Coordinates_Get (COORDINATES_t *result);

void GPS_RMC_Print(RMC_t * rmc);
void GPS_GGA_Print(GGA_t * gga);
void GPS_Coordinates_Print(COORDINATES_t * coordinates);

#endif
