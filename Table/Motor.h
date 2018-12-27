//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////
#ifndef MOTOR_H
#define MOTOR_H

#include "Arduino.h"

class Motor
{
public:
  enum MOTOR_NUM {M1,M2};
  
  Motor();
  ~Motor();
  
  void SetCurrStep( long step )
  {
  	m_CurrStep = step;
  }

  long GetCurrStep() const
  {
  	return m_CurrStep;
  }

  void SetGoalStep( long step )
  {
  	m_GoalStep = step;
  }

  long GetGoalStep() const
  {
  	return m_GoalStep;
  }

  void SetCurrSpeed( int speed )
  {
    m_CurrSpeed = speed;
  }

  int GetCurrSpeed() const
  {
    return m_CurrSpeed;
  }

  void SetAbsGoalSpeed( int speed )
  {
    m_AbsGoalSpeed = speed;
  }

  int GetAbsGoalSpeed() const
  {
    return m_AbsGoalSpeed;
  }
  
  void SetAbsAccel( int accel )
  {
    m_AbsAccel = accel;
  }

  int GetAbsAccel() const
  {
    return m_AbsAccel;
  }
  
  void SetMaxAbsAccel( int accel )
  {
    m_MaxAbsAccel = accel;
  }

  int GetMaxAbsAccel()
  {
    return m_MaxAbsAccel;
  }

  int8_t GetDir() const
  {
    return m_Dir;
  }
  
  void SetDir( int8_t dir )
  {
    m_Dir = dir;
  }

  long GetPeriod() const
  {
    return m_Period;
  }
  
  void SetMaxAbsSpeed( int s )
  {
     m_MaxAbsSpeed = s;
  }
  
  int GetMaxAbsSpeed()
  {
    return m_MaxAbsSpeed;
  }
  
  void UpdateAccel();
  void UpdateSpeed( uint16_t dt, MOTOR_NUM m );
  
private:  
  void SetCurrSpeedInternal( uint16_t dt, int goalSpeed, MOTOR_NUM m );

  //////////////
  // Position
  //////////////
	long m_CurrStep;       // in motor steps. corresponds to position_M1/2
  long m_GoalStep;       // corresponds to target_position_M1/2

  //////////////
  // Direction
  //////////////
  int8_t m_Dir;         // dir = 1 movepositive, dir = -1 move negative, dir = 0 , no move

  ///////////////////
  // Speed & Accel
  ///////////////////
	int m_CurrSpeed;      // signed speed. corresponds to speed_M1/2
  int m_AbsGoalSpeed;      // signed speed. corresponds to target_speed_M1/2
  int m_MaxAbsSpeed;    // unsighed speed. corresponds to max_speed

  int m_AbsAccel;       // unsigned acceleration. corresponds to acceleration_M1/2
  int m_MaxAbsAccel;    // unsighed max acceleration
  
  long m_Period;        // for setting timer use. corresponds to timer_period. arduino "long" is 32 bit  
};

#endif
