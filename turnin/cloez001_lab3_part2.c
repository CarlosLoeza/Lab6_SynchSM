/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0;
unsigned char button = 0;
unsigned char i = 0;

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1;	// Start counter from here to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal clock of 1ms ticks

void TimerOn(){
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;	// bit3 = 0: CTC mode(clear timer on compare)
			// bit2bit1bit0 = 011;
			// 0000 1011 : 0x0B
			// So 8 MHz clock or 8,000,000 / 64  = 125,000 ticks
			// Thus TCNT1 register will count 125,000 ticks

    // AVR out compare register OCR1A
    OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
			// We want a 1ms tick. .001s * 125,000 ticks = 125
			// So when TCNT1 == 125 then that means 1ms has passed

    // AVR timer register interrupt mask register
    TIMSK1 = 0x02;	// bit1: OCIE1A -- enables compare match register

    // Initiliaze AVR counter
    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;

    // enable global interrupts
    SREG |= 0x80;	// 1000 0000 
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR(){
    TimerFlag = 1;
}

// In our approach C program does not touch this ISR, only TimerISR()
ISR(TIMER1_COMPA_vect){
    // CPU automatically calls when TCNT1 == OCR1 (Every 1ms per TimerOn settings)
    _avr_timer_cntcurr--;	// count down to 0
    if (_avr_timer_cntcurr == 0){
	TimerISR();	// call the ISR that the user uses
	_avr_timer_cntcurr = _avr_timer_M;
    }
}

// Set timer to tick every M ms.
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr =  _avr_timer_M;
}


enum TurnOn_State {TurnOn_Start, TurnOn_B0, TurnOn_B1, TurnOn_B2, TurnOn_waitB0, TurnOn_waitB1} TurnOn_State;

void Tick(){
    // Transitions
    switch(TurnOn_State){
        case TurnOn_Start:
            TurnOn_State = TurnOn_B0;
            i =0;
            break;
        // light up B0 or continue to next state
        case TurnOn_B0:
            if (button){
                TurnOn_State =  TurnOn_waitB0;
            }
            else{
                TurnOn_State = TurnOn_B1;
            }
            break;

        // Light up B1 again
        case TurnOn_waitB0:
            if (!button)
                TurnOn_State = TurnOn_waitB0;
            else if (button && i ==0)
                TurnOn_State = TurnOn_B1;
            else if (button && i == 1)
                TurnOn_State = TurnOn_B0;
            break;

        // wait
        case TurnOn_B1:
            if(!button && i == 0)
                TurnOn_State = TurnOn_B2;
            else if (!button && i==1)
                TurnOn_State = TurnOn_B0;
            else if (button && i == 0)
                TurnOn_State = TurnOn_waitB1;
            else if (button && i==1)
                TurnOn_State = TurnOn_waitB0;           
            break;
            
        // Wait
        case TurnOn_waitB1:
            if(!button)
                TurnOn_State = TurnOn_waitB1;
            else if (button && i==0)
                TurnOn_State = TurnOn_B2;
            else if (button && i==1)
                TurnOn_State = TurnOn_B1;
            break;
        
        // Light up B2
        case TurnOn_B2:
            if(!button)
                TurnOn_State = TurnOn_B1;
            else if (button && i==1)
                TurnOn_State = TurnOn_waitB1;
            break;
         
            
        default:
            TurnOn_State = TurnOn_Start;
            break;
    }

    // Actions
    switch(TurnOn_State){
        case TurnOn_B0:
            i =0;
            PORTB = 0x01;
            break;
        
        case TurnOn_B1:
            PORTB = 0x02;
            break;
        
        case TurnOn_B2:
            i =1;
            PORTB = 0x04;
            break;

        default:
            break;
        }
}

int main(void) {

    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    TimerSet(300);
    TimerOn();

    TurnOn_State = TurnOn_Start;
    
    /* Insert your solution below */
    while (1) {
        button = ~PINA & 0x01;
        Tick();

        while(!TimerFlag); // wait 1 sec
            TimerFlag = 0;
        }
    return 1;
}
