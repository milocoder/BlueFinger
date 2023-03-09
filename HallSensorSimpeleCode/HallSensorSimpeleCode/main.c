#include <avr/io.h>
#define hallIn PC0

int main(void)
{
	DDRF = 0xFF; // configure PORTF as output
	DDRC = 0x00; // configure PORTB as input
	
	while (1)
	{
		if (~PINC & (1 << hallIn))
		{
			PORTF = 1; // turn on every other light on PORTF, inverse logic
			} else {
			PORTF = 0; // default case, all lights off on PORTA, inverse logic
		}
	}
}