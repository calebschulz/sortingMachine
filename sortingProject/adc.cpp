#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "motor.h"
#include "Framebuffer.h"

char cValue = 7;
volatile unsigned int lowestRefl = 1023; //Global used when finding object reflectivity
unsigned int calibReading = 1023;


extern volatile unsigned int blackMinRef;
extern volatile unsigned int whiteMinRef;
extern volatile unsigned int aluminumMinRef;
extern volatile unsigned int steelMinRef;
extern volatile unsigned char reflQueueCount;
extern Framebuffer myDisplay;

void initADC() {
	
	//Set Port F as Input
	DDRF = 0;
	
	//ADC enable | prescaler 
	ADCSRA |= _BV(ADEN);
	ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //0b111 -> 1/128
	// AVCC with external capacitor on AREF pin
	ADMUX |= _BV(MUX0) | _BV(REFS0);// | _BV(ADLAR);
}

void startADC() {
	//Enable ADC interrupt
	ADCSRA |= _BV(ADIE);
	//Start ADC conversion
	ADCSRA |= _BV(ADSC);
}

void stopADC() {
	//Disable ADC interrupt
	ADCSRA &= ~_BV(ADIE);
	//Cancel any pending ADC conversions
	ADCSRA &= ~_BV(ADSC);
}

void findLowestReading(const char * objectType){
	while(reflQueueCount < 8){
		if(lowestRefl < calibReading){
			calibReading = lowestRefl;
		}
		myDisplay.clear();
		myDisplay.drawString(0,0,"Calibrate");
		myDisplay.drawString(0,16,objectType);
		myDisplay.drawNumber(0,32,reflQueueCount);
		myDisplay.show();
	}
	reflQueueCount = 0;
}

void calibrateADC(){
	
	myDisplay.clear();
	myDisplay.drawString(0,0,"Calibrate");
	myDisplay.drawString(0,16,"Black");
	myDisplay.drawNumber(0,32,0);
	myDisplay.show();
	
	sei();
	motorForward();
	//Black -> White -> Steel -> Aluminum
	findLowestReading("Black");
	blackMinRef = calibReading - REFL_VALUE_BUFFER;
	findLowestReading("White");
	whiteMinRef = calibReading - REFL_VALUE_BUFFER;
	findLowestReading("Steel");
	steelMinRef = calibReading - REFL_VALUE_BUFFER;
	findLowestReading("Aluminum");
	aluminumMinRef = calibReading - REFL_VALUE_BUFFER;
		
	motorBrake();
	myDisplay.clear();
	myDisplay.drawString(0,0,"B:");
	myDisplay.drawNumber(24,0,blackMinRef);
	myDisplay.drawString(0,16,"W:");
	myDisplay.drawNumber(24,16,whiteMinRef);
	myDisplay.drawString(0,32,"S:");
	myDisplay.drawNumber(24,32,steelMinRef);
	myDisplay.drawString(0,48,"A:");
	myDisplay.drawNumber(24,48,aluminumMinRef);
	myDisplay.show();
	
	cli();
}

/*
	ISR for For ADC
	
	Trigger: ADC reading finished
	
	Description:
	For each series of readings this will find what
	the lowest reflection reading is and save it in
	lowestRefl. Then it will start another ADC reading.
*/
ISR(ADC_vect) {
	PORTC |= 2;

	if(ADC < lowestRefl){
		lowestRefl = ADC;
	}
	//Start another ADC conversion
	ADCSRA |= _BV(ADSC);
}