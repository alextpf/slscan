#include "Configuration.h"

//================================================================
void SetPINS()
{
  pinMode(LED_PIN, OUTPUT);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  
#ifdef TWO_MOTOR  
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
#else 
  pinMode(Y_LEFT_STEP_PIN, OUTPUT);
  pinMode(Y_LEFT_DIR_PIN, OUTPUT);
  pinMode(Y_LEFT_ENABLE_PIN, OUTPUT);

  pinMode(Y_RIGHT_STEP_PIN, OUTPUT);
  pinMode(Y_RIGHT_DIR_PIN, OUTPUT);
  pinMode(Y_RIGHT_ENABLE_PIN, OUTPUT);
#endif

} //SetPINS
//================================================================
void SetTimerInterrupt()
{  
  // Use TIMER 1 for stepper motor X AXIS and Timer 3 for Y AXIS
  // STEPPER MOTORS INITIALIZATION
  // TIMER1 CTC MODE
  TCCR1B &= ~(1<<WGM13);
  TCCR1B |=  (1<<WGM12);
  TCCR1A &= ~(1<<WGM11); 
  TCCR1A &= ~(1<<WGM10);

  // output mode = 00 (disconnected)
  TCCR1A &= ~(3<<COM1A0); 
  TCCR1A &= ~(3<<COM1B0); 

  // Set the timer pre-scaler
  // Generally we use a divider of 8, resulting in a 2MHz timer on 16MHz CPU
  TCCR1B = (TCCR1B & ~(0x07<<CS10)) | (2<<CS10);

  OCR1A = ZERO_SPEED;   // Motor stopped
  TCNT1 = 0;

  // We use TIMER 3 for stepper motor Y AXIS 
  // STEPPER MOTORS INITIALIZATION
  // TIMER3 CTC MODE
  TCCR3B &= ~(1<<WGM13);
  TCCR3B |=  (1<<WGM12);
  TCCR3A &= ~(1<<WGM11); 
  TCCR3A &= ~(1<<WGM10);

  // output mode = 00 (disconnected)
  TCCR3A &= ~(3<<COM1A0); 
  TCCR3A &= ~(3<<COM1B0); 

  // Set the timer pre-scaler
  // Generally we use a divider of 8, resulting in a 2MHz timer on 16MHz CPU
  TCCR3B = (TCCR3B & ~(0x07<<CS10)) | (2<<CS10);

  OCR3A = ZERO_SPEED;   // Motor stopped
  TCNT3 = 0;

  delay(1000);
  TIMSK1 |= (1<<OCIE1A);  // Enable Timer1 interrupt
  TIMSK3 |= (1<<OCIE1A);  // Enable Timer1 interrupt
} // SetTimerInterrupt

//=========================================================
// TIMER 1 : STEPPER MOTOR SPEED CONTROL motor1
ISR(TIMER1_COMPA_vect)
{
  int8_t dir = TurnTable.GetM1().GetDir();
  if ( dir == 0 )
    return;

  SET(PORTF,0); // STEP X-AXIS

  long currStep = TurnTable.GetM1().GetCurrStep();
  TurnTable.GetM1().SetCurrStep( currStep + dir );
  
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop");  // Wait 2 cycles. With the other instruction and this we ensure a more than 1 microsenconds step pulse
  CLR(PORTF,0);
}

//====================================================================================================================
// TIMER 3 : STEPPER MOTOR SPEED CONTROL motor2
ISR(TIMER3_COMPA_vect)
{
  int8_t dir = TurnTable.GetM2().GetDir();
  if ( dir == 0 )
    return;

#ifdef TWO_MOTOR
  SET(PORTF,6); // STEP Y-AXIS
#else
  SET(PORTF,6); // STEP Y-AXIS (Y-left)
  SET(PORTL,3); // STEP Z-AXIS (Y-right)
#endif
  
  long currStep = TurnTable.GetM2().GetCurrStep();
  TurnTable.GetM2().SetCurrStep( currStep + dir );
  
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop");  // Wait 2 cycles. With the other instruction and this we ensure a more than 1 microsenconds step pulse
#ifdef TWO_MOTOR
  CLR(PORTF,6);
#else
  CLR(PORTF,6);
  CLR(PORTL,3);
#endif
}

//================================================================
// Test sequence to check mechanics, motor drivers...
void testMovements()
{
  String receivedString;
  
  if ( Serial.available() > 0 )
  {
    receivedString = Serial.readStringUntil('\n');
  }
  
  if ( receivedString.equals("1") )
  {
    TurnTable.SetPosStraight(ROBOT_MAX_X, ROBOT_MAX_Y); // upper right
  }
  else if ( receivedString.equals("2")  )
  {
    TurnTable.SetPosStraight(ROBOT_MIN_X, ROBOT_MAX_Y); // upper left
  }
  else if ( receivedString.equals("3") )
  {
    TurnTable.SetPosStraight(ROBOT_MIN_X, ROBOT_MIN_Y); // lower left
  }
  else if ( receivedString.equals("4") )
  {
    TurnTable.SetPosStraight(ROBOT_MAX_X, ROBOT_MIN_Y); // lower right
  }
  else if ( receivedString.equals("5") )
  {
    // x + 5
    RobotPos pos = TurnTable.GetTablePos();
    RobotPos d(5,0);
    RobotPos newPos = pos + d;
    TurnTable.SetPosStraight( newPos.m_X, newPos.m_Y );
  }
  else if ( receivedString.equals("6") )
  {
    // x - 5
    RobotPos pos = TurnTable.GetTablePos();
    RobotPos d(5,0);
    
    RobotPos newPos = pos - d;
    TurnTable.SetPosStraight( newPos.m_X, newPos.m_Y );
  }
  else if ( receivedString.equals("7") )
  {
    // y + 5
    RobotPos pos = TurnTable.GetTablePos();
    RobotPos d(0,5);
    
    RobotPos newPos = pos + d;
    TurnTable.SetPosStraight( newPos.m_X, newPos.m_Y );
  }
  else if ( receivedString.equals("8") )
  {
    // y - 5
    RobotPos pos = TurnTable.GetTablePos();
    RobotPos d(0,5);
    
    RobotPos newPos = pos - d;
    TurnTable.SetPosStraight( newPos.m_X, newPos.m_Y );
  }
  else if ( receivedString.equals("9") )
  {
    TurnTable.SetPosStraight( ROBOT_CENTER_X,ROBOT_MAX_Y );
  }
  else if ( receivedString.equals("10") )
  {
    TurnTable.SetPosStraight( ROBOT_CENTER_X,ROBOT_MIN_Y );
  }
  
}

