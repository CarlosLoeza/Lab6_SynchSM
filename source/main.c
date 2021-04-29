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


unsigned char tmpA;
unsigned char count;
unsigned char i;
enum Count_States {Count_Start, Count_Wait, Count_Up, Count_Up_Wait, Count_Down, Count_Down_Wait, Count_Zero, Count_Reset} Count_State;

void Increment_Decrement(){

    switch(Count_State){
	case Count_Start:
	    Count_State = Count_Wait;
	    break;
	case Count_Wait:
	    if(tmpA == 0){
		Count_State = Count_Wait;
	    } else if(tmpA == 0x01){
		Count_State = Count_Up;
	    } else if(tmpA == 0x02){
		Count_State = Count_Down;
	    } else if(tmpA == 0x03){
		Count_State = Count_Zero;
	    }
	    break;
	case Count_Up:
	    if(tmpA == 0x01){
	        Count_State = Count_Up_Wait;
	    } else if (tmpA == 0x02){
	   	Count_State = Count_Down;
	    } else if(tmpA == 0x03){
		Count_State = Count_Zero;
	    } else if(tmpA == 0){
		Count_State = Count_Wait;
	    }
	    break;
	case Count_Up_Wait:
	    if(tmpA == 0){
		Count_State = Count_Wait;
	    }
	    else if(tmpA == 0x01 && i<10){
		Count_State = Count_Up_Wait;
	    	i++;
	    } else if(tmpA == 0x01 && i == 10){
		Count_State = Count_Up;
		i = 0;
	    }else if (tmpA == 0x02){
		Count_State = Count_Down;
	    } else if (tmpA == 0x03){
		Count_State = Count_Zero;
	    } 
	    break;
	case Count_Down:
	    if(tmpA == 0x01){
	        Count_State = Count_Up;
	    } else if(tmpA == 0x02){
		Count_State = Count_Down_Wait;
  	    } else if(tmpA == 0x03){
		Count_State = Count_Zero;
	    } else if(tmpA == 0){
		Count_State = Count_Wait;
	    }
  	    break;
	case Count_Down_Wait:
	    if(tmpA ==0){
		Count_State = Count_Wait;
	    }
	    else if(tmpA == 0x01){ 
                Count_State = Count_Up;
            } else if (tmpA == 0x02 && i > 0){ 
                Count_State = Count_Down_Wait;
		i--;
            } else if(tmpA == 0x02 && i == 0){
		Count_State = Count_Down;
		i = 7;
	    } 
	    else if (tmpA == 0x03){ 
                Count_State = Count_Zero;
            }
	    break;
	case Count_Zero:
	    if(tmpA == 0x03){
		Count_State = Count_Zero;
	    }
	    else if(tmpA == 0x01){
		Count_State = Count_Up;
	    } else if(tmpA ==0x02){
		Count_State = Count_Down;
	    }else if(tmpA == 0){
		Count_State = Count_Wait;
	    }
	    break;
	default:
	    Count_State = Count_Wait;
	    break;
    }

    switch(Count_State){
	case Count_Up:
	    if(count <9){
		count = count+1;
	    }
	    break;
	case Count_Down:
	    if(count > 0){
		count = count-1;;
	    }
	    break;
   	case Count_Zero:
	    count = 0;
	    break;
    }

    PORTC = count;
}



int main(void)
{
    // PORTA: input   PORTC: output
    DDRA = 0x00; PORTA = 0xFF; 
    DDRC = 0xFF; PORTC = 0x00; 
    count = 7;
    i=0;
    
    TimerSet(100);
    TimerOn();
    Count_State = Count_Wait; 
    while (1) 
    {
	tmpA = ~PINA & 0x03;
	Increment_Decrement();
        while(!TimerFlag); // wait 1 sec
        TimerFlag = 0;
        
    }
    return 1;
}
