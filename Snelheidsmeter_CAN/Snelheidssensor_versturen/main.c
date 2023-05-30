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
	init();
	
	while(1) {
		float snelheidKmh = (float)12.22;
		verstuurCan(snelheidKmh);
		_delay_ms(3000);
	}
}

void verstuurCan(float snelheid)
{
	// volgende stuk initaliseert de canmessage struct, verstuurt snelheid met data[0] en data[1]
	int voorKomma = (int)snelheid; // pak getal voor de komma
	int naKomma = (int)((snelheid-voorKomma)*100); // pak kommagetal en vermenigvuldig keer 100
	
	
	CANMessage tx_message;
	tx_message.id = 0x05; // can ID
	tx_message.length = 8;
	tx_message.data[0] = voorKomma;
	tx_message.data[1] = naKomma;
	for(int i = 2; i < 8; i++)
		tx_message.data[i] = 0;
	sendCAN(&tx_message);	
}

void init()
{
	init_millis(F_CPU);
	sei();  // Zet interrupts aan
	initCAN();	
}