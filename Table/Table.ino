//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////
// [note]: How to debug:
// - define SHOW_LOG in configuration.h
// - comment out "if( reader.ReadPacket() )" and enable "testmode"
// - only print out few iterations, ex: add "&& TurnTable.GetLoopCounter()< 20"

#include "Configuration.h"
#include "TurnTable.h"
#include "PacketReader.h"

long curr_time;                 // used in main loop
long prev_time;
bool testmode = false;
//
TurnTable TurnTable;
PacketReader reader;

//#define DEBUG_PACKET_READER
void setup()
{
  Serial.begin(BAUD_RATE);
  delay(5000);

  SetPINS();

  // Enable Motors
  digitalWrite(X_ENABLE_PIN    , LOW);

#ifdef TWO_MOTOR
  digitalWrite(Y_ENABLE_PIN    , LOW);
#else
  digitalWrite(Y_LEFT_ENABLE_PIN    , LOW);
  digitalWrite(Y_RIGHT_ENABLE_PIN    , LOW);
#endif

  SetTimerInterrupt();

  // init TurnTable params
  RobotPos initPos( ROBOT_INITIAL_POSITION_X, ROBOT_INITIAL_POSITION_Y ); // mm
  TurnTable.SetTablePos( initPos );

  #ifdef SHOW_LOG
      // log
      Serial.println("AidenBot init:");
      Serial.print("Init pos: x = ");
      Serial.print(ROBOT_INITIAL_POSITION_X);
      Serial.print(", y = ");
      Serial.println(ROBOT_INITIAL_POSITION_Y);
      //=============================================
  #endif

  long m1s, m2s;
  TurnTable::TurnTablePosToMotorStep(initPos, m1s, m2s);

  #ifdef SHOW_LOG
      // log
      Serial.print("Motor1 step = ");
      Serial.print( m1s );
      Serial.print(", Motor2 step = ");
      Serial.println( m2s ); Serial.println("");
      //=============================================
  #endif

  TurnTable.GetM1().SetCurrStep( m1s ); // this sets m_CurrStep for Motor1 & Motor2
  TurnTable.GetM2().SetCurrStep( m2s );

  TurnTable.  int GetMaxAbsAccel()
SetMaxAbsSpeed( MAX_X_ABS_SPEED );
  TurnTable.SetYMaxAbsSpeed( MAX_Y_ABS_SPEED );
  TurnTable.SetMaxAbsAccel( MAX_X_ABS_ACCEL );
  TurnTable.SetYMaxAbsAccel( MAX_Y_ABS_ACCEL );

  TurnTable.SetPosStraight( ROBOT_CENTER_X, ROBOT_INITIAL_POSITION_Y ); // this sets m_GoalStep, and internally set m_AbsGoalSpeed for M1 & M2

  prev_time = micros();
  TurnTable.SetTime( prev_time );
}

void loop()
{
  curr_time = micros();
  if ( curr_time - prev_time >= 1000  /*&& TurnTable.GetLoopCounter()< 20*/ )  // 1Khz loop
  {
    prev_time = curr_time; // update time

    if (testmode)
    {
      testMovements();
    }

    // there's new data coming
    if( reader.ReadPacket() )
    //if(false)
    {
    #ifdef SHOW_LOG
      //reader.showNewData();
      Serial.print( reader.GetDesiredBotPos().m_X );
      Serial.print(' ');
      Serial.print( reader.GetDesiredBotPos().m_Y );
      Serial.print(' ');

      Serial.print( reader.GetDetectedBotPos().m_X );
      Serial.print(' ');
      Serial.print( reader.GetDetectedBotPos().m_Y );
      Serial.print(' ');

      Serial.print( reader.GetDesiredYMotorSpeed() );
      Serial.println();
    #endif

      const RobotPos detectedBotPos = reader.GetDetectedBotPos();

      // if we do missing step correction, don't SetPosStraight
      if( detectedBotPos.m_X >= 0 && detectedBotPos.m_Y >= 0 )
      {
        long m1s, m2s;
        TurnTable::TurnTablePosToMotorStep(detectedBotPos, m1s, m2s);

  #ifdef SHOW_LOG
        // log
        Serial.print("missing step correction: Motor1 step = ");
        Serial.print( m1s );
        Serial.print(", Motor2 step = ");
        Serial.println( m2s ); Serial.println("");
        //=============================================
  #endif
        // TurnTable.SetTablePos( detectedBotPos ); // this is not needed becuase Robot pos is later overwrite in Update(), by converting motor steps to robot pos
        TurnTable.GetM1().SetCurrStep( m1s ); // this sets m_CurrStep for Motor1 & Motor2
        TurnTable.GetM2().SetCurrStep( m2s );
      }
      else
      {
        TurnTable.  int GetMaxAbsAccel()
SetMaxAbsSpeed( reader.GetDesiredXMotorSpeed() );
        TurnTable.SetYMaxAbsSpeed( reader.GetDesiredYMotorSpeed() );
        TurnTable.SetPosStraight( reader.GetDesiredBotPos().m_X, reader.GetDesiredBotPos().m_Y );
      }
    }

    TurnTable.Update(); // internally update

    if ( TurnTable.GetLoopCounter() % 10 == 0 )
    {
      TurnTable.UpdatePosStraight();  // update straight line motion algorithm
    }
  }
}
