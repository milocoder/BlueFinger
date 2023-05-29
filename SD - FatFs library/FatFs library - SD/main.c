/*
 * FatFs library - SD.c
 *
 * Created: 19-5-2023 11:39:46
 * Author : Johan Hogendoorn
 
 Bronnen:
 - Datasheet AT90CAN
 - http://exploreembedded.com/wiki/AVR_Timer_Interrupts
 - https://onlinedocs.microchip.com/pr/GUID-93DE33AC-A8E1-4DD9-BDA3-C76C7CB80969-en-US-2/index.html?GUID-0BED8014-6B01-47DD-A495-68F9F70287A3
 */ 

#define F_CPU 1600000UL
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <string.h>
#include "ff.h"
#include <avr/interrupt.h>

void init_timer(void); 

ISR(TIMER1_OVF_vect)
{
	//PORTC ^= (1 << PC0);
	TCNT1 = 0xFF61;
	disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
}

void delayByEnumValue(FRESULT result) {
	uint16_t delay_ms = 400;

	switch (result) {
		case FR_OK:
		delay_ms = 400;
		break;
		case FR_DISK_ERR:
		delay_ms = 450;
		break;
		case FR_INT_ERR:
		delay_ms = 500;
		break;
		case FR_NOT_READY:
		delay_ms = 550;
		break;
		case FR_NO_FILE:
		delay_ms = 600;
		break;
		case FR_NO_PATH:
		delay_ms = 650;
		break;
		case FR_INVALID_NAME:
		delay_ms = 700;
		break;
		case FR_DENIED:
		delay_ms = 750;
		break;
		case FR_EXIST:
		delay_ms = 800;
		break;
		case FR_INVALID_OBJECT:
		delay_ms = 850;
		break;
		case FR_WRITE_PROTECTED:
		delay_ms = 900;
		break;
		case FR_INVALID_DRIVE:
		delay_ms = 950;
		break;
		case FR_NOT_ENABLED:
		delay_ms = 1000;
		break;
		case FR_NO_FILESYSTEM_a:
		delay_ms = 1050;
		break;
		case FR_MKFS_ABORTED:
		delay_ms = 1100;
		break;
		case FR_TIMEOUT:
		delay_ms = 1150;
		break;
		case FR_LOCKED:
		delay_ms = 1200;
		break;
		case FR_NOT_ENOUGH_CORE:
		delay_ms = 1250;
		break;
		case FR_TOO_MANY_OPEN_FILES:
		delay_ms = 1300;
		break;
		case FR_INVALID_PARAMETER:
		delay_ms = 1350;
		break;
	}

	while (1) {
		PORTC ^= (1 << PC0);
		_delay_ms(delay_ms);
	}
}

int main(void)
{
	init_timer();  
	
	FATFS FatFs;

	// init sdcard
	UINT bw;
	FRESULT res = f_mount(&FatFs, "", 1);		// Give a work area to the FatFs module
	
	

	if(res == FR_NO_FILESYSTEM_a)
	{
		while(1)
		{
			PORTC ^= (1 << PC0);
			_delay_ms(250);
		}
	}
	
	
	// open file
	FIL *fp = (FIL *)malloc(sizeof (FIL));
	if (f_open(fp, "LOG.TXT", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) // Create a file
	{
		char *text = "Hello World! SDCard up and running!\r\n";
		f_write(fp, text, strlen(text), &bw);	// Write data to the file
		f_close(fp);// Close the file
	}
	
	
	while(1)
	{
		
	}
	
}

void init_timer(void) {	
	CLKPR = 0x80; CLKPR = 0x00; // reset prescaler
	TCNT1 = 0xFF61; // start counting at (2^16 - 159 = 65377 = 0xFF61)
	TCCR1A = 0x00;
	TCCR1B = (( 1 << CS12) | ( 1 << CS10)); // prescaler op 1024
	TIMSK1 = (1 << TOIE1) ;		// Enable timer 1 overflow interrupt(TOIE2), pag. 142 datasheet
	sei();					// Enable global interrupts by setting global interrupt enable bit in SRE
}