//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////
#ifndef TurnTable_H
#define TurnTable_H

#include "Motor.h"
#include "Arduino.h"

class TurnTable
{
public:
  
  TurnTable(); // ctor
  ~TurnTable(); // dtor

  //==================================================================================================================
  // @brief set m_GoalStep for Motor
  //==================================================================================================================
  void SetPosInternal( int deg ); // in degree

  //==================================================================================================================
  void SetPosStraight( int deg ); // in degree

  //==================================================================================================================
  // @brief use algorithm to calculate goal speed and set m_AbsGoalSpeed for M1 & M2
  //==================================================================================================================
  void UpdatePosStraight();
  
  //==================================================================================================================
  // @brief 
  // 1. update accel by: the faster the current speed, the higher the accel
  // 2. based on the target position, compute a "stopping position", based on which, decite whether we increase
  //    or decrease current speed. The amount of in/decrement is by accel
  // 3. based on speed, determine the update period. 
  //==================================================================================================================
	void Update(); // aka positionControl()

  //==================================================================================================================
  Motor& GetMotor()
  {
    return m_Motor;
  }

  void ResetMotor()
  {
    m_Motor.SetCurrStep(0);
  }
  
  void SetTablePos(const int deg)
  {
    m_Pos = deg;
  }
  
  int GetTablePos() const // in deg
  {
    return m_Pos;
  }
  
  void SetMaxAbsSpeed( int s )
  {
     m_Motor.SetMaxAbsSpeed(s);
  }
  
  int GetMaxAbsSpeed()
  {
    return m_Motor.GetMaxAbsSpeed(); // assuming max speed for 2 motors are the same
  }
  
  void SetMaxAbsAccel( int accel )
  {
    m_Motor.SetMaxAbsAccel( accel );
  }  
  
  int GetMaxAbsAccel()
  {
    return m_Motor.GetMaxAbsAccel(); // assuming max accel for 2 motors are the same
  }

  void SetTime( long t )
  {
    m_Time = t;
  }

  unsigned long GetLoopCounter()
  {
    return m_LoopCounter; 
  }

  int GetIsGoalCounter()
  {
    return m_IsGoalCounter;
  }
  
  // utility function
  static int MotorStepToTurnTablePos( long motorStep ); // in deg
  static long TurnTablePosToMotorStep(const int pos/*deg*/);
private:  

  int     m_Pos; // current, in degree
  int     m_GoalPos; // goal ,in degree
	Motor   m_Motor; 
	uint32_t m_Time; // time stamp, in micro sec. 
  unsigned long m_LoopCounter; 
  int     m_IsGoalCounter;
};
#endif
