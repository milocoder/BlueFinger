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
#include <util/delay.h>
#include <avr/interrupt.h>
#include "can.h"

#define CAN_ID_ARDUINO 0x778


int main(void)
{
   CANMessage rx_message; 
  // uint8_t result; 
   
    DDRC |= (1<<PC0);
	initCAN(); 
   
   
    while (1) 
    {
		
		//result = listenForMessage(CAN_ID_ARDUINO, 8); 			
			
	if (getMessage(&rx_message)) {
		 if (rx_message.id == CAN_ID_ARDUINO) {
			 PORTC |= (1<<PC0);
				
				//uint16_t data = rx_message.data[1] << 8 | rx_message.data[0];
				//char buf[17];
				//itoa(data, buf, 10);			// 10 is decimaal
		 }
			_delay_ms(500);
		}
		 
	}
}

