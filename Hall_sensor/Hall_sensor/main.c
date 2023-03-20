#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "millis.h"

volatile int rpmaantal = 0; 
float snelheidKmH = 0.0; // Snelheid in km/h
float snelheidms = 0.0; 


void writeFloatToEEPROM(float value, int address);


int main(void)
{	
	init_millis(12000000UL);
	sei();  // Enable interrupts
	
	//int huidige_status_hall = 1; 
	//int vorige_status_hall = 0; 
	//uint32_t huidige_tijd_ms = 0; 
	unsigned long VorigeAantalMili = 0; 	
	
	int addressHall = 0;
	float omtrek_wiel = 1.35;
	float TijdVerschilSeconden = 0; 
		
	DDRF = 0xFF;		//output ledje
	DDRC = 0;		//input hall sensor
	PORTC = 0;
	
	while(1) {
	
		unsigned long HuidigeAantalMili = millis();
		TijdVerschilSeconden = (float)(HuidigeAantalMili - VorigeAantalMili)/1000;
		
		if(TijdVerschilSeconden > 0.1)
		{
			if (!(~PINC & (1 << PC0))) {
				rpmaantal++;
			}			
		}
		
		// Meet de snelheid alleen als er minstens één omwenteling is gedetecteerd en 0.1 seconde voorbij is
		if(rpmaantal > 0) {
			PORTF = 0xFF;	//led lichtje voor feedback
			snelheidms = omtrek_wiel / TijdVerschilSeconden; 
			snelheidKmH = snelheidms * 3.6;
			
			writeFloatToEEPROM(snelheidKmH, addressHall);
			
			addressHall += 2;

			// Reset de teller en de timer voor de volgende meting
			rpmaantal = 0;
			VorigeAantalMili = millis();
			PORTF = 0x00;
		}		
	}
}

void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100)+1; // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}