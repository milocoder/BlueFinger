/*
 * PIB_led_code.c
 *
 * Created: 21-2-2023 15:11:47
 * Author : Johan
 */


#define F_CPU 16000000UL

#define	BUILD_FOR_AT90CAN	1
#define	SUPPORT_EXTENDED_CANID	1


#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


#include "at90can_private.h"
#include "config.h"
#include "can_private.h"
#include "can.h"
#include "utils.h"
#include "can_buffer.h"


int main(void)
{
   DDRF = 0xFF; 
	
   
   at90can_init(4); 
   at90can_set_mode(NORMAL_MODE); 
    
   while (1) {
	   bool receivedMessage = at90can_check_message();
	   
	   if (receivedMessage)
	   {
		   PORTF = 0xFF;
	   } else {
		   PORTF = 0x00;
	   }
	   
   } 

}


/*
can_filter_t filter = {
	.id = 0,
	.mask = 0,
	.flags = {
		.rtr = 0,
		.extended = 0
	}
};

int main(void) {
	// Initialize at90can
	can_init(BITRATE_125_KBPS);
	
	// Load filters and masks
	can_set_filter(0, &filter);
	
	// Create a test message
	can_t msg;
	
	msg.id = 0x123456;
	msg.flags.rtr = 0;
	msg.flags.extended = 1;
	
	msg.length = 4;
	msg.data[0] = 0xde;
	msg.data[1] = 0xad;
	msg.data[2] = 0xbe;
	msg.data[3] = 0xef;
	
	while(1){
		// Send the message
		at90can_send_message(&msg);
		
	} 
}
*/
