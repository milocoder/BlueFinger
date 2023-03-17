/*
 * Hall_sensor2.c
 *
 * Created: 17-3-2023 14:11:32
 * Author : johan
 */ 

#define F_CPU 16000000UL
#define OMTREK_WIEL	 135

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "millis.h"


void writeFloatToEEPROM(float value, int address);

int main(void)
{	
	init_millis(16000000UL);
	sei();  // Enable interrupts
	
	int huidige_status_hall = 0; 
	int vorige_status_hall = 0; 
	unsigned long huidige_tijd_ms = 0; 
	unsigned long vorige_tijd_ms = 0; 	
	
	int addressHall = 1;
	
	DDRF = 1;		//output ledje
	DDRC = 0;		//input hall sensor
	PORTC = 0;
		
	
	/* Replace with your application code */
    while (1) 
    {	
		huidige_status_hall = PINC & (1 << PC0); 
		
		if (vorige_status_hall != huidige_status_hall && huidige_status_hall == 1) {
			huidige_tijd_ms = millis(); 
			unsigned long verschil_tijd_ms = huidige_tijd_ms - vorige_tijd_ms; 
			
			float afstand_cm = OMTREK_WIEL; 
			float tijd_uren = (float)verschil_tijd_ms / 1000.0 / 3600.0; 
			float snelheidKmH = afstand_cm / 100000.0 / tijd_uren; 
			
			writeFloatToEEPROM(snelheidKmH, addressHall);
			addressHall += 3;		
			
			
			vorige_status_hall = huidige_status_hall;
			vorige_tijd_ms = huidige_tijd_ms; 

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
