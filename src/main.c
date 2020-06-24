/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f10x.h"

#include "RFMaster.h"
#include "RFSlave.h"
#include "IRQPriorityConfig.h"
#include "RFModuleID.h"

#include "CommunicationTest.h"
#include "NoACKTransmissionTest.h"
#include "WithACKTransmissionTest.h"
#include "StreamTransmissionTest.h"
#include "MultiNRF24L01pDriver.h"

void initRFTimer();

volatile uint8_t passedAllTests, failedTest, test;

void initLED(){
	GPIO_InitTypeDef pinInitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	pinInitStruct.GPIO_Pin=GPIO_Pin_9;
	pinInitStruct.GPIO_Speed=GPIO_Speed_10MHz;
	pinInitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_ResetBits(GPIOC, GPIO_Pin_9);
	GPIO_Init(GPIOC, &pinInitStruct);
}
//TODO: test: send PL and flush fifo
int main(void)
{
	SystemInit();
	initLED();
	nRF24L01p_InitDriver();
	RFMaster_powerOnInit();
	RFSlave_powerOnInit();
	initRFTimer();

	passedAllTests=0;
	test=1;
	failedTest=0;
	do{
		//Check SPI communication
		test=1; if(!passCommunicationTest(RFMaster)){failedTest=test; break;}
		test=2; if(!passCommunicationTest(RFSlave)){failedTest=test; break;}
		//Check transmissions without ACK
		test=3; if(!passMultipleNoAckTransmTest(RFMaster, RFSlave)){failedTest=test; break;}
		test=4; if(!passNoAckTransmWithAddrChangeTest(RFMaster, RFSlave)){failedTest=test; break;}
		test=5; if(!passNoAckTransmWithChannelChangeTest(RFMaster, RFSlave)){failedTest=test; break;}
		test=6; if(!passNoAckTransmWithPowerAndDataRateChangeTest(RFMaster, RFSlave)){failedTest=test; break;}
		test=7; if(!passNoAckTransmWithRoleChangeTest(RFMaster, RFSlave)){failedTest=test; break;}

		test=8; if(!passMultipleNoAckTransmTest(RFSlave, RFMaster)){failedTest=test; break;}
		test=9; if(!passNoAckTransmWithAddrChangeTest(RFSlave, RFMaster)){failedTest=test; break;}
		test=10; if(!passNoAckTransmWithChannelChangeTest(RFSlave, RFMaster)){failedTest=test; break;}
		test=11; if(!passNoAckTransmWithPowerAndDataRateChangeTest(RFMaster, RFSlave)){failedTest=test; break;}
		test=12; if(!passNoAckTransmWithRoleChangeTest(RFSlave, RFMaster)){failedTest=test; break;}
		//Check transmissions with ACK
		test=13; if(!passWithAckMultTransmTest(RFMaster, RFSlave)){failedTest=test; break;}
		test=14; if(!passWithAckReTransmTest(RFMaster, RFSlave)){failedTest=test; break;}

		test=15; if(!passWithAckMultTransmTest(RFSlave, RFMaster)){failedTest=test; break;}
		test=16; if(!passWithAckReTransmTest(RFSlave, RFMaster)){failedTest=test; break;}
		//Check streaming
		test=17;  if(!passStreamTransmTest(RFMaster, RFSlave)){failedTest=test; break;}

		test=18;  if(!passStreamTransmTest(RFSlave, RFMaster)){failedTest=test; break;}

		passedAllTests=1;
	}while(0);

	for(;;);
}

//MultiNRF24L01pDriver needs 1ms timer, use Timer3
//Let this init. be here for simpler project structure
void initRFTimer(){
	TIM_TimeBaseInitTypeDef timInitStruct;
	TIM_OCInitTypeDef timOcInitStruct;
	NVIC_InitTypeDef nvicInitStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	timInitStruct.TIM_Prescaler = 0;
	timInitStruct.TIM_Period = 24000; //get 1ms
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &timInitStruct);

	//Timer for driver
	timOcInitStruct.TIM_OCMode = TIM_OCMode_Timing;
	//timOcInitStruct.the rest = no effect
	TIM_OC1Init(TIM3, &timOcInitStruct);
	TIM_SetCompare1(TIM3, (24000>>1)-(24000>>2));
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	//Timer for test-cases
	timOcInitStruct.TIM_OCMode = TIM_OCMode_Timing;
	//timOcInitStruct.the rest = no effect
	TIM_OC1Init(TIM3, &timOcInitStruct);
	TIM_SetCompare4(TIM3, (24000>>1)+(24000>>2));
	TIM_ITConfig(TIM3, TIM_IT_CC4, ENABLE);

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	NVIC_ClearPendingIRQ(TIM3_IRQn);
	nvicInitStruct.NVIC_IRQChannel = TIM3_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = IRQ_RF_TimerPriority;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvicInitStruct);

	TIM_Cmd(TIM3, ENABLE);
}
