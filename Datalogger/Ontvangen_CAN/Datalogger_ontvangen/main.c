/*
 * Datalogger_ontvangen.c
 *
 * Created: 30-3-2023 13:27:06
 * Author : Johan
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "can.h"

#define EXPECTED_CAN_ID 0x778


int main(void)
{
   CANMessage rx_message; 
   uint8_t result; 
   
   
   
    /* Replace with your application code */
    while (1) 
    {
		
		result = listenForMessage(EXPECTED_CAN_ID, 8); 
		
		if(getMessage(&rx_message)) {
			if (rx_message.id == EXPECTED_CAN_ID) {
				uint16_t data = rx_message.data[1] << 8 | rx_message.data[0];
				char buf[17];
				itoa(data, buf, 10);			// 10 is decimaal
				//uart -> puts buf; 
				
    }
}

