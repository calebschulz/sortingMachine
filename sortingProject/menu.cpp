/*
 * menu.cpp
 *
 * Created: 11/17/2017 3:45:03 PM
 *  Author: Owner
 */ 

#include "Framebuffer.h"
#include "extInt.h"

extern Framebuffer myDisplay;
extern volatile unsigned char reflQueue[];
extern volatile char frontOfQueue;
extern volatile char backOfQueue;
extern volatile unsigned char reflQueueCount;
extern volatile int stepCurrentPosition;
extern volatile int stepGoalPosition;
extern volatile unsigned char stepperDelay;
extern volatile unsigned int lowestRefl;
extern volatile unsigned char blackCount;
extern volatile unsigned char whiteCount;
extern volatile unsigned char steelCount;
extern volatile unsigned char aluminumCount;

void menu1Start(){
	myDisplay.clear();
	myDisplay.drawString(0,0,"Item Sorter");
	myDisplay.drawRectangle(0,16,127,17,1);
	myDisplay.drawString(0,20,">Start");
	myDisplay.drawString(0,36," Calibrate");
	myDisplay.show();
}

void menu1Calibration(){
	myDisplay.clear();
	myDisplay.drawString(0,0,"Item Sorter");
	myDisplay.drawRectangle(0,16,127,17,1);
	myDisplay.drawString(0,20," Start");
	myDisplay.drawString(0,36,">Calibrate");
	myDisplay.show();
}
void menu2Stepper(){
	myDisplay.clear();
	myDisplay.drawString(0,0,"Calibrate");
	myDisplay.drawRectangle(0,16,127,17,1);
	myDisplay.drawString(0,20,">Stepper");
	myDisplay.drawString(0,36," Optical S");
	myDisplay.show();
}
void menu2Refl(){
	myDisplay.clear();
	myDisplay.drawString(0,0,"Calibrate");
	myDisplay.drawRectangle(0,16,127,17,1);
	myDisplay.drawString(0,20," Stepper");
	myDisplay.drawString(0,36,">Optical S");
	myDisplay.show();
}

void menuDebugQ(){
	myDisplay.clear();
	myDisplay.drawString(0,0,"Queue");
	myDisplay.drawNumber(72,0,lowestRefl);
	myDisplay.drawRectangle(0,16,127,17,1);
	myDisplay.drawNumber(0,20,reflQueue[0]);
	myDisplay.drawNumber(12,20,reflQueue[1]);
	myDisplay.drawNumber(24,20,reflQueue[2]);
	myDisplay.drawNumber(36,20,reflQueue[3]);
	myDisplay.drawNumber(48,20,reflQueue[4]);
	myDisplay.drawNumber(60,20,reflQueue[5]);
	myDisplay.drawNumber(72,20,reflQueue[6]);
	myDisplay.drawNumber(84,20,reflQueue[7]);
	myDisplay.drawNumber(108,20,reflQueueCount);
	myDisplay.drawString(12*frontOfQueue,36,"f");
	myDisplay.drawString(12*(backOfQueue),36,"b");
	myDisplay.show();
}

void menuDebugS(){
	myDisplay.clear();
	myDisplay.drawString(0,0,"Goal:");
	myDisplay.drawNumber(60,0,stepGoalPosition);
	myDisplay.drawString(0,16,"Cur:");
	myDisplay.drawNumber(60,16,stepGoalPosition);
	myDisplay.drawString(0,32,"Delay:");
	myDisplay.drawNumber(72,32,stepperDelay);
	myDisplay.show();
}

void menuDisplayItemCount(){
	myDisplay.clear();
	myDisplay.drawString(column(1),row(1),"On Belt:");
	myDisplay.drawNumber(column(9),row(1),reflQueueCount);
	myDisplay.drawString(column(1),row(2),"Sorted:");
	myDisplay.drawString(column(1),row(3),"B=");
	myDisplay.drawNumber(column(3),row(3),blackCount);
	myDisplay.drawString(column(6),row(3),"W:");
	myDisplay.drawNumber(column(8),row(3),whiteCount);
	myDisplay.drawString(column(1),row(4),"S:");
	myDisplay.drawNumber(column(3),row(4),steelCount);
	myDisplay.drawString(column(6),row(4),"A:");
	myDisplay.drawNumber(column(8),row(4),aluminumCount);
	myDisplay.show();
}