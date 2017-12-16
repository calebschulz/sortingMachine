/*
 * 	adc.h
 *
 *  Author: Caleb Schulz
 */ 

#ifndef ADC_H_
#define ADC_H_

#define REF_SENSOR_PORT PIND 
#define REF_SENSOR_PIN 0x08 //OR sensor
 
#define REFL_VALUE_BUFFER 5
 
#define BLACK 0
#define WHITE 1
#define STEEL 2
#define ALUMINUM 3
#define UNKNOWN	4

//Note: that this value must be a multiple of 2
//You must change count and mod_div in conjucture 
#define ADC_FILTER_COUNT 32
#define ADC_FILTER_MOD_DIV 5 //bitwise division (i.e. >> 3  ==  /8)


void initADC();
void startADC();
void stopADC();
void calibrateADC();
void findLowestReading(const char * objectType);
unsigned int findAverageReading(const char * objectType);

#endif