#ifndef __SPI_H__
#define __SPI_H__

// pin definitions
#define DDR_SPI				DDRB
#define PORT_SPI			PORTB
#define CS					PINB0
#define SCK					PINB1
#define MOSI				PINB2
#define MISO				PINB3

// macros
#define CS_ENABLE()         PORT_SPI &= ~(1 << CS)
#define CS_DISABLE()        PORT_SPI |= (1 << CS)

// initialization definitions
#define SPI_MASTER          0b0001000000000000
#define SPI_SLAVE           0b0000000000000000
#define SPI_FOSC_4          0b0000000000000000
#define SPI_FOSC_16         0b0000000100000000
#define SPI_FOSC_64         0b0000001000000000
#define SPI_FOSC_128        0b0000001100000000
#define SPI_2X_FOSC_2       0b0000000000000001
#define SPI_2X_FOSC_8       0b0000000100000001
#define SPI_2X_FOSC_32      0b0000001000000001
#define SPI_2X_FOSC_64      0b0000001100000001
#define SPI_INTERRUPTS      0b1000000000000000
#define SPI_MODE_0          0b0000000000000000
#define SPI_MODE_1          0b0000010000000000
#define SPI_MODE_2          0b0000100000000000
#define SPI_MODE_3          0b0000110000000000
#define SPI_DEFAULT         SPI_MASTER | SPI_FOSC_128 | SPI_MODE_0

// SPI functions
//void SPI_init(uint16_t initParams);
void SPI_MasterInit(void); 
uint8_t SPI_transfer(uint8_t data);

#endif
