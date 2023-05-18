
#define F_CPU   (4000000UL)  

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <string.h>
#include "FatFS/ff.h"
#include <avr/interrupt.h>


ISR(TCA0_OVF_vect)
{
	disk_timerproc();	// Drive timer procedure of low level disk I/O module
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}


int main(void)
{
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.PER = 156; // tick every 10 msec = 100 hz: clock = 4000000 / 256 = 15625 hz
	TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL1_bm | TCA_SINGLE_CLKSEL2_bm);
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

	sei();

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
	
	while(1)
	{
	}
}


