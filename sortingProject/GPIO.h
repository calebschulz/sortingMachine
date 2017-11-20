/*
 * GPIO.h
 *
 * Created: 11/17/2017 12:31:25 PM
 *  Author: Owner
 */ 


#ifndef GPIO_H_
#define GPIO_H_

//Joystick defines

#define JS_UP_PORT PINB
#define JS_UP_PIN 0b10000000
#define JS_UP_PRESSED ((JS_UP_PORT & JS_UP_PIN) == 0)
#define JS_LEFT_PORT PINB
#define JS_LEFT_PIN 0b01000000
#define JS_LEFT_PRESSED ((JS_LEFT_PORT & JS_LEFT_PIN) == 0)
#define JS_SELECT_PORT PINB
#define JS_SELECT_PIN 0b00100000
#define JS_SELECT_PRESSED ((JS_SELECT_PORT & JS_SELECT_PIN) == 0)
#define JS_DOWN_PORT PINE
#define JS_DOWN_PIN 0b00100000
#define JS_DOWN_PRESSED ((JS_DOWN_PORT & JS_DOWN_PIN) == 0)
#define JS_RIGHT_PORT PINE
#define JS_RIGHT_PIN 0b00010000
#define JS_RIGHT_PRESSED ((JS_RIGHT_PORT & JS_RIGHT_PIN) == 0)

#endif /* GPIO_H_ */