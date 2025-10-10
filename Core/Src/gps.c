/*
 * gps.c
 *
 *  Created on: Sep 18, 2025
 *      Author: ASUS
 */

#include "gps.h"

volatile uint32_t __gps_ctrl_reg = 0x07;

uint8_t gps_uart_idx = 0;
uint8_t gps_uart_tranfer_count = 0;
uint8_t gps_uart_rx_buffer[GPS_UART_BUFFER_SIZE];
uint8_t gps_main_buffer[GPS_BUFFER_SIZE];
uint8_t gps_alt_buffer[GPS_BUFFER_SIZE];
GGA_t gga_tmp = {0};
RMC_t rmc_tmp = {0};
COORDINATES_t coordinates_tmp = {0};
uint8_t gps_uart_copy_flag = 0;
char sentence[MAX_SENTENCE] = {0};

uint8_t GPS_CaculateChecksum(const char *sentence)
{
    uint8_t checksum = 0;
    int i = 0;

    // Nếu bắt đầu bằng '$' thì bỏ qua
    if (sentence[0] == '$')
        sentence++;

    // XOR tất cả ký tự cho đến khi gặp '*' hoặc kết thúc chuỗi
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

    // Nếu chuỗi chưa có '$', ta thêm vào
    if (data[0] != '$')
    {
        snprintf(buffer, sizeof(buffer), "$%s", data);
        data = buffer; // trỏ lại buffer mới
    }

    // Tính checksum cho câu lệnh
    checksum = GPS_CaculateChecksum(data);

    // Ghép chuỗi hoàn chỉnh: <lệnh>*<checksum>\r\n
    char finalCmd[128];
    len = snprintf(finalCmd, sizeof(finalCmd), "%s*%02X\r\n", data, checksum);

    // Gửi qua UART
    if (HAL_UART_Transmit(&GPS_UART_PORT, (uint8_t *)finalCmd, len, 1000) == HAL_OK)
        return 0;
    else
        return 1;
}

uint8_t GPS_Init()
{
    // Gửi 4 lệnh khởi tạo đến GPS
    GPS_SendCommand("$PAIR062,1,0");
    HAL_Delay(100); // delay nhỏ giữa các lệnh

    GPS_SendCommand("$PAIR062,2,0");
    HAL_Delay(100);

    GPS_SendCommand("$PAIR062,3,0");
    HAL_Delay(100);

    GPS_SendCommand("$PAIR062,5,0");
    HAL_Delay(100);

    HAL_UARTEx_ReceiveToIdle_DMA(&GPS_UART_PORT, gps_uart_rx_buffer, GPS_UART_BUFFER_SIZE);

    return 0;
}

RMC_t parseRMC(const char* sentence) {
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

    // Ngày/Tháng/Năm
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

GGA_t parseGGA(const char* sentence) {
    GGA_t data = {0};
    char latStr[15]={0}, lonStr[15]={0};

    int fix = 0, numSV = 0;
    double hdop=0.0, alt=0.0, geoid=0.0;
    char latArea=0, lonArea=0;

    // Parse từng trường
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

uint8_t GPS_ReceiveRawData(uint8_t* uart_rx_buffer,uint16_t size){
	if(size > GPS_BUFFER_SIZE) return GPS_BUFFER_OVERFLOW;
	if(gps_ctrl_check(GPS_READ_BIT)){
		memcpy(gps_alt_buffer,uart_rx_buffer,size);
		gps_ctrl_set(GPS_ALT_BIT);
		return GPS_ALT_BUFFER;
	}
	else{
		gps_ctrl_set(GPS_WRITE_BIT);
		memcpy(gps_main_buffer,uart_rx_buffer,size);
		gps_ctrl_clear(GPS_WRITE_BIT);
		return 0;
	}
}

uint8_t GPS_Data_Update() {
	const char *p;
	const char *start;
	if(gps_ctrl_check(GPS_ALT_BIT)){
		gps_ctrl_clear(GPS_ALT_BIT);
		p = (char*)gps_alt_buffer;
		start = (char*)gps_alt_buffer;

	}
	else{
		if(gps_ctrl_check(GPS_WRITE_BIT)){
			int cnt = 0;
			while(cnt < 20){
				if(gps_ctrl_check(GPS_WRITE_BIT)){
					++cnt;
					//delay_ms(1);
					HAL_Delay(2);
				}
				else{
					break;
				}
			}
			if(cnt >= 20) return GPS_TIMEOUT;
		}
		gps_ctrl_set(GPS_READ_BIT);
		p = (char*)gps_main_buffer;
		start = (char*)gps_main_buffer;
	}

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

	coordinates_tmp.Lat = rmc_tmp.Lat;
	coordinates_tmp.Lon = rmc_tmp.Lon;
	coordinates_tmp.Lat_area = rmc_tmp.Lat_area;
	coordinates_tmp.Lon_area = rmc_tmp.Lon_area;

    gps_ctrl_clear(GPS_READ_BIT);

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
    *result = coordinates_tmp;
    return 0;
}

void GPS_RMC_Print(RMC_t *rmc){
    char tmp[256];
    int len = snprintf(tmp, sizeof(tmp),
        "RMC: Talker=%s, Time=%02d:%02d:%02d, Status=%c\r\n"
        " Lat=%.6f %c, Lon=%.6f %c\r\n"
        " SOG=%.2f kn, COG=%.2f deg\r\n"
        " Date=%02d/%02d/%02d, Mode=%c, Nav=%c\r\n",
        rmc->TalkerID,
        rmc->Time_H + 7, rmc->Time_M, rmc->Time_S,  // cộng thêm timezone VN
        rmc->Status,
        rmc->Lat, rmc->Lat_area,
        rmc->Lon, rmc->Lon_area,
        rmc->SOG, rmc->COG,
        rmc->Day, rmc->Month, rmc->Year,
        rmc->ModeInd, rmc->NavStatus
    );
    HAL_UART_Transmit(&huart1, (uint8_t*)tmp, len, HAL_MAX_DELAY);
}

void GPS_GGA_Print(GGA_t *gga){
    char tmp[256];
    int len = snprintf(tmp, sizeof(tmp),
        "GGA: Lat=%.6f %c, Lon=%.6f %c\r\n"
        " FixQuality=%d, NumSV=%d, HDOP=%.2f\r\n"
        " Altitude=%.3f m, GeoidSep=%.3f m\r\n",
        gga->Lat, gga->Lat_area,
        gga->Lon, gga->Lon_area,
        gga->FixQuality, gga->NumSV, gga->HDOP,
        gga->Altitude, gga->GeoidSep
    );
    HAL_UART_Transmit(&huart1, (uint8_t*)tmp, len, HAL_MAX_DELAY);
}

void GPS_Coordinates_Print(COORDINATES_t * coordinates){
	char tmp[50];
	int len = snprintf(tmp, sizeof(tmp),
		"Lat: %.6f %c, Lon: %.6f %c\r\n",
		coordinates->Lat, coordinates->Lat_area, coordinates->Lon, coordinates->Lon_area);
	HAL_UART_Transmit(&huart1, (uint8_t*)tmp, len, HAL_MAX_DELAY);
}
