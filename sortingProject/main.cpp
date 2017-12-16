/*
 * simpleClassifier.cpp
 *
 * Created: 11/9/2017 3:26:55 PM
 * Author : Owner
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "SSD1306.h"
#include "Framebuffer.h"
#include "adc.h"
#include "extInt.h"
#include "motor.h"
#include "timer.h"
#include "stepper.h"
#include "GPIO.h"
#include "menu.h"



volatile unsigned char debug = 1;
unsigned char deQueue = 0;
unsigned volatile char pauseSystem = 0;

extern volatile unsigned char reflQueue[];
extern volatile char reflQueueCount;
extern volatile char reflQueueChange;
extern volatile unsigned char reflInARow;
extern volatile int stepCurrentPosition; //*** debug
extern volatile int stepGoalPosition;
extern volatile unsigned int lowestRefl;
extern volatile char frontOfQueue;
extern volatile char backOfQueue;
extern volatile unsigned char delayStepper;
extern volatile unsigned char blockReady;
extern volatile unsigned char stepperReady;


Framebuffer myDisplay;

int main(void){
	unsigned char menuSelector = 0;
	unsigned char rampDown = 0;
	
	cli(); //Ensure interrupts are turned off
	//Set system clock to 8MHz
	CLKPR = _BV(CLKPCE);
	CLKPR = 0;
	
	//pin 6 input for stepper calibration pin
	DDRE &= 0x0e; 
	//Leds output
	DDRC |= 0xff;
	//Joystick input w/ pullup up(pin7) left(pin6) select(pin5) 
	DDRB &= ~JS_UP_PIN & ~JS_LEFT_PIN & ~JS_SELECT_PIN; 
	PORTB |= JS_UP_PIN | JS_LEFT_PIN | JS_SELECT_PIN;
	//Joystick input w/ pullup down(pin5) right(pin4)
	DDRE &= ~JS_DOWN_PIN & ~JS_RIGHT_PIN;
	PORTE |= JS_DOWN_PIN | JS_RIGHT_PIN;
	
	initADC();
	initMotor();
	initTimer();
	initExtInt();
	initMotor();
	initStepper();
	
	motorBrake();
	motorSpeed(0xc2);//doesn't seem to like 0xdf
	homeStepper();
	/*//Manual Optical sensor calibration
		static unsigned int min = 1023;
		sei();
		motorForward();
		while(1){
			if(min > lowestRefl){
				min = lowestRefl;
			}
			myDisplay.clear();
			myDisplay.drawString(0,0,"ADC Min V");
			myDisplay.drawRectangle(0,16,127,17,1);
			myDisplay.drawNumber(0,20,min);
			myDisplay.show();
			if((PINE & JS_DOWN_PIN) == 0){
				min = 1023;
			}
		}*/
	
	
	////////MENU SCREEN 1
	menu:
	menuSelector = MENU_START;
	menu1Start();
 	while(1){
		//If Joystick down -> highlight Calibrate
 		if((PINE & JS_DOWN_PIN) == 0){
			menu1Calibration();
			menuSelector = MENU_CALIBRATION;
 		}
		//If Joystick Up -> highlight Start
		if((PINB & JS_UP_PIN) == 0){
			menu1Start();
			menuSelector = MENU_START;
		}
		//If Joystick select -> run currently selected option
		if((PINB & JS_SELECT_PIN) == 0){
			//sei(); ***test without
			//Debounce
			mTimer(20);
			while((PINB & JS_SELECT_PIN) == 0){};
			mTimer(20);
			//cli(); *** test without
			if(menuSelector == MENU_CALIBRATION){
				goto calibration;
			}
			else{
				goto mainLoop;
			}
		}
 	}

	//////////CALIBRATION LOOP SCREEN 2
	calibration:
	menuSelector = MENU_HOME;
	menu2Home();
	while(1){
		//If Joystick down -> highlight Optical S
		if((PINE & JS_DOWN_PIN) == 0 && menuSelector == MENU_STEPPER){
			menu2Refl();
			menuSelector = MENU_OPTICALS;
		}
		else if((PINE & JS_DOWN_PIN) == 0 && menuSelector == MENU_HOME){
			mTimer(20);
			while((PINE & JS_DOWN_PIN) == 0){};
			mTimer(20);
			menu2Stepper();
			menuSelector = MENU_STEPPER;
		}
		//If Joystick Up -> highlight Stepper
		else if((PINB & JS_UP_PIN) == 0 && menuSelector == MENU_OPTICALS){
			mTimer(20);
			while((PINB & JS_UP_PIN) == 0){};
			mTimer(20);
			menu2Stepper();
			menuSelector = MENU_STEPPER;
		}
		else if((PINB & JS_UP_PIN) == 0 && menuSelector == MENU_STEPPER){
			menu2Home();
			menuSelector = MENU_HOME;
		}
		else if((PINB & JS_LEFT_PIN) == 0){
			goto menu;
		}
		//If Joystick select -> run currently selected option
		else if((PINB & JS_SELECT_PIN) == 0){
			mTimer(20);
			while((PINB & JS_SELECT_PIN) == 0){};
			mTimer(20);
			if(menuSelector == MENU_OPTICALS){
				calibrateADC();
				//Reset Queue and front/back 
				for(unsigned char i = 0; i<8;i++){
					reflQueue[i] = 0;
				}
				frontOfQueue = 0;
				backOfQueue = 0;
				//Continue showing calibration results till user presses Select
				while(1){
					if((PINB & JS_SELECT_PIN) == 0){
						mTimer(20);
						while((PINB & JS_SELECT_PIN) == 0){};
						mTimer(20);
						goto calibration;
					}
				}
				
			}
			else if(menuSelector == MENU_HOME){
				homeStepper();
				goto menu;
			}
			else if(menuSelector == MENU_STEPPER){
				stepperCalibration();
				goto calibration;
			}
		}
	}
		
	//////////DISPLAY RESULTS MENU SCREEN 3
	displayResults:
	menuDebugQ();
	while(1){
		if((PINB & JS_LEFT_PIN) == 0){
			rampDown = 0;
			goto mainLoop;
		}
		else if(JS_UP_PRESSED){
			menuDebugQ();
		}
		else if(JS_DOWN_PRESSED){
			menuDisplayItemCount();
		}
	}
		
    //////////MAIN LOOP
	mainLoop:
	myDisplay.clear();
	myDisplay.drawString(column(1),row(1),"Running...");
	myDisplay.show();
	motorForward(); 
	startStepper();
	sei();
    while (1){
		//Debugging routines
		//menuDebugQ();
		//menuDebugS();
		
		//while(1){};
		if(reflQueueChange){
			//turn off interrupts? ***
			cli();
			if(delayStepper){
// 				if(delayStepper == 2){
// 					mTimer(100);
// 				}
// 				else{
					mTimer(STEPPER_MOVE_DELAY);
				//}
				delayStepper = 0;
			}	
			
			
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

			reflQueueChange = 0;
			//turn interrupts back on? ***
			sei();
		}
		
		//Allows user to go to results display
		if((PINE & JS_RIGHT_PIN) == 0){
			//Brake motor
			MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_BRAKE;
			//Wait for motor to stop before turning off interrupts
			mTimer(MOTOR_BRAKE_TIME_MS); 
			cli();
			goto displayResults;
		}
		else if(JS_DOWN_PRESSED){
			rampDown = 1;	
		}
		
		if(rampDown){
			menuDisplayValue(rampDown,"RampD");
			if(rampDown < 40){
				rampDown++;
			}
			else{
				if(!reflQueueCount){
					mTimer(500);
					MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_BRAKE;
					goto displayResults;
				}
			}
		}
		
    }
}