#define F_CPU 16000000UL
#define _PROTECTED_WRITE(register, value)
#define BUFFER_SIZE 512
#define CAN_ID_SNELHEIDSMETER 0x10

#include <avr/io.h>
#include <stdbool.h>
#include "diskio.h"
#include "pff.h"
#include "pffconf.h"
#include "can.h"
#include <string.h>
#include <avr/interrupt.h>

FATFS file_system;

uint8_t write_buffer[BUFFER_SIZE] = {};
uint8_t read_buffer[BUFFER_SIZE]  = {};
UINT    byte_counter              = 0;
int bufferAmt = 0;
int overflowCounter = 0;
bool startWriting = false;
unsigned long curOffset = 0;
unsigned long timer = 0;

void init_timer(void);
void init_sd_card(void);
void fill_buffer(void);
void start_log_message(void);
unsigned long find_offset(void);
void writeToCard(void);

ISR(TIMER2_OVF_vect)
{
	overflowCounter++;
	TCNT2 = 0x00;
}
	
int main(void)
{	
	_PROTECTED_WRITE(CLKPR, ( (1<<CLKPCE) | ((0<<CLKPS3) | (0<<CLKPS2) | (1<<CLKPS1) | (0<<CLKPS0))));
	init_sd_card(); // initialize sd-card
	sei();
	initCAN(); // init can-bus
	listenForMessage(CAN_ID_SNELHEIDSMETER, 8);
	init_timer();
	curOffset = find_offset(); // returnt offset van eerstvolgende lege sector
	start_log_message(); // schrijf start log bericht naar de kaart zodat je kan zien wanneer de auto opnieuw aan is gezet
	while(1)
	{
		if(overflowCounter>=61) // 61 komt van 16.000.000 / 1024 (klokfrequentie / timer2prescaler) =  15625. 15625 / 256 (8 bit timer) = 61
		{
			fill_buffer();
			overflowCounter = 0;
		}
		
		if(startWriting)
		{
			writeToCard(); // schrijf buffer op kaart
			PORTC ^= (1 << PC0); // toggle ter indicatie op oscilloscoop dat 1 sector geschreven is
			curOffset += 512; // zet pointer naar volgende sector
			startWriting = false;
		}
		
	}
	while(1)
	{
		
	}
}

void init_timer(void)
{	
	TCNT2 = 0x00; // start counting at 0
	TCCR2A = ((1 << CS22) | (1 << CS21) | (1 << CS20)); // prescaler op 1024
	TIMSK2 = (1 << TOIE2);
}


void fill_buffer(void)
{
	/*CANMessage bericht;
	int resultaat = getMessage(&bericht); // returnt 0 als het niet lukt om bericht uit te lezen, anders returnt 1
	if(resultaat == 0)
	{
		// geen bericht ontvangen > can-bus werkt niet
		const char* string = "FOUT MET CAN-BUS SYSTEEM";
		int length = strlen(string);
		for (int i = 0; i < length; i++)
		{
			write_buffer[bufferAmt++] = string[i];
		}
	} else {
		// bericht ontvangen > schrijf data naar buffer
		if(bericht.id == CAN_ID_SNELHEIDSMETER)
		{
			const char* string = "snelheid: ";
			int length = strlen(string);
			for (int i = 0; i < length; i++)
			{
				write_buffer[bufferAmt++] = string[i];
			}
			write_buffer[bufferAmt++] = bericht.data[0];
			write_buffer[bufferAmt++] = bericht.data[1];
		}
	
			
		// if statement hierboven kan herhaald worden voor andere ID's.
	}*/
	
	write_buffer[bufferAmt++] = '\n';
	const char* string = "testen: ";
	int length = strlen(string);
	for (int i = 0; i < length; i++)
	{
		write_buffer[bufferAmt++] = string[i];
	}
	char c = 48;
	for (int i = 0; i < 10; i++)
	{
		write_buffer[bufferAmt++] = c++;
	}
	
	if(bufferAmt>=(512-30)) // sectorgrootte van 512 bytes - ongeveer 30 bytes (iets meer dan 2 lines)
	{
			
		bufferAmt = 0;
		startWriting = true; // als dit op true staat gaat de microcontroller in de main>while(logging) in de if-statement en vult vanaf daar de sd-kaart 
	}
}


void writeToCard(void)
{
	// Set file pointer to beginning of sector 
	pf_lseek(curOffset); // offset van 0 is te vinden in sector 8448 van de sd-kaart
	// Write buffer 
	pf_write(write_buffer, BUFFER_SIZE, &byte_counter);
	if (byte_counter < BUFFER_SIZE) {
		// End of file 
	}
	// Finalize write 
	pf_write(0, 0, &byte_counter);
	// Reset file pointer to beginning of sector 1 
	pf_lseek(curOffset);
	// Read back the same bytes 
	pf_read(read_buffer, BUFFER_SIZE, &byte_counter);
	/* de while statement hieronder vergelijkt 1 voor 1 alle bytes die in de write_buffer zitten met de bytes die in de read_buffer zitten
	als iets niet overeen komt, dan blijft hij vast zitten in de while(1).
	in het geval van de huidige code, heb je in de fill_buffer functie de volgende regel: if(bufferAmt>=(512-30))
	dit houdt in dat de code begint met de sector van 512 bytes schrijven voordat je de hele buffer eigenlijk gevuld hebt.
	maar omdat deze petitfat library alleen werkt als je over een bestaand bestand heen schrijft, leest hij de laatste paar bytes uit van wat al in de
	sector stond. stel: na herhaaldelijk de fill_buffer functie uitgevoerd te hebben, heb je 500 bytes in de write_buffer array staan.
	als de code hieronder dan de 512 bytes uit leest van de sector van de sd kaart, komen de 12 laatste bytes niet overeen. daarom gaat het fout.
	omdat het in dit geval niet uit maakt is de code hieronder uitgecommentaard, maar als je deze functie toch wil gebruiken, 
	moet je rekening houden met dit probleem. je kan bijvoorbeeld in de fill_buffer functie, zodra je startWriting = true zet, alsnog de rest van
	de write_buffer vullen met een bepaald karakter, zodat de write_buffer en read_buffer helemaal overeen komen.
	while (byte_counter) 
	{
		if (write_buffer[byte_counter] != read_buffer[byte_counter]) {
			while(1)
			{
				// error
			}
		}
		byte_counter--;
	}
	*/
}

unsigned long find_offset(void)
{
	// begin bij sector 0. kijk naar waarde 1e char. als die erop wijst dat de sector al geschreven is, ga naar sector 1. ga zo door. return offset als sector leeg lijkt te zijn
	uint8_t read_first[1]  = {};
	UINT bytecounter = 1;
	unsigned long tempOffset = 0;
	unsigned long maxOffset = 51200; // als hij geen lege sector kan vinden na de 1e 100 sectoren
	while(1)
	{
		pf_lseek(tempOffset);
		pf_read(read_first, 1, &bytecounter); // read first byte of sector
		if(read_first[0] == 32)
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
	const char* string = "START LOG HIER";
	write_buffer[bufferAmt++] = '\n';
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
	// Initialize physical drive 
	do {
		status = disk_initialize();
		if (status != 0) {
			ERROR = 1; 
		} else {
			ERROR = 0;			
			// Set SPI clock faster after initialization 
			SPCR = (1<<MSTR) | (0<<SPR1) | (0<<SPR0) | (1<<SPE);
			SPSR = (1<<SPI2X); 		
			//SPR1 en 0 op 0 SPI clock set to fck/4 (blaz. 174)
			//MSTR, in mastermode zetten
			//SPE, SPI enable maken.
			//SPI2X, het verdubbelen van de snelheid.			
		}
	} while (ERROR == 1);

	// Mount volume 
	result = pf_mount(&file_system);
	if (result != FR_OK) {} // error

	// Open file 
	result = pf_open("LOG.txt");
	if (result != FR_OK) {} // error
}
