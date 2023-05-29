#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "can.h"
#include "millis.h"

void init();
void verstuurCan(float snelheid);

int main(void)
{
	float omtrek_wiel = 1.728; // 2 * Pi * r. r = 0.275 m
	unsigned long timer = 0; // hierin wordt de huidige tijd gestopt in miliseconden
	volatile int rpmaantal = 0;
	int vorigeStatusHall = 0;
	int huidigeStatusHall = 0;
	
	init();
	
	while(1) {
		// volgende stuk leest hall sensor input en incrementeert rpmaantal als de magneet langs is gekomen
		/*huidigeStatusHall = PINC & (1 << PC0);
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
		*/
		// volgende stuk kijkt eens in de 3000ms naar het aantal rotaties (rpmaantal), berekent daarmee de snelheid in kmh en verstuurt deze snelheid
		if(millis() - timer >= 3000)
		{
			//float snelheidms = (float) (omtrek_wiel * rpmaantal) / 3;
			//float snelheidKmh = snelheidms * 3.6;
			float snelheidKmh = 5.63;
			//rpmaantal = 0; // reset rpm
			verstuurCan(snelheidKmh);
			timer = millis(); // reset timer
		}
		
	}
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
	init_millis(F_CPU);
	sei();  // Zet interrupts aan
	initCAN();	
	
	DDRC = 0;	//input sensor (p5)
	PORTC = 0;
	DDRF = 1; // output led (p4)
	PORTF = 0;	
}