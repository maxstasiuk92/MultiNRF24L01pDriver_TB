/*
 * Util.h
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */

#ifndef TESTUTIL_H_
#define TESTUTIL_H_
#include <stdint.h>
#include "stm32f10x.h"

uint8_t rxDataBuffer[32], txDataBuffer[32];
volatile uint8_t rxDataLength, txDataLength;

volatile uint8_t masterCommandDone, slaveCommandDone;
volatile uint8_t dataRx, dataTx, txSuccess;

volatile uint8_t testStep, failedTestStep;
volatile uint8_t rfMaster, rfSlave;

uint8_t ackTxDataBuffer[32], ackRxDataBuffer[32];
volatile uint8_t ackTxDataLength, ackRxDataLength, ackTx;

#define TimeForCommand	150 //power-on needs 100ms
#define TimeForTransfer	10
volatile uint8_t masterTimer, slaveTimer, transferTimer;
volatile uint32_t testTimer;

#define delayTest(time)	{__disable_irq(); testTimer=0; __enable_irq(); while(testTimer<time);}

#define commandMaster(command) {masterCommandDone=0;\
								__disable_irq(); masterTimer=0; __enable_irq();\
								if(!command){failedTestStep=testStep; break;}}


#define commandSlave(command) {slaveCommandDone=0;\
								__disable_irq(); slaveTimer=0; __enable_irq();\
								if(!command){failedTestStep=testStep; break;}}


#define waitMasterCommandDone()	{while(!masterCommandDone  && masterTimer<TimeForCommand); if(!masterCommandDone){failedTestStep=testStep; break;}}
#define waitSlaveCommandDone()	{while(!slaveCommandDone  && slaveTimer<TimeForCommand); if(!slaveCommandDone){failedTestStep=testStep; break;}}
#define waitSendData()			while(!dataTx && transferTimer<TimeForTransfer);
#define waitReceiveData()		while(!dataRx && transferTimer<TimeForTransfer);
#define waitSendAck()			while(!ackTx && transferTimer<TimeForTransfer);

void initTestUtil();

void Master_onCmdDone();
void Slave_onCmdDone();

void onDataSent(uint8_t success, uint8_t * buf, uint8_t len);
void onAckSent();
void onDataReceived(uint8_t * buf, uint8_t len);

#define prepareToReceive() {__disable_irq(); transferTimer=0; dataRx=0; __enable_irq();}
void prepareToSendData_0to31();
void prepareToSendData_Random1();
void prepareToSendData_Random3();
void prepareToSendData_Random5();
void prepareToSendData_Random7();

void prepareToSendAck_0to31();
void prepareToSendAck_Random4();

uint8_t areDataBuffersEqual();
uint8_t areAckBuffersEqual();

void UtilTimer();


#endif /* TESTUTIL_H_ */
