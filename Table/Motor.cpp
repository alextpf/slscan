//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////
#include "Motor.h"
#include "Configuration.h"

extern int freeRam ();
extern int myAbs(int param);
extern long myAbs(long param);
extern int sign(int val);

//=========================================================
Motor::Motor()
: m_CurrStep( 0 )
, m_GoalStep( 0 )
, m_Dir( 0 )
, m_AbsAccel( 0 )
, m_CurrSpeed( 0 )
, m_AbsGoalSpeed( 0 )
, m_MaxAbsSpeed( 0 )
, m_MaxAbsAccel( 0 )
, m_Period( 0 )
{}

//=========================================================
Motor::~Motor()
{}

//=========================================================
void Motor::UpdateAccel()
{  
  m_AbsAccel = m_MaxAbsAccel;
  
  const unsigned int absSpeed = abs( m_CurrSpeed );

  if( absSpeed < SCURVE_LOW_SPEED )
  {
    m_AbsAccel = map( absSpeed, 0, SCURVE_LOW_SPEED, MIN_ACCEL, m_MaxAbsAccel ); 
  }

  #ifdef SHOW_LOG
      //log...
      Serial.println("Motor::UpdateAccel: ");
      Serial.print("absSpeed = ");
      Serial.println(absSpeed);
      Serial.print("m_AbsAccel = ");
      Serial.println(m_AbsAccel);
      Serial.println();
      //===========================
  #endif
  
} // UpdateAccel

//=========================================================
void Motor::UpdateSpeed( uint16_t dt )
{
  const int tmp = sign( m_CurrSpeed ) * ( (long)m_CurrSpeed * (long)m_CurrSpeed / ( STOP_COEF * (long)m_AbsAccel ) );
  const long stepsToGoal = m_GoalStep - m_CurrStep; // error term

  #ifdef SHOW_LOG
      Serial.println( "Motor::UpdateSpeed: " );
      Serial.print( "m_CurrStep= " );
      Serial.println( m_CurrStep );
      Serial.print( "stepsToGoal= " );
      Serial.println( stepsToGoal );
      Serial.print( "m_CurrSpeed= " );
      Serial.println( m_CurrSpeed );
      Serial.print( "tmp= " );
      Serial.println( tmp );
      Serial.println( "" );
  #endif
      
  int goalSpeed = 0;
  
  if( m_GoalStep > m_CurrStep ) // Positive move
  {
    #ifdef SHOW_LOG
                Serial.println( "m_GoalStep > m_CurrStep" );
                Serial.print( "m_GoalStep= " );
                Serial.println( m_GoalStep );
                Serial.print( "m_CurrStep= " );
                Serial.println( m_CurrStep );
    #endif
    
    // Start decelerating ?
    if ( tmp >= stepsToGoal )
    {          
      goalSpeed = 0;

      #ifdef SHOW_LOG
          Serial.println( "Postive move. tmp >= stepsToGoal: " );
          Serial.print( "goalSpeed = " );
          Serial.println( goalSpeed );
     #endif
     
    }
    else
    {          
      goalSpeed = m_AbsGoalSpeed;

      #ifdef SHOW_LOG
          //log...
          Serial.println( "Postive move. tmp < stepsToGoal" );
          Serial.print( "goalSpeed = " );
          Serial.println( goalSpeed );
          //================================
      #endif
      
    }        
  }
  else // negative move
  {
    #ifdef SHOW_LOG
                //log...  
                Serial.println( "m_GoalStep <= m_CurrStep" );
                Serial.print( "m_GoalStep= " );
                Serial.println( m_GoalStep );
                Serial.print( "m_CurrStep= " );
                Serial.println( m_CurrStep );
                //=============================
    #endif
            
    if ( tmp <= stepsToGoal )
    {              
      goalSpeed = 0;

      #ifdef SHOW_LOG
        //log...
        Serial.println( "Negative move. tmp <= stepsToGoal" );
        Serial.print( "goalSpeed = " );
        Serial.println( goalSpeed );
        //===========================
      #endif
    }
    else
    {        
      goalSpeed = -m_AbsGoalSpeed;

      #ifdef SHOW_LOG
        //log...
        Serial.println( "Negative move. tmp > stepsToGoal" );
        Serial.print( "goalSpeed = " );
        Serial.println( goalSpeed );
        //===========================
      #endif
    }
  }
  
  #ifdef SHOW_LOG
        Serial.print( "m_AbsGoalSpeed = " );
        Serial.println( m_AbsGoalSpeed );
  #endif
  
  SetCurrSpeedInternal( dt, goalSpeed );
} // UpdateSpeed

//=========================================================
void Motor::SetCurrSpeedInternal( uint16_t dt, int goalSpeed )
{
  goalSpeed = constrain( goalSpeed, -MAX_ABS_SPEED, MAX_ABS_SPEED );
  
  // We limit acceleration => speed ramp
  const int absAccel = ((long)m_AbsAccel * (long)dt) / 1000; // We divide by 1000 because dt are in microseconds

  const int speedDif = goalSpeed - m_CurrSpeed;
  
  #ifdef SHOW_LOG
      Serial.print( "dt = " );
      Serial.println( dt );
      Serial.print( "m_AbsAccel = " );
      Serial.println( m_AbsAccel );
      Serial.print( "absAccel = " );
      Serial.println( absAccel );
      Serial.print( "speedDif = " );
      Serial.println( speedDif );
      Serial.println("");
  #endif
        
  if ( speedDif > absAccel ) // We use long here to avoid overflow on the operation
  { 
    m_CurrSpeed += absAccel;
    
    #ifdef SHOW_LOG
      Serial.print( "m_CurrSpeed += absAccel= " );
      Serial.println( m_CurrSpeed );
      Serial.println("");
    #endif
  }
  else if ( speedDif < -absAccel )
  {
    m_CurrSpeed -= absAccel;
    
    #ifdef SHOW_LOG
      Serial.print( "m_CurrSpeed -= absAccel= " );
      Serial.println( m_CurrSpeed );
      Serial.println("");
    #endif
  }
  else
  {
    m_CurrSpeed = goalSpeed;

    #ifdef SHOW_LOG
        Serial.println( "SetCurrSpeedInternal: 3rd clause " );
        Serial.print( "m_CurrSpeed = " );
        Serial.println( m_CurrSpeed );
        Serial.println("");
    #endif    
  }  
  
  // Check if we need to change the direction pins
  if ( ( m_CurrSpeed == 0 ) && ( m_Dir != 0 ) )
  {
    m_Dir = 0;
  }
  else if ( ( m_CurrSpeed > 0 ) && ( m_Dir != 1 ) )
  {
    m_Dir = 1;
    SET(PORTF,1);
  }
  else if ( ( m_CurrSpeed < 0 ) && ( m_Dir != -1 ) )
  {
    m_Dir = -1;
    CLR(PORTF,1);
  }

  #ifdef SHOW_LOG
        Serial.print("m_CurrSpeed =  ");
        Serial.println(m_CurrSpeed);
        Serial.print("m_Dir =  ");
        Serial.println(m_Dir);
        Serial.println("");
  #endif
      
  if (m_CurrSpeed == 0)
  {
    m_Period = ZERO_SPEED;
  }
  else if (m_CurrSpeed > 0)
  {
    m_Period = 2000000 / m_CurrSpeed; // 2Mhz timer
  }
  else
  {
    m_Period = 2000000 / -m_CurrSpeed;
  }

  if (m_Period > 65535)   // Check for minimun speed (maximun period without overflow)
  {
    m_Period = ZERO_SPEED;
  }
      //log...
    //    Serial.print("m_Period =  ");
    //    Serial.println(m_Period);
      //===========================
  
  OCR1A = m_Period;
  // Check  if we need to reset the timer...
  if ( TCNT1 > OCR1A )
  {
    TCNT1 = 0;
  }
  
} // SetCurrSpeedInternal
