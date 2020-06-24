/*
 * nRF24L01p_Driver_HAL.c
 *
 *  Created on: 23 черв. 2020 р.
 *      Author: MaxCm
 */
#include "nRF24L01p_Handlers.h"
#include "MultiNRF24L01pDriver_HAL.h"

uint8_t * nRF24L01p_getSpiBuffer(uint8_t moduleId){
	return nrf24l01p_context[moduleId].spiBuffer;
}


void nRF24L01p_onFinishSpiExchange(uint8_t moduleId){
	nRF24L01p_stateOfNCS(1, moduleId);
	nrf24l01p_context[moduleId].handler(moduleId); //non-zero by design
}


void nRF24L01p_onIRQ(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t executeNow=0;
	ENTER_ATOMIC();
	if(context->handler){
		context->flags |= Flg_HandleIRQ;
	} else {
		if(context->irqHandler){
			context->handler=context->irqHandler;
			executeNow=1;
		}
	}
	EXIT_ATOMIC();
	if(executeNow){
		context->handlerStep=0;
		context->handler(moduleId);
	}
}


void nRF24L01p_onTimer(uint8_t moduleId){
	if(nrf24l01p_context[moduleId].flags & Flg_HandleOnTimer){
		nrf24l01p_context[moduleId].handler(moduleId);
	}
}
