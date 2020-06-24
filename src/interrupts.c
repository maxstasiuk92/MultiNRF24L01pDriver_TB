/*
 * interrupts.c
 *
 *  Created on: 1 черв. 2020 р.
 *      Author: MaxCm
 */
#include "stm32f10x.h"

#include "RFMaster_HAL.h"
#include "RFSlave_HAL.h"
#include "RFModuleID.h"
#include "MultiNRF24L01pDriver_HAL.h"
#include "../TestUtil/TestUtil.h"

//RFMaster, pin IRQ(PB10)
void EXTI15_10_IRQHandler(){
	if(SET==EXTI_GetITStatus(EXTI_Line10)){
		EXTI_ClearITPendingBit(EXTI_Line10);
		nRF24L01p_onIRQ(RFMaster);
	} else {
		//failure
		while(1);
	}

}

void DMA1_Channel4_IRQHandler(){
	DMA_ClearITPendingBit(DMA1_IT_TC4);
	RFMaster_onFinishSpiExchange();
	nRF24L01p_onFinishSpiExchange(RFMaster);
}



//RFSlave, pin IRQ(PB10)
void EXTI4_IRQHandler(){
	if(SET==EXTI_GetITStatus(EXTI_Line4)){
		EXTI_ClearITPendingBit(EXTI_Line4);
		nRF24L01p_onIRQ(RFSlave);
	} else {
		//failure
		while(1);
	}

}

void DMA1_Channel2_IRQHandler(){
	RFSlave_onFinishSpiExchange();
	DMA_ClearITPendingBit(DMA1_IT_TC2);
	nRF24L01p_onFinishSpiExchange(RFSlave);
}


void blinkGreenLED(){
	static uint32_t ledTimer=0;
	ledTimer++;
	if(ledTimer>=5000){
		if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_9)){
			GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		}else{
			GPIO_SetBits(GPIOC, GPIO_Pin_9);
		}
		ledTimer=0;
	}
}
//Common Timer
void TIM3_IRQHandler(){
	//NVIC_ClearPendingIRQ(IRQn_Typ)
	if(SET == TIM_GetITStatus(TIM3, TIM_IT_CC1)){
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
		nRF24L01p_onTimer(RFMaster);
	}
	if(SET == TIM_GetITStatus(TIM3, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		nRF24L01p_onTimer(RFSlave);

	}
	if(SET == TIM_GetITStatus(TIM3, TIM_IT_CC4)){
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
		UtilTimer();
		blinkGreenLED();
	}
}
