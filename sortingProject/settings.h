/*
 * settings.h
 *
 * Created: 12/6/2017 10:04:13 AM
 *  Author: Owner
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

//Used for calibrating the reflectivity values for blocks
//It will make it so that the max and min reflectivity values
//are continually saved (not reinitialized on each pass).
//It also sets it so that the display will show the min and max
//values that have been found.
//This allows the user to keep on putting the same type of block
//multiple times to find the values to use for calibration.
//Comment it out to turn this mode off.
#define CALIBRATE_REFL

//volatile unsigned char mode




#endif /* SETTINGS_H_ */