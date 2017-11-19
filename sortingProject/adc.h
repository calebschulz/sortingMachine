#define REF_SENSOR_PORT PIND 
#define REF_SENSOR_PIN 0x08 //OR sensor
 
#define REFL_VALUE_BUFFER 5
 
#define BLACK 0
#define WHITE 1
#define STEEL 2
#define ALUMINUM 3
#define UNKNOWN	4


void initADC();
void startADC();
void stopADC();
void calibrateADC();
void findLowestReading(const char * objectType);
