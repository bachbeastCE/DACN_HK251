/*
 * gps.c
 *
 *  Created on: Sep 18, 2025
 *      Author: ASUS
 */

#include "gps.h"
uint8_t gps_uart_rx_buffer[GPS_UART_BUFFER_SIZE];

static char sentence[128] = {0};
static GGA_t gga_tmp = {0};
static RMC_t rmc_tmp = {0};


static uint8_t GPS_CaculateChecksum(const char *sentence)
{
    uint8_t checksum = 0;
    int i = 0;

    // If begin letter is '$' then continue
    if (sentence[0] == '$')
        sentence++;

    // XOR all letter until meet '*' or end of string
    while (sentence[i] != '\0' && sentence[i] != '*')
    {
        checksum ^= (uint8_t)sentence[i];
        i++;
    }
    return checksum;
}

uint8_t GPS_SendCommand(const char *data)
{
    char buffer[128];
    uint8_t checksum;
    int len;

    // If string doesn't have '$', we add it
    if (data[0] != '$')
    {
        snprintf(buffer, sizeof(buffer), "$%s", data);
        data = buffer; // point to new buffer
    }

    // check sum for command
    checksum = GPS_CaculateChecksum(data);

    // Complete command: <command>*<checksum>\r\n
    char finalCmd[128];
    len = snprintf(finalCmd, sizeof(finalCmd), "%s*%02X\r\n", data, checksum);

    // Send it via UART
    if (HAL_UART_Transmit(&GPS_UART_PORT, (uint8_t *)finalCmd, len, 1000) == HAL_OK)
        return 0;
    else
        return 1;
}

uint8_t GPS_Init()
{
//    // Turn on only GGA and RMC
//    GPS_SendCommand("$PAIR062,1,0");
//    HAL_Delay(100); // tiny delay between commands
//
//    GPS_SendCommand("$PAIR062,2,0");
//    HAL_Delay(100);
//
//    GPS_SendCommand("$PAIR062,3,0");
//    HAL_Delay(100);
//
//    GPS_SendCommand("$PAIR062,5,0");
//    HAL_Delay(100);

	GPS_SendCommand(SET_GGA_SENTENCE_RATE(1));
	GPS_SendCommand(SET_GLL_SENTENCE_RATE(0));
	GPS_SendCommand(SET_GSA_SENTENCE_RATE(0));
	GPS_SendCommand(SET_GSV_SENTENCE_RATE(0));
	GPS_SendCommand(SET_RMC_SENTENCE_RATE(1));
	GPS_SendCommand(SET_VTG_SENTENCE_RATE(0));
	GPS_SendCommand(SET_FIX_RATE(500));
	GPS_SendCommand(SET_DECIMAL_PRECISION(3));

    HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART_PORT, gps_uart_rx_buffer, GPS_UART_BUFFER_SIZE);
    return 0;
}

static RMC_t parseRMC(const char* sentence) {
    RMC_t data = {0};
    char timeStr[11] = {0}, latStr[15] = {0}, lonStr[15] = {0}, dateStr[7] = {0};
    char latArea=0, lonArea=0, status=0, modeInd=0, navStatus=0;

    // Parse TalkerID + các trường quan trọng
    sscanf(sentence, "$%2sRMC,%10[^,],%c,%14[^,],%c,%14[^,],%c,%lf,%lf,%6[^,],,,%c,%c",
           data.TalkerID, timeStr, &status, latStr, &latArea, lonStr, &lonArea,
           &data.SOG, &data.COG, dateStr, &modeInd, &navStatus);

    data.Status = status;
    data.ModeInd = modeInd;
    data.NavStatus = navStatus;

    // Thời gian
    int timeInt = atoi(timeStr);
    data.Time_H = timeInt / 10000;
    data.Time_M = (timeInt / 100) % 100;
    data.Time_S = timeInt % 100;

    //Area
    data.Lat_area = latArea;    // 'N' hoặc 'S'
    data.Lon_area = lonArea;    // 'E' hoặc 'W'

    // Latitude
    double lat = atof(latStr);
    int latDeg = (int)(lat / 100);
    double latMin = lat - latDeg * 100;
    data.Lat = latDeg + latMin / 60.0;
    if(latArea == 'S') data.Lat = -data.Lat;

    // Longitude
    double lon = atof(lonStr);
    int lonDeg = (int)(lon / 100);
    double lonMin = lon - lonDeg * 100;
    data.Lon = lonDeg + lonMin / 60.0;
    if(lonArea == 'W') data.Lon = -data.Lon;

    // Date DD/MM/YYYY
    if(strlen(dateStr) == 6){
        char temp[3] = {0};
        temp[0] = dateStr[0]; temp[1] = dateStr[1];
        data.Day = atoi(temp);
        temp[0] = dateStr[2]; temp[1] = dateStr[3];
        data.Month = atoi(temp);
        temp[0] = dateStr[4]; temp[1] = dateStr[5];
        data.Year = atoi(temp);
    }

    return data;
}

static GGA_t parseGGA(const char* sentence) {
    GGA_t data = {0};
    char latStr[15]={0}, lonStr[15]={0};

    int fix = 0, numSV = 0;
    double hdop=0.0, alt=0.0, geoid=0.0;
    char latArea=0, lonArea=0;

    // Parse data fields
    sscanf(sentence, "$%*2sGGA,%*[^,],%14[^,],%c,%14[^,],%c,%d,%d,%lf,%lf,%*c,%lf,%*c",
           latStr, &latArea, lonStr, &lonArea, &fix, &numSV, &hdop, &alt, &geoid);

    data.FixQuality = fix;
    data.NumSV = numSV;
    data.HDOP = hdop;
    data.Altitude = alt;
    data.GeoidSep = geoid;
    data.Lat_area = latArea;
    data.Lon_area = lonArea;

    // Convert latitude sang decimal degree
    double lat = atof(latStr);
    int latDeg = (int)(lat / 100);
    double latMin = lat - latDeg*100;
    data.Lat = latDeg + latMin/60.0;
    if(latArea=='S') data.Lat = -data.Lat;

    // Convert longitude sang decimal degree
    double lon = atof(lonStr);
    int lonDeg = (int)(lon / 100);
    double lonMin = lon - lonDeg*100;
    data.Lon = lonDeg + lonMin/60.0;
    if(lonArea=='W') data.Lon = -data.Lon;

    return data;
}

uint8_t GPS_Data_Update() {
	const char *p;
	const char *start;
	p = (char*)gps_uart_rx_buffer;
	start = (char*)gps_uart_rx_buffer;

    while (*p) {
        if (*p == '\r' && *(p+1) == '\n') {
            int len = p - start;
            if (len <= 0) { p += 2; start = p; continue; }

            if (len >= (int)sizeof(sentence)) len = sizeof(sentence) - 1;

            memcpy(sentence, start, len);
            sentence[len] = '\0';

            // Kiểm tra loại câu
            if (strstr(sentence, "GGA")) {
                gga_tmp = parseGGA(sentence);
            } else if (strstr(sentence, "RMC")) {
                rmc_tmp = parseRMC(sentence);
            }

            // bỏ qua \r\n
            p += 2;
            start = p;
        } else {
            p++;
        }
    }
    return 0;
}

uint8_t GPS_RMC_Get(RMC_t *result){
    *result = rmc_tmp;
    return 0;
}

uint8_t  GPS_GGA_Get(GGA_t *result){
    *result = gga_tmp;
    return 0;
}

uint8_t GPS_Coordinates_Get (COORDINATES_t *result){
	result->Lat = gga_tmp.Lat;
	result->Lon = gga_tmp.Lon;
	result->Alt = gga_tmp.Altitude;
    return 0;
}

uint8_t GPS_Status_Get(){
	if(gga_tmp.HDOP != 0) return 1;
	else return 0;
}

double GPS_HDOP_Get(){
	return gga_tmp.HDOP;
}


// CONVERT WGS84 <=> ENU

/* Deg/Rad conversion */
static inline float deg2rad(float deg) { return deg * (float)M_PI / 180.0f; }
static inline float rad2deg(float rad) { return rad * 180.0f / (float)M_PI; }

/* -------------------------------------------------------------------------- */
/* WGS84 -> ECEF */
void wgs84_to_ecef(float lat_deg, float lon_deg, float h,
                   float *X, float *Y, float *Z)
{
    float lat = deg2rad(lat_deg);
    float lon = deg2rad(lon_deg);

    float sin_lat = sinf(lat);
    float cos_lat = cosf(lat);
    float sin_lon = sinf(lon);
    float cos_lon = cosf(lon);

    float N = WGS84_A / sqrtf(1.0f - WGS84_E2 * sin_lat * sin_lat);

    *X = (N + h) * cos_lat * cos_lon;
    *Y = (N + h) * cos_lat * sin_lon;
    *Z = (N * (1.0f - WGS84_E2) + h) * sin_lat;
}

/* ECEF -> WGS84 */
void ecef_to_wgs84(float X, float Y, float Z,
                   float *lat_deg, float *lon_deg, float *h)
{
    float a = WGS84_A;
    float e2 = WGS84_E2;
    float eps = 1e-6f;

    float lon = atan2f(Y, X);
    float p = sqrtf(X*X + Y*Y);

    float lat = atan2f(Z, p * (1 - e2));
    float lat_prev;
    float N;

    for (int i = 0; i < 5; i++) {
        lat_prev = lat;
        N = a / sqrtf(1.0f - e2 * sinf(lat) * sinf(lat));
        *h = p / cosf(lat) - N;
        lat = atan2f(Z + e2 * N * sinf(lat), p);
        if (fabsf(lat - lat_prev) < eps)
            break;
    }

    *lat_deg = rad2deg(lat);
    *lon_deg = rad2deg(lon);
}

void enu_rotation_matrix(float lat0_deg, float lon0_deg, float R[3][3])
{
    float lat0 = deg2rad(lat0_deg);
    float lon0 = deg2rad(lon0_deg);

    float sin_lat0 = sinf(lat0);
    float cos_lat0 = cosf(lat0);
    float sin_lon0 = sinf(lon0);
    float cos_lon0 = cosf(lon0);

    /* East-North-Up rotation matrix */
    R[0][0] = -sin_lon0;
    R[0][1] =  cos_lon0;
    R[0][2] =  0.0f;

    R[1][0] = -sin_lat0 * cos_lon0;
    R[1][1] = -sin_lat0 * sin_lon0;
    R[1][2] =  cos_lat0;

    R[2][0] =  cos_lat0 * cos_lon0;
    R[2][1] =  cos_lat0 * sin_lon0;
    R[2][2] =  sin_lat0;
}

void wgs84_to_enu(float lat, float lon, float h,
                  float lat0, float lon0, float h0,
                  float *E, float *N, float *U)
{
    float X, Y, Z;
    float X0, Y0, Z0;
    float R[3][3];

    enu_rotation_matrix(lat0, lon0, R);
    wgs84_to_ecef(lat, lon, h, &X, &Y, &Z);
    wgs84_to_ecef(lat0, lon0, h0, &X0, &Y0, &Z0);

    float dX = X - X0;
    float dY = Y - Y0;
    float dZ = Z - Z0;

    *E = R[0][0]*dX + R[0][1]*dY + R[0][2]*dZ;
    *N = R[1][0]*dX + R[1][1]*dY + R[1][2]*dZ;
    *U = R[2][0]*dX + R[2][1]*dY + R[2][2]*dZ;
}

void enu_to_wgs84(float E, float N, float U,
                  float lat0, float lon0, float h0,
                  float *lat, float *lon, float *h)
{
    float R[3][3];
    float lat0_rad = deg2rad(lat0);
    float lon0_rad = deg2rad(lon0);
    float sin_lat0 = sinf(lat0_rad), cos_lat0 = cosf(lat0_rad);
    float sin_lon0 = sinf(lon0_rad), cos_lon0 = cosf(lon0_rad);

    enu_rotation_matrix(lat0, lon0, R);

    // ECEF của gốc
    float N0 = WGS84_A / sqrtf(1.0f - WGS84_E2 * sin_lat0 * sin_lat0);
    float X0 = (N0 + h0) * cos_lat0 * cos_lon0;
    float Y0 = (N0 + h0) * cos_lat0 * sin_lon0;
    float Z0 = (N0 * (1.0f - WGS84_E2) + h0) * sin_lat0;

    // ENU -> ECEF (R^T)
    float dX = R[0][0]*E + R[1][0]*N + R[2][0]*U;
    float dY = R[0][1]*E + R[1][1]*N + R[2][1]*U;
    float dZ = R[0][2]*E + R[1][2]*N + R[2][2]*U;

    float X = X0 + dX;
    float Y = Y0 + dY;
    float Z = Z0 + dZ;

    // ECEF -> WGS84
    ecef_to_wgs84(X, Y, Z, lat, lon, h);
}




