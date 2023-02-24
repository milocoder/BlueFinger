/*
* PIB_led_code.c
*
* Created: 21-2-2023 15:11:47
* Author : Milo
*/
#define F_CPU 12000000UL
#include <avr/io.h>
//#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include "can_lib.h"



int main(void)
{
	DDRC = 0xFF;
	while (1)
	{
		
		PORTC = 0xFF;
		_delay_ms(50);
		PORTC = 0x00;
		_delay_ms(50);
		
		
		
	}
}
