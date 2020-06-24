/*
 * nRF24L01p_Utils.c
 *
 *  Created on: 23 черв. 2020 р.
 *      Author: MaxCm
 */
#include "stdint.h"

void copyBuffer(uint8_t * dst, uint8_t * src, uint8_t bytes){
	uint8_t i;
	uint8_t n=bytes>>2;
	for(i=0; i<n; i++){
		*((uint32_t *)dst)=*((uint32_t *)src);
		dst+=4;
		src+=4;
	}
	for(i=n<<2; i<bytes; i++){
		*dst=*src;
		dst+=1;
		src+=1;
	}
}
