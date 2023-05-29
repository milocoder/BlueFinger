#define F_CPU 16000000UL

#define _PROTECTED_WRITE(register, value)

#include <avr/io.h>
#include <stdbool.h>
#include "diskio.h"
#include "pff.h"
#include "pffconf.h"

#define LED_OFF()		PORTC = (0 << PC0);
#define LED_ON()		PORTC = (1 << PC0);

#define BUFFER_SIZE 10

FATFS file_system;

uint8_t write_buffer[BUFFER_SIZE] = {};
uint8_t read_buffer[BUFFER_SIZE]  = {};
UINT    byte_counter              = 0;

void init_sd_card(void);
void fill_buffer(void);
	
int main(void)
{	
	LED_OFF();
	_PROTECTED_WRITE(CLKPR, ( (1<<CLKPCE) | ((0<<CLKPS3) | (0<<CLKPS2) | (1<<CLKPS1) | (0<<CLKPS0)))); 

	init_sd_card(); // initialize sd-card
	
	fill_buffer(); // fill buffer

	/* Set file pointer to beginning of sector */
	pf_lseek(0); // offset = 000420000 in sector 8.448

	/* Write buffer */
	pf_write(write_buffer, BUFFER_SIZE, &byte_counter);
	
	if (byte_counter < BUFFER_SIZE) {
		/* End of file */
	}

	/* Finalize write */
	pf_write(0, 0, &byte_counter);

	/* Reset file pointer to beginning of sector 1 */
	pf_lseek(0);

	/* Read back the same bytes */
	pf_read(read_buffer, BUFFER_SIZE, &byte_counter);

	/* Check they're the same */
	while (byte_counter) 
	{
		if (write_buffer[byte_counter] != read_buffer[byte_counter]) {
			while(1)
			{
				LED_ON(); // Error
			}
		}
		byte_counter--;
	}
	/* SUCCESS! */
	while (1) 
	{
		PORTC = (1 << PC0);
		_delay_ms(1500);
		PORTC = (0 << PC0);
		_delay_ms(1500);
	}
}

void fill_buffer(void)
{
	int grootte = 10;
	for(int i = 0; i < grootte; i++)
	{
		write_buffer[i] = '5';
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
			ERROR = 1; 
		} else {
			LED_OFF();
			ERROR = 0;			
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
	} while (ERROR == 1);

	/* Mount volume */
	result = pf_mount(&file_system);
	if (result != FR_OK){ // 0
		
	}

	/* Open file */
	result = pf_open("LOG.txt");
	if (result != FR_OK)
	{
		// error
	}
	
	
}