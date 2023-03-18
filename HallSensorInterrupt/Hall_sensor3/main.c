/*
 * Hall_sensor3.c
 *
 * Created: 17-3-2023 18:08:43
 * Author : Johan
 
 Code maakt gebruik van een timer - millis() functie. Deze wordt iedere keer op 0 gezet als meeting gedaan is. 
 Andere suggestie is om oude waardes van de timer te gebruiken. Deze staan achter de // 
 */ 

#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "millis.h"

#define OMTREK_WIEL_METER 1.35
volatile rpm_aantal = 0; 

void writeFloatToEEPROM(double value, int address);	
	
ISR (INT0_vect) {
	rpm_aantal++;
}


int main(void)
{
	
	DDRD &= ~(1 << PD0);				//PD0 als input
	PORTD |= (1 << PD0);				//pull-up resistor aanzetten
	EIMSK |= (1<<INT0);					//aanzetten interrupt IN0
	EICRA |= (1 << ISC00)|(1 << ISC10);	//interrupt voor opgaande flank	
	
	//MCUCR |= (1<<ISC01) | (1<<ISC00);	//zet INTO als trigger voor risig edge
	//EICRA |= (1 << ISC01) | (1 << ISC00);		//andere definitie of hetzelfde voor rising edge?
	
	int address_hall = 0;
	double snelheid_kmh = 0.0;
	double snelheid_metersec = 0.0; 
	 
	init_millis(12000000UL);
	sei();								//enable


    while (1) 
    {
		uint32_t huidig_aantal_ms = millis(); 
		uint32_t huidig_aantal_sec = huidig_aantal_ms/1000; 
		
		
		if (rpm_aantal > 0 && huidig_aantal_sec => 0.25) {
			snelheid_metersec = (rpm_aantal * OMTREK_WIEL_METER)/huidig_aantal_sec;
			snelheid_kmh = snelheid_metersec *3.6; 
			
			writeFloatToEEPROM(snelheid_kmh, address_hall);
			address_hall += 2; 
			
			timer1_millis = 0;				//reset millis() functie op 0, anders in milllis atomic block weg halen
			rpm_aantal = 0;					//reset rpm
			
			//
		}
		
		if (rpm_aantal == 0 && huidig_aantal_sec > 2.5) {
			snelheid_kmh = 0.0; 
			writeFloatToEEPROM(snelheid_kmh, address_hall); 
			address_hall += 2; 
		}
		
		
    }
}


void writeFloatToEEPROM(double value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100)+1; // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}

