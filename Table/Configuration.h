//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////

////////////////////////////
// debug
////////////////////////////
//#define SHOW_LOG            1 // comment it out if you don't want to show debug info
////////////////////////////
// Pin definition
////////////////////////////
//#define TWO_MOTOR // two-motor system v.s. 3-motor system
  #define X_STEP_PIN          54
  #define X_DIR_PIN           55
  #define X_ENABLE_PIN        38

#ifdef TWO_MOTOR

  #define Y_STEP_PIN          60
  #define Y_DIR_PIN           61
  #define Y_ENABLE_PIN        56
#else // 3-motor system
  // X MOTOR
  //     X-STEP: A0    (PF0)
  //     X-DIR:  A1    (PF1)
  //     X-ENABLE: D38 (PD7)
  // Y MOTOR (Y-LEFT)
  //     Y-STEP: A6    (PF6)
  //     Y-DIR:  A7    (PF7)
  //     Y-ENABLE: A2  (PF2)
  // Z MOTOR (Y-RIGHT): the reason we can't use Z for both left and right is the power can't support
  //     Z-STEP: D46   (PL3)
  //     Z-DIR:  D48   (PL1)
  //     Z-ENABLE: A8  (PK0)

  #define Y_LEFT_STEP_PIN          60
  #define Y_LEFT_DIR_PIN           61
  #define Y_LEFT_ENABLE_PIN        56

  #define Y_RIGHT_STEP_PIN         46
  #define Y_RIGHT_DIR_PIN          48
  #define Y_RIGHT_ENABLE_PIN       62

#endif

#define LED_PIN             13
#define BAUD_RATE           115200

//========================================================================================================================================
////////////////////////////
// Physical dimenssion:
////////////////////////////

// This is the center of the table. All units in milimeters
// This are custum dimensions
#define TABLE_LENGTH    1003                      // 39.5''
#define TABLE_WIDTH     597                       // 23.5''
#define ROBOT_CENTER_X  TABLE_WIDTH/2             // Center of robot = 298.5
#define ROBOT_CENTER_Y  TABLE_LENGTH/2

// Absolute Min and Max robot positions in mm (measured from center of robot pusher)
#define ROBOT_MIN_X     50
#define ROBOT_MIN_Y     100 // Note: this can't be greater than ROBOT_INITIAL_POSITION_Y or "Robot defense and attack lines"
#define ROBOT_MAX_X     TABLE_WIDTH-ROBOT_MIN_X
#define ROBOT_MAX_Y     ROBOT_CENTER_Y-55 //=445

// Initial robot position in mm
// The robot must be at this position at start time
// Default: Centered in X and minimun position in Y
#define ROBOT_INITIAL_POSITION_X              TABLE_WIDTH/2
#define ROBOT_INITIAL_POSITION_Y              100            // Measured from center of the robot pusher to the table border

// Robot defense and attack lines
#define ROBOT_DEFENSE_POSITION_DEFAULT        140
#define ROBOT_DEFENSE_ATTACK_POSITION_DEFAULT 320
#define ROBOT_DEFENSE_POSITION_MIN            60
#define ROBOT_DEFENSE_POSITION_MAX            150
#define ROBOT_DEFENSE_ATTACK_POSITION_MIN     140
#define ROBOT_DEFENSE_ATTACK_POSITION_MAX     400

#define PUCK_SIZE               27                        // PuckSize (puck radius in mm)
#define PRE_ATTACK_DIST         4 * PUCK_SIZE

//========================================================================================================================================
//////////////////////////////////
// Max Robot Speed & Acceleration
//////////////////////////////////

// THIS VALUES DEPENDS ON YOUR ROBOT CONSTRUCTION (MOTORS, MECHANICS...)
// RECOMMENDED VALUES FOR 12V POWER SUPPLY
#define MAX_X_ABS_ACCEL         250//275                        // Maximun motor acceleration in (steps/seg2)/1000. Max recommended value:280
#define MAX_Y_ABS_ACCEL         120//140
#define MAX_X_ABS_SPEED         30000//30000//32000                    // Maximun speed in steps/seg. Max absolute value: 32767!!
#define MAX_Y_ABS_SPEED         15000//25000

#define MIN_ACCEL         50						// make sure this is not 0
#define MIN_SPEED         5000

#define SCURVE_LOW_SPEED  2500

#define ZERO_SPEED        65535

#define MIN_PUCK_Y_SPEED1        -280                    // used in Robot::newDataStrategy()
#define MIN_PUCK_Y_SPEED2        -160                    // used in Robot::newDataStrategy()
#define FAST_IN_Y_SPEED          -25
#define BOUNCE_TIME_THRESHOLD     200
#define ATTACK_TIME_THRESHOLD     500
#define IMPACT_TIME_THRESHOLD     300 // if robot is slow (low speed limist), make this value larger
#define PREDICT_TIME_THRESHOLD    500
#define MIN_ABS_Y_SPEED           60
#define MIN_ABS_X_SPEED           100
#define MIN_PREDICT_TIME          120                   // used in Robot::robotStrategy().
#define STOP_COEF         1800//1800
//========================================================================================================================================
//////////////////////////////////
// Geometric calibration
//////////////////////////////////

// This depends on the pulley teeth. DEFAULT: 200(steps/rev)*8(microstepping) = 1600 steps/rev. 1600/32teeth*2mm(GT2) = 25 steps/mm
#define STEPS_PER_UNIT 16

// CORRECTION FOR VISION SYSTEM LAG
#define VISION_SYSTEM_LAG 60   // in miliseconds

// CORRECTION OF MISSING STEPS ON MOTORS
// Coment this lines if you don´t want to make the corrections
#define CORRECT_MISSING_STEPS 1
#define MISSING_STEPS_MAX_ERROR_X 5
#define MISSING_STEPS_MAX_ERROR_Y 5
#define ROBOT_POSITION_CAMERA_CORRECTION_Y 0 // Correction of the position of the camera because the camera point of view and mark position

//========================================================================================================================================
//////////////////////////////////
// Utils (don´t modify)
//////////////////////////////////

#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))
