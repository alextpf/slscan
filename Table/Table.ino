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
      Serial.print("Motor1 step = ");
      Serial.print( m1s );
      Serial.print(", Motor2 step = ");
      Serial.println( m2s ); Serial.println("");
      //=============================================
  #endif

  TurnTable.GetMotor().SetCurrStep( ms ); // this sets m_CurrStep for Motor1 & Motor2

  TurnTable.SetMaxAbsSpeed( MAX_ABS_SPEED );
  TurnTable.SetMaxAbsAccel( MAX_ABS_ACCEL );

  TurnTable.SetPosStraight( 0 );

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
      Serial.print( reader.GetDesiredTablePos().m_X );
      Serial.print(' ');
      Serial.print( reader.GetDesiredTablePos().m_Y );
      Serial.print(' ');

    #endif

      {
        TurnTable.SetMaxAbsSpeed( reader.GetDesiredMotorSpeed() );
        TurnTable.SetPosStraight( reader.GetDesiredTablePos() );
      }
    }

    TurnTable.Update(); // internally update

    if ( TurnTable.GetLoopCounter() % 10 == 0 )
    {
      TurnTable.UpdatePosStraight();  // update straight line motion algorithm
    }
  }
}
