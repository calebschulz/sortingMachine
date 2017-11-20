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
#define MENU_STEPPER 2
#define MENU_OPTICALS 3

void menu1Start();
void menu1Calibration();
void menu2Stepper();
void menu2Refl();
void menuDebugQ();
void menuDebugS();
void menuDisplayItemCount();



#endif /* MENU_H_ */