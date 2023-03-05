#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "can.h"

int main(void)
{
	/*
	DDRC = 1; // 1 geeft aan dat alle pinnen van port c een output zijn. met 0 geef je aan dat de pinnen input zijn
	while (1)
	{
		PORTC = 1;
		_delay_ms(25);
		PORTC = 0;
		_delay_ms(25);	
	}
	
	*/
	DDRC = 1;
	
	
	
}