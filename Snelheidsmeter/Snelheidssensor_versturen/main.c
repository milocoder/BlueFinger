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
	//float omtrek_wiel = 1.728; // 2 * Pi * r. r = 0.275 m
	float omtrek_wiel = 0.864;			//omtrek wiel delen door aantal magneten, dus 1.728 /2 = 0.864
	unsigned long timer = 0; // hierin wordt de huidige tijd gestopt in miliseconden
	volatile int rpmaantal = 0;
	int vorigeStatusHall = 0;
	int huidigeStatusHall = 0;
	float snelheidms = 0;
	float snelheidKmh = 0;
	
	init();
	
	while(1) {
		huidigeStatusHall = PINE & (1 << PE4);			//uitlezen input (=output comparator)
		if(huidigeStatusHall) {
			if(vorigeStatusHall == 0)
			{
				rpmaantal += 1;
				vorigeStatusHall = 1;
			}
		}
		vorigeStatusHall = huidigeStatusHall;
		
		// volgende stuk kijkt eens in de 1000ms naar het aantal rotaties (rpmaantal), berekent daarmee de snelheid in kmh en verstuurt deze snelheid
		if(millis() - timer >= 1000) 
		{
			snelheidms = (float) (omtrek_wiel * rpmaantal);		//v = s/t 
			snelheidKmh = snelheidms * 3.6; 
			rpmaantal = 0; // reset rpm
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
	tx_message.id = 0x10; // can ID
	tx_message.length = 8;
	tx_message.data[0] = voorKomma;
	tx_message.data[1] = naKomma;
	for (int i = 2; i < 8; i++)
	{
		tx_message.data[i] = 0x00;
	}
	
	sendCAN(&tx_message);
}

void init()
{
	init_millis(F_CPU);
	sei();  // Zet interrupts aan
	initCAN();
	
	DDRE = 0;	//input sensor (e4)
	PORTE = 0;
}
