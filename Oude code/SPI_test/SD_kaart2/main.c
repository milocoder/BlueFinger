/*
 * SD_kaart2.c
 *
 * Created: 1-5-2023 10:53:27
 * Author : Johan
 */ 
#define F_CPU 16000000UL


#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "spi.h"
//#include "sdcard.h"
//#include "diskio.h"
//#include "pff.h"
//#include "pffconf.h"




int main(void) {
	 SPI_MasterInit();
	 
	 while(1) {
		 SPI_transfer(0x55);
		 _delay_ms(100); 
		 //SPI_transfer(0xFE); 
		 //_delay_ms(500); 
	 }
}

	 
	/* 
	 
	 // array to hold responses
	 uint8_t res[5], buf[512], token;
	 uint32_t addr = 0x00000000;
	 
	 DDRC |= (1 << PC0);     // set pin 3 of Port B as output



	// initialize SPI
	SPI_MasterInit(); 

	// initialize sd card
	if(SD_init() != SD_SUCCESS)
	{
			 PORTC |= (1 << PC0);     // set pin 3 of Port B high
	}
	else
	{
	 
	  // update address to 0x00000100
	  addr = 0x00000100;

	  // fill buffer with 0x55
	  for(uint16_t i = 0; i < 512; i++) buf[i] = 0x55;

	  // write data to sector
	  res[0] = SD_writeSingleBlock(addr, buf, &token);
	}
	*/
 
 

