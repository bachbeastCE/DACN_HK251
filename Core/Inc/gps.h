
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
#include "global.h"

//======================== CONFIG =================================//
#define GPS_UART_PORT huart2
extern UART_HandleTypeDef GPS_UART_PORT;

#define GPS_UART_BUFFER_SIZE 256
extern uint8_t gps_uart_rx_buffer[GPS_UART_BUFFER_SIZE];

#define WGS84_A  6378137.0f
#define WGS84_E2 6.69437999014e-3f
//======================== COMMAND =================================//

//Startup mode
#define HOT_START       "$PMTK101"
#define WARM_START      "$PMTK102"
#define COLD_START      "$PMTK103"
#define FULL_COLD_START "$PMTK104"


//Set NMEA Sentences Rate
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define SET_GGA_SENTENCE_RATE(f) "$PAIR062,0," TOSTRING(f)
#define SET_GLL_SENTENCE_RATE(f) "$PAIR062,1," TOSTRING(f)
#define SET_GSA_SENTENCE_RATE(f) "$PAIR062,2," TOSTRING(f)
#define SET_GSV_SENTENCE_RATE(f) "$PAIR062,3," TOSTRING(f)
#define SET_RMC_SENTENCE_RATE(f) "$PAIR062,4," TOSTRING(f)
#define SET_VTG_SENTENCE_RATE(f) "$PAIR062,5," TOSTRING(f)

//Set the message interval,100ms~1000ms (Only for GGA and RMC)
#define SET_FIX_RATE(f) "$PAIR050," TOSTRING(f)


//======================== DATA STRUCT  =================================//

typedef struct {
	double Lat;       // Latitude (decimal degree)
	double Lon;       // Longtiude (decimal degree)
	double Alt;
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
uint8_t GPS_SendCommand(const char *data);
uint8_t GPS_Init();
uint8_t GPS_Data_Update();
uint8_t GPS_Get_Status();
uint8_t GPS_RMC_Get (RMC_t *result);
uint8_t GPS_GGA_Get (GGA_t *result);
uint8_t GPS_Coordinates_Get (COORDINATES_t *result);
void enu_to_wgs84(float E, float N, float U,
                  float lat0, float lon0, float h0,
                  float *lat, float *lon, float *h);
void wgs84_to_enu(float lat, float lon, float h,
                  float lat0, float lon0, float h0,
                  float *E, float *N, float *U);
#endif
