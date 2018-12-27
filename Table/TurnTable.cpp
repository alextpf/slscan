//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////

#include "TurnTable.h"
#include "Configuration.h"
#include "Util.h"

////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////
//=========================================================
int TurnTable::MotorStepToTurnTablePos(long motorStep)
{
  return = motorStep / STEPS_PER_UNIT;
} // MotorStepToTurnTablePos

//=========================================================
long TurnTable::TurnTablePosToMotorStep(const int pos/*deg*/)
{
  return = (long)(pos)* (long)STEPS_PER_UNIT;
} // TurnTablePosToMotorStep

////////////////////////////////////////////
// class functions
////////////////////////////////////////////
//=========================================================
TurnTable::TurnTable()
: m_Time( 0 )
, m_LoopCounter( 0 )
{}

//=========================================================
TurnTable::~TurnTable()
{}

//=========================================================
void TurnTable::Update() // aka positionControl()
{  
  m_LoopCounter++;
  
  // convert from motor steps to robot position
  m_Pos = MotorStepToTurnTablePos( m_Motor.GetCurrStep() ); // deg. update m_Pos

//  if (m_LoopCounter % 100 == 0 )
  {
  #ifdef SHOW_LOG      
      Serial.print("Current table Pos: ");
      Serial.print(m_Pos);
      Serial.print(" deg");
      Serial.println("");
  #endif
  }   

  // record time
  uint32_t currTime = micros();

  long delta = currTime - m_Time;
  
  #ifdef SHOW_LOG
    Serial.print("delta =  ");
    Serial.println( delta );
    Serial.println("");
  #endif

  uint16_t dt;
  if( delta < 0 )
  {
    dt = 2000;

//    #ifdef SHOW_LOG
      Serial.print("delta < 0 !! ");
      Serial.println("");
//    #endif
  }
  else
  {
    dt = delta;
  }
  
  m_Time = currTime; // update time

  // update motor acceleration
  m_Motor.UpdateAccel(); // update m_AbsAccel for Motor  
  m_Motor.UpdateSpeed( dt, Motor::MOTOR_NUM::M1 ); // update m_CurrSpeed, m_Dir, m_Period for M1 & M2
} // Update

//=========================================================
void TurnTable::SetPosInternal( int x, int y )
{    
  // Constrain to robot limits...  
  RobotPos goal(
    constrain( x, ROBOT_MIN_X, ROBOT_MAX_X ), 
    constrain( y, ROBOT_MIN_Y, ROBOT_MAX_Y ) ); // mm

  long m1s, m2s;
  TurnTablePosToMotorStep(goal, m1s, m2s);

  #ifdef SHOW_LOG
      Serial.print("SetPosInternal: Motor1 step = ");
      Serial.print( m1s );
      Serial.print(", Motor2 step = ");
      Serial.println( m2s );
  #endif
    
  m_Motor.SetGoalStep( m1s ); // set m_GoalStep
  m_M2.SetGoalStep( m2s );
  
} // SetPosInternal

//=========================================================
void TurnTable::UpdatePosStraight()
{  
  // Speed adjust to draw straight lines (aproximation)
  // First, we calculate the distante to target on each axis
  const long diff_M1 = m_Motor.GetGoalStep() - m_Motor.GetCurrStep();
  const long diff_M2 = m_M2.GetGoalStep() - m_M2.GetCurrStep();

  const unsigned long absDiffM1 = abs( diff_M1 );
  const unsigned long absDiffM2 = abs( diff_M2 );

  #ifdef SHOW_LOG
      // log
      Serial.println("UpdatePosStraight: ");
      Serial.print("m1 goal step = ");
      Serial.print( m_Motor.GetGoalStep() );
      Serial.print(", m1 curr step = ");
      Serial.println( m_Motor.GetCurrStep() );
      
      Serial.print("m2 goal step = ");
      Serial.print( m_M2.GetGoalStep() );
      Serial.print(", m2 curr step = ");
      Serial.println( m_M2.GetCurrStep() );  Serial.println("");

      Serial.print("diff_M1 = ");
      Serial.print( diff_M1 );
      Serial.print(", diff_M2 = ");
      Serial.println( diff_M2 );

      Serial.print("absDiffM1 = ");
      Serial.print( absDiffM1 );
      Serial.print(", absDiffM2 = ");
      Serial.println( absDiffM2 );
      //===================================
  #endif
  
  // Now, we calculate the factor to apply to draw straight lines. Speed adjust based on target distance
  float factor1 = 1.0;
  float factor2 = 1.0;
  if ( absDiffM2 == 0 ) // to avoid division by 0
  {
    factor2 = 0.0;
  }
  else if ( absDiffM1 > absDiffM2 )
  {
    factor2 = (float)absDiffM2 / (float)absDiffM1;
  }
  else
  {
    factor1 = (float)absDiffM1 / (float)absDiffM2;
  }

  #ifdef SHOW_LOG
      // log
      Serial.print("factor1 = ");
      Serial.print( factor1 );
      Serial.print(", factor2 = ");
      Serial.println( factor2 );Serial.println( "" );
      //=====================================
  #endif
  
  // Calculate the target speed (with sign) for each motor
  long targetSpeed1 = sign( diff_M1 ) * GetMaxAbsSpeed() * factor1; // arduino "long" is 32 bit
  long targetSpeed2 = sign( diff_M2 ) * GetMaxAbsSpeed() * factor2; // arduino "long" is 32 bit
  
  // Now we calculate a compensation factor. This factor depends on the acceleration of each motor (difference on speed we need to apply to each motor)
  // This factor was empirically tested (with a simulator) to reduce overshoots
  const int difS1 = m_Motor.GetCurrSpeed() - targetSpeed1;
  const int difS2 = m_M2.GetCurrSpeed() - targetSpeed2;

  const unsigned int diffspeed1 = abs( difS1 );
  const unsigned int diffspeed2 = abs( difS2 );

  #ifdef SHOW_LOG
      // log
      Serial.print("M1 curr speed = ");
      Serial.print( m_Motor.GetCurrSpeed() );
      Serial.print(", targetSpeed1 = ");
      Serial.println( targetSpeed1 );
      Serial.print("M2 curr speed = ");
      Serial.print( m_M2.GetCurrSpeed() );
      Serial.print(", targetSpeed2 = ");
      Serial.println( targetSpeed2 );  
      Serial.println( "" );
      //=====================================
  #endif
  
  float tmp = ((float)diffspeed2 - (float)diffspeed1) / (2.0 * (float)GetMaxAbsSpeed());
        
  float speedfactor1 = 1.05 - tmp;  
  speedfactor1 = constrain( speedfactor1, 0.0, 1.0 );
  
  float speedfactor2 = 1.05 + tmp;
  speedfactor2 = constrain( speedfactor2, 0.0, 1.0 );

  // Set motor speeds. We apply the straight factor and the "acceleration compensation" speedfactor
  const int target_speed_M1 = GetMaxAbsSpeed() * factor1 * speedfactor1 * speedfactor1; // unsigned speed
  const int target_speed_M2 = GetMaxAbsSpeed() * factor2 * speedfactor2 * speedfactor2;

  #ifdef SHOW_LOG
      // log
      Serial.print("M1 target speed = ");
      Serial.print( target_speed_M1 );
      Serial.print(", speedfactor1 = ");
      Serial.println( speedfactor1 );
      Serial.print("M2 target speed = ");
      Serial.print( target_speed_M2 );
      Serial.print(", speedfactor2 = ");
      Serial.println( speedfactor2 );  
      Serial.println( "" );
      //=====================================
  #endif
  
  m_Motor.SetAbsGoalSpeed( target_speed_M1 ); // set m_AbsGoalSpeed
  m_M2.SetAbsGoalSpeed( target_speed_M2 );  
} // UpdatePosStraight

//=========================================================
void TurnTable::SetPosStraight( int x, int y )
{  
  #ifdef SHOW_LOG
      Serial.print("Current Pos: x = ");
      Serial.print( m_Pos.m_X );
      Serial.print(", y = ");
      Serial.println( m_Pos.m_Y ); Serial.println("");
      Serial.print("SetPosStraight: x = ");
      Serial.print( x );
      Serial.print(", y = ");
      Serial.println( y ); Serial.println("");
  #endif
      
  SetPosInternal( x, y ); // set m_GoalStep for M1 & M2
  UpdatePosStraight(); // use algorithm to calculate goal speed and set m_AbsGoalSpeed for M1 & M2
} // SetPosStraight
