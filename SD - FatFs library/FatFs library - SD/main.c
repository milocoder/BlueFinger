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

#include <avr/io.h>
#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <string.h>
#include "ff.h"
#include <avr/interrupt.h>

void init_timer(void); 

ISR(TIMER1_OVF_vect)				//timer 2 ISR
{
	disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
	
	//ook nog omschrijven? Waarvoor nodig?
	//TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}



int main(void)
{
	init_timer();   
  
	FATFS FatFs;

	// init sdcard
	UINT bw;
	f_mount(&FatFs, "", 1);		// Give a work area to the FatFs module
	
	// open file
	FIL *fp = (FIL *)malloc(sizeof (FIL));
	if (f_open(fp, "file.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) // Create a file
	{
		char *text = "Hello World! SDCard up and running!\r\n";
		f_write(fp, text, strlen(text), &bw);	// Write data to the file
		f_close(fp);// Close the file
	}
	
    while (1) 
    {
    }
}



void init_timer(void) {
	//Oude code:
	//TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	//TCA0.SINGLE.PER = 156; // tick every 10 msec = 100 hz: clock = 4000000 / 256 = 15625 hz
	//TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL1_bm | TCA_SINGLE_CLKSEL2_bm);
	//TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
	
	TCCR1A = 0x00; 
	TCCR1B = ((1 << CS02) | (1<<CS00)); //prescaler on 1024, pag. 139 datasheet	
	TCNT1 = 159; //for 10 msec by 16 Mhz, 
	//t = N(per+1)/F_CPU:  1024(1+159)/16000000 = 0.01 seconden (=10 msec)
	TIMSK1 = (1 << TOIE1) ;		// Enable timer 1 overflow interrupt(TOIE2), pag. 142 datasheet
	sei();						// Enable global interrupts by setting global interrupt enable bit in SRE
}

