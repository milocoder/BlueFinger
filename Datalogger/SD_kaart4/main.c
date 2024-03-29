#define F_CPU 16000000UL
#define _PROTECTED_WRITE(register, value)
#define BUFFER_SIZE 512

//can ID's
#define CAN_ID_SNELHEIDSMETER 0x10
#define CAN_ID_MONITORINGSSYSTEEM 0x123
#define CAN_ID_ACCU_SPANNING   0xFE00
#define CAN_ID_ACCU_STROOM 0xFE01
#define CAN_ID_ACCU_VERMOGEN  0xFE02
#define CAN_ID_ACCU_TEMPERATUUR 0xFE06
//nog can id's van motorcontrollers

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include "diskio.h"
#include "pff.h"
#include "pffconf.h"
#include "can.h"
#include <string.h>
#include <avr/interrupt.h>

FATFS file_system;

uint8_t write_buffer[BUFFER_SIZE] = {};
//uint8_t read_buffer[BUFFER_SIZE]  = {};
UINT    byte_counter              = 0;
int bufferAmt = 0;
int overflowCounter = 0;
bool startWriting = false;
unsigned long curOffset = 0;
unsigned long timer = 0;

// predeclaratie van char-array die gevuld wordt in de switch(case)
char zin_buffer[30]; // 30 is max waarde, dit kan vergroot worden als dat nodig is. in de write_sentence functie geef je als argument mee hoeveel karakters er geschreven moeten worden

void init_timer(void);
void init_sd_card(void);
void fill_buffer(void);
void start_log_message(void);
unsigned long find_offset(void);
void writeToCard(void);
void write_to_buffer(char* zin, int length);

ISR(TIMER2_OVF_vect)
{
	overflowCounter++;
	TCNT2 = 0x00;
}
	
int main(void)
{	
	_PROTECTED_WRITE(CLKPR, ( (1<<CLKPCE) | (1<<CLKPS1)));
	init_sd_card(); // initialize sd-card
	sei();
	initCAN(); // init can-bus
	listenForMessage(CAN_ID_SNELHEIDSMETER, 8);
	//listenForMessage(CAN_ID_MONITORINGSSYSTEEM, 8);
	// voor elke ID die je uit wilt lezen moet je hier een keer de listenForMessage functie aanroepen
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
			PORTC ^= (1 << PC0); // toggle ter indicatie op oscilloscoop
			writeToCard(); // schrijf buffer op kaart
			curOffset += 512; // zet pointer naar volgende sector
			startWriting = false;
		}
		
	}
	
	while(1) {}
}

void init_timer(void)
{	
	TCNT2 = 0x00; // start counting at 0
	TCCR2A = ((1 << CS22) | (1 << CS21) | (1 << CS20)); // prescaler op 1024
	TIMSK2 = (1 << TOIE2);
}

void write_to_buffer(char* zin, int length)
{
	for (int i = 0; i < length; i++)
	{
		write_buffer[bufferAmt++] = zin[i];
	}
}


void fill_buffer(void)
{
	// predeclaratie van variabelen omdat dat in de switch(case) niet kan
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	int val4 = 0;
	int val5 = 0;
	int val6 = 0;
	int val7 = 0;
	int val8 = 0;
	
	write_buffer[bufferAmt++] = '\n';
	
	CANMessage bericht;
	int resultaat;
	do 
	{
		resultaat = getMessage(&bericht); // 0 als er geen bericht is, anders 1
		if(resultaat == 0)
		{
			char* string = "GEEN DATA ONTVANGEN";
			int length = strlen(string);
			write_to_buffer(string, length);
		} else
		{
			switch(bericht.id)
			{
				case CAN_ID_SNELHEIDSMETER:
					val1 = bericht.data[0];
					val2 = bericht.data[1];
					sprintf(zin_buffer, "snelheid: %03d.%02d", val1, val2);
					write_to_buffer(zin_buffer, 16);
					break;
				case CAN_ID_ACCU_SPANNING:
					val1 = bericht.data[0];
					val2 = bericht.data[1];
					sprintf(zin_buffer, "accuspanning: %02d.%02d", val1, val2);
					write_to_buffer(zin_buffer, 19);
					break;
				// schrijf een case voor alle can_id's
			}
		}
	} while (resultaat != 0);
	
	
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
	// pf_read(read_buffer, BUFFER_SIZE, &byte_counter);
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
			SPCR = (1<<MSTR) | (1<<SPE);
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
