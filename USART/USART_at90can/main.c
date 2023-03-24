/*
 * USART_at90can.c
 *
 * Created: 11-3-2023 19:25:00
 * Author : johan
 */ 

#include "serial.h"

#define F_CPU 16000000UL

#define BAUD 9600
#define UBRR_VAL (F_CPU / (16UL * BAUD)) - 1

int main(void)
{
	initUart(UBRR_VAL);
	putString("Hello world!");
}