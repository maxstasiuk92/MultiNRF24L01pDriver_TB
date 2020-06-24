/*
 * CommunicationTest.c
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */
#include <nRF24L01p.h>
#include <stdint.h>
#include "MultiNRF24L01pDriver.h"
#include "TestUtil.h"



uint8_t passCommunicationTest(uint8_t moduleId){
	uint8_t result=0;

	initTestUtil();

	nRF24L01p_invokeOnCommandHandled(Master_onCmdDone, moduleId);
	nRF24L01p_invokeOnDataReceived(onDataReceived, moduleId);
	delayTest(101); //for start-up

	do{
		testStep=1; //write CONFIG
		commandMaster(nRF24L01p_writeRegister(CONFIG, (PWR_UP | MASK_TX_DS | MASK_RX_DR), moduleId));
		waitMasterCommandDone();
		if(failedTestStep){break;}

		testStep=2; //write RF_CH
		commandMaster(nRF24L01p_writeRegister(RF_CH, 0x75, moduleId));
		waitMasterCommandDone();
		if(failedTestStep){break;}

		testStep=3; //read CONFIG
		prepareToReceive();
		commandMaster(nRF24L01p_readRegister(CONFIG, moduleId));
		waitReceiveData();
		if(rxDataBuffer[0]!=(PWR_UP | MASK_TX_DS | MASK_RX_DR)){failedTestStep=testStep; break;}
		if(failedTestStep){break;}

		testStep=4; //read RF_CH
		prepareToReceive();
		commandMaster(nRF24L01p_readRegister(RF_CH, moduleId));
		waitReceiveData();
		if(rxDataBuffer[0]!=0x75){failedTestStep=testStep; break;}
		if(failedTestStep){break;}

		testStep=5; //clear RF_CH
		commandMaster(nRF24L01p_writeRegister(RF_CH, 0x00, moduleId));
		waitMasterCommandDone();
		if(failedTestStep){break;}

		testStep=6; //clear CONFIG
		commandMaster(nRF24L01p_writeRegister(CONFIG, 0x00, moduleId))
		waitMasterCommandDone();
		if(failedTestStep){break;}

		testStep=7; //read RF_CH
		prepareToReceive();
		commandMaster(nRF24L01p_readRegister(RF_CH, moduleId));
		waitReceiveData();
		if(rxDataBuffer[0]!=0x00){failedTestStep=testStep; break;}
		if(failedTestStep){break;}

		testStep=8;//read CONFIG
		prepareToReceive()
		commandMaster(nRF24L01p_readRegister(CONFIG, moduleId));
		waitReceiveData();
		if(rxDataBuffer[0]!=0x00){failedTestStep=testStep; break;}
		if(failedTestStep){break;}

		result=1;
	}while(0);

	nRF24L01p_invokeOnCommandHandled(0, moduleId);
	nRF24L01p_invokeOnDataReceived(0, moduleId);

	return result;
}


