/*
 * RFMaster_HAL.c
 *
 *  Created on: 1 черв. 2020 р.
 *      Author: MaxCm
 */
#include "RFMaster_HAL.h"
#include "DMAPriorityConfig.h"
#include "IRQPriorityConfig.h"
#include "RFModuleID.h"

#include "../MultiNRF24L01pDriver/MultiNRF24L01pDriver_HAL.h"

/*
 * RFMaster
 * SPI - SPI2
 * MOSI - PB15
 * MISO - PB14
 * SCK - PB13
 * CE - PB12
 * IRQ - PB10
 * CSN - PB11
 */


void RFMaster_configPeripherals(){
	GPIO_InitTypeDef pinInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	SPI_InitTypeDef spiInitStruct;
	DMA_InitTypeDef dmaInitStruct;
	EXTI_InitTypeDef extiInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//Config. pins
	//PB15 - MOSI
	pinInitStruct.GPIO_Pin=GPIO_Pin_15;
	pinInitStruct.GPIO_Speed=GPIO_Speed_10MHz;
	pinInitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &pinInitStruct);
	//PB13 - SCK
	pinInitStruct.GPIO_Pin=GPIO_Pin_13;
	GPIO_Init(GPIOB, &pinInitStruct);
	//PB14 - MISO
	pinInitStruct.GPIO_Pin=GPIO_Pin_14;
	pinInitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &pinInitStruct);

	//PB12 - CE
	pinInitStruct.GPIO_Pin=GPIO_Pin_12;
	pinInitStruct.GPIO_Speed=GPIO_Speed_10MHz;
	pinInitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	GPIO_Init(GPIOB, &pinInitStruct);
	//PB11 - CSN
	pinInitStruct.GPIO_Pin=GPIO_Pin_11;
	GPIO_SetBits(GPIOB, GPIO_Pin_11);
	GPIO_Init(GPIOB, &pinInitStruct);
	//PB10 - IRQ
	pinInitStruct.GPIO_Pin=GPIO_Pin_10;
	pinInitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &pinInitStruct);

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
	SPI_Init(SPI2, &spiInitStruct);
	SPI_CalculateCRC(SPI2, DISABLE);
	//Config. TX with DMA
	dmaInitStruct.DMA_Mode=DMA_Mode_Normal;
	dmaInitStruct.DMA_DIR=DMA_DIR_PeripheralDST;
	dmaInitStruct.DMA_M2M=DMA_M2M_Disable;
	dmaInitStruct.DMA_Priority=DMA1_RFMaster_TxPriority;

	dmaInitStruct.DMA_MemoryBaseAddr=(uint32_t)nRF24L01p_getSpiBuffer(RFMaster);
	dmaInitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	dmaInitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;

	dmaInitStruct.DMA_PeripheralBaseAddr=(uint32_t)&(SPI2->DR);
	dmaInitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	dmaInitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;

	dmaInitStruct.DMA_BufferSize=0; //will be set before Tx/Rx
	DMA_Init(DMA1_Channel5, &dmaInitStruct);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

	//Config. RX with DMA
	dmaInitStruct.DMA_Mode=DMA_Mode_Normal;
	dmaInitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;
	dmaInitStruct.DMA_M2M=DMA_M2M_Disable;
	dmaInitStruct.DMA_Priority=DMA1_RFMaster_RxPriority;

	dmaInitStruct.DMA_MemoryBaseAddr=(uint32_t)nRF24L01p_getSpiBuffer(RFMaster);
	dmaInitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	dmaInitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;

	dmaInitStruct.DMA_PeripheralBaseAddr=(uint32_t)&(SPI2->DR);
	dmaInitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	dmaInitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;

	dmaInitStruct.DMA_BufferSize=0; //will be set before Tx/Rx
	DMA_Init(DMA1_Channel4, &dmaInitStruct);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE); //interrupt after Rx

	nvicInitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = IRQ_RFMaster_RxPriority;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvicInitStruct);

	//PB10 - IRQ
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

	extiInitStruct.EXTI_Line=EXTI_Line10;
	extiInitStruct.EXTI_LineCmd=ENABLE;
	extiInitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	extiInitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_Init(&extiInitStruct);

	nvicInitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = IRQ_RFMaster_pinIrqPriority;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvicInitStruct);

	//Enable SPI
	SPI_Cmd(SPI2, ENABLE);
	SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set); //TODO: any sense?
}
