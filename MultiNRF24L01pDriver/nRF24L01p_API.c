/*
 * nRF24L01_Controll.c
 *
 *  Created on: 29 трав. 2020 р.
 *      Author: MaxCm
 */
#include "MultiNRF24L01pDriver_Config.h"
#include "nRF24L01p.h"
#include "MultiNRF24L01pDriver.h"
#include "MultiNRF24L01pDriver_HAL.h"
#include "nRF24L01p_Handlers.h"
#include "nRF24L01p_Utils.h"

/*return type and values of API functions*/
#define Result_OK	1
#define Result_FAIL	0


//TODO: check if LSB order in address

/*TODO: add license in nrf24l01p file*/


void nRF24L01p_InitDriver(){
	uint8_t i;
	nRF24L01p_DriverContext * context;
	for(i=0; i<nRF24L01p_NumberOfModules; i++){
		context=&(nrf24l01p_context[i]);
		context->flags=Flg_State_PowerOff;
		context->txFifoLoad=0;
		context->continuousTxByteNumber=0;
		context->maxContinuousTxByteNumber=0;
		context->irqHandler=0;
		context->handler=0;
		context->lockCallback=0;
		context->commandHandler=0;
		context->onCommandHandled=0;
		context->onDataReceived=0;
		context->onAckSent=0;
	}
}

uint8_t nRF24L01p_invokeOnCommandHandled(void (*func)(), uint8_t moduleId){
	uint8_t result=Result_FAIL;
	ENTER_ATOMIC();
	if(!(nrf24l01p_context[moduleId].lockCallback & Lock_OnCommandHandled)){
		nrf24l01p_context[moduleId].onCommandHandled=func;
		result=Result_OK;
	}
	EXIT_ATOMIC();
	return result;
}

uint8_t nRF24L01p_invokeOnDataSent(void (*func)(uint8_t, uint8_t *, uint8_t), uint8_t moduleId){
	uint8_t result=Result_FAIL;
	ENTER_ATOMIC();
	if(!(nrf24l01p_context[moduleId].lockCallback & Lock_OnDataSent)){
		nrf24l01p_context[moduleId].onDataSent=func;
		result=Result_OK;
	}
	EXIT_ATOMIC();
	return result;
}

uint8_t nRF24L01p_invokeOnDataReceived(void (*func)(uint8_t *, uint8_t), uint8_t moduleId){
	uint8_t result=Result_FAIL;
	ENTER_ATOMIC();
	if(!(nrf24l01p_context[moduleId].lockCallback & Lock_OnDataReceived)){
		nrf24l01p_context[moduleId].onDataReceived=func;
		result=Result_OK;
	}
	EXIT_ATOMIC();
	return result;
}

uint8_t nRF24L01p_invokeOnAckSent(void (*func)(), uint8_t moduleId){
	uint8_t result=Result_FAIL;
	ENTER_ATOMIC();
	if(!(nrf24l01p_context[moduleId].lockCallback & Lock_OnAckSent)){
		nrf24l01p_context[moduleId].onAckSent=func;
		result=Result_OK;
	}
	EXIT_ATOMIC();
	return result;
}

//applicable in any state
uint8_t nRF24L01p_writeRegister(uint8_t reg, uint8_t val, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	ENTER_ATOMIC();
	if(0==context->commandHandler){
		context->commandHandler=writeRegister_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=writeRegister_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		context->commandArgs[0]=reg;
		context->commandArgs[1]=val;
		if(handleNow){
			context->handlerStep=0;
			writeRegister_handler(moduleId);
		}
	}
	return result;
}

//applicable in any state, on onCommandHandled will NOT be invoked
uint8_t nRF24L01p_readRegister(uint8_t reg, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	ENTER_ATOMIC();
	if(0==context->commandHandler){
		context->commandHandler=readRegister_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=readRegister_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		context->commandArgs[0]=reg;
		if(handleNow){
			context->handlerStep=0;
			readRegister_handler(moduleId);
		}
	}
	return result;
}

//applicable in Power-Off state
uint8_t nRF24L01p_powerOn(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_PowerOff==(context->flags & Flg_State_Mask)){
		context->commandHandler=powerOn_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=powerOn_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		if(handleNow){
			context->handlerStep=0;
			powerOn_handler(moduleId);
		}
	}
	return result;
}

//applicable in any state EXCEPT Power-Off
uint8_t nRF24L01p_powerOff(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_PowerOff!=(context->flags & Flg_State_Mask)){
		context->commandHandler=powerOff_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=powerOff_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		if(handleNow){
			context->handlerStep=0;
			powerOff_handler(moduleId);
		}
	}
	return result;
}


//applicable in any state EXCEPT Power-Off
uint8_t nRF24L01p_standby(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_PowerOff!=(context->flags & Flg_State_Mask)) {
		context->commandHandler=standby_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=standby_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		if(handleNow){
			context->handlerStep=0;
			standby_handler(moduleId);
		}
	}
	return result;
}

//applicable in Standby, Tx and StreamTx states
uint8_t nRF24L01p_setRxAddress(uint32_t address, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	uint8_t state;
	ENTER_ATOMIC();
	state=context->flags & Flg_State_Mask;
	if(0==context->commandHandler
			&& (Flg_State_Standby==state || Flg_State_Tx==state || Flg_State_StreamTx==state)){
		context->commandHandler=setRxAddress_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=setRxAddress_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		*(uint32_t *)(context->commandArgs)=address;
		if(handleNow){
			context->handlerStep=0;
			setRxAddress_handler(moduleId);
		}
	}
	return result;
}

//applicable in all states EXCEPT Power-Off
//all transfers should be finished in Tx and StreamTx states
uint8_t nRF24L01p_setTxAddress(uint32_t address, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	uint8_t state;
	ENTER_ATOMIC();
	state=context->flags & Flg_State_Mask;
	if(0==context->commandHandler
			&& (Flg_State_Standby==state
					|| ((Flg_State_Tx==state || Flg_State_StreamTx==state) && 0==context->txFifoLoad))){
		context->commandHandler=setTxAddress_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=setTxAddress_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		*(uint32_t *)context->commandArgs=address;
		if(handleNow){
			context->handlerStep=0;
			setTxAddress_handler(moduleId);
		}
	}
	return result;
}

//applicable in Standby state
uint8_t nRF24L01p_setChannel(uint8_t channel, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	if(channel>nRF24L01p_RFChannel_max){
		return Result_FAIL;
	}
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_Standby==(context->flags & Flg_State_Mask)){
		context->commandHandler=setChannel_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=setChannel_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		context->commandArgs[0]=channel;
		if(handleNow){
			context->handlerStep=0;
			setChannel_handler(moduleId);
		}
	}
	return result;
}

//applicable in Standby state
uint8_t nRF24L01p_setPowerAndDataRate(uint8_t power, uint8_t dataRate, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t setup_retr;
	uint8_t rf_setup;
	uint16_t maxContTxBytes;
	uint8_t result=Result_FAIL;
	uint8_t executeNow=0;
	//dataRate impacts auto-retransmission delay
	switch(dataRate){
	case nRF24L01p_DataRate_250kbps:
		setup_retr=ARC_VALUE | (7<<ARD_BASE); //delay=2ms, max time for tx=1.252ms
		rf_setup=RF_DR_LOW;
		maxContTxBytes=MaxContTxBytesFor250kbps;
		break;
	case nRF24L01p_DataRate_1Mbps:
		setup_retr=ARC_VALUE | (1<<ARD_BASE); //delay=0.5ms, max time for tx=0.313ms
		rf_setup=0;
		maxContTxBytes=MaxContTxBytesFor1Mbps;
		break;
	case nRF24L01p_DataRate_2Mbps:
		setup_retr=ARC_VALUE | (1<<ARD_BASE); //delay=0.5ms, max time for tx=0.1567ms
		rf_setup=RF_DR_HIGH;
		maxContTxBytes=MaxContTxBytesFor2Mbps;
		break;
	default:
		return Result_FAIL;
	}

	switch(power){
	case nRF24L01p_OutputPower_m18dBm:
		rf_setup |= 0x00<<RF_PWR_BASE;
		break;
	case nRF24L01p_OutputPower_m12dBm:
		rf_setup |= 0x01<<RF_PWR_BASE;
		break;
	case nRF24L01p_OutputPower_m6dBm:
		rf_setup |= 0x02<<RF_PWR_BASE;
		break;
	case nRF24L01p_OutputPower_0dBm:
		rf_setup |= 0x03<<RF_PWR_BASE;
		break;
	default:
		return Result_FAIL;
	}

	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_Standby==(context->flags & Flg_State_Mask)){
		context->commandHandler=setPowerAndDataRate_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=setPowerAndDataRate_handler;
			executeNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		context->commandArgs[0]=rf_setup;
		context->commandArgs[1]=setup_retr;
		*(uint16_t *)(context->commandArgs+2)=maxContTxBytes;
		if(executeNow){
			context->handlerStep=0;
			setPowerAndDataRate_handler(moduleId);
		}
	}
	return result;
}

//applicable in any state EXCEPT Power-Off
uint8_t nRF24L01p_switchToTx(uint8_t streamTx, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_PowerOff!=(context->flags & Flg_State_Mask)){
		context->commandHandler=switchToTx_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=switchToTx_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		context->commandArgs[0]=streamTx;
		if(handleNow){
			context->handlerStep=0;
			switchToTx_handler(moduleId);
		}
	}
	return result;
}

//applicable in any state EXCEPT Power-Off
//all transfers should be finished in Tx and StreamTx states
uint8_t nRF24L01p_switchToRx(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	uint8_t state;
	ENTER_ATOMIC();
	state=context->flags & Flg_State_Mask;
	if(0==context->commandHandler
			&& (Flg_State_Standby==state
					|| ((Flg_State_Tx==state || Flg_State_StreamTx==state) && 0==context->txFifoLoad))){
		context->commandHandler=switchToRx_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=switchToRx_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		if(handleNow){
			context->handlerStep=0;
			switchToRx_handler(moduleId);
		}
	}
	return result;
}

//applicable in Tx state
//previous transfer should be finished
uint8_t nRF24L01p_send(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	if(0==dataLength || dataLength>32){
		return Result_FAIL;
	}
	ENTER_ATOMIC();
	if(0==context->commandHandler
			&& Flg_State_Tx==(context->flags & Flg_State_Mask) && 0==context->txFifoLoad){
		context->commandHandler=send_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=send_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		copyBuffer(&(context->commandArgs[1]), dataBuffer, dataLength);
		context->commandArgs[0]=dataLength;
		if(handleNow){
			context->handlerStep=0;
			send_handler(moduleId);
		}
	}
	return result;
}

//applicable in Tx state
//previous transfer should be finished
uint8_t nRF24L01p_sendWithAck(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	if(0==dataLength || dataLength>32){
		return Result_FAIL;
	}
	ENTER_ATOMIC();
	if(0==context->commandHandler
			&& Flg_State_Tx==(context->flags & Flg_State_Mask) && 0==context->txFifoLoad){
		context->commandHandler=sendWithAck_handler;
		result=Result_OK;
		if(!context->handler){
			context->handler=sendWithAck_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		copyBuffer(&(context->commandArgs[1]), dataBuffer, dataLength);
		context->commandArgs[0]=dataLength;
		if(handleNow){
			context->handlerStep=0;
			sendWithAck_handler(moduleId);
		}
	}
	return result;
}

//applicable in StreamTx state
//returns Result_OK(non-zero) if possible to put next payload to stream
uint8_t nRF24L01p_canSendToStream(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	if(Flg_State_StreamTx==(context->flags & Flg_State_Mask) && context->txFifoLoad<3){
		return Result_OK;
	}
	return Result_FAIL;
}

//applicable in StreamTx state
uint8_t nRF24L01p_sendInStream(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	if(dataLength==0 || dataLength>32){
		return Result_FAIL;
	}
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_StreamTx==(context->flags & Flg_State_Mask)
			&& context->txFifoLoad<3){
		context->commandHandler=sendInStream_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=sendInStream_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		copyBuffer(&(context->commandArgs[1]), dataBuffer, dataLength);
		context->commandArgs[0]=dataLength;
		if(handleNow){
			context->handlerStep=0;
			sendInStream_handler(moduleId);
		}
	}
	return result;
}

//applicable in Rx state
//there should be no pending payload(for ACK)
uint8_t nRF24L01p_writePayloadForAck(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t result=Result_FAIL;
	uint8_t handleNow=0;
	if(dataLength==0 || dataLength>32){
		return Result_FAIL;
	}
	ENTER_ATOMIC();
	if(0==context->commandHandler && Flg_State_Rx==(context->flags & Flg_State_Mask)
			&& 0==context->txFifoLoad){
		context->commandHandler=writePayloadForAck_handler;
		result=Result_OK;
		if(0==context->handler){
			context->handler=writePayloadForAck_handler;
			handleNow=1;
		}
	}
	EXIT_ATOMIC();
	if(Result_OK==result){
		copyBuffer(&(context->commandArgs[1]), dataBuffer, dataLength);
		context->commandArgs[0]=dataLength;
		if(handleNow){
			context->handlerStep=0;
			writePayloadForAck_handler(moduleId);
		}
	}
	return result;
}
