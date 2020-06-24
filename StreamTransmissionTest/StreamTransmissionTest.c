/*
 * StreamTransmissionTest.c
 *
 *  Created on: 20 черв. 2020 р.
 *      Author: MaxCm
 */
#include "MultiNRF24L01pDriver.h"
#include "TestUtil.h"

volatile uint32_t txWord, refRxWord, lostPayloads;
volatile uint8_t errorInReceived;

void fillTxDataBuffer(){
	uint8_t * dst=txDataBuffer;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	*(uint32_t *)dst=txWord; dst+=4;
	__disable_irq();
	txWord++;
	__enable_irq();
}

void Stream_onDataReceived(uint8_t * data, uint8_t length){
	uint8_t validPL=1;
	uint32_t word;
	if(0!=errorInReceived || length!=32){
		errorInReceived=1;
	}else{
		word=*(uint32_t *)data;
		if(word<refRxWord || word>txWord){
			validPL=0;
		}else{
			for(uint8_t i=0; i<8; i++){
				if(*(uint32_t *)data!=word){
					validPL=0;
					break;
				}
				data+=4;
			}
		}
		if(validPL){
			lostPayloads+=word-refRxWord;
			refRxWord=word+1;
		}else{
			errorInReceived=1;
		}
		__disable_irq();
		transferTimer=0;
		__enable_irq();
	}
}

uint8_t testStream(){
	uint8_t result=1;

	testTimer=0;
	transferTimer=0;
	txWord=0;
	refRxWord=0;
	lostPayloads=0;
	errorInReceived=0;

	while(testTimer<5000 && txWord<0xFFFF){
		if(txWord-refRxWord<3 && nRF24L01p_canSendToStream(rfMaster)){
			fillTxDataBuffer();
			commandMaster(nRF24L01p_sendInStream(txDataBuffer, 32, rfMaster));
			waitMasterCommandDone();
		}
		if(0!=errorInReceived || lostPayloads>100){
			result=0;
			break;
		}
	}
	//read the rest from RX FIFO
	__disable_irq();
	testTimer=0;
	__enable_irq();
	while(testTimer<10 && refRxWord<=txWord);

	if(0!=errorInReceived || lostPayloads>100){
		result=0;
	}

	return result;
}

uint8_t passStreamTransmTest(uint8_t masterId, uint8_t slaveId){
	rfMaster=masterId;
	rfSlave=slaveId;

	initTestUtil();

	uint8_t result=0;
	do{
		testStep=1;
		nRF24L01p_invokeOnCommandHandled(Master_onCmdDone, rfMaster);

		nRF24L01p_invokeOnCommandHandled(Slave_onCmdDone, rfSlave);
		nRF24L01p_invokeOnDataReceived(Stream_onDataReceived, rfSlave);
		//turn-on modules
		commandMaster(nRF24L01p_powerOn(rfMaster));
		commandSlave(nRF24L01p_powerOn(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//set addresses
		commandMaster(nRF24L01p_setTxAddress(0x10000001, rfMaster));
		commandSlave(nRF24L01p_setRxAddress(0x10000001, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		if(failedTestStep) break;

		testStep=2; //stream at 2Mbps
		//set baud rate
		commandMaster(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_2Mbps, rfMaster));
		commandSlave(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_2Mbps, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(1, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		if(!testStream()){failedTestStep=testStep; break;}
		if(failedTestStep) break;
		commandMaster(nRF24L01p_standby(rfMaster));
		commandSlave(nRF24L01p_standby(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		testStep=3; //stream at 1Mbps
		//set baud rate
		commandMaster(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_1Mbps, rfMaster));
		commandSlave(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_1Mbps, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(1, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		if(!testStream()){ failedTestStep=testStep; break;}
		if(failedTestStep) break;
		commandMaster(nRF24L01p_standby(rfMaster));
		commandSlave(nRF24L01p_standby(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		testStep=4; //stream at 250kbps
		//set baud rate
		commandMaster(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_250kbps, rfMaster));
		commandSlave(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_250kbps, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(1, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		if(!testStream()){failedTestStep=testStep; break;}
		if(failedTestStep) break;

		commandMaster(nRF24L01p_powerOff(rfMaster));
		commandSlave(nRF24L01p_powerOff(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		result=1;
	}while(0);

	nRF24L01p_invokeOnCommandHandled(0, rfMaster);

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnDataReceived(0, rfSlave);

	return result;
}

