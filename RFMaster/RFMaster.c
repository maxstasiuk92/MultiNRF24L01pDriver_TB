/*
 * RFMaster.c
 *
 *  Created on: 1 ����. 2020 �.
 *      Author: MaxCm
 */
#include "RFMaster_HAL.h"
#include "RFModuleID.h"
#include "../MultiNRF24L01pDriver/MultiNRF24L01pDriver.h"

void RFMaster_powerOnInit(){
	RFMaster_configPeripherals();
}



