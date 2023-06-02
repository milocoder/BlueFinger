/*
 * SD-kaart (SPI).c
 *
 * Created: 30-3-2023 15:17:10
 * Author : johan
 */ 

#include <avr/io.h>
#define DDR_SPI         DDRB
#define PORT_SPI        PORTB
#define CS              PINB0
#define MOSI            PINB2
#define MISO            PINB3
#define SCK             PINB1


void SPI_init(void); 
uint8_t SPI_transfer(uint8_t data)




int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}
void SPI_init()
{
	// set CS, MOSI and SCK to output
	DDR_SPI |= (1 << CS) | (1 << MOSI) | (1 << SCK);

	// enable pull up resistor in MISO
	DDR_SPI |= (1 << MISO);

	// enable SPI, set as master, and clock to fosc/128
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}


void SPI_init(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1<<MOSI)|(1<<SCK);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
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

