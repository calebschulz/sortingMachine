#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

void initTimer(void){
	//Set the waveform generation mode bit to clear timer
	//on Compare Math mode (CTC) only
	TCCR1B |= _BV(WGM12);
	
	//Set output compare register for 1000 cycles (1 ms)
	OCR1A = 0x3e8;
}

//Timer function
void mTimer(int count){
	int i = 0;
	
	//Enable
	//Sets bit 0 of the Timer/Counter control register (prescaler)
	//CS = 0b010 (8 MHz Clock prescaled by 1/8)
	TCCR1B |= _BV(CS11);
	
	//Set the initial value of the timer counter to 0
	TCNT1 = 0x0;
	
	//Clear the interrupt flag and begin timer
	TIFR1 |= _BV(OCF1A);
	
	//Poll the timer to determine when the timer has reached OCR1A
	while(i<count){

		if((TIFR1 & 0x02) == 0x02){
			//Clear the interrupt flag by writing a one to the bit
			TIFR1 |= _BV(OCF1A);
			
			i++; //Increment the loop counter
		}
	}
	TCCR1B &= ~DISABLE_TIMER1;
	return;
}
