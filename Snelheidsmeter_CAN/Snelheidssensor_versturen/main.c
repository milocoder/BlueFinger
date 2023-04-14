#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "can.h"
//#include "millis.h"

#define CAN_ID    0x50
void writeFloatToEEPROM(float value, int address)

int main(void)
{
	initCAN();
	uint8_t result;
	
	DDRC = 0xFF;	//output led
	PORTC = 0;
	
	int a = 1;
	while(1) {
		CANMessage tx_message;
		tx_message.id = CAN_ID;
		
		for (uint8_t i=0; i < 8; i++) {
			tx_message.data[i] = 0;
		}
		
		tx_message.length = 8;
		tx_message.data[0] = a;
		tx_message.data[1] = 8;
		tx_message.data[2] = 8;
		tx_message.data[3] = 8;
		tx_message.data[4] = 0;
		tx_message.data[5] = 0;
		tx_message.data[6] = 0;
		tx_message.data[7] = 3;
		result  = sendCAN(&tx_message);
		a++;
		
		for (uint8_t i=0; i < 8; i++) {
			tx_message.data[i] = 0;
		}
		_delay_ms(2000);
	}
}


void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100); // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
}