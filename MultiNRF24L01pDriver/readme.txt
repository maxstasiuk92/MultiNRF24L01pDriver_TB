About this driver
1) multiple modules may be controlled by this driver
2) driver may function in interrupts(consume small CPU time)
3) variable length of transfered data blocks: 1..32


About required HAL and integration of this driver
1) nRF24L01p_onTimer should be invoked every 1ms. It is used for delays during start-up of nRF24L01+
2) nRF24L01p_startSpiExchange should start SPI communication with nRF24L01+. Also see description
	of nRF24L01p_getSpiBuffer in MultiNRF24L01pDriver_HAL.h
3) nRF24L01p_onFinishSpiExchange should be invoked after SPI communication finished.
4) Pins CE and NCS are controlled by this driver, provide implementation of nRF24L01p_stateOfCE and
	nRF24L01p_stateOfNCS to controll pins
5) nRF24L01p_onIRQ should be invoked at falling edge of IRQ pin of nRF24L01+
6) nRF24L01p_NumberOfModules in MultiNRF24L01pDriver_Config.h defines number of controlled modules
7) ENTER_ATOMIC()/EXIT_ATOMIC() - macros that provide atomic operation
8) SMALL_DELAY() - macro for small delay(~0.5us)


States description
Power-Off - turns of nRF24L01+. State after nRF24L01p_powerOff

Standby - small current consumption(26us). State after nRF24L01p_powerOn or nRF24L01p_standby

Rx - receive data, state after nRF24L01p_switchToRx; onDataReceived callback is invoked, after 
	next packed received.
	Parameters of onDataReceived
	dataBuffer - pointer to the buffer with data
	dataLength - number of bytes in buffer.
	nRF24L01p_writePayloadForAck is used to attach data with ACK packet. Next data may be attached,
	after transmission of previous. onAckSent callback is invoked after data transmission with ACK.
	Pay attention to scenario of attaching data with ACK:
	User invokes nRF24L01p_writePayloadForAck->other module transmits data->
	nRF24L01+ receives data and sends payload with ACK->onDataReceived callback is invoked->
	other module transmits data again(maybe without ACK reques)->nRF24L01+ receives data->
	onAckSent callback is invoked->onDataReceived callback is invoked.	 

Tx - transmit data using nRF24L01p_send or nRF24L01p_sendWithAck, state after nRF24L01p_switchToTx;
	next data may be sent, after transmission of previous. nRF24L01p_sendWithAck requires
	ACK from receiving module.
	Scenario:
	user invokes nRF24L01p_send or nRF24L01p_sendWithAck->
	onCommandHandled callback is invoked after commands and data clocked into nRF24L01+ module->
	onDataSent callback is invoked after actual data transfer and ACK reception.
	NOTE: onDataSent may be invoked before onCommandHandled, with slow CPU and fast baud rate of
	nRF24L01+
	Parameters of onDataSent callback:
	success - non-zero if transfer successful(recieved ACK); transfer with nRF24L01p_send is always
	successful.
	dataWithAckBuffer - buffer with data, "attached" with ACK(in case nRF24L01p_sendWithAck).
	dataWithAckLength - number of bytes in dataWithAckBuffer.
	
StreamTx - transmit data using nRF24L01p_sendInStream; state after nRF24L01p_switchToTx.
	Organizes continuous stream of data.
	Non-zero parameter streamTx of nRF24L01p_switchToTx selects StreamTx state.
	Scenario:
	user frequently invokes nRF24L01p_sendInStream to keep TX FIFO full->
	onCommandHandled callback is invoked as described for Tx state->
	onDataSent callback is invoked as described for Tx state.
	TX FIFO may contain 3 payloads of data. nRF24L01p_canSendToStream may be used to define
	if next payload may be clocked to nRF24L01+.



