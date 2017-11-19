#ifndef MOTOR_H_
#define MOTOR_H_


#define MOTOR_FORWARD 0b01000000
#define MOTOR_REVERSE 0b10000000
#define MOTOR_BRAKE 0b00000000
#define MOTOR_PORT PORTA
#define MOTOR_PINS 0xC0
#define MOTOR_ENABLE_PORT PORTB
#define MOTOR_ENABLE_PIN 0x1
#define MOTOR_BRAKE_TIME_MS 100 //*** find correct value

void initMotor();
void motorSpeed(char speed);
void motorForward();
void motorReverse();
void motorBrake();

#endif