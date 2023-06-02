#include <avr/io.h>
#include "spi.h"

/*

void SPI_init(uint16_t initParams)
{
    // set CS, MOSI and SCK to output
    DDR_SPI |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // enable pull up resistor in MISO
    DDR_SPI |= (1 << MISO);

    // set SPI params
    SPCR |= ((uint8_t) (initParams >> 8)) | (1 << SPE);
    SPSR |= ((uint8_t) initParams);
}

uint8_t SPI_transfer(uint8_t data)
{
    // load data into register
    SPDR = data;

    // Wait for transmission complete
    while(!(SPSR & (1 << SPIF)));

    // return SPDR
    return SPDR;
}

*/



void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI |= ((1 << MOSI) | (1 << SCK) | (1 << CS));			
	
	// enable pull up resistor in MISO
	PORTB |= (1 << MISO);
	
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR |= ((1<<SPE) | (1<<MSTR) | (1<<SPR0)); 
	PORTB |= (1<<CS); 				//hoog maken CS
}

uint8_t SPI_transfer(uint8_t data)
{
	PORTB &= ~(1<<CS); //laag maken
	/* Start transmission */
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	PORTB |= (1<<CS);		//hoog maken CS
	
	return SPDR; 
}