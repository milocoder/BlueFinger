/*
 * SD_kaart4.c
 *
 * Created: 3-5-2023 16:17:55
 * Author : Johan
 */ 

#define F_CPU 16000000UL

#define _PROTECTED_WRITE(register, value)

#include <avr/io.h>
#include <stdbool.h>
#include "diskio.h"
#include "pff.h"
#include "pffconf.h"

#define EXAMPLE_FILENAME "log.txt"

#define MINI_BOARD		false
	

#define LED_OFF()		PORTC &= ~(1 << PC0);	
#define LED_ON()		PORTC |= (1 << PC0);

#define BUFFER_SIZE 10

FATFS file_system;

uint8_t write_buffer[BUFFER_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
uint8_t read_buffer[BUFFER_SIZE]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
UINT    byte_counter              = 0;

void init_sd_card(void);


	
int main(void)
{
	
	
	
	DDRC = 1; // output led (p4)
	PORTC = 0; 
	
	//_PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, (CLKCTRL_PEN_bm | CLKCTRL_PDIV_4X_gc))
	
	_PROTECTED_WRITE(CLKPR, ( (1<<CLKPCE) | ((0<<CLKPS3) | (0<<CLKPS2) | (1<<CLKPS1) | (0<<CLKPS0)))); 

	/* Initialize card */
	init_sd_card();

	/* Set file pointer to beginning of file */
	pf_lseek(0);

	/* Write buffer */
	pf_write(write_buffer, BUFFER_SIZE, &byte_counter);
	if (byte_counter < BUFFER_SIZE) {
		/* End of file */
	}

	/* Finalize write */
	pf_write(0, 0, &byte_counter);

	/* Reset file pointer to beginning of file */
	pf_lseek(0);

	/* Read back the same bytes */
	pf_read(read_buffer, BUFFER_SIZE, &byte_counter);

	/* Check they're the same */
	while (byte_counter) {
		if (write_buffer[byte_counter] != read_buffer[byte_counter]) {
			/* ERROR! */
		//	LED_ON();
			while (1)
				;
		}
		byte_counter--;
	}
	/* SUCCESS! */
	while (1) 
		{
			
		}
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
			
			//LED_ON();
		ERROR = 1; 
		} else {
			//LED_OFF();			
			/* Set SPI clock faster after initialization */
			SPCR = (1<<MSTR) | (0<<SPR1) | (0<<SPR0) | (1<<SPE);
			SPSR = (1<<SPI2X); 			
			//SPR1 en 0 op 0 SPI clock set to fck/4 (blaz. 174)
			//MSTR, in mastermode zetten
			//SPE, SPI enable maken.
			//SPI2X, het verdubbelen van de snelheid.
			
			//hieronder de oude code
			//SPI0.CTRLA = (SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESC_DIV4_gc | SPI_ENABLE_bm);
			
		}
		/* The application will continue to try and initialize the card.
		 * If the LED is on, try taking out the SD card and putting
		 * it back in again.  After an operation has been interrupted this is
		 * sometimes necessary.
		 */
	} while (ERROR == 1);

	/* Mount volume */
	result = pf_mount(&file_system);
	if (result != FR_OK){
		//LED_ON();
	}

	/* Open file */
	result = pf_open(EXAMPLE_FILENAME);
	if (result != FR_OK) {
		//LED_ON();
	}

		
		
}




