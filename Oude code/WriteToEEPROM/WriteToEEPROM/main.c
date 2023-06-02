#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>

void writeFloatToEEPROM(float value, int address);

int main(void)
{
	for (int i = 0; i<2000; i++)
	{
		eeprom_write_byte((uint8_t*)i, 0xFF);
	}
}
