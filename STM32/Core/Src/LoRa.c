/*
 * LoRa.c
 *
 *  Created on: Feb 13, 2026
 *      Author: ASUS
 */

#include "LoRa.h"

/**
 * @brief Create a LoRa object with default configuration
 *
 * Default configuration:
 *  - Frequency        : 433 MHz
 *  - Spreading Factor : 7
 *  - Bandwidth        : 125 kHz
 *  - Coding Rate      : 4/5
 *  - Output Power     : 20 dBm
 *  - Preamble Length  : 8 symbols
 *
 * @return LoRa structure initialized with default values
 */
LoRa LoRa_create(GPIO_TypeDef* CS_port, uint16_t CS_pin, GPIO_TypeDef* reset_port, uint16_t reset_pin, SPI_HandleTypeDef* hSPIx){

	LoRa new_LoRa;

	new_LoRa.hSPIx                 	= hSPIx;
	new_LoRa.CS_port               	= CS_port;
	new_LoRa.CS_pin                	= CS_pin;
	new_LoRa.reset_port            	= reset_port;
	new_LoRa.reset_pin             	= reset_pin;
	new_LoRa.frequency             	= 433       		; //Defaut Fre 433(Asia)
	new_LoRa.spredingFactor        	= SF_7      		; //Defaut 7kHZ
	new_LoRa.bandWidth			   	= BW_125KHz 		; //Defaut 125kHZ
	new_LoRa.crcRate               	= CR_4_5    		; //Defaut 4/5
	new_LoRa.power				   	= POWER_20db		; //Defaut max 20db
	new_LoRa.overCurrentProtection 	= 100       		; //Defaut Imax 100 --> OcpTrim = 0x0b
	new_LoRa.preamble			   	= 8         		;

#if LORA_RTOS
	new_LoRa.lora_lock			   	= xSemaphoreCreateMutex();
#endif

#if LORA_TX_DMA
	new_LoRa.dma_tx_buffer_size   	= 0		   		;
	new_LoRa.dma_tx_buffer	      	= NULL			;
#endif

	return new_LoRa;
}

/**
 * @brief Reset the LoRa module
 *
 * This function toggles the reset pin of the LoRa module
 * to perform a hardware reset.
 *
 * @param _Lora Pointer to LoRa object handler
 *
 * @return None
 */
void LoRa_reset(LoRa* _LoRa){
	HAL_GPIO_WritePin(_LoRa->reset_port, _LoRa->reset_pin, GPIO_PIN_RESET);
	//HAL_Delay(10);
	osDelay(20);
	HAL_GPIO_WritePin(_LoRa->reset_port, _LoRa->reset_pin, GPIO_PIN_SET);
	//HAL_Delay(100);
	osDelay(10);
}

/**
 * @brief Check the LoRa instruct values
 *
 * This function checks the LoRa instruct values
 *
 * @param _lora Pointer to LoRa object handler
 *
 * @return Returns 1 if all of the values were given, otherwise returns 0
 */
uint8_t LoRa_isvalid(LoRa* _LoRa){
	if(_LoRa == NULL) return 0;
	return 1;
}



/**
 * @brief Read a register by an address
 *
 * This function uses for reading a register by an address
 *
 * @param _lora Pointer to LoRa object handler
 * @param address Address of the register e.g 0x1D
 *
 * @return register value(byte)
 */
uint8_t LoRa_read(LoRa* _LoRa, uint8_t address){
	uint8_t read_data;
	uint8_t addr= address & 0x7F ;

#if LORA_RTOS
	if(xSemaphoreTake(_LoRa->lora_lock, pdMS_TO_TICKS(100)) == pdTRUE){
		HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(_LoRa->hSPIx, &addr, 1, TRANSMIT_TIMEOUT);
		HAL_SPI_Receive(_LoRa->hSPIx, &read_data, 1, RECEIVE_TIMEOUT);
		HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
		xSemaphoreGive(_LoRa->lora_lock);
	}
#else
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_LoRa->hSPIx, &addr, 1, TRANSMIT_TIMEOUT);
	HAL_SPI_Receive(_LoRa->hSPIx, &read_data, 1, RECEIVE_TIMEOUT);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
#endif

	return read_data;
}

/**
 * @brief Write a value in a register by an address
 *
 * This function uses for write a value in a register by an address
 *
 * @param _lora Pointer to LoRa object handler
 * @param address Address of the register e.g 0x1D
 * @param value Value that you want to write
 *
 * @return None
 */
void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value){
	uint8_t msg[2];
	msg[0] = address | 0x80; //Write mode
	msg[1] = value;

#if LORA_RTOS
	if(xSemaphoreTake(_LoRa->lora_lock, pdMS_TO_TICKS(100)) == pdTRUE){
		HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(_LoRa->hSPIx, msg, 2, TRANSMIT_TIMEOUT);
		HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
		xSemaphoreGive(_LoRa->lora_lock);
	}
#else
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_LoRa->hSPIx, msg, 2, TRANSMIT_TIMEOUT);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
#endif
	//HAL_Delay(5);
}

/**
 * @brief  Write a set of values in a register by an address respectively
 *
 * This function use for writing a set of values in a register by an address respectively via SPI
 *
 * @param _lora Pointer to LoRa object handler
 * @param address Address of the register e.g 0x1D
 * @param output Pointer to the beginning of values array
 * @param length Number of address bytes that you want to write
 *
 * @return None
 */
void LoRa_burstWrite(LoRa* _LoRa, uint8_t address, uint8_t *value, uint8_t length){
	uint8_t addr;
	addr = address | 0x80; //Write mode

#if LORA_RTOS
	if(xSemaphoreTake(_LoRa->lora_lock, pdMS_TO_TICKS(100)) == pdTRUE){
			HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
			HAL_SPI_Transmit(_LoRa->hSPIx, &addr, 1, TRANSMIT_TIMEOUT);
			HAL_SPI_Transmit(_LoRa->hSPIx, value, length, TRANSMIT_TIMEOUT);
			HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
			xSemaphoreGive(_LoRa->lora_lock);
	}

#else
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_LoRa->hSPIx, &addr, 1, TRANSMIT_TIMEOUT);
	HAL_SPI_Transmit(_LoRa->hSPIx, value, length, TRANSMIT_TIMEOUT);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);

#endif
	//HAL_Delay(5);
}

void LoRa_gotoMode(LoRa* _LoRa, int mode){
	uint8_t    read;
	uint8_t    data;

	//Read the current value of RegOpMode
	read = LoRa_read(_LoRa, RegOpMode);
	data = read;

	//Read & F8: Mask (7:3) bits
	if(mode == SLEEP_MODE){
		data = (read & 0xF8) | 0x00;
		_LoRa->current_mode = SLEEP_MODE;
	}else if (mode == STDBY_MODE){
		data = (read & 0xF8) | 0x01;
		_LoRa->current_mode = STDBY_MODE;
	}else if (mode == FREQUENCY_SYSTHESIS_TX_MODE){
			data = (read & 0xF8) | 0x02;
			_LoRa->current_mode = FREQUENCY_SYSTHESIS_TX_MODE;
	}else if (mode == TRANSMIT_MODE){
		data = (read & 0xF8) | 0x03;
		_LoRa->current_mode = TRANSMIT_MODE;
	}else if (mode == FREQUENCY_SYSTHESIS_RX_MODE){
			data = (read & 0xF8) | 0x04;
			_LoRa->current_mode = FREQUENCY_SYSTHESIS_RX_MODE;
	}else if (mode == RXCONTINUOUS_MODE){
		data = (read & 0xF8) | 0x05;
		_LoRa->current_mode = RXCONTINUOUS_MODE;
	}else if (mode == RXSINGLE_MODE){
		data = (read & 0xF8) | 0x06;
		_LoRa->current_mode = RXSINGLE_MODE;
	}else if (mode == CAD_MODE){
		data = (read & 0xF8) | 0x07;
		_LoRa->current_mode = CAD_MODE;
	}else return;

	LoRa_write(_LoRa, RegOpMode, data);
	//HAL_Delay(10);
	return;
}

/**
 * @brief Enable or disable Low Data Rate Optimization (LDRO)
 *
 * Low Data Rate Optimization must be enabled when the LoRa
 * symbol duration exceeds 16 ms. This typically occurs when
 * using low bandwidth and/or high spreading factor.
 *
 * @param lora    Pointer to LoRa object handler
 * @param enable  0 = Disable LDRO
 *                1 = Enable LDRO
 *
 * @return None
 */
void LoRa_setLowDaraRateOptimization(LoRa* _LoRa, uint8_t value){
	uint8_t	data;
	uint8_t	read;

	read = LoRa_read(_LoRa, RegModemConfig3);

	if(value)
		data = read | 0x08;
	else
		data = read & 0xF7;

	LoRa_write(_LoRa, RegModemConfig3, data);
	HAL_Delay(10);
}

/**
 * @brief Automatically configure Low Data Rate Optimization (LDRO)
 *
 * This function enables or disables the Low Data Rate Optimization
 * flag based on the calculated symbol duration.
 *
 * LDRO is automatically enabled when:
 *      SymbolDuration > 16 ms
 *
 * SymbolDuration = (2^SpreadingFactor) / Bandwidth
 *
 * @param lora  Pointer to LoRa object handler
 *
 * @return None
 */
void LoRa_setAutoLDO(LoRa* _LoRa){
	double BW[] = {7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0, 500.0};

	LoRa_setLowDaraRateOptimization(_LoRa, (long)((1 << _LoRa->spredingFactor) / ((double)BW[_LoRa->bandWidth])) > 16.0);
}

/**
 * @brief Set LoRa carrier frequency
 *
 * Configures the RF carrier frequency of the SX1278.
 *
 * @param _LoRa       Pointer to LoRa object handler
 * @param freq  Desired frequency in MHz (e.g. 433 for 433 MHz)
 *
 * @return None
 */
void LoRa_setFrequency(LoRa* _LoRa, int freq){
	uint8_t  data;
	uint32_t F;
	F = (freq * 524288)>>5;

	// write Msb:
	data = F >> 16;
	LoRa_write(_LoRa, RegFrMsb, data);
	HAL_Delay(5);

	// write Mid:
	data = F >> 8;
	LoRa_write(_LoRa, RegFrMid, data);
	HAL_Delay(5);

	// write Lsb:
	data = F >> 0;
	LoRa_write(_LoRa, RegFrLsb, data);
	HAL_Delay(5);
}

/**
 * @brief Configure LoRa Spreading Factor
 *
 * Sets the spreading factor (SF) for LoRa modulation.
 * Supported values: 7 to 12.
 *
 * Higher SF increases sensitivity and range,
 * but reduces data rate and increases airtime.
 *
 * @param _Lora Pointer to LoRa object handler
 * @param SF   Spreading Factor value (7–12)
 *
 * @return None
 */
void LoRa_setSpreadingFactor(LoRa* _LoRa, int SF){
	uint8_t	data;
	uint8_t	read;

	if(SF>12)
		SF = 12;
	if(SF<7)
		SF = 7;

	read = LoRa_read(_LoRa, RegModemConfig2);
	HAL_Delay(10);

	data = (SF << 4) + (read & 0x0F);
	LoRa_write(_LoRa, RegModemConfig2, data);
	HAL_Delay(10);

	LoRa_setAutoLDO(_LoRa);
}

/**
 * @brief Configure TX output power
 *
 * Sets the transmission output power level of the SX1278.
 * The power level depends on the PA configuration (RFO or PA_BOOST).
 *
 * @param lora  Pointer to LoRa object handler
 * @param power Desired output power level (e.g. POWER_17db)
 *
 * @return None
 */
void LoRa_setPower(LoRa* _LoRa, uint8_t power){
	LoRa_write(_LoRa, RegPaConfig, power);
	HAL_Delay(10);
}

/**
 * @brief Configure Over-Current Protection (OCP)
 *
 * Sets the maximum allowed PA current limit by configuring
 * the OCP register (RegOcp).
 *
 * @param _LoRa   Pointer to LoRa object handler
 * @param current Maximum allowed current in mA
 *
 * @return None
 */
void LoRa_setOCP(LoRa* _LoRa, uint8_t current){
	uint8_t	OcpTrim = 0;

	if(current<45)
		current = 45;
	if(current>240)
		current = 240;

	if(current <= 120)
		OcpTrim = (current - 45)/5;
	else if(current <= 240)
		OcpTrim = (current + 30)/10;

	OcpTrim = OcpTrim + (1 << 5);
	LoRa_write(_LoRa, RegOcp, OcpTrim);
	HAL_Delay(10);
}

/**
 * @brief Configure symbol timeout MSB and enable CRC
 *
 * Sets the most significant byte of the symbol timeout register
 * and enables CRC checking for received packets.
 *
 * @param _LoRa Pointer to LoRa object handler
 *
 * @return None
 */
void LoRa_setTOMsb_setCRCon(LoRa* _LoRa){
	uint8_t read, data;

	read = LoRa_read(_LoRa, RegModemConfig2);

	data = read | 0x07;
	LoRa_write(_LoRa, RegModemConfig2, data);\
	HAL_Delay(10);
}

/**
 * @brief Set LoRa sync word
 *
 * Configures the synchronization word used to identify
 * LoRa packets on the network.
 *
 * Only packets with matching sync word will be received.
 *
 * @param _LoRa    Pointer to LoRa object handler
 * @param syncword Sync word value (1 byte)
 *
 * @return None
 */
void LoRa_setSyncWord(LoRa* _LoRa, uint8_t syncword){
	LoRa_write(_LoRa, RegSyncWord, syncword);
	HAL_Delay(10);
}

/**
 * @brief Transmit a LoRa packet
 *
 * Sends a data buffer over LoRa using TX mode.
 * The function waits until transmission is completed
 * (TxDone flag) or the timeout expires.
 *
 * @param _LoRa   Pointer to LoRa object handler
 * @param data    Pointer to data buffer to transmit
 * @param length  Size of data in bytes
 * @param timeout Timeout in milliseconds
 *
 * @return 0 if transmission completed successfully
 * @return 1 if timeout occurred
 */
uint8_t LoRa_transmit(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout){
	uint8_t read;
	int mode = _LoRa->current_mode;
	LoRa_gotoMode(_LoRa, STDBY_MODE);
	read = LoRa_read(_LoRa, RegFiFoTxBaseAddr);
	LoRa_write(_LoRa, RegFiFoAddPtr, read);
	if (length > MAX_FIFO_SIZE) length = MAX_FIFO_SIZE;
	LoRa_write(_LoRa, RegPayloadLength, length);
	LoRa_burstWrite(_LoRa, RegFiFo, data, length);
	LoRa_gotoMode(_LoRa, TRANSMIT_MODE);

	//Polling for checking if tranfer successfully.
#if LORA_RTOS
	uint32_t startTick = xTaskGetTickCount();
	uint32_t timeoutTicks = pdMS_TO_TICKS(timeout);

	while(1)
	{
	    read = LoRa_read(_LoRa, RegIrqFlags);
	    if((read & 0x08)!=0) // TxDone
	    {
	        LoRa_write(_LoRa, RegIrqFlags, 0xFF); // clear all IRQ
	        LoRa_gotoMode(_LoRa, mode);
	        return 0;
	    }
	    if((xTaskGetTickCount() - startTick) > timeoutTicks)
	    {
	        LoRa_gotoMode(_LoRa, mode);
	        return 1;
	    }
	    vTaskDelay(pdMS_TO_TICKS(1));
	}

#else
		while(1){
			read = LoRa_read(_LoRa, RegIrqFlags);
			if((read & 0x08)!=0){
				LoRa_write(_LoRa, RegIrqFlags, 0xFF);
				LoRa_gotoMode(_LoRa, mode);
				return 0;
			}
			else{
				if(--timeout==0){
					LoRa_gotoMode(_LoRa, mode);
					return 1;
				}
			}
			HAL_Delay(1);
		}
#endif
}

/**
 * @brief Start continuous LoRa reception
 *
 * Configures the SX1278 to enter RX continuous mode.
 * The module will continuously listen for incoming LoRa
 * packets until mode is changed.
 *
 * @param _LoRa Pointer to LoRa object handler
 *
 * @return None
 */
void LoRa_startReceiving(LoRa* _LoRa){
	LoRa_gotoMode(_LoRa, RXCONTINUOUS_MODE);
}

/**
 * @brief Receive data from LoRa module
 *
 * Reads received payload bytes from the SX1278 FIFO buffer
 * and stores them into the provided data buffer.
 *
 * @param _LoRa  Pointer to LoRa object handler
 * @param data   Pointer to buffer where received bytes will be stored
 * @param length Maximum number of bytes to read
 *
 * @return Number of bytes actually received
 */
uint8_t LoRa_receive(LoRa* _LoRa, uint8_t* data, uint8_t length){
	uint8_t read;
	uint8_t number_of_bytes;
	uint8_t min = 0;

	for(int i=0; i<length; i++)
		data[i]=0;

	LoRa_gotoMode(_LoRa, STDBY_MODE);
	read = LoRa_read(_LoRa, RegIrqFlags);
	if((read & 0x40) != 0){
		LoRa_write(_LoRa, RegIrqFlags, 0xFF);
		number_of_bytes = LoRa_read(_LoRa, RegRxNbBytes);
		read = LoRa_read(_LoRa, RegFiFoRxCurrentAddr);
		LoRa_write(_LoRa, RegFiFoAddPtr, read);
		min = length >= number_of_bytes ? number_of_bytes : length;
		for(int i=0; i<min; i++)
			data[i] = LoRa_read(_LoRa, RegFiFo);
	}
	LoRa_gotoMode(_LoRa, RXCONTINUOUS_MODE);
    return min;
}

/**
 * @brief Get RSSI of the last received packet
 *
 * Reads the RSSI value from the SX1278 after a successful
 * packet reception and converts it to dBm.
 *
 * @param _LoRa Pointer to LoRa object handler
 *
 * @return RSSI value in dBm
 */
int LoRa_getRSSI(LoRa* _LoRa){
	uint8_t read;
	read = LoRa_read(_LoRa, RegPktRssiValue);
	return -164 + read;
}

/**
 * @brief Initialize SX1278 with current configuration
 *
 * Applies all parameters from the LoRa structure
 * and prepares the module for operation.
 *
 * @param lora Pointer to LoRa object handler
 *
 * @return Status code
 */
uint16_t LoRa_init(LoRa* _LoRa){
	uint8_t    data;
	uint8_t    read;

	if(LoRa_isvalid(_LoRa)){
		// goto sleep mode:
			LoRa_gotoMode(_LoRa, SLEEP_MODE);
			HAL_Delay(10);

		// turn on LoRa mode:
			read = LoRa_read(_LoRa, RegOpMode);
			HAL_Delay(10);
			data = read | 0x80;
			LoRa_write(_LoRa, RegOpMode, data);
			HAL_Delay(100);

		// set frequency:
			LoRa_setFrequency(_LoRa, _LoRa->frequency);

		// set output power gain:
			LoRa_setPower(_LoRa, _LoRa->power);

		// set over current protection:
			LoRa_setOCP(_LoRa, _LoRa->overCurrentProtection);

		// set LNA gain:
			LoRa_write(_LoRa, RegLna, 0x23);

		// set spreading factor, CRC on, and Timeout Msb:
			LoRa_setTOMsb_setCRCon(_LoRa);
			LoRa_setSpreadingFactor(_LoRa, _LoRa->spredingFactor);

		// set Timeout Lsb:
			LoRa_write(_LoRa, RegSymbTimeoutLsb, 0xFF);

		// set bandwidth, coding rate and expilicit mode:
			// 8 bit RegModemConfig --> | X | X | X | X | X | X | X | X |
			//       bits represent --> |   bandwidth   |     CR    |I/E|
			data = 0;
			data = (_LoRa->bandWidth << 4) + (_LoRa->crcRate << 1);
			LoRa_write(_LoRa, RegModemConfig1, data);
			LoRa_setAutoLDO(_LoRa);

		// set preamble:
			LoRa_write(_LoRa, RegPreambleMsb, _LoRa->preamble >> 8);
			LoRa_write(_LoRa, RegPreambleLsb, _LoRa->preamble >> 0);

		// DIO mapping:   --> DIO: RxDone
			read = LoRa_read(_LoRa, RegDioMapping1);
			data = read | 0x3F;
			LoRa_write(_LoRa, RegDioMapping1, data);

		// goto standby mode:
			LoRa_gotoMode(_LoRa, STDBY_MODE);
			_LoRa->current_mode = STDBY_MODE;
			HAL_Delay(10);

			read = LoRa_read(_LoRa, RegVersion);
			if(read == 0x12)
				return LORA_OK;
			else
				return LORA_NOT_FOUND;
	}
	else {
		return LORA_UNAVAILABLE;
	}
}



#if LORA_TX_DMA && LORA_RTOS
uint8_t LoRa_setTX_DMA(LoRa* _LoRa,  uint8_t* dma_buffer, uint16_t dma_buffer_size){
	if(_LoRa == NULL || dma_buffer == NULL ) return 1;
	if(dma_buffer_size <= 0) return 2;
	_LoRa->dma_tx_buffer_size = dma_buffer_size;
	_LoRa->dma_tx_buffer = dma_buffer;
	return 0;
}

void LoRa_resetDMA(LoRa* _LoRa){
	if(_LoRa == NULL) return;
	_LoRa->dma_tx_buffer_size = 0;
	_LoRa->dma_tx_buffer = NULL;
	return;
}

uint8_t LoRa_transmit_DMA(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout){

	uint8_t read;
	int mode = _LoRa->current_mode;
	LoRa_gotoMode(_LoRa, STDBY_MODE);
	read = LoRa_read(_LoRa, RegFiFoTxBaseAddr);
	LoRa_write(_LoRa, RegFiFoAddPtr, read);
	if (length > MAX_FIFO_SIZE) length = MAX_FIFO_SIZE;
	LoRa_write(_LoRa, RegPayloadLength, length);

//	_LoRa->dma_tx_buffer [0] = RegFiFo | 0x80; //Write mode
//	memcpy(_LoRa->dma_tx_buffer + 1, data, length);

	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);

	//Open writemode
	uint8_t cmd = RegFiFo | 0x80;
	HAL_SPI_Transmit(_LoRa->hSPIx, &cmd, 1, 10);

	//Transmit data
	if (HAL_SPI_Transmit_DMA(_LoRa->hSPIx, data, length) != HAL_OK)
	{
		//Serial_Print("DMA not started\n");
		HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
		return 2;
	}
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
	//Polling for checking if tranfer successfully.

	LoRa_gotoMode(_LoRa, TRANSMIT_MODE);
	uint32_t startTick = xTaskGetTickCount();
	uint32_t timeoutTicks = pdMS_TO_TICKS(timeout);

	while(1)
	{
	    read = LoRa_read(_LoRa, RegIrqFlags);
	    if((read & 0x08)!=0) // TxDone
	    {
	        LoRa_write(_LoRa, RegIrqFlags, 0xFF); // clear all IRQ base on dataset
	        LoRa_gotoMode(_LoRa, mode);
	        return 0;
	    }
	    if((xTaskGetTickCount() - startTick) > timeoutTicks)
	    {
	        LoRa_gotoMode(_LoRa, mode);
	        return 1;
	    }
	    vTaskDelay(pdMS_TO_TICKS(1));
	}
}

#endif






