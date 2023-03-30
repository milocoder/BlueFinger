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


void SPI_Init(void); 
void SPI_Transmit(char cData)



int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}


void SPI_Init(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1<<MOSI)|(1<<SCK);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}


void SPI_Transmit(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
}
