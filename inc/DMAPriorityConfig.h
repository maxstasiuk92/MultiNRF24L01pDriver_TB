#ifndef _DMAPRIORITYCONFIG_H_
#define _DMAPRIORITYCONFIG_H_
#include "stm32f10x.h"

//TODO: refractor
#define DMA1_RFMaster_TxPriority DMA_Priority_Low
#define DMA1_RFMaster_RxPriority DMA_Priority_Low

#define DMA1_RFSlave_TxPriority DMA_Priority_Medium
#define DMA1_RFSlave_RxPriority DMA_Priority_Medium

#endif /*_DMAPRIORITYCONFIG_H_*/
