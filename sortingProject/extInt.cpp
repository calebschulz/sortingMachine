/*
 * extInt.cpp
 *
 * Created: 11/9/2017 3:36:11 PM
 *  Author: Caleb Schulz
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "motor.h"
#include "stepper.h"
#include "timer.h"
#include "Framebuffer.h"
#include "settings.h"
#include "GLOBALS.h"

volatile unsigned char reflQueue[8];
volatile unsigned char reflQueueCount = 0;
volatile char frontOfQueue = 0;
volatile char backOfQueue = 0;
volatile char reflQueueChange = 0;
volatile unsigned char reflInARow = 0;
volatile unsigned char risingEdge = 1;
volatile unsigned int blackMinRef = 940; 
volatile unsigned int whiteMinRef = 880; 
volatile unsigned int steelMinRef = 300;
volatile unsigned int aluminumMinRef = 15; 
volatile unsigned char blackCount = 0;
volatile unsigned char whiteCount = 0;
volatile unsigned char steelCount = 0;
volatile unsigned char aluminumCount = 0;

volatile char blockReady = 0;
volatile char stepperRdy = 1; //*** turn this into extern when adding stepper

void initExtInt(void){
	//External interrupts on pin 2,3
	DDRD &= 0b11110011; 
	
	//Turn off interrupt while configuring
	EIMSK = 0;
	
	//Any edge interrupt on PD2 (EXT INT2)
	//Falling edge interrupt on PD3 (EXT INT3)
	EICRA |= 0b10010000;
	
	//Turn on External Interrupt 2 and 3
	EIMSK = 0b1100;
}

/*
	ISR for First detector
	
	Pin: PD2
	Trigger: Rising and falling edge
	
	Description:
	
	Rising edge:
	Will run when an item is first detected and start taking
	reoccurring ADC readings (ADCinterrupt). 
	
	Falling edge:
	Will run when the item leaves the reflectometer reading
	area and turn off the reoccurring ADC readings. It then 
	classifies the block based of of what the minimum ADC
	reading was. Add item to front of FIFO queue.
	
	TODO: Add a low range for each classification to make it
	more rubust.
*/
ISR(INT2_vect){
	//////////"Debounce" *** seems to work fine with 1 ms delay
	mTimer(5);
	//////////
	
	//if(REF_SENSOR_PORT & REF_SENSOR_PIN){
	if(PIND & 0x4){
		//////////START ADC
		lowestRefl = 1023;
		adcTotalCount = 0; 
		adcAverage = 0;
		
		//Enable ADC interrupt
		ADCSRA |= _BV(ADIE);
		//Start ADC conversion
		ADCSRA |= _BV(ADSC);
		
	}
	else if((PIND & 0x4) == 0){
		char unsigned itemValue = 0;
		//////////STOP ADC
		//Disable ADC interrupt
		ADCSRA &= ~_BV(ADIE);
		//Cancel any pending ADC conversions
		ADCSRA &= ~_BV(ADSC);
	
		#ifdef CALIBRATE_REFL
		if(lowestRefl > maxRefl){
			maxRefl = lowestRefl;
		}
		#endif
		
		//////////CLASSIFY BLOCK
		if(lowestRefl > blackMinRef){
			itemValue = BLACK;
		}
		else if(lowestRefl > whiteMinRef){
			itemValue = WHITE;
		}
		else if (lowestRefl > steelMinRef){
			itemValue = STEEL;
		}
		else if(lowestRefl > aluminumMinRef){
			itemValue = ALUMINUM;
		}
		else{
			#ifdef FAST_MODE
			itemValue = BLACK;
			#else
			itemValue = UNKNOWN;
			//pauseSystem = 1;
			#endif
		}

		//////////ADD BLOCK TO QUEUE
		//Check to make sure we aren't passed max
		if(reflQueueCount > 7){ 
			PORTC = 0xaa;
		}
		else if(reflQueueCount == 0){
			reflQueueCount++;
			reflQueue[backOfQueue] = itemValue;
			reflQueueChange = 1;
		}
		else{
			reflQueueCount++;
			backOfQueue = (backOfQueue + 1) & 7;
			reflQueue[backOfQueue] = itemValue;
 		}	
		//Error checking for < 0? ***
		
	}
	//Clear interrupt flag
	EIFR = 0x4; 
	
}

ISR(INT3_vect){
	char nextItem = 0;
	//////////"Debounce"
	mTimer(5);

	if((PIND & 0x8) == 0){
		if(stepperReady){
			//////////PUT BLOCK IN BIN (leave motor on) 

			//////////COUNT NUMBER OF EACH SORTED
			if(reflQueue[frontOfQueue] == BLACK){
				blackCount++;
			}
			else if(reflQueue[frontOfQueue] == WHITE){
				whiteCount++;
			}
			else if(reflQueue[frontOfQueue] == STEEL){
				steelCount++;
			}
			else if(reflQueue[frontOfQueue] == ALUMINUM){
				aluminumCount++;
			}

			//////////DEQUEUE BLOCK
			if(reflQueueCount < 2){
				reflQueueCount = 0;
			}
			else{
				nextItem = (frontOfQueue+1) & 7; //& 7 implements a rotating array position
				if(reflQueue[frontOfQueue] != reflQueue[nextItem]){
					delayStepper = 1;
					stepperReady = 0;
				}
				frontOfQueue = nextItem; 
				reflQueueCount--;
				reflQueueChange = 1;
			}
		}
		else{
			//Motor brake
			MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_BRAKE;
			blockReady = 1;
		}
	}
	//Clear interrupt flag (else ISR runs twice)
	EIFR = 0x8; 
}

ISR(BADISR_vect){
	PORTC = 0xaa;
}