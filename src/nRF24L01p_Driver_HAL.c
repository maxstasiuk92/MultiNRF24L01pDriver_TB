/*
 * nRF24L01_HAL.c
 *
 *  Created on: 29 трав. 2020 р.
 *      Author: MaxCm
 */
#include "RFMaster_HAL.h"
#include "RFSlave_HAL.h"
#include "RFModuleID.h"

void nRF24L01p_startSpiExchange(uint8_t length, uint8_t moduleId){
	switch(moduleId){
	case RFMaster:
		RFMaster_startSpiExchange(length);
		break;
	case RFSlave:
		RFSlave_startSpiExchange(length);
		break;
	default:;
	}
}

void nRF24L01p_stateOfCE(uint8_t state, uint8_t moduleId){
	switch(moduleId){
	case RFMaster:
		if(state){
			RFMaster_setCE();
		} else {
			RFMaster_resetCE();
		}
		break;
	case RFSlave:
		if(state){
			RFSlave_setCE();
		} else {
			RFSlave_resetCE();
		}
		break;

	default:;
	}
}

void nRF24L01p_stateOfNCS(uint8_t state, uint8_t moduleId){
	switch(moduleId){
	case RFMaster:
		if(state){
			RFMaster_setNCS();
		} else {
			RFMaster_resetNCS();
		}
		break;
	case RFSlave:
		if(state){
			RFSlave_setNCS();
		} else {
			RFSlave_resetNCS();
		}
		break;

	default:;
	}
}

