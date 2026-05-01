/*
 * global.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Admin
 */

#include "global.h"
osSemaphoreId spiDmaSemHandle;
osSemaphoreId i2cDmaSemHandle;
osSemaphoreDef(I2C_DMA_SEM);
osSemaphoreDef(SPI_DMA_SEM);
void Semaphore_init(void){
	i2cDmaSemHandle = osSemaphoreCreate(osSemaphore(I2C_DMA_SEM), 1);
	spiDmaSemHandle = osSemaphoreCreate(osSemaphore(SPI_DMA_SEM), 1);
}
