/*
 * nRF24L01p_Driver_HAL.h
 *
 *  Created on: 30 трав. 2020 р.
 *      Author: MaxCm
 */

#ifndef _MultiNRF24L01pDriver_HAL_h_
#define _MultiNRF24L01pDriver_HAL_h_

//Returns pointer to buffer for SPI communication
//Provided HAL should transfer all data from buffer via SPI
//Input from SPI should be stored to this buffer
uint8_t * nRF24L01p_getSpiBuffer(uint8_t moduleId);

//Should be implemented by user
void nRF24L01p_startSpiExchange(uint8_t length, uint8_t moduleId);
void nRF24L01p_stateOfCE(uint8_t state, uint8_t moduleId);
void nRF24L01p_stateOfNCS(uint8_t state, uint8_t moduleId);

//Should be invoked by user
void nRF24L01p_onIRQ(uint8_t moduleId);
void nRF24L01p_onFinishSpiExchange(uint8_t moduleId);
void nRF24L01p_onTimer(uint8_t moduleId);

#endif /* _MultiNRF24L01pDriver_HAL_h_ */
