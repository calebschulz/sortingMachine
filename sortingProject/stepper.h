//Initial values used 11-27: accel 15, decel 11
#define NUMBER_STEPS_ACCELERATION 6 //20
#define NUMBER_STEPS_DECELERATION 6 //20
//Initial values used 11-27: max 112, min 73, 
//Below 2 were working well for our station
//#define MAX_STEPPER_DELAY 120 //1 ms = 7.8125~8 -> 18ms ~ 144  //1ms = 31.25 , 
//#define MIN_STEPPER_DELAY 60 //8 ms
#define MAX_STEPPER_DELAY 132 //1 ms = 7.8125~8 -> 18ms ~ 144  //1ms = 31.25 ,
#define MIN_STEPPER_DELAY 72 //8 ms

#define STEPPER_MOVE_DELAY 50 //was 10
#define STEPPER_POSITION_OFFSET 0
#define STEPPER_HOME_PIN 0b01000000
#define STEPPER_DELAY_REGISTER OCR3A
#define STEPPER_BLACK_POSITION 0 //Find correct values for these ***
#define STEPPER_WHITE_POSITION 100
#define STEPPER_STEEL_POSITION 150
#define STEPPER_ALUMINIUM_POSITION 50 
//Initial values used 11-27: acell rate 4
#define STEPPER_ACCELERATION_RATE 12
#define CLOSE_ENOUGH 15

void initStepper();
void homeStepper();
void startStepper();
void rotateStepper(int numSteps, int directionCW);
void rotateStepperToGoal();
void stepperCalibration();