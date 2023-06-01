/*
 * FatFs library - SD.c
 *
 * Created: 19-5-2023 11:39:46
 * Author : Johan Hogendoorn
 
 Bronnen:
 - Datasheet AT90CAN
 - http://exploreembedded.com/wiki/AVR_Timer_Interrupts
 - https://onlinedocs.microchip.com/pr/GUID-93DE33AC-A8E1-4DD9-BDA3-C76C7CB80969-en-US-2/index.html?GUID-0BED8014-6B01-47DD-A495-68F9F70287A3
 */ 

#define F_CPU 1600000UL
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <string.h>
#include "ff.h"
#include <avr/interrupt.h>

void init_timer(void); 

int val = 0;

ISR(TIMER2_OVF_vect)
{
	val++;
	TCNT2 = 0x00;
}

int main(void)
{
	init_timer();  
	sei();
	
	
	while(1)
	{
		if(val >= 61)
		{
			PORTC ^= (1 << PC0);
			val = 0;
		}
	}
	
}

void init_timer(void)
{
	TCNT2 = 0x00; // start counting at 0
	TCCR2A = ((1 << CS22) | (1 << CS21) | (1 << CS20)); // prescaler op 1024
	TIMSK2 = (1 << TOIE2);
}

/*
void init_timer(void) {
	CLKPR = 0x80; CLKPR = 0x00; // reset prescaler
	TCNT3 = 0xC2F7; // start counting at (2^16 - 159 = 65377 = 0xFF61)
	TCCR3B = (( 1 << CS32) | ( 1 << CS30)); // prescaler op 1024
	TIMSK3 = (1 << TOIE3) ;		// Enable timer 1 overflow interrupt(TOIE2), pag. 142 datasheet
	sei();					// Enable global interrupts by setting global interrupt enable bit in SRE
}*/
