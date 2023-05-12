
#ifndef SD_H_
#define SD_H_

/*************************************************************
	INCLUDES
**************************************************************/
#include <avr/io.h>
#include <util/delay.h>

/*************************************************************
	USER DEFINED SETTINGS
**************************************************************/
// SPI0 I/O pins (MOSI and SCK)
#define SPI0_PORT								PORTB // SPI PORT
#define SPI0_DDR								DDRB  // SPI DDR
#define SPI0_PINS								PINB  // Holds the state of each pin on port where SPI pins are

#define SPI0_MOSI_PIN							PB5   // SDA pin
#define SPI0_SCK_PIN							PB7   // SCK pin

// SPI Chip Select pin
#define CARD_CS_DDR								DDRB
#define CARD_CS_PORT							PORTB
#define CARD_CS_PIN								PB4

// Card detect pin for card sockets with this feature (optional)
#define CARD_DETECT_DDR							DDRB
#define CARD_DETECT_PORT						PORTB
#define CARD_DETECT_PIN							PB1
#define CARD_DETECT_PINS						PINB

#define SPI0_CLK_DIV							2	// smallest SPI prescaler gives highest speed

/*************************************************************
	SYSTEM DEFINES
**************************************************************/
// Block length can be set in Standard Capacity SD cards using CMD16 (SET_BLOCKLEN).
// For SDHC and SDXC cards the block length is always set to 512 bytes.
#define SD_BUFFER_SIZE							512 // CAUTION: only 512 bytes/block is implemented

// Some bits for SPI registers
#define SPIF0   7
#define SPIE0   7
#define SPR00   0
#define SPR01   1
#define MSTR0   4
#define SPE0    6

// R1 Response
#define PARAM_ERROR(X)							X & 0b01000000
#define ADDR_ERROR(X)							X & 0b00100000
#define ERASE_SEQ_ERROR(X)						X & 0b00010000
#define CRC_ERROR(X)							X & 0b00001000
#define ILLEGAL_CMD(X)							X & 0b00000100
#define ERASE_RESET(X)							X & 0b00000010
#define IN_IDLE(X)								X & 0b00000001

// R3 Response
#define POWER_UP_STATUS(X)						X & 0x80
#define CCS_VAL(X)								X & 0x40	// Card Capacity Status
#define VDD_2728(X)								X & 0b10000000
#define VDD_2829(X)								X & 0b00000001
#define VDD_2930(X)								X & 0b00000010
#define VDD_3031(X)								X & 0b00000100
#define VDD_3132(X)								X & 0b00001000
#define VDD_3233(X)								X & 0b00010000
#define VDD_3334(X)								X & 0b00100000
#define VDD_3435(X)								X & 0b01000000
#define VDD_3536(X)								X & 0b10000000

// R7 Response
#define CMD_VER(X)								((X >> 4) & 0xF0) // command version
#define VOL_ACC(X)								(X & 0x1F)	// voltage accepted
#define VOLTAGE_ACC_27_33						0b00000001	// 2.7-3.6V
#define VOLTAGE_ACC_LOW							0b00000010	// low voltage
#define VOLTAGE_ACC_RES1						0b00000100	// reserved
#define VOLTAGE_ACC_RES2						0b00001000	// reserved

// Data Error Token
#define SD_TOKEN_OOR(X)							X & 0b00001000	// Data Out Of Range
#define SD_TOKEN_CECC(X)						X & 0b00000100	// Card ECC Failed
#define SD_TOKEN_CC(X)							X & 0b00000010	// CC Error
#define SD_TOKEN_ERROR(X)						X & 0b00000001

/* Command List */
// CMD0 - GO_IDLE_STATE
#define CMD0					0
#define CMD0_ARG				0x00000000
#define CMD0_CRC				0x94

// CMD8 - SEND_IF_COND (Send Interface Condition)
#define CMD8					8
#define CMD8_ARG				0x0000001AA
#define CMD8_CRC				0x86 // (1000011 << 1)

// CMD55 - APP_CMD
#define CMD55					55
#define CMD55_ARG				0x00000000
#define CMD55_CRC				0x00

// ACMD41 - SD_SEND_OP_COND (Send Operating Condition)
// ACMD stands for Application Specific Command and before issued, the CMD55 must be sent first
#define ACMD41					41
#define ACMD41_ARG				0x40000000
#define ACMD41_CRC				0x00

// CMD58 - read OCR (Operation Conditions Register)
#define CMD58					58
#define CMD58_ARG				0x00000000
#define CMD58_CRC				0x00

// CMD17 - READ_SINGLE_BLOCK
// For a 16MHz oscillator and SPI clock set to divide by 2. 
// Thus, to get the number of bytes we need to send over SPI to reach 100ms, we do 
// (0.1s * 16000000 MHz) / (2 * 8 bytes) = 100000
#define CMD17                   17
#define CMD17_CRC               0x00
#define SD_MAX_READ_ATTEMPTS    (0.1 * F_CPU) / (SPI0_CLK_DIV * 8)

// CMD24 - WRITE_BLOCK
// For a 16MHz oscillator and SPI clock set to divide by 2.
// Thus, to get the number of bytes we need to send over SPI to reach 250ms, we do
// (0.25s * 16000000 MHz) / (2 * 8 bytes) = 250000
#define CMD24                   24
#define CMD24_CRC               0x00
#define SD_MAX_WRITE_ATTEMPTS   (0.25 * F_CPU) / (SPI0_CLK_DIV * 8)

// Card Type
#define SD_V1_SDSC				1
#define SD_V2_SDSC				2
#define SD_V2_SDHC_SDXC			3

/*************************************************************
	MACRO FUNCTIONS
**************************************************************/
#define SPI_CARD_CS_DISABLE()			CARD_CS_PORT |= (1 << CARD_CS_PIN)
#define SPI_CARD_CS_ENABLE()			CARD_CS_PORT &= ~(1 << CARD_CS_PIN)

/*************************************************************
	GLOBALS
**************************************************************/
uint8_t SD_Buffer[SD_BUFFER_SIZE + 1]; // reserve 1 byte for the null
uint8_t SD_ResponseToken;
// Sectors below this value will not be written by the write functions.
// Protected sectors could include Boot Sectors, FATs, Root Directory.
//uint16_t SD_MaxProtectedSector;

static uint8_t SD_CardType;

/* Card initialization return codes (SD_RETURN_CODES) */
typedef enum {
	SD_OK,
	SD_IDLE_STATE_TIMEOUT,
	SD_GENERAL_ERROR,
	SD_CHECK_PATTERN_MISMATCH,
	SD_NONCOMPATIBLE_VOLTAGE_RANGE,
	SD_POWER_UP_BIT_NOT_SET,
	SD_NOT_SD_CARD,
	//SD_OP_COND_TIMEOUT,
	//SD_SET_BLOCKLEN_TIMEOUT,
	//SD_WRITE_BLOCK_TIMEOUT,
	//SD_WRITE_BLOCK_FAIL,
	//SD_READ_BLOCK_TIMEOUT,
	//SD_READ_BLOCK_DATA_TOKEN_MISSING,
	//SD_DATA_TOKEN_TIMEOUT,
	//SD_SELECT_CARD_TIMEOUT,
	//SD_SET_RELATIVE_ADDR_TIMEOUT
} SD_RETURN_CODES;


/*************************************************************
	FUNCTION PROTOTYPES
**************************************************************/
SD_RETURN_CODES sd_init(void);
uint8_t sd_write_single_block(uint32_t addr, uint8_t *buf);
uint8_t sd_read_single_block(uint32_t addr, uint8_t *buf);

static void sd_assert_cs(void);
static void sd_deassert_cs(void);
static SD_RETURN_CODES sd_command_ACMD41(void);
static uint8_t sd_read_response1(void);
static void sd_read_response3_7(uint8_t *res);
static void sd_command(uint8_t cmd, uint32_t arg, uint8_t crc);

static void SPI_Init(void);
void SPI_Send(uint8_t *buf, uint16_t length);
static void SPI_SendByte(uint8_t byte);
void SPI_Receive(uint8_t *buf, uint16_t length);
static uint8_t SPI_ReceiveByte(void);


/*************************************************************
	FUNCTIONS
**************************************************************/
/*______________________________________________________________________________________________
	Initialize the SD card into SPI mode
	returns 0 on success or error code
________________________________________________________________________________________________*/
SD_RETURN_CODES sd_init(void){
	uint8_t SD_Response[5]; // array to hold response
	uint8_t cmdAttempts = 0;
	
	// SPI Setup
	SPI_Init();
		
	// Optional - Card detect pin set as input high
	CARD_DETECT_PORT |= (1 << CARD_DETECT_PIN);
	CARD_DETECT_DDR &= ~(1 << CARD_DETECT_PIN);
	
	// Give SD card time to power up
	_delay_ms(1);
	
	// Send 80 clock cycles to synchronize
	// The card must be deselected during this time
	sd_deassert_cs();
	for(uint8_t i = 0; i < 10; i++) SPI_SendByte(0xFF);
	
	// Card powers up in SD Bus protocol mode and switch to SPI 
	// when the Chip Select line is driven low and CMD0 is sent.
	// In SPI mode CRCs are ignored but because they start in SD Bus mode
	// a correct checksum must be provided.
	
	// Send CMD0 (GO_IDLE_STATE) - R1 response
	// Resets the SD Memory Card
	// Argument is 0x00 for the reset command, precalculated checksum
	while(SD_Response[0] != 0x01){
		// Assert chip select
		sd_assert_cs();
		
		// Send CMD0
		sd_command(CMD0, CMD0_ARG, CMD0_CRC);
		
		// Read R1 response
		SD_Response[0] = sd_read_response1();
		
		// Send some dummy clocks after GO_IDLE_STATE
		// Deassert chip select
		sd_deassert_cs();
		
		cmdAttempts++;
		
		if(cmdAttempts > 10){
			SPI_CARD_CS_DISABLE();
			return SD_IDLE_STATE_TIMEOUT;
		}
	}

	
	// Send CMD8 - SEND_IF_COND (Send Interface Condition) - R7 response (or R1 for < V2 cards)
	// Sends SD Memory Card interface condition that includes host supply voltage information and asks the
	// accessed card whether card can operate in supplied voltage range.
	// Check whether the card is first generation or Version 2.00 (or later).
	// If the card is of first generation, it will respond with R1 with bit 2 set (illegal command)
	// otherwise the response will be 5 bytes long R7 type
	// Voltage Supplied (VHS) argument is set to 3.3V (0b0001)
	// Check Pattern argument is the recommended pattern '0b10101010'
	// CRC is 0b1000011 and is precalculated
	sd_assert_cs();
	sd_command(CMD8, CMD8_ARG, CMD8_CRC);
	sd_read_response3_7(SD_Response);
	sd_deassert_cs();
	
	// Enable maximum SPI speed
	SPCR0 &= ~(1 << SPR01); // set clock rate fosc/2
	
	// Select initialization sequence path
	if(SD_Response[0] == 0x01){
		// The card is Version 2.00 (or later) or SD memory card
		// Check voltage range
		if(SD_Response[3] != 0x01){
			return SD_NONCOMPATIBLE_VOLTAGE_RANGE;
		}
		
		// Check echo pattern
		if(SD_Response[4] != 0xAA){
			return SD_CHECK_PATTERN_MISMATCH;
		}
		
		// CMD58 - read OCR (Operation Conditions Register) - R3 response
		// Reads the OCR register of a card
		sd_assert_cs();
		sd_command(CMD58, CMD58_ARG, CMD58_CRC);
		sd_read_response3_7(SD_Response);
		sd_deassert_cs();
		
		// ACMD41 - starts the initialization process - R1 response
		// Continue to send ACMD41 (always preceded by CMD55) until the card responds 
		// with 'in_idle_state', which is R1 = 0x00.
		if(sd_command_ACMD41() > 0) return SD_IDLE_STATE_TIMEOUT;
		
		// CMD58 - read OCR (Operation Conditions Register) - R3 response
		sd_assert_cs();
		sd_command(CMD58, CMD58_ARG, CMD58_CRC);
		sd_read_response3_7(SD_Response);
		sd_deassert_cs();
		
		// Check if the card is ready
		// Read bit OCR 31 in R3
		if(!(SD_Response[1] & 0x80)){
			return SD_POWER_UP_BIT_NOT_SET;
		}
		
		// Read CCS bit OCR 30 in R3
		if(SD_Response[1] & 0x40){
			// SDXC or SDHC card
			SD_CardType = SD_V2_SDHC_SDXC;
		}else{
			// SDSC
			SD_CardType = SD_V2_SDSC;
		}
		
	}else if(SD_Response[0] == 0x05){
		// Response code 0x05 = Idle State + Illegal Command indicates
		// the card is of first generation. SD or MMC card.
		SD_CardType = SD_V1_SDSC;
		
		// ACMD41
		SD_Response[0] = sd_command_ACMD41();
		if(ILLEGAL_CMD(SD_Response[0])) return SD_NOT_SD_CARD;
		if(SD_Response[0]) return SD_IDLE_STATE_TIMEOUT;
		
	}else{
		return SD_GENERAL_ERROR;
	}
	
	
	// Read and write operations are performed on SD cards in blocks of set lengths. 
	// Block length can be set in Standard Capacity SD cards using CMD16 (SET_BLOCKLEN). 
	// For SDHC and SDXC cards the block length is always set to 512 bytes.
	
	return SD_OK;
}



/*______________________________________________________________________________________________
	Write a single block of data
				
	addr	32-bit address. SDHC and SDXC are block addressed, meaning an address of 0 
			will read back bytes 0-511, address 1 will read back 512-1023, etc.
			With 32 bits to specify a 512 byte block, the maximum size a card can support is 
			(2^32)*512 = 2048 megabytes or 2 terabytes.
					
	buff	512 bytes of data to write
	
	token	0x00 - busy timeout
			0x05 - data accepted
			0xFF - response timeout
_______________________________________________________________________________________________*/
uint8_t sd_write_single_block(uint32_t addr, uint8_t *buf){
	uint8_t res1;
	uint32_t readAttempts;
	
	if(SD_CardType == SD_V1_SDSC) addr *= 512;

	// set token to none
	SD_ResponseToken = 0xFF;

	sd_assert_cs();
	sd_command(CMD24, addr, CMD24_CRC);

	// read response
	res1 = sd_read_response1();

	// if no error
	if(res1 == 0){
		// send start token
		SPI_SendByte(0xFE);

		// write buffer to card
		for(uint16_t i = 0; i < SD_BUFFER_SIZE; i++) SPI_SendByte(buf[i]);

		// wait for a response (timeout = 250ms)
		// maximum timeout is defined as 250 ms for all write operations
		readAttempts = 0;
		while(++readAttempts < SD_MAX_WRITE_ATTEMPTS){
			if((res1 = SPI_ReceiveByte()) != 0xFF) break;
		}

		// if data accepted
		if((res1 & 0x1F) == 0x05){
			// set token to data accepted
			SD_ResponseToken = 0x05;

			// wait for write to finish (timeout = 250ms)
			readAttempts = 0;
			while(SPI_ReceiveByte() == 0x00){
				if(++readAttempts > SD_MAX_WRITE_ATTEMPTS){
					SD_ResponseToken = 0x00;
					break;
				}
			}
		}
	}

	sd_deassert_cs();
	return res1;
}


/*______________________________________________________________________________________________
	Read a single block of data
				
	addr	32-bit address. SDHC and SDXC are block addressed, meaning an address of 0 
			will read back bytes 0-511, address 1 will read back 512-1023, etc.
			With 32 bits to specify a 512 byte block, the maximum size a card can support is 
			(2^32)*512 = 2048 megabytes or 2 terabytes.
					
	buff	a buffer of at least 512 bytes to store the data in
	
	token	0xFE - Successful read
			0x0X - Data error (Note that some cards don't return an error token instead timeout will occur)
			0xFF - Timeout
_______________________________________________________________________________________________*/
uint8_t sd_read_single_block(uint32_t addr, uint8_t *buf){
	uint8_t res1, read = 0;
	uint32_t readAttempts;
	
	if(SD_CardType == SD_V1_SDSC) addr *= 512;

	// set token to none
	SD_ResponseToken = 0xFF;

	sd_assert_cs();
	sd_command(CMD17, addr, CMD17_CRC);

	// read R1
	res1 = sd_read_response1();

	// if response received from card
	if(res1 != 0xFF){
		// wait for a response token (timeout = 100ms)
		// The host should use 100ms timeout (minimum) for single and multiple read operations
		readAttempts = 0;
		while(++readAttempts != SD_MAX_READ_ATTEMPTS){
			if((read = SPI_ReceiveByte()) != 0xFF) break;
		}

		// if response token is 0xFE
		if(read == 0xFE){
			// read 512 byte block
			for(uint16_t i = 0; i < SD_BUFFER_SIZE; i++) *buf++ = SPI_ReceiveByte();
			
			// add null to the end
			*buf = 0;

			// read and discard 16-bit CRC
			SPI_ReceiveByte();
			SPI_ReceiveByte();
		}

		// set token to card response
		SD_ResponseToken = read;
	}

	sd_deassert_cs();
	return res1;
}


/*______________________________________________________________________________________________
	Assert Chip Select Line by sending a byte before and after changing the CS line
_______________________________________________________________________________________________*/
static void sd_assert_cs(void){
	SPI_SendByte(0xFF);
	SPI_CARD_CS_ENABLE();
	SPI_SendByte(0xFF);
}


/*______________________________________________________________________________________________
	De-Assert Chip Select Line by sending a byte before and after changing the CS line
_______________________________________________________________________________________________*/
static void sd_deassert_cs(void){
	SPI_SendByte(0xFF);
	SPI_CARD_CS_DISABLE();
	SPI_SendByte(0xFF);
}


/*______________________________________________________________________________________________
	Send ACMD41 - Sends host capacity support information and activates the card's
	initialization process. Reserved bits shall be set to '0'.
_______________________________________________________________________________________________*/
static SD_RETURN_CODES sd_command_ACMD41(void){
	uint8_t response;
	uint8_t i = 100;
	
	// Initialization process can take up to 1 second so we add a 10ms delay
	// and a maximum of 100 iterations
	
	do{
		// CMD55 - APP_CMD - R1 response
		sd_assert_cs();
		sd_command(CMD55, CMD55_ARG, CMD55_CRC);
		sd_read_response1();
		sd_deassert_cs();
		
		// ACMD41 - SD_SEND_OP_COND (Send Operating Condition) - R1 response
		sd_assert_cs();
		
		if(SD_CardType == SD_V1_SDSC)
			sd_command(ACMD41, 0, ACMD41_CRC);
		else
			sd_command(ACMD41, ACMD41_ARG, ACMD41_CRC);
		
		response = sd_read_response1();
		sd_deassert_cs();
		
		i--;
		_delay_ms(10);
		
	}while((response != 0) && (i > 0));
	
	// Timeout
	if(i == 0) return SD_IDLE_STATE_TIMEOUT;
	
	return response;
}



/*______________________________________________________________________________________________
	Wait for a response from the card other than 0xFF which is the normal state of the
	MISO line. Timeout occurs after 16 bytes.
			
	return		response code
_______________________________________________________________________________________________*/
static uint8_t sd_read_response1(void){
	uint8_t i = 0, response = 0xFF;

	// Keep polling until actual data received
	while((response = SPI_ReceiveByte()) == 0xFF){
		i++;

		// If no data received for 16 bytes, break
		if(i > 16) break;
	}

	return response;
}



/*______________________________________________________________________________________________
	Read Response 3 or 7 which is 5 bytes long
				
	res			Pointer to a 5 bytes array
_______________________________________________________________________________________________*/
static void sd_read_response3_7(uint8_t *res){
	res[0] = sd_read_response1(); // read response 1 in R3 or R7
	
	if(res[0] > 1) return; // if error reading R1, return
	
	res[1] = SPI_ReceiveByte();
	res[2] = SPI_ReceiveByte();
	res[3] = SPI_ReceiveByte();
	res[4] = SPI_ReceiveByte();
}


/*______________________________________________________________________________________________
	Send a command to the card
				
	cmd			8-bit command index
	arg			a 32-bit argument
	crc			an 8-bit CRC
_______________________________________________________________________________________________*/
static void sd_command(uint8_t cmd, uint32_t arg, uint8_t crc){
	// Transmit command
	SPI_SendByte(cmd | 0x40);

	// Transmit argument
	SPI_SendByte((uint8_t)(arg >> 24));
	SPI_SendByte((uint8_t)(arg >> 16));
	SPI_SendByte((uint8_t)(arg >> 8));
	SPI_SendByte((uint8_t)(arg));

	// Transmit CRC
	SPI_SendByte(crc | 0x01);
}



/*______________________________________________________________________________________________
	SPI - Initialization
_______________________________________________________________________________________________*/
static void SPI_Init(void){
	// Set MOSI and SCK as output
	SPI0_DDR |= (1 << SPI0_MOSI_PIN) | (1 << SPI0_SCK_PIN);
	CARD_CS_DDR |= (1 << CARD_CS_PIN); // CS pin
	
	// Enable SPI, Master mode
	SPCR0 = (1 << SPE0) | (1 << MSTR0);

	// Start with a lower speed for older cards (100-400kHz)
	SPSR0 = (1 << 0);
	SPCR0 |= (1 << SPR01); // set clock rate fosc/32
}


/*______________________________________________________________________________________________
	SPI - Send a block of data based on the data length
				
	buf			Data to send
	length		Number of bytes
_______________________________________________________________________________________________*/
void SPI_Send(uint8_t *buf, uint16_t length){
	while(length != 0){
		SPDR0 = *buf;
		// Wait until the Busy bit is cleared
		while(!(SPSR0 & (1 << SPIF0)));
		length--;
		buf++;
	}
}


/*______________________________________________________________________________________________
	SPI - Send a byte
_______________________________________________________________________________________________*/
static void SPI_SendByte(uint8_t byte){
	SPDR0 = byte;
		
	// Wait until the Busy bit is cleared
	while(!(SPSR0 & (1 << SPIF0)));
}


/*______________________________________________________________________________________________
	SPI - Receives a block of data based on the length
				
	buf			Pointer to an array where to receive the data
	length		Number of bytes
_______________________________________________________________________________________________*/
void SPI_Receive(uint8_t *buf, uint16_t length){
	for(uint16_t i = 0; i < length; i++){
		*buf = SPI_ReceiveByte();
		buf++;
	}
}


/*______________________________________________________________________________________________
	SPI - Receive and returns one byte only
_______________________________________________________________________________________________*/
static uint8_t SPI_ReceiveByte(void){
	// Write dummy byte out to generate clock, then read data from MISO
	SPDR0 = 0xFF;
	
	// Wait until the Busy bit is cleared
	while(!(SPSR0 & (1 << SPIF0)));

	return SPDR0;
}

#endif /* SD_H_ */