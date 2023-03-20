#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "millis.h"

// if(!(~PINC & (1 << PC0))) hiermee kijk je of pin hoog is

void writeFloatToEEPROM(float value, int address);

int main(void)
{	
	init_millis(12000000UL);
	sei();  // Enable interrupts
	
	// initialiseer in- en outputs
	DDRF = 0xFF;	//output led
	DDRC = 0;		// input hall sensor
	PORTC = 0;
	PORTF = 0;
	
	int addressHall = 0;
	float omtrek_wiel = 1.35;
	volatile int rpmaantal = 0;
	int vorigeStatusHall = 0;
	int huidigeStatusHall = 0;
	unsigned long timer = 0; // hierin wordt de huidige tijd gestopt in miliseconden
		
	while(1)
	{
		
		huidigeStatusHall = PINC & (1 << PC0); //was eerst: if(!(~PINC & (1 << PC0)))
		if(huidigeStatusHall) {
			if(vorigeStatusHall == 0)
			{
				PORTF = 1;
				rpmaantal += 1;
				vorigeStatusHall = 1;
			} else {
				PORTF = 0;
			}
		}
		vorigeStatusHall = huidigeStatusHall;
		
		if(millis() - timer >= 418) // 418 komt overeen met 5 seconden in werkelijkheid
		{			
			float snelheidms = (float) (omtrek_wiel * rpmaantal) / 5;
			float snelheidkmh = snelheidms * 3.6;
			writeFloatToEEPROM(snelheidkmh, addressHall);
			
			addressHall += 2;
			rpmaantal = 0; // reset rpm
			timer = millis(); // reset timer
		}
		
	}
}

void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100); // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}