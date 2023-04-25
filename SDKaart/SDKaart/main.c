/*
Gebruikte handleidingen: 
http://www.rjhcoding.com/avrc-sd-interface-1.php
https://openlabpro.com/guide/interfacing-microcontrollers-with-sd-card/
*/

#include <avr/io.h>
#include <util/delay.h>

#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define CS PB0
#define SCK PB1
#define MOSI PB2
#define MISO PB3

#define CS_ENABLE() PORT_SPI &= ~(1 << CS)
#define CS_DISABLE() PORT_SPI |= (1 << CS)

#define CMD0 0
#define CMD0_ARG 0x00000000
#define CMD0_CRC 0x94

void SPI_init();
uint8_t SPI_transfer(uint8_t data);
void SD_powerUp();
void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc);
uint8_t SD_readRes1();
uint8_t SD_goIdleState();

int main(void)
{
	SPI_init(); // initialiseer SPI
	SD_powerUp(); // start power up sequence
	SD_goIdleState(); // zet sd-kaart in idle stand
	while (1) 
    {
		
    }
}

uint8_t SD_goIdleState()
{
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);
	// verstuurt CMD0
	SD_command(CMD0, CMD0_ARG, CMD0_CRC);
	// lees resultaat
	uint8_t res1 = SD_readRes1();
	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
	
	return res1;
}

uint8_t SD_readRes1()
{
	uint8_t i = 0, res1;
	// blijf checken tot data is ontvangen
	while((res1 = SPI_transfer(0xFF)) == 0xFF)
	{
		i++;
		// als je voor 8 bytes geen data hebt ontvangen, break, dan is er een error
		if(i > 8) break;
	}
	return res1;
}

void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	// verstuurt command
	SPI_transfer(cmd|0x40);
	// verstuurt een 4 byte (32 bit) argument, 1 byte per keer, door telkens 8 bits te verschuiven
	SPI_transfer((uint8_t)(arg >> 24));
	SPI_transfer((uint8_t)(arg >> 16));
	SPI_transfer((uint8_t)(arg >> 8));
	SPI_transfer((uint8_t)(arg));
	// verstuurt crc
	SPI_transfer(crc|0x01);
}

void SD_powerUp()
{
	CS_DISABLE(); // deselecteer sd-kaart
	_delay_ms(1); // wacht op opstarten
	for(uint8_t i = 0; i < 10; i++) // stuur 80 clockcycles om te synchroniseren
	{
		SPI_transfer(0xFF);
	}
	CS_DISABLE(); // deselecteer sd-kaart
	SPI_transfer(0xFF);
}

void SPI_init()
{
	// zet CS, MOSI en SCK als output
	DDR_SPI |= (1 << CS) | (1 << MOSI) | (1 << SCK);

	// enable pull up resistor in MISO
	DDR_SPI |= (1 << MISO);

	// enable SPI, zet as master, and clock to fosc/128
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

uint8_t SPI_transfer(uint8_t data)
{
	SPDR = data;
	
	while(!(SPSR & (1 << SPIF))); // wacht tot transmissie voltooid is
	
	return SPDR;
}