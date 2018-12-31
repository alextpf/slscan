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
  return motorStep / STEPS_PER_DEG;
} // MotorStepToTurnTablePos

//=========================================================
long TurnTable::TurnTablePosToMotorStep(const int pos/*deg*/)
{
  return (long)(pos)* (long)STEPS_PER_DEG;
} // TurnTablePosToMotorStep

////////////////////////////////////////////
// class functions
////////////////////////////////////////////
//=========================================================
TurnTable::TurnTable()
: m_Time( 0 )
, m_LoopCounter( 0 )
, m_IsGoalCounter( 0 )
, m_GoalPos( 0 )
, m_Pos( 0 )
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

  if( m_GoalPos == m_Pos )
  {
    m_IsGoalCounter++;
  }

  if( m_IsGoalCounter >= 20 )
  {
    m_IsGoalCounter = 0;    
  }

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
  m_Motor.UpdateSpeed( dt ); // update m_CurrSpeed, m_Dir, m_Period for M1 & M2
} // Update

//=========================================================
void TurnTable::SetPosInternal( int deg )
{    
  // Constrain to robot limits...  
  //m_GoalPos = constrain( deg, TABLE_MIN_DEG, TABLE_MAX_DEG ); // deg
  m_GoalPos = deg;

  long ms = TurnTablePosToMotorStep( m_GoalPos );

  #ifdef SHOW_LOG
      Serial.print("SetPosInternal: Motor1 step = ");
      Serial.println( m_GoalPos );
  #endif
    
  m_Motor.SetGoalStep( ms ); // set m_GoalStep
  
} // SetPosInternal

//=========================================================
void TurnTable::UpdatePosStraight()
{  
  // Speed adjust to draw straight lines (aproximation)
  // First, we calculate the distante to target on each axis
  const long diff = m_Motor.GetGoalStep() - m_Motor.GetCurrStep();

  const unsigned long absDiff = abs( diff );

  #ifdef SHOW_LOG
      // log
      Serial.println("UpdatePosStraight: ");
      Serial.print("motor goal step = ");
      Serial.print( m_Motor.GetGoalStep() );
      Serial.print(", motor curr step = ");
      Serial.println( m_Motor.GetCurrStep() );
      
      Serial.print("diff = ");
      Serial.print( diff );

      Serial.print("absDiff = ");
      Serial.print( absDiff );
      //===================================
  #endif
  
  // Calculate the target speed (with sign) for each motor
  long targetSpeed = sign( diff ) * GetMaxAbsSpeed(); // arduino "long" is 32 bit
  
  // Now we calculate a compensation factor. This factor depends on the acceleration of each motor (difference on speed we need to apply to each motor)
  // This factor was empirically tested (with a simulator) to reduce overshoots
  const int difS = m_Motor.GetCurrSpeed() - targetSpeed;
  
  const unsigned int diffspeed = abs( difS );

  #ifdef SHOW_LOG
      // log
      Serial.print("Motor curr speed = ");
      Serial.print( m_Motor.GetCurrSpeed() );
      Serial.print(", targetSpeed = ");
      Serial.println( targetSpeed );
      Serial.println( "" );
      //=====================================
  #endif
  
  float tmp = - (float)diffspeed / (2.0 * (float)GetMaxAbsSpeed());
        
  float speedfactor = 1.05 - tmp;  
  speedfactor = constrain( speedfactor, 0.0, 1.0 );
  

  // Set motor speeds. We apply the straight factor and the "acceleration compensation" speedfactor
  const int target_speed = GetMaxAbsSpeed() * speedfactor * speedfactor; // unsigned speed

  #ifdef SHOW_LOG
      // log
      Serial.print("Motor target speed = ");
      Serial.print( target_speed );
      Serial.print(", speedfactor = ");
      Serial.println( speedfactor );      
      Serial.println( "" );
      //=====================================
  #endif
  
  m_Motor.SetAbsGoalSpeed( target_speed ); // set m_AbsGoalSpeed
} // UpdatePosStraight

//=========================================================
void TurnTable::SetPosStraight( int deg )
{  
  #ifdef SHOW_LOG
      Serial.print("Current Pos: pos = ");
      Serial.print( m_Pos ); Serial.println("");
      Serial.print("SetPosStraight: deg = ");
      Serial.print( deg ); Serial.println("");
  #endif
      
  SetPosInternal( deg ); // set m_GoalStep for M1 & M2
  UpdatePosStraight(); // use algorithm to calculate goal speed and set m_AbsGoalSpeed for M1 & M2
} // SetPosStraight
