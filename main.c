/*
 * File:   main.c
 * Author: Hans
 *
 * Created on May 7, 2017, 9:05 PM
 */

#include "main.h"

#include <xc.h>
#include <string.h>
#pragma warning disable 520
#include "usb.h"
#include "usb_ch9.h"

#pragma config FOSC = INTOSC
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config BOREN = ON
#pragma config CLKOUTEN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config WRT = OFF
#pragma config CPUDIV = NOCLKDIV
#pragma config USBLSCLK = 48MHz
#pragma config PLLMULT = 3x
#pragma config PLLEN = ENABLED
#pragma config STVREN = ON
#pragma config BORV = LO
#pragma config LPBOR = ON
#pragma config LVP = ON


void interrupt isr()
{
	usb_service();
}

void main()
{
	setup();
    
    send_state();

	while (1)
	{
		if (usb_is_configured() && endpoint_ready())
		{
			send_state_if_changed();
		}
	}
}

unsigned char state[2];
unsigned char last_state[2];

void setup()
{
	OSCCONbits.IRCF = 0b1111; // 16MHz HFINTOSC postscaler.

	// Setup USB clock-tuning.
	ACTCONbits.ACTSRC = 1;
	ACTCONbits.ACTEN = 1;

	// Enable interrupts.
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;
    
	// Setup pins.
    ANSELA = 0;
    TRISAbits.TRISA4 = 1;
    TRISAbits.TRISA5 = 1;
    
    ANSELB = 0;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    TRISBbits.TRISB6 = 1;
    TRISBbits.TRISB7 = 1;
    
    ANSELC = 0;
    TRISCbits.TRISC2 = 1;
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;
    TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC7 = 1;

	usb_init();
    
    state[0] = 0;
    state[1] = 0;
    last_state[0] = 0xff;
    last_state[1] = 0xff;
}


void get_state()
{
    state[0] = (PORTB & 0b11110000) >> 4;
    state[1] = ((PORTA & 0b00110000) >> 4) | (PORTC & 0b11111100);
}

void send_state_if_changed()
{
    get_state();
	if (state[0] != last_state[0] || state[1] != last_state[1])
	{
		send_state();
		last_state[0] = state[0];
        last_state[1] = state[1];
	}
}

void send_state(void)
{
	unsigned char *buffer = usb_get_in_buffer(1);

	buffer[0] = state[0];
    buffer[1] = state[1];
	usb_send_in_buffer(1, 2);
}

bool endpoint_ready()
{
   return !usb_in_endpoint_halted(1) && !usb_in_endpoint_busy(1);
}
