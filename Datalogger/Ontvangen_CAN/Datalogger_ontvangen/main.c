/*
 * Datalogger_ontvangen.c
 *
 * Created: 30-3-2023 13:27:06
 * Author : Johan
 */ 

#define F_CPU 16000000UL 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "can.h"

#define CAN_ID_SNELHEIDSMETER 0x50


int main(void)
{
	CANMessage rx_message; 
	uint8_t result; 
   
    DDRC |= (1<<PC0);
	initCAN(); 
   
   	result = listenForMessage(CAN_ID_SNELHEIDSMETER, 8);
	   
   
    while (1) 
    {
		// PORTC |= (1<<PC0);				//hoog maken pin
	
		// PORTC &= ~(1 << PC0);			//laag maken pin	
		
		
			uint8_t myMessage = getMessage(&rx_message);
			if (myMessage) {
				
						PORTC = (1<<PC0);	
						if (rx_message.id == CAN_ID_SNELHEIDSMETER) {
							//PORTC = (0<<PC0);
						}
			}
		
			/*
			
			if(getMessage(&rx_message)) {
				if (rx_message.id == CAN_ID_ARDUINO) {
					PORTC ^= (1<<PC0);
				}
			}
			
			*/

		
		
		 
	}
}

