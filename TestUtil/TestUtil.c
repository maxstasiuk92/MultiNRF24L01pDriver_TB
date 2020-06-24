/*
 * Util.c
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */
#include "../TestUtil/TestUtil.h"

void initTestUtil(){
	masterCommandDone=1;
	slaveCommandDone=1;

	dataRx=1;
	dataTx=1;
	ackTx=1;

	testStep=0;
	failedTestStep=0;
}

void Master_onCmdDone(){
	if(masterCommandDone){
		//masterCommandDone not cleared - not expected
		failedTestStep=testStep;
	}
	masterCommandDone=1;
}

void onDataSent(uint8_t success, uint8_t * buf, uint8_t len){
	if(dataTx){
		//because not expected
		failedTestStep=testStep;
	}else{
		//save PL
		for(uint8_t i=0; i<len; i++){
			ackRxDataBuffer[i]=buf[i];
		}
		ackRxDataLength=len;

		txSuccess=success;
		dataTx=1;
	}
}

void onAckSent(){
	if(ackTx){
		//because not expected
		failedTestStep=testStep;
	}else{
		ackTx=1;
	}
}

void Slave_onCmdDone(){
	if(slaveCommandDone){
		//slaveCommandDone not cleared - not expected
		failedTestStep=testStep;
	}
	slaveCommandDone=1;
}

void onDataReceived(uint8_t * buf, uint8_t len){
	if(dataRx){
		//because not expected
		failedTestStep=testStep;
	} else {
		for(uint8_t i=0; i<len; i++){
			rxDataBuffer[i]=buf[i];
		}
		rxDataLength=len;
		dataRx=1;
	}
}

void prepareToSendData_0to31(){
	for(uint8_t i=0; i<32; i++){
		txDataBuffer[i]=i;
		rxDataBuffer[i]=0;
	}
	txDataLength=32;
	rxDataLength=0;
	__disable_irq();
	dataTx=0;
	transferTimer=0;
	__enable_irq();
}

void prepareToSendData_Random3(){
	txDataBuffer[0]=28;
	txDataBuffer[1]=0;
	txDataBuffer[2]=8;
	for(uint8_t i=0; i<3; i++){
		rxDataBuffer[i]=0;
	}
	txDataLength=3;
	rxDataLength=0;
	__disable_irq();
	dataTx=0;
	transferTimer=0;
	__enable_irq();
}

void prepareToSendData_Random5(){
	txDataBuffer[0]=154;
	txDataBuffer[1]=93;
	txDataBuffer[2]=15;
	txDataBuffer[3]=1;
	txDataBuffer[4]=3;
	for(uint8_t i=0; i<5; i++){
		rxDataBuffer[i]=0;
	}
	txDataLength=5;
	rxDataLength=0;
	__disable_irq();
	dataTx=0;
	transferTimer=0;
	__enable_irq();
}

void prepareToSendData_Random1(){
	txDataBuffer[0]=17;
	rxDataBuffer[0]=0;
	txDataLength=1;
	rxDataLength=0;
	__disable_irq();
	dataTx=0;
	transferTimer=0;
	__enable_irq();
}

void prepareToSendData_Random7(){
	txDataBuffer[0]=170;
	txDataBuffer[1]=20;
	txDataBuffer[2]=99;
	txDataBuffer[3]=1;
	txDataBuffer[4]=0;
	txDataBuffer[5]=31;
	txDataBuffer[6]=51;
	for(uint8_t i=0; i<7; i++){
		rxDataBuffer[i]=0;
	}
	txDataLength=7;
	rxDataLength=0;
	__disable_irq();
	dataTx=0;
	transferTimer=0;
	__enable_irq();
}


void prepareToSendAck_0to31(){
	for(uint8_t i=0; i<32; i++){
		ackTxDataBuffer[i]=i;
		ackRxDataBuffer[i]=0;
	}
	ackTxDataLength=32;
	ackRxDataLength=0;
	__disable_irq();
	ackTx=0;
	__enable_irq();
}

void prepareToSendAck_Random4(){
	ackTxDataBuffer[0]=170;
	ackTxDataBuffer[1]=20;
	ackTxDataBuffer[2]=99;
	ackTxDataBuffer[3]=1;
	for(uint8_t i=0; i<4; i++){
		ackRxDataBuffer[i]=0;
	}
	ackTxDataLength=4;
	ackRxDataLength=0;
	__disable_irq();
	ackTx=0;
	__enable_irq();
}

uint8_t areDataBuffersEqual(){
	uint8_t equal=1;
	if(txDataLength!=rxDataLength){
		equal=0;
	}else{
		for(uint8_t i=0; i<txDataLength; i++){
			if(txDataBuffer[i]!=rxDataBuffer[i]){
				equal=0;
				break;
			}
		}
	}
	return equal;
}

uint8_t areAckBuffersEqual(){
	uint8_t equal=1;
	if(ackTxDataLength!=ackRxDataLength){
		equal=0;
	}else{
		for(uint8_t i=0; i<ackTxDataLength; i++){
			if(ackTxDataBuffer[i]!=ackRxDataBuffer[i]){
				equal=0;
				break;
			}
		}
	}
	return equal;
}

void UtilTimer(){
	//no race
	masterTimer++;
	slaveTimer++;
	transferTimer++;
	testTimer++;
}


