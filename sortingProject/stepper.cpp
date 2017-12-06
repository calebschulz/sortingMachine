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
volatile unsigned char *calibSelectedValue;
unsigned int adcEightAverage = 0;
unsigned char adcEightCount = 0;
const char *valueLabel;
volatile unsigned char stepperMaxDelay = MAX_STEPPER_DELAY;
volatile unsigned char stepperMinDelay = MIN_STEPPER_DELAY;
volatile unsigned char stepperAccelRate = STEPPER_ACCELERATION_RATE;
volatile unsigned char waitToReachGoal = 0;


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
	char menuSelection;
	//Stepper test loop for delay 
	stepperSettings:
	menuSelection = 0;
	menu3Display(menuSelection);
	while(1){
		if(JS_UP_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_UP_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);	
			if(menuSelection == 0){
				menuSelection = 0;
			}
			else{
				menuSelection--;
			}
			
			
		}
		else if(JS_DOWN_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_DOWN_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);
			menuSelection++;
			if(menuSelection > 2){
				menuSelection = 2;
			}
		}
		else if(JS_SELECT_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_SELECT_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);
			if(menuSelection == 0){
				valueLabel = "Max:";
				calibSelectedValue = &stepperMaxDelay;
			}
			else if(menuSelection == 1){
				valueLabel = "IncrValue";
				calibSelectedValue = &stepperAccelRate;
			}
			else if(menuSelection == 2){
				valueLabel = "Min:";
				calibSelectedValue = &stepperMinDelay;
			}
			goto valueSelection;
		}
		else if(JS_RIGHT_PRESSED){
			goto startStepperTest;
		}
		menu3Display(menuSelection);
	}
	valueSelection:
	//Change from ADC1 to ADC 2 *** must change back
	ADMUX = (ADMUX & ~_BV(MUX0)) | _BV(MUX1);
	//Left shifted
	ADMUX |= _BV(ADLAR);
	//Disable ADC interrupt
	ADCSRA &= ~_BV(ADIE);
	
	menuSelection = 0;
	menuDisplayValue(*calibSelectedValue, valueLabel);
	while(1){
		//Start ADC conversion
		ADCSRA |= _BV(ADSC);
		while(ADCSRA & _BV(ADSC)){}; //wait for conversion to finish
		
		if(adcEightCount < 8){
			adcEightAverage = adcEightAverage + ADCH - (adcEightAverage >> 3); //MA[n]* = MA[n-1]* + x[n] - MA[n-1]*/N
			adcEightCount++;
		}
		else{
			//adcEightAverage >>= 3;	//MA*/N
			*calibSelectedValue = adcEightAverage >> 3;
			menuDisplayValue(*calibSelectedValue, valueLabel);
			
			adcEightCount = 0;
		}
		if(JS_LEFT_PRESSED){
			mTimer(BUTTON_DEBOUNCE_DELAY);
			while(JS_LEFT_PRESSED){};
			mTimer(BUTTON_DEBOUNCE_DELAY);
			//Change back from ADC 2 to 1
			ADMUX = (ADMUX & ~_BV(MUX1)) | _BV(MUX0);
			//Turn off left shifted
			ADMUX &= ~_BV(ADLAR);
			//Reenable ADC interrupt
			ADCSRA |= _BV(ADIE);
			
			goto stepperSettings;
		}

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

/*
	if close enough
		if blockready 
			turn motor on
			dequeue
			queue change
			if current != next
				delay
		else
			stepper ready
*/
	//////////PLACE BLOCK INTO BIN ONCE CLOSE ENOUGH
	if(shortAbsDifference < CLOSE_ENOUGH){
		if(waitToReachGoal){
			stepperReady = 0; //*** this may not be needed?
			PORTC = 0x0;
			if(shortAbsDifference == 0){ //*** needs to be tested
				
				delayStepper = 2;
				//mTimer(50);
				waitToReachGoal = 0;
				//reflQueueChange = 1;
				if(reflQueue[frontOfQueue] == BLACK){
					stepGoalPosition = STEPPER_BLACK_POSITION;
				}
				else if(reflQueue[frontOfQueue] == WHITE){
					stepGoalPosition = STEPPER_WHITE_POSITION;
				}
				else if(reflQueue[frontOfQueue] == STEEL){
					stepGoalPosition = STEPPER_STEEL_POSITION;
				}
				else if(reflQueue[frontOfQueue] == ALUMINUM){
					stepGoalPosition = STEPPER_ALUMINIUM_POSITION;
				}
			}
		}
		else if(blockReady){
			stepperReady = 0;
			PORTC = 0xf;
			//////////MOTOR ON
			MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_FORWARD;
			blockReady = 0;
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
					waitToReachGoal = 1;
				}
				frontOfQueue = nextItem; //& 7 implements a rotating array pointer
				reflQueueCount--;
			}
		} 
		else{
			stepperReady = 1;
			PORTC = 0xf;
		}
	}
	else{
		stepperReady = 0;
		PORTC = 0;
	}
	//////////MOVE TOWARDS GOAL POSITION
	if(((difference > 0) && (difference < 100)) || (difference < -100)){//***change to make up for faster direction of stepper
		stepCurrentPosition++;
		if(stepCurrentPosition > 199){
			stepCurrentPosition = 0;
		}
		stepAPosition = (stepAPosition + 1) & 3; //& 3 is a bitwise version of %4, creates a circular loop through stepArray	
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
	if(shortAbsDifference > 30 && stepperDelay > stepperMinDelay){//MIN_STEPPER_DELAY){
		stepperDelay -= stepperAccelRate; // STEPPER_ACCELERATION_RATE; 
	}
	else if(shortAbsDifference < NUMBER_STEPS_DECELERATION && stepperDelay <= stepperMaxDelay){//MAX_STEPPER_DELAY){
		stepperDelay += stepperAccelRate; //STEPPER_ACCELERATION_RATE;
	}
	//Set the initial value of the timer counter to 0
	TCNT0 = 0x0;
	//Sets delay till next step
	OCR0A = stepperDelay;
}