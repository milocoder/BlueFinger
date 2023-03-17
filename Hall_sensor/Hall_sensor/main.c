/*
 * Hall_sensor2.c
 *
 * Created: 17-3-2023 14:11:32
 * Author : johan
 */ 

#define F_CPU 16000000UL
#define OMTREK_WIEL	 1.35

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
	uint32_t huidige_tijd_ms = 0; 
	uint32_t vorige_tijd_ms = 0; 	
	
	int addressHall = 0;
	
	DDRF = 1;		//output ledje
	DDRC = 0;		//input hall sensor
	PORTC = 0;
		
	
	/* Replace with your application code */
    while (1) 
    {	
		huidige_status_hall = PINC & (1 << PC0); 
		
		if (vorige_status_hall != huidige_status_hall && huidige_status_hall == 1) {
		
			huidige_tijd_ms = millis();
			float verschil_tijd_ms = huidige_tijd_ms - vorige_tijd_ms; 
			float verschil_tijd_s = verschil_tijd_ms / 1000; 
	
					
			
			float snelheidms = OMTREK_WIEL / verschil_tijd_s;
			float snelheidKmH = snelheidms * 3.6;  		
			writeFloatToEEPROM(snelheidKmH, addressHall);
			addressHall += 2;		
			
			
			timer1_millis = 0;				//reset millis() functie op 0; 
			//vorige_tijd_ms = huidige_tijd_ms; 

			}
			
		vorige_status_hall = huidige_status_hall; 
		
    }	
}


void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100)+1; // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}