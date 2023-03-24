#ifndef _SERIAL_H
#define _SERIAL_H

#include <avr/io.h>

void initUart (unsigned int);
void putChar (char);
char getChar (void);
void putString (char *);

#endif /* serial.h */
