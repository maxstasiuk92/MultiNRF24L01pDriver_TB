/*
 * WithACKTransmissionTest.h
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */

#ifndef WITHACKTRANSMISSIONTEST_H_
#define WITHACKTRANSMISSIONTEST_H_

void withAck_onTimer();

uint8_t passWithAckMultTransmTest(uint8_t masterId, uint8_t slaveId);
uint8_t passWithAckReTransmTest(uint8_t masterId, uint8_t slaveId);

#endif /* WITHACKTRANSMISSIONTEST_H_ */
