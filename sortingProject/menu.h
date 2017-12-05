/*
 * menu.h
 *
 * Created: 11/17/2017 3:45:23 PM
 *  Author: Owner
 */ 


#ifndef MENU_H_
#define MENU_H_

#define MENU_START 0
#define MENU_CALIBRATION 1
#define MENU_HOME 2
#define MENU_OPTICALS 3
#define MENU_STEPPER 4

#define MENU_ACEL 3
#define MENU_MAX_DELAY 2
#define MENU_MIN_DELAY 1
#define MENU_ACEL_STEPS 0

void menu1Start();
void menu1Calibration();
void menu2Home();
void menu2Stepper();
void menu2Refl();
void menuDebugQ();
void menuDebugS();
void menuDisplayItemCount();
void menu3Display(unsigned char display);
void menu3Running();
void menu3ValueSelection(unsigned char select);
void menuDisplayValue(unsigned char value, const char*label);

#endif /* MENU_H_ */