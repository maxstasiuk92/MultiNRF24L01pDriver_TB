/*
 * nRF24L01p_Driver_Config.h
 *
 *  Created on: 29 трав. 2020 р.
 *      Author: MaxCm
 */
#include "stm32f10x.h"

#ifndef _NRF24L01P_DRIVER_CONFIG_H_
#define _NRF24L01P_DRIVER_CONFIG_H_

/*IDs and number of modules*/
#define nRF24L01p_NumberOfModules	2

//macros for atomic operation
#define ENTER_ATOMIC() do{__disable_irq()
#define EXIT_ATOMIC() __enable_irq();}while(0)

//macro for small delay(comparable with signal settling, ~0.5us)
#define SMALL_DELAY() do{__NOP(); __NOP(); __NOP(); __NOP(); \
						 __NOP(); __NOP(); __NOP(); __NOP(); \
						 __NOP(); __NOP(); __NOP(); __NOP(); \
						 __NOP(); __NOP(); __NOP(); __NOP();}while(0)

#endif /* _NRF24L01P_DRIVER_CONFIG_H_ */
