/*
Gebruikte handleidingen: 
http://www.rjhcoding.com/avrc-sd-interface-1.php
https://openlabpro.com/guide/interfacing-microcontrollers-with-sd-card/

DOEN:
Op regel 73 en 77: schrijf code om een lamp uit of aan te zetten. initialiseer deze port/led ook van te voren
*/

#define F_CPU 16000000UL

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

// cmd0 zet in idle state
#define CMD0 0
#define CMD0_ARG 0x00000000
#define CMD0_CRC 0x94
// cmd8 kijk naar sd-kaart versie (1 of 2)
#define CMD8        8
#define CMD8_ARG    0x0000001AA
#define CMD8_CRC    0x86 //(1000011 << 1)
// cmd17 verstuurt een signaal met argument, returnt data die je uitleest. READ
#define CMD17                   17
#define CMD17_CRC               0x00
#define SD_MAX_READ_ATTEMPTS    1563
// acmd41 verstuurt info over de kaart en start het initalisatieproces
#define ACMD41      41
#define ACMD41_ARG  0x40000000
#define ACMD41_CRC  0x00
// cmd55 geef aan dat het hiernavolgende command applicatie-specifiek is
#define CMD55       55
#define CMD55_ARG   0x00000000
#define CMD55_CRC   0x00
// cmd58 leest het Operation Conditions Register - geeft o.a. Card Capacity Status aan
#define CMD58       58
#define CMD58_ARG   0x00000000
#define CMD58_CRC   0x00

#define SD_SUCCESS 0
#define SD_ERROR 1
#define SD_READY 0x00
#define SD_R1_NO_ERROR(X) X < 0x02

void SPI_init();
uint8_t SPI_transfer(uint8_t data);
void SD_powerUpSeq();
void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc);
uint8_t SD_readRes1();
uint8_t SD_goIdleState();
void SD_readRes7(uint8_t *res);
void SD_sendIfCond(uint8_t *res);
uint8_t SD_sendApp();
uint8_t SD_sendOpCond();
void SD_readOCR(uint8_t *res);
void SD_readRes3_7(uint8_t *res);
uint8_t SD_init();
uint8_t SD_readSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token);

int main(void)
{
	uint8_t res[5], sdBuf[512], token;
	SPI_init(); // initialiseer SPI
	if(SD_init() != SD_SUCCESS)
	{
		// lamp indicator dat er een fout is
		while(1);
	} else
	{
		// lamp indicator dat het gelukt is te initialiseren
	}
	// lees 1e blok van de sd-kaart
	res[0] = SD_readSingleBlock(0x00000000, sdBuf, &token);
	
	// doe iets met deze data
	if(SD_R1_NO_ERROR(res[0]) && (token == 0xFE))
	{
		uint8_t eenbyte = sdBuf[4]; // de 5e byte = sdBuf[4] van het 1e blok 0x00000000 wordt in eenbyte gestopt
	} else {
		// error met het uitlezen van de sector
	}
	
	while(1);
	
}

// lees een 512 byte block uit het geheugen
// token = 0xFE - succesvolle performance
// token = 0x0X - data error
// token = 0xFF - timeout
uint8_t SD_readSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token)
{
	uint8_t res1, read;
	uint16_t readAttempts;

	// set token to none
	*token = 0xFF;

	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD17
	SD_command(CMD17, addr, CMD17_CRC);

	// read R1
	res1 = SD_readRes1();

	// if response received from card
	if(res1 != 0xFF)
	{
		// wait for a response token (timeout = 100ms)
		// SD_MAX_READ_ATTEMPTS wordt berekend door te bepalen hoeveel bytes er over SPI
		// verzonden moeten worden om 100ms te wachten
		// vb: spi clock set divided by 128. daarom 128.
		// deze divider kunnen we aanpassen bij SPI_init()
		// 0.1 * 16 000 000 (mhz) / (128 * 8 bytes) = 1563.
		readAttempts = 0;
		while(++readAttempts != SD_MAX_READ_ATTEMPTS) 
			if((read = SPI_transfer(0xFF)) != 0xFF) break;

		// if response token is 0xFE
		if(read == 0xFE)
		{
			// read 512 byte block
			for(uint16_t i = 0; i < 512; i++) *buf++ = SPI_transfer(0xFF);

			// read 16-bit CRC
			SPI_transfer(0xFF);
			SPI_transfer(0xFF);
		}

		// set token to card response
		*token = read;
	}

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}

uint8_t SD_init()
{
	uint8_t res[5], cmdAttempts = 0;
	
	SD_powerUpSeq();
	// zet kaart in idle stand
	while((res[0] = SD_goIdleState()) != 0x01)
	{
		cmdAttempts++;
		if(cmdAttempts > 10) return SD_ERROR; // return error als idle stand niet lukt
	}
	
	// verstuur interface conditie (welke generatie de kaart is)
	SD_sendIfCond(res);
	if(res[0] != 0x01)
	{
		return SD_ERROR; // error als de sd-kaart 1e generatie is ipv Version 2.00
	}
	
	// check echo pattern (geen idee wat dit inhoudt maar heb het maar overgenomen)
	if(res[4] != 0xAA)
	{
		return SD_ERROR;
	}
	
	// sd-kaart initaliseren
	cmdAttempts = 0;
	do 
	{
		if(cmdAttempts > 100) return SD_ERROR;
		
		// stuur commando om aan te geven dat het commando hierna applicatie-specifiek is
		res[0] = SD_sendApp();
		
		// als er geen error is -> verdergaan
		if(res[0] < 2)
		{
			res[0] = SD_sendOpCond();
		}
		
		_delay_ms(10);
		cmdAttempts++;	
	} 
	while (res[0] != SD_READY);
	
	SD_readOCR(res);
	
	if(!(res[1] & 0x80)) return SD_ERROR;
	
	return SD_SUCCESS;
	
}

void SD_readOCR(uint8_t *res)
{
	 // assert chip select
	 SPI_transfer(0xFF);
	 CS_ENABLE();
	 SPI_transfer(0xFF);

	 // send CMD58
	 SD_command(CMD58, CMD58_ARG, CMD58_CRC);

	 // read response
	 SD_readRes3_7(res);

	 // deassert chip select
	 SPI_transfer(0xFF);
	 CS_DISABLE();
	 SPI_transfer(0xFF);
}

void SD_readRes3_7(uint8_t *res)
{
	// read R1
	res[0] = SD_readRes1();

	// if error reading R1, return
	if(res[0] > 1) return;

	// read remaining bytes
	res[1] = SPI_transfer(0xFF);
	res[2] = SPI_transfer(0xFF);
	res[3] = SPI_transfer(0xFF);
	res[4] = SPI_transfer(0xFF);
}

uint8_t SD_sendApp()
{
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);
	
	SD_command(CMD55, CMD55_ARG, CMD55_CRC);
	uint8_t res1 = SD_readRes1();
	
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
	
	return res1;
}

uint8_t SD_sendOpCond()
{
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);
	
	SD_command(ACMD41, ACMD41_ARG, ACMD41_CRC);
	uint8_t res1 = SD_readRes1();
	
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
	
	return res1;
}

void SD_sendIfCond(uint8_t *res)
{
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);
	// send CMD8
	SD_command(CMD8, CMD8_ARG, CMD8_CRC);
	SD_readRes7(res); // read response
	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
}

void SD_readRes7(uint8_t *res)
{
	res[0] = SD_readRes1();
	if(res[0] > 1) return ; // als er een error in R1 is, return
	// lees de resterende bytes
	res[1] = SPI_transfer(0xFF);
	res[2] = SPI_transfer(0xFF);
	res[3] = SPI_transfer(0xFF);
	res[4] = SPI_transfer(0xFF);
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

void SD_powerUpSeq()
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

	// enable SPI, zet als master, and clock to fosc/128
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

uint8_t SPI_transfer(uint8_t data)
{
	SPDR = data;
	
	while(!(SPSR & (1 << SPIF))); // wacht tot transmissie voltooid is
	
	return SPDR;
}