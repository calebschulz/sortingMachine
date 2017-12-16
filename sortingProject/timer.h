/*
 * timer.h
 *
 *  Author: Caleb Schulz
 */ 

#ifndef TIMER_H_
#define TIMER_H_

#define DISABLE_TIMER1 0x7
#define BUTTON_DEBOUNCE_DELAY 20

void initTimer(void);
void mTimer(int count);

#endif