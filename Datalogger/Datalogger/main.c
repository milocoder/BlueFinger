#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "can.h"

#define CAN_ID    0x780


int main(void)
{
	initCAN();
	uint8_t result; 
		
	
	while(1) {	
		
		CANMessage tx_message;
		tx_message.id = CAN_ID; 
		
		for (uint8_t i=0; i < 8; i++) {		
			tx_message.data[i] = 0;
		}
		
	
		tx_message.length = 8;
		tx_message.data[0] = 0;
		tx_message.data[1] = 1;
		tx_message.data[2] = 2;
		tx_message.data[3] = 3;
		tx_message.data[4] = 4;
		tx_message.data[5] = 5;
		tx_message.data[6] = 6;
		tx_message.data[7] = 7;
		result  = sendCAN(&tx_message); 
		
		
	}
}
