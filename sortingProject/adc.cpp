/*
 * adc.cpp
 *
 * Created: 11/9/2017 3:26:03 PM
 *  Author: Caleb Schulz
 */ 

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "motor.h"
#include "Framebuffer.h"
#include "settings.h"
#include "GLOBALS.h"

char cValue = 7;
volatile unsigned int lowestRefl = 1023; //Globals used when finding object reflectivity
volatile unsigned int pLowestRefl = 1023; //used for calibration
volatile unsigned int maxRefl = 0; 
volatile unsigned int adcAverage = 0;
volatile unsigned char adcTotalCount = 0;
unsigned int calibReading = 1023;
volatile unsigned int debugCount=0;

//Initialize ADC
void initADC() {
	
	//Set Port F as Input
	DDRF = 0;
	
	//ADC enable | prescaler 
	ADCSRA |= _BV(ADEN);
	ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //0b111 -> 1/128
	// ADC1 - AVCC with external capacitor on AREF pin
	ADMUX |= _BV(MUX0) | _BV(REFS0);// | _BV(ADLAR);
}

//Start taking ADC readings
void startADC() {
	//Enable ADC interrupt
	ADCSRA |= _BV(ADIE);
	//Start ADC conversion
	ADCSRA |= _BV(ADSC);
}

//Stop taking ADC readings
void stopADC() {
	//Disable ADC interrupt
	ADCSRA &= ~_BV(ADIE);
	//Cancel any pending ADC conversions
	ADCSRA &= ~_BV(ADSC);
}

//ADC calibration function
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

//Calibration testing
unsigned int findAverageReading(const char * objectType){
	
	unsigned char calibCount = 0;
	unsigned int calibReading = 0;
	
	while(1){
		if(adcTotalCount == 128){
			//////////Moving Average filter 
			if(calibCount < 8){
				calibReading = calibReading + adcAverage - (calibReading >> 3); //MA[n]* = MA[n-1]* + x[n] - MA[n-1]*/N  where N=8
				calibCount++;
			}
			else{
				calibReading >>= 3;	//MA*/N
				return  calibReading;
			}
		}
		myDisplay.clear();
		myDisplay.drawString(0,0,"Calibrate");
		myDisplay.drawString(0,16,objectType);
		myDisplay.drawNumber(0,32,calibCount);
		myDisplay.show();
	}
}


//Calibrate ADC for aluminum, steel, black, and white
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
	Speed = ~0xa -> 309 readings per block.
	Speed = 0xff -> ~195 readings per block
*/
ISR(ADC_vect) {
	//*** debug
	//debugCount++;
 	
	//////////Moving Average filter (Note: need to calibrate motor speed so that adc reads 128 times going across blocks)
	//This one overflows and as such won't work for this project unless we significantly reduce the sampling rate
	//It also will not work in special cases where the belt stops a block right by the sensor.
	if(adcTotalCount < ADC_FILTER_COUNT){
		adcAverage += ADC;
		adcTotalCount++;
	}
	else if(adcTotalCount == ADC_FILTER_COUNT){
		adcAverage >>= ADC_FILTER_MOD_DIV;	//MA*/N
		adcTotalCount = 0;
		
		if(adcAverage < lowestRefl){
			lowestRefl = adcAverage;
		}
		
		#ifdef CALIBRATE_REFL
		if(adcAverage < pLowestRefl){
			pLowestRefl = adcAverage;
		}
		#endif
		
		adcAverage = 0;
	}
	
	//Without filtering
// 	if(ADC < lowestRefl){
// 		lowestRefl = ADC;
// 	}
// 	if(ADC < pLowestRefl){
// 		pLowestRefl = ADC;
// 	}

	//Start another ADC conversion
	ADCSRA |= _BV(ADSC);
}