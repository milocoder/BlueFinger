/*
 * Hall_sensor3.c
 *
 * Created: 17-3-2023 18:08:43
 * Author : Johan
 
 Code maakt gebruik van een timer - millis() functie. Deze wordt iedere keer op 0 gezet als meeting gedaan is. 
 Andere suggestie is om oude waardes van de timer te gebruiken. Deze staan achter de // 
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "can.h"
#include "millis.h"

void init();
void verstuurCan(float snelheid);

float omtrek_wiel = 1.728; // 2 * Pi * r. r = 0.275 m
volatile int rpmaantal = 0;

ISR (INT4_vect) {
	rpmaantal++;
}


ISR(TIMER1_OVF_vect)				//timer 0 ISR
{
	//PORTC ^= (1 << PC0);
	TCNT1 = 49636;							//1 seconde
	float snelheidms = (float) (omtrek_wiel * rpmaantal); 
	float snelheidKmh = snelheidms * 3.6;
	rpmaantal = 0; // reset rpm
	verstuurCan(snelheidKmh);
	
}

int main(void)
{	
	init();	
}

void verstuurCan(float snelheid)
{
		// volgende stuk initaliseert de canmessage struct, verstuurt snelheid met data[0] en data[1]
		int voorKomma = (int)snelheid; // pak getal voor de komma
		int naKomma = (int)((snelheid-voorKomma)*100); // pak kommagetal en vermenigvuldig keer 100
		
		CANMessage tx_message;
		tx_message.id = 0x50; // can ID
		tx_message.length = 2;
		tx_message.data[0] = voorKomma;
		tx_message.data[1] = naKomma;
		sendCAN(&tx_message);
}

void init()
{
	//init external interrupt	
	EIMSK |= (1<<INT4);					//aanzetten interrupt INT0, dat is de pin waarop wordt aangesloten (blaz 95)
	EICRA |= (1 << ISC00)|(1 << ISC01);	//interrupt voor opgaande flank	asynchronous (blaz. 94)
	
	//init counter1
	CLKPR = 0x80; CLKPR = 0x00; // reset prescaler
	TCNT1 = 49636;				//start counting vanaf 49636. (49636*1024) /16000000 = 1 sec
	TCCR1A = 0x00;
	TCCR1B = (( 1 << CS12) | ( 1 << CS10)); // prescaler op 1024
	TIMSK1 = (1 << TOIE1) ;		// Enable timer 1 overflow interrupt(TOIE2), pag. 142 datasheet
	sei();					// Enable global interrupts by setting global interrupt enable bit in SRE

	//init CAN
	initCAN();
}

