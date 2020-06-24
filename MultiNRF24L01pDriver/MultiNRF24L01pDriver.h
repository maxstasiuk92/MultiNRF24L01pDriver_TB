/*
 * nRF24L01p_Driver.h
 *
 *  Created on: 29 трав. 2020 р.
 *      Author: MaxCm
 */
#include <stdint.h>

#ifndef _MultiNRF24L01pDriver_h_
#define _MultiNRF24L01pDriver_h_

#define nRF24L01p_RFChannel_min		0x00
#define nRF24L01p_RFChannel_max		0x7F

#define nRF24L01p_DataRate_250kbps	0
#define nRF24L01p_DataRate_1Mbps	1
#define nRF24L01p_DataRate_2Mbps	2

#define nRF24L01p_OutputPower_m18dBm	0
#define nRF24L01p_OutputPower_m12dBm	1
#define nRF24L01p_OutputPower_m6dBm		2
#define nRF24L01p_OutputPower_0dBm		3

/* Functions start communication with nRF24L01+ modules, non-zero value will be
 * returned if function was invoked in correct state with correct parameters.
 * User callback(onCommandHandled) will be invoked after this communication end.
 *
 * All functions may be invoked from interrupt routines
 * Max. length of data - 32bytes
 *
 * Interrupt handler should not be interrupted by the same handler
 * moduleId is never checked to improve performance
 */

//Must be called during initialization after start-up
//clears variables
void nRF24L01p_InitDriver();

//attach callbacks
uint8_t nRF24L01p_invokeOnCommandHandled(void (*func)(), uint8_t moduleId);
uint8_t nRF24L01p_invokeOnDataSent(void (*func)(uint8_t success, uint8_t * dataWithAckBuffer, uint8_t dataWithAckLength), uint8_t moduleId);
uint8_t nRF24L01p_invokeOnDataReceived(void (*func)(uint8_t * dataBuffer, uint8_t dataLength), uint8_t moduleId);
uint8_t nRF24L01p_invokeOnAckSent(void (*func)(), uint8_t moduleId);

//applicable in any state
uint8_t nRF24L01p_writeRegister(uint8_t reg, uint8_t val, uint8_t moduleId);
//applicable in any state, on onCommandHandled will NOT be invoked
uint8_t nRF24L01p_readRegister(uint8_t reg, uint8_t moduleId);
//applicable in Power-Off state
uint8_t nRF24L01p_powerOn(uint8_t moduleId);
//applicable in any state EXCEPT Power-Off
uint8_t nRF24L01p_powerOff(uint8_t moduleId);
//applicable in any state EXCEPT Power-Off
uint8_t nRF24L01p_standby(uint8_t moduleId);
//applicable in Standby, Tx and StreamTx states
uint8_t nRF24L01p_setRxAddress(uint32_t address, uint8_t moduleId);
//applicable in all states EXCEPT Power-Off
//all transfers should be finished in Tx and StreamTx states
uint8_t nRF24L01p_setTxAddress(uint32_t address, uint8_t moduleId);
//applicable in Standby state
uint8_t nRF24L01p_setChannel(uint8_t channel, uint8_t moduleId);
//applicable in Standby state
uint8_t nRF24L01p_setPowerAndDataRate(uint8_t power, uint8_t dataRate, uint8_t moduleId);
//applicable in any state EXCEPT Power-Off
uint8_t nRF24L01p_switchToTx(uint8_t streamTx, uint8_t moduleId);
//applicable in any state EXCEPT Power-Off
//all transfers should be finished in Tx and StreamTx states
//nRF24L01+ module will be able to receive in 130us
uint8_t nRF24L01p_switchToRx(uint8_t moduleId);
//applicable in Tx state
//previous transfer should be finished
uint8_t nRF24L01p_send(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId);
//applicable in Tx state
//previous transfer should be finished
uint8_t nRF24L01p_sendWithAck(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId);
//applicable in StreamTx state
uint8_t nRF24L01p_sendInStream(uint8_t * dataDuffer, uint8_t dataLength, uint8_t moduleId);
//applicable in Rx state
//there should be no pending payload(for ACK)
uint8_t nRF24L01p_writePayloadForAck(uint8_t * dataBuffer, uint8_t dataLength, uint8_t moduleId);
//applicable in StreamTx state
//returns Result_OK(non-zero) if possible to put next payload to stream
uint8_t nRF24L01p_canSendToStream(uint8_t moduleId);

#endif /* _MultiNRF24L01pDriver_h_ */
