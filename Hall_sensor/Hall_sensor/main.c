/*
 * Hall_sensor.c
 *
 * Created: 8-3-2023 17:56:37
 * Author : johan
 */ 

 
/*
//#define LED  1
#define HALL 0
//#define  util_BitSet(x,bit)            ((x) |=  util_GetBitMask(bit))
//#define  util_BitClear(x,bit)          ((x) &= ~util_GetBitMask(bit))

 

int main(void)
{ 

//DDRC |=  (1 << LED); //output 
DDRC &= ~(1<< HALL); //input
DDRF = 0xFF; 


// make PA0 as input for hall sensor and PA1 output for LED



while(1)
{
  if (DDRC &= ~(1<< HALL)) {
	 PORTF = 0xFF; 
			
		} else {
			 PORTF = 0x00; 
		}

	}

}


int main(void)
{
	// configure PORTA as output
	DDRF = 0xFF;
	// configure PORTB as input
	DDRC = 0;
	// make sure PORTB is high impedance and will not source
	PORTC = 0;
	// main loop
	while (1)
	{
		if (~PINC & (1 << PC0))
		{
			// turn on every other light on PORTA, inverse logic
			PORTF = 0xFF;
				}
		else
		{
			// default case, all lights off on PORTA, inverse logic
			PORTF = 0x00;
		}
	}
}
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "millis.h"

// Pin waarop de Hall-sensor is aangesloten
//#define HALL_PIN PC0

// Variabelen voor het berekenen van de snelheid

unsigned long VorigeMilli = 0;
float OmtrekWiel = 1.35; // Omtrek van het wiel in meters
float TijdsVerschilSeconden = 0.0; // Tijd tussen twee metingen in seconden

void writeFloatToEEPROM(float value, int address);
float CalculateSpeed(); 


int main() {
	init_millis(16000000UL);  
	sei();  // Enable interrupts
	
	DDRF = 1;
	DDRC = 0; 
	PORTC = 0; 	
	
	int addressHall = 1;

	while (1) {
		float Speed = CalculateSpeed();	

		//naar EEPROM schrijven
		writeFloatToEEPROM(Speed, addressHall);
		addressHall += 3;
		
	}
	
}

float CalculateSpeed() {
	float snelheidKmH = 0.0; // Snelheid in km/h
	volatile int rpmaantal = 0; // Aantal keer magneet langs hall
	unsigned long HuidigAantalMili = millis();
	unsigned long TijdsVerschilMilli = HuidigAantalMili - VorigeMilli;
	TijdsVerschilSeconden = TijdsVerschilMilli / 1000.0;

	// Uitlezen van pinwaarde. Als pin hoog is: rpm++ en ledje aan zetten
	if(PINC & (1 << PC0)){
		rpmaantal++;
		PORTF = 0xFF;	//led lichtje voor feedback
	}
	
		// Meet de snelheid alleen als er minstens één omwenteling is gedetecteerd en een kwart seconde voorbij is
		if(rpmaantal > 0 && TijdsVerschilSeconden > 0.25) {
			snelheidKmH = (OmtrekWiel* rpmaantal * 3.6) / TijdsVerschilSeconden;
			
		

			// Reset de teller en de timer voor de volgende meting
			rpmaantal = 0;
			VorigeMilli = HuidigAantalMili;
		}


		// Als er meer dan 2 seconden voorbij is dan en de rmpaantal is gelijk aan 0 -> auto staat stil of aan opstarten. Min. meting snelheid = 2.5 kmh
		if(TijdsVerschilSeconden > 2 && rpmaantal == 0) snelheidKmH = 0.0;
		
	
	}
	

}

void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100)+1; // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}
