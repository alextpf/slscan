//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////

////////////////////////////
// debug
////////////////////////////
//#define NO_STOP
//#define SHOW_LOG            1 // comment it out if you don't want to show debug info
////////////////////////////
// Pin definition
////////////////////////////
#define X_STEP_PIN          54
#define X_DIR_PIN           55
#define X_ENABLE_PIN        38


#define LED_PIN             13
#define BAUD_RATE           115200

//========================================================================================================================================
////////////////////////////
// Physical dimenssion:
////////////////////////////

// Absolute Min and Max robot positions in mm (measured from center of robot pusher)
#define TABLE_MIN_DEG     0
#define TABLE_MAX_DEG     360

#define TABLE_INIT_POS              0
//========================================================================================================================================
//////////////////////////////////
// Max  Speed & Acceleration
//////////////////////////////////

// THIS VALUES DEPENDS ON YOUR ROBOT CONSTRUCTION (MOTORS, MECHANICS...)
// RECOMMENDED VALUES FOR 12V POWER SUPPLY
#define MAX_ABS_ACCEL         300                        // Maximun motor acceleration in (steps/seg2)/1000. Max recommended value:280
#define MAX_ABS_SPEED         5000//30000//32000                    // Maximun speed in steps/seg. Max absolute value: 32767!!

#define MIN_ACCEL         30						// make sure this is not 0
#define MIN_SPEED         50

#define SCURVE_LOW_SPEED  1000

#define ZERO_SPEED        65535

#define STOP_COEF         100//1800
//========================================================================================================================================
//////////////////////////////////
// Geometric calibration
//////////////////////////////////

// This depends on the pulley teeth. DEFAULT: 200(steps/rev)*8(microstepping) = 1600 steps/rev. 1600/32teeth*2mm(GT2) = 25 steps/mm
#define STEPS_PER_DEG 52

//========================================================================================================================================
//////////////////////////////////
// Utils (don´t modify)
//////////////////////////////////

#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))
