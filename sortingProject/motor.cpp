#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "motor.h"

void initMotor(void){
	
	DDRB |= 0x11;//Set PB4 as output (will be used for motor PWM Tim2)
	//old
	//debug
	DDRA |= MOTOR_PINS;
	DDRB |= MOTOR_ENABLE_PIN;

	//Fast PWM
	TCCR2A |= _BV(WGM21) | _BV(WGM20); 
	//Clear OC0A on Compare Match, set OC0A at TOP
	TCCR2A|= _BV(COM2A1); 

	//No prescaler
	TCCR2B |= _BV(CS21); //_BV(CS20);

	//Timer/Counter0 Output Compare Match A Interrupt Enable
	//TIMSK2 |= _BV(OCIE2A);

	//Set duty cycle
	OCR2A = 0x80; //~50%
	
	//Enable motor ***
	PORTB &= ~MOTOR_ENABLE_PIN;
}

void motorSpeed(char speed){
	TCNT2 = 0;
	OCR2A = speed;
}

void motorForward(){
	MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_FORWARD;
}

void motorReverse(){
	MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_REVERSE;
}

void motorBrake(){
	MOTOR_PORT = (MOTOR_PORT & ~MOTOR_PINS) | MOTOR_BRAKE;
}
