/*
 * SPI_test.c
 *
 * Created: 15-5-2023 16:05:09
 * Author : Johan
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

/*
#define DDR_SPI				DDRB
#define SPI_PORT			PORTB
#define CS					0b00000001
#define SCK					0b00000010
#define MOSI				0b00000100
#define MISO				0b00001000
*/

static void init_spi(void);  
char transmit(char a);


int main(void)
{
	init_spi(); 
    /* Replace with your application code */
    while (1) 
    {
		transmit(0x87); 
		_delay_ms(500); 
		transmit(0x50);	
		_delay_ms(500); 	
    }
}


static void init_spi(void)
{
	
	/* SPI pins */
	//PORTMUX.CTRLB    = PORTMUX_SPI0_ALTERNATE_gc;   /* Alternative comms location for SPI */
	//SPIPORT.DIRSET   = SPI_MOSI | SPI_CS | SPI_SCK; /* Set outputs */
	//SPIPORT.PIN1CTRL = PORT_PULLUPEN_bm;            /* Pull up on SPI_MISO (SD card DO) */

	/* Customize SPI prescaler to give 100-400kHz clock */
	//SPI0.CTRLA = SPI_MASTER_bm | SPI_PRESC_DIV16_gc; /* With 5MHz F_CPU */
	/* Slave select pin controlled in software */
	//SPI0.CTRLB = SPI_SSD_bm;														nog even vragen wat dit betekent en of nodig is voor onze toepassing
	/* SPI Enable */
	//SPI0.CTRLA |= SPI_ENABLE_bm;
	
	//enable uit voor instellen
	SPCR = (0<<SPE); 
	
	
	/* Set MOSI, SCK and CS output, all others input */
	DDRB |= 0b00000111; 
	//DDR_SPI = (1 << MOSI) | (1 << SCK) | (1 << CS);			//geen cs - dan in IDLE stand?
	
	// enable pull up resistor in MISO
	//DDR_SPI |= (1 << MISO);
	PORTB |= 0b00001000; 
	
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<MSTR) | (1<<SPR0) | (1<<SPIE); 
	SPCR = (1<<SPE); 
}

char transmit(char a)
{
	/* Start transmission */
	SPDR = a;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	
	return SPDR;
}



