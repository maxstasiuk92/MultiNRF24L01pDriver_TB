/*
 * RFMaster_HAL.h
 *
 *  Created on: 1 черв. 2020 р.
 *      Author: MaxCm
 */

#ifndef RFMASTER_HAL_H_
#define RFMASTER_HAL_H_
#include "stm32f10x.h"

#define RFMaster_startSpiExchange(byteNum) do{\
								DMA_SetCurrDataCounter(DMA1_Channel4, byteNum);\
								DMA_SetCurrDataCounter(DMA1_Channel5, byteNum);\
								DMA_Cmd(DMA1_Channel4, ENABLE);\
								DMA_Cmd(DMA1_Channel5, ENABLE);}while(0)

#define RFMaster_onFinishSpiExchange() do{DMA_Cmd(DMA1_Channel5, DISABLE);\
										  DMA_Cmd(DMA1_Channel4, DISABLE);}while(0)

#define RFMaster_setCE() GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define RFMaster_resetCE() GPIO_ResetBits(GPIOB, GPIO_Pin_12)

#define RFMaster_setNCS() GPIO_SetBits(GPIOB, GPIO_Pin_11)
#define RFMaster_resetNCS() GPIO_ResetBits(GPIOB, GPIO_Pin_11)

void RFMaster_configPeripherals();

#endif /* RFMASTER_HAL_H_ */
