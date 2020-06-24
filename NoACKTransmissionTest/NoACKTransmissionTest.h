/*
 * NoACKTransmitionTest.h
 *
 *  Created on: 2 черв. 2020 р.
 *      Author: MaxCm
 */

#ifndef NOACKTRANSMISSIONTEST_H_
#define NOACKTRANSMISSIONTEST_H_

uint8_t passMultipleNoAckTransmTest(uint8_t masterId, uint8_t slaveId);
uint8_t passNoAckTransmWithAddrChangeTest(uint8_t masterId, uint8_t slaveId);
uint8_t passNoAckTransmWithChannelChangeTest(uint8_t masterId, uint8_t slaveId);
uint8_t passNoAckTransmWithPowerAndDataRateChangeTest(uint8_t masterId, uint8_t slaveId);
uint8_t passNoAckTransmWithRoleChangeTest(uint8_t masterId, uint8_t slaveId);


#endif /* NOACKTRANSMISSIONTEST_H_ */
