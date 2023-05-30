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
#include <stdbool.h>
#include "can.h"

#define CAN_ID_SNELHEIDSMETER 5

int main(void)
{
	sei();
	initCAN();
	
	int waarde = 0;
	uint8_t result1 = 0;
	uint8_t result2 = 0;
	
	CANMessage bericht;
	result1 = listenForMessage(CAN_ID_SNELHEIDSMETER, 8); // 0-geen MOB's available. 1- wel available
	while(1)
	{
		result2 = getMessage(&bericht); // 1-bericht gevuld. 0-niet gevuld
		waarde = 1;
		if(result2)
		{
			waarde = 2;
			if(bericht.id == CAN_ID_SNELHEIDSMETER)
			{
				waarde = 3;
			}
		}
	}
}