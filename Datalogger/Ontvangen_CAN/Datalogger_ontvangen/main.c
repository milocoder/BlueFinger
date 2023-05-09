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

#define CAN_ID_ARDUINO 0x10

void writeFloatToEEPROM(int value, int address);


int main(void)
{
	CANMessage rx_message; 
	uint8_t result; 
	int addressEEPROM = 0;

   
    DDRC |= (1<<PC0);
	initCAN(); 
   
   	result = listenForMessage(CAN_ID_ARDUINO, 8);
	   
   
    while (1) 
    {
		// PORTC |= (1<<PC0);				//hoog maken pin
	
		// PORTC &= ~(1 << PC0);			//laag maken pin	
		
		
			uint8_t myMessage = getMessage(&rx_message);
			if (myMessage) {
				
						PORTC ^= (1<<PC0);	
						writeFloatToEEPROM(rx_message.id, addressEEPROM);
					
						uint16_t data = rx_message.data[1] << 8 | rx_message.data[0];						
						char buf[17];
						itoa(data, buf, 16);
						
						for (int i = 0; i <= 8; i++){
							writeFloatToEEPROM(buf[i], addressEEPROM); 
							addressEEPROM += 2; 
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



void writeFloatToEEPROM(int value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	//int val2 = (int)((value-val1)*100); // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	//eeprom_write_byte((uint8_t*)address+1, val2);
	
}