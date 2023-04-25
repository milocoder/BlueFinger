#include "can.h"
#include "avr/interrupt.h"

#include <util/delay.h>
#include <avr/io.h>



// Where the CAN ISR writes received messages to
CANMessage messageBuffer[ MESSAGE_BUFFER_LENGTH ];

// Index of the next index in the buffer to service
uint8_t serviceIndex = 0;

// Next available buffer index
uint8_t bufIndex = 0;

// Indicates any errors received on the bus.
uint8_t lastError = 0;

/*
 * Reset and configure the AT90 to use CAN.
 *
 */
void initCAN()
{
	// Issue a software reset of the CAN Controller
	CANGCON |= ( 1 << SWRES );

	// Set the baud rate to the compile-time determined value
	CANBT1 = CANBT1_VAL;
	CANBT2 = CANBT2_VAL;
	CANBT3 = CANBT3_VAL;

	// Set all the Message Objects to 'disabled'. This frees them up for use
	for (uint8_t i = 0; i < MOB_COUNT; ++i )
	{
		CANPAGE = i << 4;
		CANCDMOB = 0;
	}

	// Ensure interrupts are enabled on the AT90
	sei();

	// Clear all interrupt flags
	CANGIT = 0;

	// Enable CAN interrupts, Transmit interrupt
	CANGIE = (1 << ENIT) | (1 << ENTX) | (1 << ENRX);

	// Activate the CAN Controller
	CANGCON |= ( 1 << ENASTB );
}

/*
 * Find a Message Object that is not in use.
 * @return free MOB index, otherwise 0xFF
 *
 */
uint8_t getFreeMob()
{
	// This commented out code is to prevent context switching when using this function.
	// When it's not commented, it caused both CAN RX and TX to fail. The reasons for this are being investigated.
	//uint8_t canPageSaved = CANPAGE;
	// Iterate through the MOBs and check their 
	for( uint8_t i = 0; i < MOB_COUNT; ++i )
	{
		// Switch to the MOB
		CANPAGE = i << 4;

		// Check the bits indicating the type of MOB this is.
		// If they are both 0, the MOB is 'disabled' and therefore available
		if ( (CANCDMOB & 0xC0) == 0 )
		{
			//CANPAGE = canPageSaved;
			return i;
		}
	}

	// No Free MOBs available
	//CANPAGE = canPageSaved;
	return 0xFF;
}

/*
 * Get an indication of any errors detected by the CAN controller.
 * @return error status structure
 *
 */
CANErrorStatus getLastCANError()
{
	CANErrorStatus status;
	status.busOff = (lastError & (1 << BOFFIT)) != 0;
	status.overrun = (lastError & (1 << OVRTIM)) != 0;
	status.frameBufferReceive = (lastError & (1 << BXOK)) != 0;
	status.stuff = (lastError & (1 << SERG)) != 0;
	status.CRC = (lastError & (1 << CERG)) != 0;
	status.form = (lastError & (1 << FERG)) != 0;
	status.acknowledgement = (lastError & (1 << AERG)) != 0;
	return status;
}

/*
 * Configure a Message Object to transmit the provided message.
 * @param[in] message - the message to transmit
 * @return 1 if a MOb was successfully configured
 *
 */
uint8_t sendCAN( CANMessage* message )
{
	uint8_t mobIndex = getFreeMob();

	if ( mobIndex == 0xFF )
	{
		// No Free MOBs available, return false
		return 0;
	}

	// Otherwise, switch to the MOB
	CANPAGE |= (mobIndex << 4);

	// Set up the MOB based on the message

	// This library currently only supports standard CAN IDs
	CANIDT4 = (uint8_t)	 message->id >> 3;
	CANIDT3 = (uint32_t) message->id << 5;
	CANIDT2 = (uint32_t) message->id << 13;
	CANIDT1 = (uint32_t) message->id << 21;


	// Ensure nothing bigger than 8 is written to the CANCDMOB register
	if( message->length > 8 )
	{
		message->length = 8;
	}

	// Set the length of the data (Data Length Code)
	// Use an assignment operator to clear the upper bits, which will be set later
	CANCDMOB = message->length;

	// Fill in the data to send
	for( uint8_t i = 0; i < message->length; ++i )
	{
		// CANMSG points to a byte within the MOB's data bytes.
		// CANMSG auto-increments after each read if configured to.
		CANMSG = message->data[ i ];
	}

	// Enable interrupts for this MOb
	if( mobIndex < 8 )
	{
		CANIE2 |= (1 << mobIndex);
	}
	else
	{
		CANIE1 |= (1 << (mobIndex-8));
	}

	// Set the MOB's mode to transmission, also activating it.
	CANCDMOB |= (0x01 << CONMOB0);

	return 1;
}

/*
 * Configure a MOb to listen indefinitely for a CAN message with the provided id.
 * Messages read are written to the buffer.
 * @param[in] id - the provided id
 * @return 0 if no free message objects left, 1 otherwise
 *
 */
uint8_t listenForMessage( uint32_t id, uint8_t expectedLength )
{
	// Try to get a free MOb
	uint8_t mobIndex = getFreeMob();

	if( mobIndex == 0xFF )
	{
		// No free Message Objects
		return 0;
	}

	// Point to the free message object
	CANPAGE |= (mobIndex << 4);

	// Set the MOb's CAN id to the id to be listened for
	CANIDT4 = id >> 3;
	CANIDT3 = id << 5;
	CANIDT2 = id << 13;
	CANIDT1 = id << 21;
	

	if( expectedLength > 8 )
	{
		expectedLength = 8;
	}

	// Set the MOb's expected length
	CANCDMOB |= expectedLength;

	// Enable interrupts for this MOb
	if( mobIndex < 8 )
	{
		CANIE2 |= (1 << mobIndex);
	}
	else
	{
		CANIE1 |= (1 << (mobIndex-8));
	}

	// Set the MOb's mode to receive, also activating it.
	CANCDMOB |= (0x02 << CONMOB0);

	return 1;
}

/*
 * Get the next message received in FIFO order.
 * @param[out] message - message data to fill
 * @return 1 if message was filled, 0 otherwise
 *
 */
uint8_t getMessage( CANMessage * message )
{
	// Default to no message available
	uint8_t messageAvailable = 0;

	if( serviceIndex != bufIndex )
	{
		// Load the data from the next message received in FIFO order into the out message
		message->id = messageBuffer[ serviceIndex ].id;
		message->length = messageBuffer[ serviceIndex ].length;

		for( uint8_t i = 0; i < message->length; ++i )
		{
			message->data[ i ] = messageBuffer[ serviceIndex ].data[ i ];
		}

		// Increment the index pointing to the next message to be serviced
		serviceIndex = (serviceIndex + 1) % MESSAGE_BUFFER_LENGTH;

		messageAvailable = 1;
	}

	return messageAvailable;
}

/*
 * Interrupt Service Routine for CAN related interrupts.
 * Determines the interrupt type and the MOb that requires attention,
 * and services that mob.
 *
 * Note that the NO_BLOCK attribute is not applied here, so while this is running,
 * all interrupts are blocked.
 *
 */
ISR( CANIT_vect )
{
	// **Save off CANPAGE to prevent application code bugs**
	uint8_t canPage = CANPAGE;

	// CANHPMOB will contain the mob index of the highest priority mob that has interrupted,
	// or 0xFF if no mob has interrupted. If multiple mobs have interrupted, they need to all be
	// serviced, so this loop is required.
	while( ((CANHPMOB & 0xF0) >> 4) != 0x0F )
	{
		// Point to the Message Object that needs to be serviced
		CANPAGE = CANHPMOB & 0xF0;

		// Service the correct kind of interrupt
		// Transmit Interrupts
		if( CANSTMOB & (1 << TXOK) )
		{
			// Acknowledge the interrupt
			CANSTMOB = 0;

			// Free up the MOb
			CANCDMOB = 0;

			// Determine the MOb index
			uint8_t mob = (CANPAGE & 0xF0) >> 8;

			// Disable interrupts for this MOb
			if( mob < 8 )
			{
				CANIE2 &= ~(1 << mob);
			}
			else
			{
				CANIE1 &= ~(1 << (mob-8));
			}
		}
		// Receive Interrupts
		else if ( CANSTMOB & (1 << RXOK) )
		{
			// Default to impossible ID
			uint16_t id = 0xFFFF;

			// Load the message data into a free message buffer object
			if( CANCDMOB & (1 << IDE) )
			{
				// Message type is CAN 2.0B
				id = (CANIDT4 >> 3) | ((CANIDT3 & 0x7F) << 5); 
			}
			else
			{
				// Message type is CAN 2.0A
				id = (CANIDT2 >> 5) | (((uint16_t)CANIDT1 << 3));
			}

			messageBuffer[bufIndex].id = id;

			messageBuffer[bufIndex].length = CANCDMOB & 0x0F;

			// This trusts the length field in CANCDMOB is between 1 and 8...
			for( uint8_t i = 0; i < messageBuffer[bufIndex].length; ++i )
			{
				// Load the next byte from the MOb data register into the buffer
				// CANMSG is set to auto increment to the next byte after each read
				messageBuffer[bufIndex].data[i] = CANMSG;
			}

			bufIndex = (bufIndex + 1) % MESSAGE_BUFFER_LENGTH;

			// Acknowledge the interrupt
			CANSTMOB = 0;

			CANCDMOB |= (2 << CONMOB0);
			
		}
		else if ( CANGIT & ~(1 << CANIT) )
		{
			// Preserve the last error received
			lastError = CANGIT;

			// Clear the error
			CANGIT = 0;
		}
	}

	// Restore CANPAGE register to where application code had it before interrupt
	CANPAGE = canPage;
}