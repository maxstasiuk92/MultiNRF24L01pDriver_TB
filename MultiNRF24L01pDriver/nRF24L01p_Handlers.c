/*
 * nRF24L01p_Handlers.c
 *
 *  Created on: 23 черв. 2020 р.
 *      Author: MaxCm
 */
#include "nRF24L01p.h"
#include "nRF24L01p_Handlers.h"
#include "MultiNRF24L01pDriver_HAL.h"
#include "nRF24L01p_Utils.h"

void tryToHandleNext(uint8_t moduleId);

void irqInRx_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t status;
	switch(context->handlerStep){
	case 0:
		//read STATUS
		context->handlerStep=1;
		context->spiBuffer[0]=NOP;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 1:
		context->handlerStep=2;
		status=context->spiBuffer[0];
		if(status & TX_DS){
			ENTER_ATOMIC();
			context->txFifoLoad=0;
			context->lockCallback |= Lock_OnAckSent;
			EXIT_ATOMIC();
			if(context->onAckSent){
				context->onAckSent();
			}
			ENTER_ATOMIC();
			context->lockCallback &= ~Lock_OnAckSent;
			EXIT_ATOMIC();
		}
		//clear flags
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 2:
		//read FIFO_STATUS
		context->handlerStep=3;
		context->spiBuffer[0]=R_REGISTER | FIFO_STATUS;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 3:
		if(context->spiBuffer[1] & RX_EMPTY){
			//finish
			tryToHandleNext(moduleId);
		}else{
			//Received data, read how many bytes
			context->handlerStep=4;
			context->spiBuffer[0]=R_RX_PL_WID;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
			break;
		}
		break;
	case 4:
		//read data from FIFO
		context->handlerStep=5;
		context->rxPLlength=context->spiBuffer[1];
		if(context->rxPLlength>32){
			context->rxPLlength=32;
		}
		context->spiBuffer[0]=R_RX_PAYLOAD;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(context->rxPLlength+1, moduleId);
		break;
	case 5:
		ENTER_ATOMIC();
		context->lockCallback |= Lock_OnDataReceived;
		EXIT_ATOMIC();
		if(context->onDataReceived){
			context->onDataReceived(&(context->spiBuffer[1]), context->rxPLlength);
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnDataReceived;
		EXIT_ATOMIC();
		//read FIFO_STATUS
		context->handlerStep=3;
		context->spiBuffer[0]=R_REGISTER | FIFO_STATUS;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	}
}

void irqInTx_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t status;
	uint8_t finish=0;

	uint8_t success;
	uint8_t * dataWithAckBuffer;
	uint8_t dataWithAckLength;
	switch(context->handlerStep){
	case 0:
		//read STATUS
		context->handlerStep=1;
		context->spiBuffer[0]=NOP;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 1:
		status=context->spiBuffer[0];
		if(status & (MAX_RT | TX_DS)){
			if(status & MAX_RT){
				//transfer not successful
				context->handlerStep=30;
			}else{
				if(status & RX_DR){
					//received PL
					context->handlerStep=10;
				}else{
					//no PL
					context->handlerStep=20;
				}
			}
			//clear flags
			context->spiBuffer[0]=W_REGISTER | STATUS;
			context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
		}else{
			//nothing to do
			tryToHandleNext(moduleId);
		}
		break;

	//steps, when received PL with ACK
	case 10:
		//Received data, read how many bytes
		context->handlerStep=11;
		context->spiBuffer[0]=R_RX_PL_WID;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 11:
		//read data from FIFO
		context->handlerStep=12;
		context->rxPLlength=context->spiBuffer[1];
		if(context->rxPLlength>32){
			context->rxPLlength=32;
		}
		context->spiBuffer[0]=R_RX_PAYLOAD;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(context->rxPLlength+1, moduleId);
		break;
	case 12:
		//finish
		//received PL
		success=1;
		dataWithAckBuffer=&(context->spiBuffer[1]);
		dataWithAckLength=context->rxPLlength;
		finish=1;
		break;

	//steps when no PL with ACK
	case 20:
		//finish
		//no PL
		success=1;
		dataWithAckBuffer=0;
		dataWithAckLength=0;
		finish=1;
		break;

	//steps when no ACK
	case 30:
		//flush TX_FIFO
		context->handlerStep=31;
		context->spiBuffer[0]=FLUSH_TX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 31:
		//finish
		//transfer not successful
		success=0;
		dataWithAckBuffer=0;
		dataWithAckLength=0;
		finish=1;
		break;

	default:;
	}

	if(finish){
		ENTER_ATOMIC();
		context->txFifoLoad=0;
		context->lockCallback |= Lock_OnDataSent;
		EXIT_ATOMIC();
		if(context->onDataSent){
			context->onDataSent(success, dataWithAckBuffer, dataWithAckLength);
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnDataSent;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
	}
}

void irqInStreamTx_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		context->handlerStep=1;
		//clear flags
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 1:
		context->handlerStep=2;
		//read TX FIFO status
		context->spiBuffer[0]=R_REGISTER | FIFO_STATUS;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 2:
		ENTER_ATOMIC();
		if(0!=(context->spiBuffer[1] & TX_EMPTY)){
			context->txFifoLoad=0;
			context->continuousTxByteNumber=0;
		}else if(context->txFifoLoad){
				context->txFifoLoad-=1;
		}
		context->lockCallback |= Lock_OnDataSent;
		EXIT_ATOMIC();
		if(context->onDataSent){
			context->onDataSent(1, 0, 0);
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnDataSent;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	}
}


void writeRegister_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		context->handlerStep=1;
		context->spiBuffer[0]=W_REGISTER | context->commandArgs[0];
		context->spiBuffer[1]=context->commandArgs[1];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		//check if start next
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void readRegister_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		context->handlerStep=1;
		context->spiBuffer[0]=R_REGISTER | context->commandArgs[0];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnDataReceived;
		EXIT_ATOMIC();
		if(context->onDataReceived){
			context->onDataReceived(&(context->spiBuffer[1]), 1);
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnDataReceived;
		EXIT_ATOMIC();
		//start next
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}


void powerOn_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		context->handlerStep=1;
		//wait 101 ticks(required 100ms) for POR
		context->commandArgs[0]=100; //will be used as counter
		ENTER_ATOMIC();
		context->flags |= Flg_HandleOnTimer;
		EXIT_ATOMIC();
		break;
	case 1:
		if(0==context->commandArgs[0]){
			ENTER_ATOMIC();
			context->flags &= ~Flg_HandleOnTimer;
			EXIT_ATOMIC();

			context->handlerStep=2;
			//power on
			context->spiBuffer[0]=W_REGISTER | CONFIG;
			context->spiBuffer[1]=EN_CRC | PWR_UP;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
		}else{
			context->commandArgs[0]-=1;
		}
		break;
	case 2:
		context->handlerStep=3;
		//wait 3 ticks(required 1.5ms) for settling
		context->commandArgs[0]=2; //will be used as counter
		ENTER_ATOMIC();
		context->flags |= Flg_HandleOnTimer;
		EXIT_ATOMIC();
		break;
	case 3:
		if(0==context->commandArgs[0]){
			ENTER_ATOMIC();
			context->flags &= ~Flg_HandleOnTimer;
			EXIT_ATOMIC();

			context->handlerStep=4;
			//write address width
			context->spiBuffer[0]=W_REGISTER | SETUP_AW;
			context->spiBuffer[1]=0x02<<AW_BASE; //4- bytes
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
		}else{
			context->commandArgs[0]-=1;
		}
		break;
	case 4:
		//enable auto-ACK
		context->handlerStep=5;
		context->spiBuffer[0]=W_REGISTER | EN_AA;
		context->spiBuffer[1]=ENAA_P0 | ENAA_P1;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 5:
		//automatic retransmissions
		context->handlerStep=6;
		context->spiBuffer[0]=W_REGISTER | SETUP_RETR;
		context->spiBuffer[1]=(1<<ARD_BASE) | ARC_VALUE; //wait 500us(enough for any payload at 1Mbps)
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 6:
		//dynamic PL length, PL with ACK, dynamic ACK
		context->handlerStep=7;
		context->spiBuffer[0]=W_REGISTER | FEATURE;
		context->spiBuffer[1]=EN_DPL | EN_ACK_PAY | EN_DYN_ACK;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 7:
		//dynamic payload
		context->handlerStep=8;
		context->spiBuffer[0]=W_REGISTER | DYNPD;
		context->spiBuffer[1]=DPL_P0 | DPL_P1;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 8:
		context->handlerStep=9;
		//set default baudrate(2Mbps) and power(0dBm)
		context->spiBuffer[0]=W_REGISTER | RF_SETUP;
		context->spiBuffer[1]=RF_DR_HIGH | (0x03<<RF_PWR_BASE);
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 9:
		context->handlerStep=10;
		//set default channel(2)
		context->spiBuffer[0]=W_REGISTER | RF_CH;
		context->spiBuffer[1]=2;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 10:
		context->handlerStep=11;
		//set default Tx address
		context->spiBuffer[0]=W_REGISTER | TX_ADDR;
		context->spiBuffer[1]=0xE7;
		context->spiBuffer[2]=0xE7;
		context->spiBuffer[3]=0xE7;
		context->spiBuffer[4]=0xE7;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(5, moduleId);
		break;
	case 11:
		context->handlerStep=12;
		//set default Rx addres(pipe 0)
		context->spiBuffer[0]=W_REGISTER | RX_ADDR_P0;
		context->spiBuffer[1]=0xE7;
		context->spiBuffer[2]=0xE7;
		context->spiBuffer[3]=0xE7;
		context->spiBuffer[4]=0xE7;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(5, moduleId);
		break;
	case 12:
		context->handlerStep=13;
		//set default Rx addres(pipe 1)
		context->spiBuffer[0]=W_REGISTER | RX_ADDR_P1;
		context->spiBuffer[1]=0xC2;
		context->spiBuffer[2]=0xC2;
		context->spiBuffer[3]=0xC2;
		context->spiBuffer[4]=0xC2;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(5, moduleId);
		break;
	case 13:
		context->handlerStep=14;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_TX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 14:
		context->handlerStep=15;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_RX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 15:
		context->handlerStep=16;
		//clear flags
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 16:
		//finish
		ENTER_ATOMIC();
		context->flags = Flg_State_Standby;
		context->irqHandler=0;
		context->txFifoLoad=0;
		context->continuousTxByteNumber=0;
		context->maxContinuousTxByteNumber=MaxContTxBytesFor2Mbps;
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	}
}


void standby_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		nRF24L01p_stateOfCE(0, moduleId);
		SMALL_DELAY();

		context->handlerStep=1;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_TX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 1:
		context->handlerStep=2;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_RX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 2:
		//clear flags
		context->handlerStep=3;
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 3:
		//finish
		ENTER_ATOMIC();
		context->flags=Flg_State_Standby;
		context->irqHandler=0;
		context->txFifoLoad=0;
		context->continuousTxByteNumber=0;
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void powerOff_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//deactivate CE
		nRF24L01p_stateOfCE(0, moduleId);
		SMALL_DELAY();

		context->handlerStep=1;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_TX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 1:
		//flush other FIFO
		context->handlerStep=2;
		context->spiBuffer[0]=FLUSH_RX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 2:
		//clear flags
		context->handlerStep=3;
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 3:
		//clear PWR_UP
		context->handlerStep=4;
		context->spiBuffer[0]=W_REGISTER | CONFIG;
		context->spiBuffer[1]=EN_CRC; //reset value
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 4:
		//finish
		ENTER_ATOMIC();
		context->flags=Flg_State_PowerOff;
		context->irqHandler=0;
		context->txFifoLoad=0;
		context->continuousTxByteNumber=0;
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void setRxAddress_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//write Rx address P1
		context->handlerStep=1; //first of all change step
		context->spiBuffer[0]=W_REGISTER | RX_ADDR_P1;
		context->spiBuffer[1]=context->commandArgs[0];
		context->spiBuffer[2]=context->commandArgs[1];
		context->spiBuffer[3]=context->commandArgs[2];
		context->spiBuffer[4]=context->commandArgs[3];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(5, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void setTxAddress_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//write Tx address
		context->handlerStep=1;
		context->spiBuffer[0]=W_REGISTER | TX_ADDR;
		context->spiBuffer[1]=context->commandArgs[0];
		context->spiBuffer[2]=context->commandArgs[1];
		context->spiBuffer[3]=context->commandArgs[2];
		context->spiBuffer[4]=context->commandArgs[3];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(5, moduleId);
		break;
	case 1:
		//write Rx address P0
		context->handlerStep=2;
		context->spiBuffer[0]=W_REGISTER | RX_ADDR_P0;
		context->spiBuffer[1]=context->commandArgs[0];
		context->spiBuffer[2]=context->commandArgs[1];
		context->spiBuffer[3]=context->commandArgs[2];
		context->spiBuffer[4]=context->commandArgs[3];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(5, moduleId);
		break;
	case 2:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void setChannel_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//write channel
		context->handlerStep=1;
		nrf24l01p_context[moduleId].spiBuffer[0]=W_REGISTER | RF_CH;
		nrf24l01p_context[moduleId].spiBuffer[1]=context->commandArgs[0];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void setPowerAndDataRate_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//write RF_SETUP
		context->handlerStep=1;
		context->spiBuffer[0]=W_REGISTER | RF_SETUP;
		context->spiBuffer[1]=context->commandArgs[0];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 1:
		//write SETUP_RETR
		context->handlerStep=2;
		context->spiBuffer[0]=W_REGISTER | SETUP_RETR;
		context->spiBuffer[1]=context->commandArgs[1];
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 2:
		ENTER_ATOMIC();
		context->maxContinuousTxByteNumber=*(uint16_t *)(context->commandArgs+2);
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		//finish
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}


void switchToTx_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		if(Flg_State_Standby!=(context->flags & Flg_State_Mask)){
			nRF24L01p_stateOfCE(0, moduleId);
			SMALL_DELAY();

			context->handlerStep=1;
			//flush FIFO
			context->spiBuffer[0]=FLUSH_TX;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(1, moduleId);
		}else{
			context->handlerStep=4;
			//enable pipe 0
			context->spiBuffer[0]=W_REGISTER | EN_RXADDR;
			context->spiBuffer[1]=ERX_P0;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
			break;
		}
		break;
	case 1:
		context->handlerStep=2;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_RX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 2:
		context->handlerStep=3;
		//clear flags
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 3:
		context->handlerStep=4;
		//enable pipe 0
		context->spiBuffer[0]=W_REGISTER | EN_RXADDR;
		context->spiBuffer[1]=ERX_P0;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 4:
		context->handlerStep=5;
		//mask interrupts, conf. prim. Tx
		context->spiBuffer[0]=W_REGISTER | CONFIG;
		context->spiBuffer[1]=MASK_RX_DR | EN_CRC | PWR_UP;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 5:
		//finish
		nRF24L01p_stateOfCE(1, moduleId);
		SMALL_DELAY();
		ENTER_ATOMIC();
		context->txFifoLoad=0;
		context->continuousTxByteNumber=0;
		if(context->commandArgs[0]){
			context->flags = Flg_State_StreamTx;
			context->irqHandler=irqInStreamTx_handler;
		}else{
			context->flags = Flg_State_Tx;
			context->irqHandler=irqInTx_handler;
		}
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void switchToRx_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		if(Flg_State_Standby!=(context->flags & Flg_State_Mask)){
			nRF24L01p_stateOfCE(0, moduleId);
			SMALL_DELAY();

			context->handlerStep=1;
			//flush FIFO
			context->spiBuffer[0]=FLUSH_TX;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(1, moduleId);
		}else{
			context->handlerStep=4;
			//enable pipe 1
			context->spiBuffer[0]=W_REGISTER | EN_RXADDR;
			context->spiBuffer[1]=ERX_P1;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
			break;
		}
		break;
	case 1:
		context->handlerStep=2;
		//flush FIFO
		context->spiBuffer[0]=FLUSH_RX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(1, moduleId);
		break;
	case 2:
		context->handlerStep=3;
		//clear flags
		context->spiBuffer[0]=W_REGISTER | STATUS;
		context->spiBuffer[1]=RX_DR | TX_DS | MAX_RT;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 3:
		context->handlerStep=4;
		//enable pipe 1
		context->spiBuffer[0]=W_REGISTER | EN_RXADDR;
		context->spiBuffer[1]=ERX_P1;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;

	case 4:
		context->handlerStep=5;
		//mask interrupts, conf. prim. Rx
		context->spiBuffer[0]=W_REGISTER | CONFIG;
		context->spiBuffer[1]= MASK_MAX_RT | EN_CRC | PWR_UP | PRIM_RX;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 5:
		//finish
		nRF24L01p_stateOfCE(1, moduleId);
		SMALL_DELAY();
		ENTER_ATOMIC();
		context->txFifoLoad=0;
		context->continuousTxByteNumber=0;
		context->flags = Flg_State_Rx;
		context->irqHandler=irqInRx_handler;
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}


void send_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//place command and send
		context->handlerStep=1;
		ENTER_ATOMIC();
		context->txFifoLoad=1;
		EXIT_ATOMIC();
		context->spiBuffer[0]=W_TX_PAYLOAD_NOACK;
		copyBuffer(&(context->spiBuffer[1]), &(context->commandArgs[1]), context->commandArgs[0]);
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(context->commandArgs[0]+1, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}

void sendWithAck_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		//place command and send
		context->handlerStep=1;
		ENTER_ATOMIC();
		context->txFifoLoad=1;
		EXIT_ATOMIC();
		context->spiBuffer[0]=W_TX_PAYLOAD;
		copyBuffer(&(context->spiBuffer[1]), &(context->commandArgs[1]), context->commandArgs[0]);
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(context->commandArgs[0]+1, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	default:;
	}
}


void sendInStream_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t lengthOfPacket;
	uint8_t executeIrq=0;
	switch(context->handlerStep){
	case 0:
		lengthOfPacket=ServiceBytesInPacket+context->commandArgs[0];
		if(context->continuousTxByteNumber+lengthOfPacket > context->maxContinuousTxByteNumber){
			//wait while TX buffer is not empty->continuousTxByteNumber is not 0
			ENTER_ATOMIC();
			if(context->flags & Flg_HandleIRQ){
				if(context->irqHandler){
					context->handler=context->irqHandler;
					executeIrq=1;
				}
				context->flags &= ~Flg_HandleIRQ;
			} else {
				context->handler=0;
			}
			EXIT_ATOMIC();
			if(executeIrq){
				context->handlerStep=0;
				context->handler(moduleId);
			}
		}else{
			context->handlerStep=1;
			//write payload to TX FIFO
			ENTER_ATOMIC();
			context->txFifoLoad+=1;
			context->continuousTxByteNumber+=lengthOfPacket;
			EXIT_ATOMIC();

			context->spiBuffer[0]=W_TX_PAYLOAD_NOACK;
			copyBuffer(&(context->spiBuffer[1]), &(context->commandArgs[1]), context->commandArgs[0]);
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(context->commandArgs[0]+1, moduleId);
		}
		break;
	case 1:
		context->handlerStep=2;
		//check that TX FIFO is not empty - data placed in FIFO(no info about delay between
		//CSN pos. edge and actual placement of data in FIFO, this is important for driver logic)
		//or
		//TX_DS flag is high(with slow processor this handler may be invoked after PL send, assume
		//that TX_DS=0 and TX FIFO empty is impossible state)
		context->spiBuffer[0]=R_REGISTER | FIFO_STATUS;
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(2, moduleId);
		break;
	case 2:
		if(!(context->spiBuffer[1] & TX_EMPTY) || (context->spiBuffer[0] & TX_DS)){
			//finish
			ENTER_ATOMIC();
			context->commandHandler=0;
			context->lockCallback |= Lock_OnCommandHandled;
			EXIT_ATOMIC();
			if(context->onCommandHandled){
				context->onCommandHandled();
			}
			ENTER_ATOMIC();
			context->lockCallback &= ~Lock_OnCommandHandled;
			EXIT_ATOMIC();
			tryToHandleNext(moduleId);
		}else{
			//repeat the same as for step 1
			context->spiBuffer[0]=R_REGISTER | FIFO_STATUS;
			nRF24L01p_stateOfNCS(0, moduleId);
			nRF24L01p_startSpiExchange(2, moduleId);
		}
		break;
	}
}

void writePayloadForAck_handler(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	switch(context->handlerStep){
	case 0:
		context->handlerStep=1;
		ENTER_ATOMIC();
		context->txFifoLoad=1;
		EXIT_ATOMIC();
		context->spiBuffer[0]=W_ACK_PAYLOAD | 0x01; //payload for pipe 1
		copyBuffer(&(context->spiBuffer[1]), &(context->commandArgs[1]), context->commandArgs[0]);
		nRF24L01p_stateOfNCS(0, moduleId);
		nRF24L01p_startSpiExchange(context->commandArgs[0]+1, moduleId);
		break;
	case 1:
		//finish
		ENTER_ATOMIC();
		context->commandHandler=0;
		context->lockCallback |= Lock_OnCommandHandled;
		EXIT_ATOMIC();
		if(context->onCommandHandled){
			context->onCommandHandled();
		}
		ENTER_ATOMIC();
		context->lockCallback &= ~Lock_OnCommandHandled;
		EXIT_ATOMIC();
		tryToHandleNext(moduleId);
		break;
	}
}

void tryToHandleNext(uint8_t moduleId){
	nRF24L01p_DriverContext * context=&(nrf24l01p_context[moduleId]);
	uint8_t executeNext=0;
	ENTER_ATOMIC();
	if(context->flags & Flg_HandleIRQ){
		if(context->irqHandler){
			context->handler=context->irqHandler;
			executeNext=1;
		}
		context->flags &= ~Flg_HandleIRQ;
	} else if(context->commandHandler){
		context->handler=context->commandHandler;
		executeNext=1;
	} else {
		context->handler=0;
	}
	EXIT_ATOMIC();
	if(executeNext){
		context->handlerStep=0;
		context->handler(moduleId);
	}
}
