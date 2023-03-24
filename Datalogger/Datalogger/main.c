#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "can.h"

int main(void)
{
	initCAN();
	
	CANMessage message;
	message.id = 2; 
	message.length = 8; 
	message.data[0] = 0; 
	message.data[1] = 1;
	message.data[2] = 2; 
	message.data[3] = 3; 
	message.data[4] = 4; 
	message.data[5] = 5; 
	message.data[6] = 6;
	message.data[7] = 7; 
	
		
	
	while(1) {	
		sendCAN(&message); 
		
		
	}
}
