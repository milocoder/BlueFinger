#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>

void writeFloatToEEPROM(float value, int address);

int main(void)
{
	float speed1 = 12.78;
	float speed2 = 23.65;
	int address = 1;
	writeFloatToEEPROM(speed1, address);
	address += 3;
	writeFloatToEEPROM(speed2, address);
	// eeprom_write_byte((uint8_t*)12, 255);
}

void writeFloatToEEPROM(float value, int address)
{
	int val1 = (int)value; // pak getal voor de komma
	int val2 = (int)((value-val1)*100)+1; // pak kommagetal en doe keer 100
	eeprom_write_byte((uint8_t*)address, val1);
	eeprom_write_byte((uint8_t*)address+1, val2);
	
}