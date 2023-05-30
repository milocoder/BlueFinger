#define F_CPU 16000000UL

#define _PROTECTED_WRITE(register, value)

#include <avr/io.h>
#include <stdbool.h>
#include "diskio.h"
#include "pff.h"
#include "pffconf.h"
#include "millis.h"
#include <string.h>
#include <avr/interrupt.h>

#define BUFFER_SIZE 512

FATFS file_system;

uint8_t write_buffer[BUFFER_SIZE] = {};
uint8_t read_buffer[BUFFER_SIZE]  = {};
UINT    byte_counter              = 0;
int bufferAmt = 0;
bool startWriting = false;
unsigned long curOffset = 0;
unsigned long timer = 0;

void init_sd_card(void);
void fill_buffer(void);
void start_log_message(void);
unsigned long find_offset(void);
void writeToCard(void);
char* getCanInfo(void);
	
int main(void)
{	
	_PROTECTED_WRITE(CLKPR, ( (1<<CLKPCE) | ((0<<CLKPS3) | (0<<CLKPS2) | (1<<CLKPS1) | (0<<CLKPS0))));
	init_sd_card(); // initialize sd-card
	init_millis(F_CPU); // init timer 
	sei();
	curOffset = find_offset(); // returnt offset van lege sector
	start_log_message(); // schrijf start log bericht naar de kaart zodat je kan zien wanneer de auto opnieuw aan is gezet
	while(1)
	{
		/*if(millis() - timer >= 1000)
		{ 	
			fill_buffer(); // 1 keer per seconde
			timer = millis();
		}*/
		fill_buffer();
		
		if(startWriting)
		{
			writeToCard(); // schrijf buffer op kaart
		}
		
	}

}

char* getCanInfo(void)
{
	// bla bla can uitlezen en in een double stoppen en dat returnen
	static char info[2];
	return info;
}

void fill_buffer(void)
{
	//char* info = getCanInfo(); // haal can_bus info op en stop die waarden in de char array
	const char* string = "snelheid: ";
	int length = strlen(string);
	for (int i = 0; i < length; i++)
	{
		write_buffer[bufferAmt++] = string[i];
	}
	//write_buffer[bufferAmt++] = info[0];
	//write_buffer[bufferAmt++] = info[1];
	char c = 48;
	for (int i = 0; i < 10; i++)
	{
		write_buffer[bufferAmt++] = c++;
	}
	write_buffer[bufferAmt++] = '\n';
	
	if(bufferAmt>=(512-30)) // sectorgrootte van 512 bytes - ongeveer 30 bytes (iets meer dan 2 lines)
	{
		bufferAmt = 0;
		startWriting = true; // als dit op true staat gaat de microcontroller in de main>while(logging) in de if-statement en vult vanaf daar de sd-kaart 
	}
}


void writeToCard(void)
{
	/* Set file pointer to beginning of sector */
	pf_lseek(curOffset); // offset van 0 is te vinden in sector 8.448
	/* Write buffer */
	pf_write(write_buffer, BUFFER_SIZE, &byte_counter);
	if (byte_counter < BUFFER_SIZE) {
		/* End of file */
	}
	/* Finalize write */
	pf_write(0, 0, &byte_counter);
	/* Reset file pointer to beginning of sector 1 */
	pf_lseek(curOffset);
	/* Read back the same bytes */
	pf_read(read_buffer, BUFFER_SIZE, &byte_counter);
	/* Check they're the same */
	while (byte_counter)
	{
		if (write_buffer[byte_counter] != read_buffer[byte_counter]) {
			while(1)
			{
				// Error
			}
		}
		byte_counter--;
	}
	curOffset = curOffset + 512; // zet pointer naar volgende sector
}

unsigned long find_offset(void)
{
	// begin bij sector 0. kijk naar waarde 1e char. als die erop wijst dat de sector al geschreven is, ga naar sector 1. ga zo door. return offset als sector leeg lijkt te zijn
	uint8_t read_first[1]  = {};
	UINT bytecounter = 1;
	unsigned long tempOffset = 0;
	unsigned long maxOffset = 512000; // als hij geen lege sector kan vinden na de 1e 1000 sectoren
	while(1)
	{
		pf_lseek(tempOffset);
		pf_read(read_first, 1, &bytecounter); // read first byte of sector
		if(read_first[0] == 20)
		{
			return tempOffset;
		} else {
			tempOffset += 512;
			if(tempOffset >= maxOffset)
				break;
		}
	}
	return 0;
}

void start_log_message(void)
{
	const char* string = "START LOG HERE";
	int length = strlen(string);
	for (int i = 0; i < length; i++)
	{
		write_buffer[bufferAmt++] = string[i];
	}
	write_buffer[bufferAmt++] = '\n';
}

void init_sd_card(void)
{
	DSTATUS status;
	FRESULT result;
	int ERROR = 0;
	/* Initialize physical drive */
	do {
		status = disk_initialize();
		if (status != 0) {
			ERROR = 1; 
		} else {
			ERROR = 0;			
			/* Set SPI clock faster after initialization */
			SPCR = (1<<MSTR) | (0<<SPR1) | (0<<SPR0) | (1<<SPE);
			SPSR = (1<<SPI2X); 		
			//SPR1 en 0 op 0 SPI clock set to fck/4 (blaz. 174)
			//MSTR, in mastermode zetten
			//SPE, SPI enable maken.
			//SPI2X, het verdubbelen van de snelheid.			
		}
	} while (ERROR == 1);

	/* Mount volume */
	result = pf_mount(&file_system);
	if (result != FR_OK) {} // error

	/* Open file */
	result = pf_open("LOG.txt");
	if (result != FR_OK) {} // error
}