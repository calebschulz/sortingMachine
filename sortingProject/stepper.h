#define NUMBER_STEPS_ACCELERATION 15
#define NUMBER_STEPS_DECELERATION 11
#define MAX_STEPPER_DELAY 112 //1 ms = 7.8125~8 -> 18ms ~ 144  //1ms = 31.25 , 
#define MIN_STEPPER_DELAY 73 //8 ms
#define STEPPER_MOVE_DELAY 50
#define STEPPER_POSITION_OFFSET 0
#define STEPPER_HOME_PIN 0b01000000
#define STEPPER_DELAY_REGISTER OCR3A
#define STEPPER_BLACK_POSITION 0 //Find correct values for these ***
#define STEPPER_WHITE_POSITION 100
#define STEPPER_STEEL_POSITION 150
#define STEPPER_ALUMINIUM_POSITION 50 
#define STEPPER_ACCELERATION_RATE 4
#define CLOSE_ENOUGH 15

void initStepper();
void homeStepper();
void startStepper();
void rotateStepper(int numSteps, int directionCW);
void rotateStepperToGoal();
void stepperCalibration();