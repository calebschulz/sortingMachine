#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "stepper.h"
#include "motor.h"
#include "timer.h"
#include "extInt.h"
#include "adc.h"
#include "menu.h"
#include "GPIO.h"

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
extern volatile unsigned char blackCount;
extern volatile unsigned char whiteCount;
extern volatile unsigned char steelCount;
extern volatile unsigned char aluminumCount;

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

void stepperCalibration(){
	unsigned char menuSelection;
	//Stepper test loop for delay 
	stepperSettings:
	menuSelection = 0;
	menu3Display(menuSelection);
	while(1){
		if(JS_UP_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_UP_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);	
			menuSelection = (menuSelection+1)&3;
		}
		else if(JS_DOWN_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_DOWN_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);
			menuSelection = (menuSelection-1)&3;
		}
		else if(JS_SELECT_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_SELECT_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);
			goto valueSelection;
		}
		menu3Display(menuSelection);
		if(JS_RIGHT_PRESSED){
			goto startStepperTest;
		}
	}
	valueSelection:
	//Change from ADC1 to ADC 2 *** must change back
	ADMUX = (ADMUX & _BV(MUX0)) | _BV(MUX1);
	
	while(1){
		
	}
	
	startStepperTest:
	menu3Running();
	homeStepper();
	startStepper();
	stepGoalPosition = 0;
	sei();
	while(1){
		//rotateStepperToGoal();
		if((PINE & JS_DOWN_PIN) == 0){
			mTimer(20);
			while((PINE & JS_DOWN_PIN) == 0){};
			mTimer(20);
			stepGoalPosition -=50;
			if(stepGoalPosition < 0){
				stepGoalPosition = 150;
			}
		}
		//If Joystick Up -> highlight Start
		if((PINB & JS_UP_PIN) == 0){
			mTimer(20);
			while((PINB & JS_UP_PIN) == 0){};
			mTimer(20);
			stepGoalPosition +=50;
			if(stepGoalPosition > 199){
				stepGoalPosition = 0;
			}
		}
		if((JS_LEFT_PORT & JS_LEFT_PIN) == 0){
			cli();
			goto stepperSettings;
		}
		menuDebugS();
	}
}

ISR(TIMER0_COMPA_vect){
	
	unsigned char nextItem = 0;
	int difference =  stepGoalPosition - stepCurrentPosition;
	unsigned char shortAbsDifference = ((difference >= 0) ? difference : -difference);//abs(difference)
	
	//Find circular shortest distance
	if(shortAbsDifference > 100){
		shortAbsDifference = 200 - shortAbsDifference;
	}

	//////////PLACE BLOCK INTO BIN ONCE CLOSE ENOUGH
	if(shortAbsDifference < CLOSE_ENOUGH){
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
			//Stepper Dequeue is ready and we have arrived at goal
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
				nextItem = (frontOfQueue+1) & 7;
				if(reflQueue[frontOfQueue] != reflQueue[nextItem]){
					delayStepper = 1;
				}
				frontOfQueue = nextItem; //& 7 implements a rotating array pointer
				reflQueueCount--;
				reflQueueChange = 1;
			}
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

	//////////ACCELERATION/DECELERATION ***
	if(shortAbsDifference > 30 && stepperDelay > MIN_STEPPER_DELAY){
		stepperDelay -= STEPPER_ACCELERATION_RATE; //+= ~ 1 ms
	}
	else if(shortAbsDifference < NUMBER_STEPS_DECELERATION && stepperDelay <= MAX_STEPPER_DELAY){
		stepperDelay += STEPPER_ACCELERATION_RATE;
	}
	//Set the initial value of the timer counter to 0
	TCNT0 = 0x0;
	//Sets stepper delay to max value
	OCR0A = stepperDelay;
}