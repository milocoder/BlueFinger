#ifndef CAN_H
#define CAN_H
#define F_CPU 16000000UL // dit moet of 16 of 8 MHz zijn
#define CAN_BAUDRATE 500 // dit moet of 500 of 1000 kHz zijn

#include <stdint.h>

#if F_CPU == 16000000UL
#if CAN_BAUDRATE == 500
#define CANBT1_VAL 0x06
#define CANBT2_VAL 0x04
#define CANBT3_VAL 0x13
#elif CAN_BAUDRATE == 1000
#define CANBT1_VAL 0x02
#define CANBT2_VAL 0x04
#define CANBT3_VAL 0x13
#else
#error CAN baud rate not supported
#endif
#elif F_CPU == 8000000UL
#if CAN_BAUDRATE == 500
#define CANBT1_VAL 0x02
#define CANBT2_VAL 0x04
#define CANBT3_VAL 0x13
#elif CAN_BAUDRATE == 1000
#define CANBT1_VAL 0x00
#define CANBT2_VAL 0x04
#define CANBT3_VAL 0x13
#else
#error CAN baud rate not supported
#endif
#else
#error CPU clock rate not supported by device
#endif

// Length of the message buffer
#define MESSAGE_BUFFER_LENGTH 20

// Number of Message Objects in the CAN Controller
#define MOB_COUNT 15

// CAN 2.0 type A message information
typedef struct
{
	uint16_t id : 11;
	char data[8];
	uint8_t length;
} CANMessage;

// Status of the CAN bus at a given time
// Any bit set to 1 indicates that error condition is present
typedef struct
{
	uint8_t busOff : 1;
	uint8_t overrun : 1;
	uint8_t frameBufferReceive : 1;
	uint8_t stuff : 1;
	uint8_t CRC : 1;
	uint8_t form : 1;
	uint8_t acknowledgement : 1;
} CANErrorStatus;

void initCAN();

CANErrorStatus getLastCANError();

uint8_t listenForMessage( uint16_t id, uint8_t expectedLength );

uint8_t getMessage( CANMessage * message );

uint8_t sendCAN( CANMessage* message );

#endif // CAN_H