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
		huidigeStatusHall = PINC & (1 << PC0);
		if(huidigeStatusHall) {
			if(vorigeStatusHall == 0)
			{
				rpmaantal += 1;
				vorigeStatusHall = 1;
			}
		}
		vorigeStatusHall = huidigeStatusHall;
		
		// volgende stuk kijkt eens in de 3000ms naar het aantal rotaties (rpmaantal), berekent daarmee de snelheid in kmh en verstuurt deze snelheid
		if(millis() - timer >= 3000)
		{
			float snelheidms = (float) (omtrek_wiel * rpmaantal) / 3;
			float snelheidKmh = snelheidms * 3.6;
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
	tx_message.data[2] = 0x00;
	tx_message.data[3] = 0x00;
	tx_message.data[4] = 0x00;
	tx_message.data[5] = 0x00;
	tx_message.data[6] = 0x00;
	tx_message.data[7] = 0x00;
	
	sendCAN(&tx_message);
}

void init()
{
	init_millis(16000000UL);
	sei();  // Zet interrupts aan
	initCAN();
	
	DDRC = 0;	//input sensor (p5)
	PORTC = 0;
}