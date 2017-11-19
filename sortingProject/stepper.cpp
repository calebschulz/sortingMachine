#include "stepper.h"
#include "motor.h"
#include "timer.h"
#include "extInt.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

//Globals
volatile unsigned char stepperDelay = MAX_STEPPER_DELAY;
volatile int stepCurrentPosition = 0;
volatile int stepGoalPosition = 0;
volatile char stepAPosition = 0;
volatile unsigned char stepperReady = 0;
const char stepArray[] = {0b110000, 0b000110, 0b101000, 0b000101};
volatile unsigned char sDequeueRdy = 0;
volatile unsigned char delayStepper = 0;
volatile unsigned char shortPath;

extern volatile char blockReady;
extern volatile char reflQueueCount;
extern volatile char reflQueueChange;
extern volatile unsigned char reflQueue[];
extern unsigned char deQueue;
extern volatile char frontOfQueue;
extern volatile unsigned char backOfQueue;

void initStepper(void){
	//PA5-0 output for stepper control lines
	DDRA |= 0x3f;

	//Set the waveform generation mode bit to clear timer
	//on Compare Math mode (CTC) only
	TCCR0A |= _BV(WGM01);
	
	//Sets stepper delay to max value
	OCR0A = MAX_STEPPER_DELAY;

	//Set the initial value of the timer counter to 0
	TCNT0 = 0x0;
}

void startStepper(){
	//Enable Timer 0 compare A interrupt
	TIMSK0 |= _BV(OCIE0A);
	
	//Clear the interrupt flag and begin timer
	TIFR0 |= _BV(OCF0A);
	
	//Enable
	//Sets bit 0 and 2 of the Timer/Counter control register (prescaler 1/1024)
	//This will allow to get an even 1ms from an integer value of 125 (if using 1/64 prescaler)
	TCCR0B |= _BV(CS02) | _BV(CS00);
}

void homeStepper(void){
	//Initial rotation to ensure we get alligned in the stepArray properly
	rotateStepper(25,1);
	//Rotate until detecting the home position
	while(PINE & STEPPER_HOME_PIN){
		rotateStepper(1,1);
	}
	stepCurrentPosition = STEPPER_POSITION_OFFSET;
}

//Ideal delay values when using 1 battery weight
//Starting delay 18 ms
//Acceleration +1 for 10 times
//Deceleration -1 for 15 times
void rotateStepper(int numSteps, int directionCW){

	char delay = 18;

	for(int i=0; i<numSteps; i++){
		if(directionCW){
			stepCurrentPosition++;// = (stepPosition++);
			stepAPosition = (stepAPosition + 1) & 3;// = (stepAPosition++);
			
			if(stepCurrentPosition > 199){
				stepCurrentPosition = 0;
			}
		}
		else{
			if(stepCurrentPosition == 0){
				stepCurrentPosition = 199;
			}
			else{
				stepCurrentPosition--;// = (stepPosition--);
			}
			stepAPosition = (stepAPosition - 1) & 3;// = (stepAPosition--);

			
		}
		PORTA = (PORTA & 0b11000000) | stepArray[stepAPosition];
		//PORTC = stepArray[stepAPosition];

		//Acceleration/deceleration handling
		if(i < 10){
			delay -= 1;
		}
		else if((numSteps-i)< 20){
			delay += 1;
		}

		mTimer(delay);
	}

}

void rotateStepperToGoal(){
	
	int difference =  stepGoalPosition - stepCurrentPosition;
	if(difference >= 0){
		shortPath = difference - 100;
	}
	else{
		shortPath = difference*(-1) - 100;
	}
	//int shortPath = ((difference >= 0) ? difference : -difference) -100;
	if(((difference > 0) && (difference < 100))){//***change to make up for faster direction of stepper
		rotateStepper(difference, 1);
	}
	else if(difference < -100){
		rotateStepper(shortPath, 1);
	}
	else if(((difference < 0) && (difference >= -100))){
		//difference = ((difference >= 0) ? difference : -difference) -100;
		rotateStepper(abs(difference), 0);
	}
	else if(difference >= 100){
		rotateStepper(abs(shortPath), 0);
	}
}

ISR(TIMER0_COMPA_vect){
	
	unsigned char nextItem = 0;
	int difference =  stepGoalPosition - stepCurrentPosition;
	
	//////////PLACE BLOCK INTO BIN ONCE CLOSE ENOUGH
	if(((difference >= 0) ? difference : -difference) < CLOSE_ENOUGH){// abs(difference) < CLOSE_ENOUGH
		PORTC |= 8;
		if(blockReady){
			//////////MOTOR ON
			MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_FORWARD;
			sDequeueRdy = 1;
			blockReady = 0;
		} 
		else if(!sDequeueRdy){
			stepperReady = 1;
		}
		else if(difference == 0){
			PORTC |= 0x10;
			//Stepper Dequeue is ready and we have arrived at goal
			//////////DEQUEUE BLOCK
			frontOfQueue = (frontOfQueue + 1) & 7; //& 7 implements a rotating array pointer
			reflQueueCount--;
			reflQueueChange = 1;
			sDequeueRdy = 0;
			
		}
	}
	else{
		stepperReady = 0;
	}
	//////////MOVE TOWARDS GOAL POSITION
	if(((difference > 0) && (difference < 100)) || (difference < -100)){//***change to make up for faster direction of stepper
		stepCurrentPosition++;
		if(stepCurrentPosition > 199){
			stepCurrentPosition = 0;
		}
		stepAPosition = (stepAPosition + 1) & 3; //& 3 is a bitwise %4, creates a circular loop through stepArray	
	}
	else if(((difference < 0) && (difference >= -100)) || (difference >= 100)){
		stepCurrentPosition--;
		if(stepCurrentPosition < 0){
			stepCurrentPosition = 199;
		}
		//***
		stepAPosition = (stepAPosition - 1) & 3; //& 3 is a bitwise %4, creates a circular loop through stepArray
	}
	PORTA = (PORTA & 0b11000000) | stepArray[stepAPosition];
	//PORTC = stepArray[stepAPosition];
	
	difference = ((difference >= 0) ? difference : -difference);
	if(difference > 100){
		difference = 200 - difference;
	}

	//////////ACCELERATION/DECELERATION ***
	if(difference > 25 && stepperDelay > MIN_STEPPER_DELAY){
		stepperDelay -= 4; //+= ~ 1 ms
	}
	else if(difference < NUMBER_STEPS_DECELERATION && stepperDelay <= MAX_STEPPER_DELAY){
		stepperDelay += 4;
	}
	//Set the initial value of the timer counter to 0
	TCNT0 = 0x0;
	//Sets stepper delay to max value
	OCR0A = stepperDelay;
}