#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "can.h"
#include "millis.h"

#define CAN_ID    0x50

int val1;
int val2;

int main(void)
{
	init_millis(16000000UL);
	sei();  // Enable interrupts
	initCAN();
	uint8_t result;
	
	DDRC = 0;	//input sensor (p5)
	PORTC = 0;
	DDRF = 1; // output led (p4)
	PORTF = 0;
	
	float omtrek_wiel = 1.35;
	volatile int rpmaantal = 0;
	int vorigeStatusHall = 0;
	int huidigeStatusHall = 0;
	unsigned long timer = 0; // hierin wordt de huidige tijd gestopt in miliseconden
	
	while(1) {
		
		// volgende stukje leest hall sensor input en doet rpmaantal + 1 als de magneet langs is gekomen
		huidigeStatusHall = PINC & (1 << PC0);
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
		
		// volgende stukje kijkt eens in de 3000ms naar het aantal rotaties (rpmaantal) en berekent daarmee de snelheid in kmh.
		// vb: snelheid is 5.36 km/h. dan val1 = 5 en val2 = 36
		if(millis() - timer >= 3000)
		{
			float snelheidms = (float) (omtrek_wiel * rpmaantal) / 3;
			float snelheidkmh = snelheidms * 3.6;
			val1 = (int)snelheidkmh; // pak getal voor de komma
			val2 = (int)((snelheidkmh-val1)*100); // pak kommagetal en doe keer 100
			rpmaantal = 0; // reset rpm
			timer = millis(); // reset timer
		}
		
		// volgende stuk initaliseert de canmessage struct, verstuurt snelheid met data[0] en data[1]. 
		// de 8'jes kunnen weggelaten worden (zorg wel voor dat length overeenkomt met aantal data[x])
		CANMessage tx_message;
		tx_message.id = CAN_ID;
		
		for (uint8_t i=0; i < 8; i++) {
			tx_message.data[i] = 0;
		}
		
		tx_message.length = 8;
		tx_message.data[0] = val1;
		tx_message.data[1] = val2;
		tx_message.data[2] = 8;
		tx_message.data[3] = 8;
		tx_message.data[4] = 8;
		tx_message.data[5] = 8;
		tx_message.data[6] = 8;
		tx_message.data[7] = 8;
		result = sendCAN(&tx_message);
		
		for (uint8_t i=0; i < 8; i++) {
			tx_message.data[i] = 0;
		}
	}
}