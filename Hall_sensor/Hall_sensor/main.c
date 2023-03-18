/*
 * Hall_sensor2.c
 *
 * Created: 17-3-2023 14:11:32
 * Author : johan
 */ 

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
		
	DDRF = 0xFF;		//output ledje
	DDRC = 0;		//input hall sensor
	PORTC = 0;
	
 	
	while(1) {
	
		unsigned long HuidigeAantalMili = millis();
		if (!(~PINC & (1 << PC0))) {
			rpmaantal++;
		}

		// Meet de snelheid alleen als er minstens één omwenteling is gedetecteerd en een kwart seconde voorbij is
		if(rpmaantal > 0) {
			
			float TijdVerschilSeconden = (float)(HuidigeAantalMili - VorigeAantalMili)/1000;
			
			if(TijdVerschilSeconden > 0.5) {
			snelheidms = omtrek_wiel / TijdVerschilSeconden; 
			snelheidKmH = snelheidms * 3.6;
			
			writeFloatToEEPROM(snelheidKmH, addressHall);
			
			addressHall += 2;
			PORTF = 0xFF;	//led lichtje voor feedback
			

			// Reset de teller en de timer voor de volgende meting
			rpmaantal = 0;
			VorigeAantalMili = HuidigeAantalMili;
			}
		
		} else {
			PORTF = 0x00;
		}
		
		
		
		
	}
}

		
		
			
			/*
			if (!(~PINC & (1 << PC0))) 	{
		
				huidige_tijd_ms = millis();			
				uint32_t verschil_tijd_ms = huidige_tijd_ms - vorige_tijd_ms; 
				uint32_t verschil_tijd_s = verschil_tijd_ms / 1000; 
				
			
				double snelheidms = omtrek_wiel / verschil_tijd_s;
				double snelheidKmH = snelheidms * 3.6;  		
				writeFloatToEEPROM(snelheidKmH, addressHall);
				
				addressHall += 2;	
				PORTF = 0xFF;			
			
			
				
				vorige_tijd_ms = huidige_tijd_ms; 

			} else {
				PORTF = 0x00; 
			}
			
		//vorige_status_hall = huidige_status_hall; 
		
	}
}
*/


void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100)+1; // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}