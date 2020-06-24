/*
 * NoACKTransmitionTest.c
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */
#include <nRF24L01p.h>
#include "stm32f10x.h"
#include "MultiNRF24L01pDriver.h"
#include "TestUtil.h"


//TODO: change to passNoAckMultipleTransmTest
/*master - sends, slave - receives*/
uint8_t passMultipleNoAckTransmTest(uint8_t masterId, uint8_t slaveId){
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
		//turn-on modules
		commandMaster(nRF24L01p_powerOn(rfMaster));
		commandSlave(nRF24L01p_powerOn(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//set addresses
		commandMaster(nRF24L01p_setTxAddress(13, rfMaster));
		commandSlave(nRF24L01p_setRxAddress(13, rfSlave));
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

		testStep=2; //send 32 values
		prepareToSendData_0to31();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=3; //send 1 value
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=4; //send 7 random values
		prepareToSendData_Random7();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
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

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnDataReceived(0, rfSlave);

	return result;
}

/*master - sends, slave - receives*/
uint8_t passNoAckTransmWithAddrChangeTest(uint8_t masterId, uint8_t slaveId){
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
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait ~2ms(130us needed)
		delayTest(2);
		if(failedTestStep) break;

		testStep=2; //to check if all is ok, send 32 values with correct address
		prepareToSendData_0to31();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(dataRx && !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=3; //change address of Slave
		commandSlave(nRF24L01p_standby(rfSlave));
		waitSlaveCommandDone();
		commandSlave(nRF24L01p_setRxAddress(0x20000002, rfSlave));
		waitSlaveCommandDone();
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(dataRx){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=4; //change Tx address on fly, check that prev. Slave change is ok
		commandMaster(nRF24L01p_setTxAddress(0x20000002, rfMaster));
		waitMasterCommandDone();
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData()
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
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

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnDataReceived(0, rfSlave);

	return result;
}

/*master - sends, slave - receives*/
uint8_t passNoAckTransmWithChannelChangeTest(uint8_t masterId, uint8_t slaveId){
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
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait ~2ms(130us needed)
		delayTest(2);
		if(failedTestStep) break;

		testStep=2; //to check if all is ok, send 32 values
		prepareToSendData_0to31();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=3; //change channel of Slave
		commandSlave(nRF24L01p_standby(rfSlave));
		waitSlaveCommandDone();
		commandSlave(nRF24L01p_setChannel(4, rfSlave)); //nearest channel for 2Mbps data rate
		waitSlaveCommandDone();
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(dataRx){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=4; //change channel of Master
		commandMaster(nRF24L01p_standby(rfMaster));
		waitMasterCommandDone();
		commandMaster(nRF24L01p_setChannel(4, rfMaster));
		waitMasterCommandDone();
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		waitMasterCommandDone();
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData()
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
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

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnDataReceived(0, rfSlave);

	return result;
}

uint8_t passNoAckTransmWithPowerAndDataRateChangeTest(uint8_t masterId, uint8_t slaveId){
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
		//switch to modes
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait ~2ms(130us needed)
		delayTest(2);
		if(failedTestStep) break;

		testStep=2; //to check if all is ok, send 32 values
		prepareToSendData_0to31();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=3; //change data rate for slave
		commandSlave(nRF24L01p_standby(rfSlave));
		waitSlaveCommandDone();
		commandSlave(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_0dBm, nRF24L01p_DataRate_250kbps, rfSlave));
		waitSlaveCommandDone();
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(dataRx){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=4; //change power and data rate of Master
		commandMaster(nRF24L01p_standby(rfMaster));
		waitMasterCommandDone();
		commandMaster(nRF24L01p_setPowerAndDataRate(nRF24L01p_OutputPower_m18dBm, nRF24L01p_DataRate_250kbps, rfMaster));
		waitMasterCommandDone();
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		waitMasterCommandDone();
		prepareToSendData_Random1();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData()
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=5; //check value in RF_SETUP
		nRF24L01p_invokeOnDataReceived(0, rfSlave);
		nRF24L01p_invokeOnDataReceived(onDataReceived, rfMaster);
		prepareToReceive();
		commandMaster(nRF24L01p_readRegister(RF_SETUP, rfMaster));
		waitReceiveData();
		if(!dataRx || rxDataBuffer[0]!=RF_DR_LOW){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=6; //check value in SETUP_RETR
		prepareToReceive();
		commandMaster(nRF24L01p_readRegister(SETUP_RETR, rfMaster));
		waitReceiveData();
		if(!dataRx || rxDataBuffer[0]!=((2<<ARC_BASE) | (7<<ARD_BASE))){
			//possibly changed number of retries or auto-retransm. delay in driver
			failedTestStep=testStep;
			break;
		}
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
	return result;
}

/*1) master - sends, slave - receives*/
/*2) master - receives, slave - sends*/
uint8_t passNoAckTransmWithRoleChangeTest(uint8_t masterId, uint8_t slaveId){
	rfMaster=masterId;
	rfSlave=slaveId;

	initTestUtil();

	uint8_t result=0;
	do{
		testStep=1; //init.
		//1) master - sends, slave - receives
		//attach callbacks
		nRF24L01p_invokeOnCommandHandled(Master_onCmdDone, rfMaster);
		nRF24L01p_invokeOnDataSent(onDataSent, rfMaster);

		nRF24L01p_invokeOnCommandHandled(Slave_onCmdDone, rfSlave);
		nRF24L01p_invokeOnDataReceived(onDataReceived, rfSlave);
		//turn-on modules
		commandMaster(nRF24L01p_powerOn(rfMaster));
		commandSlave(nRF24L01p_powerOn(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//set addresses for both cases
		commandMaster(nRF24L01p_setTxAddress(0x10000001, rfMaster));
		commandSlave(nRF24L01p_setRxAddress(0x10000001, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		//switch to modes
		commandMaster(nRF24L01p_switchToTx(0, rfMaster));
		commandSlave(nRF24L01p_switchToRx(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait ~2ms(130us needed)
		delayTest(2);
		if(failedTestStep) break;

		testStep=2; //send 32 values
		prepareToSendData_0to31();
		prepareToReceive();
		commandMaster(nRF24L01p_send(txDataBuffer, txDataLength, rfMaster));
		waitMasterCommandDone();
		waitReceiveData();
		if(dataRx && !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		//2) master - receives, slave - sends
		testStep=3; //change roles and send, addresses don't match
		nRF24L01p_invokeOnDataSent(0, rfMaster);
		nRF24L01p_invokeOnDataSent(onDataSent, rfSlave);
		nRF24L01p_invokeOnDataReceived(0, rfSlave);
		nRF24L01p_invokeOnDataReceived(onDataReceived, rfMaster);

		commandMaster(nRF24L01p_switchToRx(rfMaster));
		commandSlave(nRF24L01p_switchToTx(0, rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		prepareToSendData_Random7();
		prepareToReceive();
		commandSlave(nRF24L01p_send(txDataBuffer, txDataLength, rfSlave));
		waitSlaveCommandDone();
		waitReceiveData();
		if(dataRx){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		testStep=4; //correct addresses and send
		commandSlave(nRF24L01p_setTxAddress(0xC2C2C2C2, rfSlave)); //default Rx
		waitSlaveCommandDone();
		//wait 2ms(130us needed)
		delayTest(2);
		prepareToSendData_Random7();
		prepareToReceive();
		commandSlave(nRF24L01p_send(txDataBuffer, txDataLength, rfSlave));
		waitSlaveCommandDone();
		waitReceiveData();
		if(!dataRx || !areDataBuffersEqual()){
			failedTestStep=testStep;
			break;
		}
		if(failedTestStep) break;

		commandMaster(nRF24L01p_powerOff(rfMaster));
		commandSlave(nRF24L01p_powerOff(rfSlave));
		waitMasterCommandDone();
		waitSlaveCommandDone();

		result=1;
	}while(0);

	//clean callbacks
	nRF24L01p_invokeOnCommandHandled(0, rfMaster);
	nRF24L01p_invokeOnDataReceived(0, rfMaster);

	nRF24L01p_invokeOnCommandHandled(0, rfSlave);
	nRF24L01p_invokeOnDataSent(0, rfSlave);

	return result;
}

