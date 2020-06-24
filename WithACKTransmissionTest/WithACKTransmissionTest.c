/*
 * WithACKTransmissionTest.c
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */
#include <nRF24L01p.h>
#include "stdint.h"

#include "stm32f10x.h"
#include "MultiNRF24L01pDriver.h"
#include "TestUtil.h"


uint8_t passWithAckMultTransmTest(uint8_t masterId, uint8_t slaveId){
	rfMaster=masterId;
	rfSlave=slaveId;

	initTestUtil();

	uint8_t result=0;
	do{
		testStep=1; //init.
		//attach callbacks
		nRF24L01p_invokeOnCommandHandled(Master_onCmdDone, rfMaster);
		nRF24L01p_invokeOnDataSent(onDataSent, rfMaster);

		nRF24L01p_invokeOnCommandHandled(Slave_onCmdDone, rfSlave);
		nRF24L01p_invokeOnDataReceived(onDataReceived, rfSlave);
		nRF24L01p_invokeOnAckSent(onAckSent, rfSlave);
		//turn-on modules
		commandMaster(nRF24L01p_powerOn(rfMaster));
		commandSlave(nRF24L01p_powerOn(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//set addresses
		commandMaster(nRF24L01p_setTxAddress(0x10000001, rfMaster));
		commandSlave(nRF24L01p_setRxAddress(0x20000002, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		if(failedTestStep) break;

		testStep=2; //send to wrong address
		prepareToSendData_Random3();
		prepareToReceive();
		commandMaster(nRF24L01p_sendWithAck(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitSendData();
		waitReceiveData();
		if(!dataTx || txSuccess){failedTestStep=testStep; break;}
		if(dataRx){failedTestStep=testStep; break;}
		if(failedTestStep) break;

		testStep=3; //send with ACK needed, no PL with ACK
		commandMaster(nRF24L01p_setTxAddress(0x20000002, rfMaster));
		waitMasterCommandDone();

		prepareToSendData_Random5();
		prepareToReceive();
		commandMaster(nRF24L01p_sendWithAck(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitSendData();
		waitReceiveData();
		if(!dataTx || !txSuccess || ackRxDataLength!=0) {failedTestStep=testStep; break;}
		if(!dataRx || !areDataBuffersEqual()) {failedTestStep=testStep; break;}
		if(failedTestStep) break;

		testStep=4; //send with ACK needed, PL(length=31) with ACK
		prepareToSendAck_0to31();
		commandSlave(nRF24L01p_writePayloadForAck(ackTxDataBuffer, ackTxDataLength, rfSlave));
		waitSlaveCommandDone();

		prepareToSendData_Random3();
		prepareToReceive();
		commandMaster(nRF24L01p_sendWithAck(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitSendData();
		waitReceiveData();
		waitSendAck();
		if(!dataTx || !txSuccess) {failedTestStep=testStep; break;}
		if(!dataRx || !areDataBuffersEqual()) {failedTestStep=testStep; break;}
		//onAckSent not invoked here
		if(ackTx || !areAckBuffersEqual()) {failedTestStep=testStep; break;}

		testStep=5; //send with ACK needed, onAckSent will be invoked, no PL with ACK
		prepareToSendData_Random5();
		prepareToReceive();
		commandMaster(nRF24L01p_sendWithAck(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitSendData();
		waitReceiveData();
		waitSendAck();
		if(!dataTx || !txSuccess || ackRxDataLength!=0) {failedTestStep=testStep; break;}
		if(!dataRx || !areDataBuffersEqual()) {failedTestStep=testStep; break;}
		if(!ackTx) {failedTestStep=testStep; break;}

		commandMaster(nRF24L01p_powerOff(rfMaster));
		commandSlave(nRF24L01p_powerOff(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		result=1;
	}while(0);

	//clean callbacks
	nRF24L01p_invokeOnCommandHandled(0, rfMaster);
	nRF24L01p_invokeOnDataSent(0, rfMaster);

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnDataReceived(0, rfSlave);
	nRF24L01p_invokeOnAckSent(0, rfSlave);

	return result;
}


uint8_t passWithAckReTransmTest(uint8_t masterId, uint8_t slaveId){
	rfMaster=masterId;
	rfSlave=slaveId;

	initTestUtil();

	uint8_t result=0;
	do{
		testStep=1; //init.
		//attach callbacks
		nRF24L01p_invokeOnCommandHandled(Master_onCmdDone, rfMaster);
		nRF24L01p_invokeOnDataSent(onDataSent, rfMaster);

		nRF24L01p_invokeOnCommandHandled(Slave_onCmdDone, rfSlave);
		nRF24L01p_invokeOnDataReceived(onDataReceived, rfSlave);
		nRF24L01p_invokeOnAckSent(onAckSent, rfSlave);
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
		//set slowest baud rate
		commandMaster(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_0dBm, nRF24L01p_DataRate_250kbps, rfMaster));
		commandSlave(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_0dBm, nRF24L01p_DataRate_250kbps, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		waitMasterCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		if(failedTestStep) break;

		//8-preamble, 32-address, 9-control, 256-PL, 8-CRC=313 bits
		//time for transmission=313/250k=1.252ms
		//Auto-retransm. in 2ms
		//First retransm. after 3.51ms, second - 6.89ms
		testStep=2; //send with ACK needed, no PL with ACK
		prepareToSendData_0to31();
		commandMaster(nRF24L01p_sendWithAck(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		//wait ~6ms
		delayTest(6);
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitSlaveCommandDone();
		prepareToReceive();

		waitSendData();
		waitReceiveData();
		if(!dataTx || !txSuccess) {failedTestStep=testStep; break;}
		if(!dataRx || !areDataBuffersEqual()) {failedTestStep=testStep; break;}
		if(failedTestStep) break;

		testStep=3; //check OBSERVE_TX register
		nRF24L01p_invokeOnDataReceived(0, rfSlave);
		nRF24L01p_invokeOnDataReceived(onDataReceived, rfMaster);
		rxDataBuffer[0]=0;
		prepareToReceive();
		commandMaster(nRF24L01p_readRegister(OBSERVE_TX, rfMaster));
		waitReceiveData();
		if(!dataRx || rxDataLength!=1 || (rxDataBuffer[0] & 0x0F)!=2) {failedTestStep=testStep; break;}
		if(failedTestStep) break;

		commandMaster(nRF24L01p_powerOff(rfMaster));
		commandSlave(nRF24L01p_powerOff(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		result=1;
	}while(0);

	//clean callbacks
	nRF24L01p_invokeOnCommandHandled(0, rfMaster);
	nRF24L01p_invokeOnDataSent(0, rfMaster);
	nRF24L01p_invokeOnDataReceived(0, rfMaster);

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnAckSent(0, rfSlave);

	return result;
}



