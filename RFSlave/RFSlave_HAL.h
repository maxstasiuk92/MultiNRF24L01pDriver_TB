/*
 * RFSlave_HAL.h
 *
 *  Created on: 1 черв. 2020 р.
 *      Author: MaxCm
 */

#ifndef RFSLAVE_HAL_H_
#define RFSLAVE_HAL_H_
#include "stm32f10x.h"

#define RFSlave_startSpiExchange(byteNum) do{\
								DMA_SetCurrDataCounter(DMA1_Channel2, byteNum);\
								DMA_SetCurrDataCounter(DMA1_Channel3, byteNum);\
								DMA_Cmd(DMA1_Channel2, ENABLE);\
								DMA_Cmd(DMA1_Channel3, ENABLE);}while(0)

#define RFSlave_onFinishSpiExchange() do{DMA_Cmd(DMA1_Channel3, DISABLE);\
										 DMA_Cmd(DMA1_Channel2, DISABLE);}while(0)

#define RFSlave_setCE() GPIO_SetBits(GPIOC, GPIO_Pin_4)
#define RFSlave_resetCE() GPIO_ResetBits(GPIOC, GPIO_Pin_4)

#define RFSlave_setNCS() GPIO_SetBits(GPIOC, GPIO_Pin_5)
#define RFSlave_resetNCS() GPIO_ResetBits(GPIOC, GPIO_Pin_5)

void RFSlave_configPeripherals();

#endif /* RFSLAVE_HAL_H_ */
