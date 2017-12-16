/*
 * GLOBALS.h
 *
 *  Author: Caleb Schulz
 */ 

#ifndef GLOBALS_H_
#define GLOBALS_H_

//OLED Object
extern Framebuffer myDisplay;

//Needed in adc.cpp
extern volatile unsigned int blackMinRef;
extern volatile unsigned int whiteMinRef;
extern volatile unsigned int aluminumMinRef;
extern volatile unsigned int steelMinRef;
extern volatile unsigned char reflQueueCount;

//Needed in extInt.cpp
extern volatile unsigned char debug;
extern volatile char stepperReady;
extern volatile unsigned int lowestRefl;
extern volatile unsigned int maxRefl; 
extern unsigned char deQueue;
extern unsigned char delayStepper;
extern volatile unsigned int adcAverage;
extern volatile unsigned char adcTotalCount;
extern volatile unsigned char pauseSystem;

//Needed in main.cpp
extern volatile unsigned char reflQueue[];
//extern volatile char reflQueueCount;
extern volatile char reflQueueChange;
extern volatile unsigned char reflInARow;
extern volatile int stepCurrentPosition; //*** debug
extern volatile int stepGoalPosition;
//extern volatile unsigned int lowestRefl;
extern volatile char frontOfQueue;
extern volatile char backOfQueue;
//extern volatile unsigned char delayStepper;
extern volatile unsigned char blockReady;
//extern volatile unsigned char stepperReady;

//Needed in menu.cpp
extern Framebuffer myDisplay;
//extern volatile unsigned char reflQueue[];
// extern volatile char frontOfQueue;
// extern volatile char backOfQueue;
// extern volatile unsigned char reflQueueCount;
// extern volatile int stepCurrentPosition;
// extern volatile int stepGoalPosition;
extern volatile unsigned char stepperDelay;
//extern volatile unsigned int lowestRefl;
extern volatile unsigned int pLowestRefl;
//extern volatile unsigned int maxRefl;
extern volatile unsigned char blackCount;
extern volatile unsigned char whiteCount;
extern volatile unsigned char steelCount;
extern volatile unsigned char aluminumCount;
extern volatile unsigned int debugCount;
//extern volatile unsigned int adcAverage;
//extern volatile unsigned char adcTotalCount;

//Needed in stepper.cpp
// extern volatile char blockReady;
// extern volatile char reflQueueCount;
// extern volatile char reflQueueChange;
// extern volatile unsigned char reflQueue[];
// extern unsigned char deQueue;
// extern volatile char frontOfQueue;
// extern volatile unsigned char backOfQueue;
// extern volatile unsigned char blackCount;
// extern volatile unsigned char whiteCount;
// extern volatile unsigned char steelCount;
// extern volatile unsigned char aluminumCount;

#endif