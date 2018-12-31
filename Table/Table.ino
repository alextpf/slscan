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
  
  SetPINS();

  // Enable Motors
  digitalWrite(X_ENABLE_PIN    , LOW);

  SetTimerInterrupt();

  // init TurnTable params
  TurnTable.SetTablePos( 0 );

  #ifdef SHOW_LOG
      // log
      Serial.println("AidenBot init:");
      Serial.println("Init pos: x = 0");
      //=============================================
  #endif

  long ms = TurnTable::TurnTablePosToMotorStep(0);

  #ifdef SHOW_LOG
      // log
      Serial.print("Motor step = ");
      Serial.print( ms );Serial.println("");
      //=============================================
  #endif

  TurnTable.GetMotor().SetCurrStep( ms ); // this sets m_CurrStep for Motor1 & Motor2

  TurnTable.SetMaxAbsSpeed( MAX_ABS_SPEED );
  TurnTable.SetMaxAbsAccel( MAX_ABS_ACCEL );
#ifdef NO_STOP
  TurnTable.SetPosStraight( 10000 );
#else
  TurnTable.SetPosStraight( 0 );
#endif

  prev_time = micros();
  TurnTable.SetTime( prev_time );
}

void loop()
{
  curr_time = micros();
  if ( curr_time - prev_time >= 1000 )  // 1Khz loop //&& TurnTable.GetLoopCounter()< 20
  {
    prev_time = curr_time; // update time

    if (testmode)
    {
      testMovements();
    }
    
#ifdef NO_STOP
    if( TurnTable.GetTablePos()>8000 )
    {
      TurnTable.SetTablePos( 0 );
      TurnTable.ResetMotor();
    }
#else
    // there's new data coming
    if( reader.ReadPacket() )
    //if(false)
    {
      // every time a new command is read, reset step
      TurnTable.SetTablePos( 0 );
      TurnTable.ResetMotor();
      
    #ifdef SHOW_LOG
      //reader.showNewData();
      Serial.print( reader.GetDesiredTablePos());Serial.println();
    #endif
      {
        TurnTable.SetMaxAbsSpeed( reader.GetDesiredMotorSpeed() );
        TurnTable.SetPosStraight( reader.GetDesiredTablePos() );
      }
    }
#endif

    TurnTable.Update(); // internally update
    /*
    if (TurnTable.GetIsGoalCounter() == 0 )
    {
      Serial.println(1); // show degree
    }
    */
  }  
}
