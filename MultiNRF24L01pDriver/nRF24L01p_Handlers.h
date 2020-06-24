/*
 * nRF24L01p_Handlers.h
 *
 *  Created on: 23 черв. 2020 р.
 *      Author: MaxCm
 */

#ifndef _nRF24L01p_Handlers_h_
#define _nRF24L01p_Handlers_h_
#include "MultiNRF24L01pDriver_Config.h"

//actually 9 bits in packet "control field"; introduced ~2% error is taken into account in all calculation
#define ServiceBytesInPacket	7
#define MaxContTxBytesFor250kbps	112
#define MaxContTxBytesFor1Mbps		450
#define MaxContTxBytesFor2Mbps		900

#define Flg_State_Base		0
#define Flg_State_Mask		(0x07<<Flg_State_Base)
#define Flg_State_PowerOff	(0<<Flg_State_Base)
#define Flg_State_Standby	(1<<Flg_State_Base)
#define Flg_State_Rx		(2<<Flg_State_Base)
#define Flg_State_Tx		(3<<Flg_State_Base)
#define Flg_State_StreamTx	(4<<Flg_State_Base)

#define Flg_HandleOnTimer	(1<<3)
#define Flg_HandleIRQ		(1<<4) //request to handle IRQ


#define Lock_OnCommandHandled	(1<<0)
#define Lock_OnDataSent			(1<<1)
#define Lock_OnDataReceived		(1<<2)
#define Lock_OnAckSent			(1<<3)

#define ARC_VALUE (2<<ARC_BASE) /*2 retransmissions*/


typedef struct {
	uint8_t flags;
	uint8_t spiBuffer[33];
	uint8_t rxPLlength;
	uint8_t txFifoLoad;
	uint16_t continuousTxByteNumber;
	uint16_t maxContinuousTxByteNumber;

	void (*irqHandler)(uint8_t moduleId);
	void (*handler)(uint8_t moduleId);
	uint8_t handlerStep;
	void (*commandHandler)(uint8_t moduleId);
	uint8_t commandArgs[33];

	uint8_t lockCallback;
	void (*onCommandHandled)();
	void (*onDataSent)(uint8_t success, uint8_t * dataWithAckBuffer, uint8_t dataWithAckLength);
	void (*onDataReceived)(uint8_t * dataBuffer, uint8_t DataLength);
	void (*onAckSent)();

} nRF24L01p_DriverContext;
nRF24L01p_DriverContext nrf24l01p_context[nRF24L01p_NumberOfModules];

//void irq_handler(uint8_t moduleId);
void writeRegister_handler(uint8_t moduleId);
void readRegister_handler(uint8_t moduleId);
void powerOn_handler(uint8_t moduleId);
void powerOff_handler(uint8_t moduleId);
void standby_handler(uint8_t moduleId);
void setRxAddress_handler(uint8_t moduleId);
void setTxAddress_handler(uint8_t moduleId);
void setChannel_handler(uint8_t moduleId);
void setPowerAndDataRate_handler(uint8_t moduleId);
void switchToTx_handler(uint8_t moduleId);
void switchToRx_handler(uint8_t moduleId);
void send_handler(uint8_t moduleId);
void sendWithAck_handler(uint8_t moduleId);
void sendInStream_handler(uint8_t moduleId);
void writePayloadForAck_handler(uint8_t moduleId);

#endif /* _nRF24L01p_Handlers_h_ */
