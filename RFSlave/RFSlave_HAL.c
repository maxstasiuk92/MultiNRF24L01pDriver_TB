/*
 * RFSlave_HAL.c
 *
 *  Created on: 1 черв. 2020 р.
 *      Author: MaxCm
 */
#include "RFSlave_HAL.h"
#include "DMAPriorityConfig.h"
#include "IRQPriorityConfig.h"
#include "RFModuleID.h"
#include "MultiNRF24L01pDriver_HAL.h"

/*
 * RFSlave
 * SPI - SPI1
 * MOSI - PA7
 * MISO - PA6
 * SCK - PA5
 * CE - PC4
 * IRQ - PA4
 * CSN - PC5
 *
 */

void RFSlave_configPeripherals(){
	GPIO_InitTypeDef pinInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	SPI_InitTypeDef spiInitStruct;
	DMA_InitTypeDef dmaInitStruct;
	EXTI_InitTypeDef extiInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//Config. pins
	//PA7 - MOSI
	pinInitStruct.GPIO_Pin=GPIO_Pin_7;
	pinInitStruct.GPIO_Speed=GPIO_Speed_10MHz;
	pinInitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &pinInitStruct);
	//PA5 - SCK
	pinInitStruct.GPIO_Pin=GPIO_Pin_5;
	GPIO_Init(GPIOA, &pinInitStruct);
	//PA6 - MISO
	pinInitStruct.GPIO_Pin=GPIO_Pin_6;
	pinInitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &pinInitStruct);

	//PC4 - CE
	pinInitStruct.GPIO_Pin=GPIO_Pin_4;
	pinInitStruct.GPIO_Speed=GPIO_Speed_10MHz;
	pinInitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
	GPIO_Init(GPIOC, &pinInitStruct);
	//PC5 - CSN
	pinInitStruct.GPIO_Pin=GPIO_Pin_5;
	GPIO_SetBits(GPIOC, GPIO_Pin_5);
	GPIO_Init(GPIOC, &pinInitStruct);
	//PA4 - IRQ
	pinInitStruct.GPIO_Pin=GPIO_Pin_4;
	pinInitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &pinInitStruct);

	//Config. SPI
	spiInitStruct.SPI_Mode=SPI_Mode_Master;
	spiInitStruct.SPI_CPHA=SPI_CPHA_1Edge;
	spiInitStruct.SPI_CPOL=SPI_CPOL_Low;
	spiInitStruct.SPI_NSS=SPI_NSS_Soft;
	spiInitStruct.SPI_DataSize=SPI_DataSize_8b;
	spiInitStruct.SPI_FirstBit=SPI_FirstBit_MSB;
	spiInitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_4; //6MHz
	spiInitStruct.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	spiInitStruct.SPI_CRCPolynomial=0;
	SPI_Init(SPI1, &spiInitStruct);
	SPI_CalculateCRC(SPI1, DISABLE);
	//Config. TX with DMA
	dmaInitStruct.DMA_Mode=DMA_Mode_Normal;
	dmaInitStruct.DMA_DIR=DMA_DIR_PeripheralDST;
	dmaInitStruct.DMA_M2M=DMA_M2M_Disable;
	dmaInitStruct.DMA_Priority=DMA1_RFSlave_TxPriority;

	dmaInitStruct.DMA_MemoryBaseAddr=(uint32_t)nRF24L01p_getSpiBuffer(RFSlave);
	dmaInitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	dmaInitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;

	dmaInitStruct.DMA_PeripheralBaseAddr=(uint32_t)&(SPI1->DR);
	dmaInitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	dmaInitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;

	dmaInitStruct.DMA_BufferSize=0; //will be set before Tx/Rx
	DMA_Init(DMA1_Channel3, &dmaInitStruct);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

	//Config. RX with DMA
	dmaInitStruct.DMA_Mode=DMA_Mode_Normal;
	dmaInitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;
	dmaInitStruct.DMA_M2M=DMA_M2M_Disable;
	dmaInitStruct.DMA_Priority=DMA1_RFSlave_RxPriority;

	dmaInitStruct.DMA_MemoryBaseAddr=(uint32_t)nRF24L01p_getSpiBuffer(RFSlave);
	dmaInitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	dmaInitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;

	dmaInitStruct.DMA_PeripheralBaseAddr=(uint32_t)&(SPI1->DR);
	dmaInitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	dmaInitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;

	dmaInitStruct.DMA_BufferSize=0; //will be set before Tx/Rx
	DMA_Init(DMA1_Channel2, &dmaInitStruct);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE); //interrupt after Rx

	nvicInitStruct.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = IRQ_RFSlave_RxPriority;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvicInitStruct);

	//PA4 - IRQ
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);

	extiInitStruct.EXTI_Line=EXTI_Line4;
	extiInitStruct.EXTI_LineCmd=ENABLE;
	extiInitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	extiInitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_Init(&extiInitStruct);

	nvicInitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = IRQ_RFSlave_pinIrqPriority;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvicInitStruct);

	//Enable SPI
	SPI_Cmd(SPI1, ENABLE);
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set); //TODO: any sense?
}
