#include "Configuration.h"

//================================================================
void SetPINS()
{
  pinMode(LED_PIN, OUTPUT);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  

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
  int8_t dir = TurnTable.GetMotor().GetDir();
  if ( dir == 0 )
    return;

  SET(PORTF,0); // STEP X-AXIS

  long currStep = TurnTable.GetMotor().GetCurrStep();
  TurnTable.GetMotor().SetCurrStep( currStep + dir );
  
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
    TurnTable.SetPosStraight(TABLE_MAX_DEG); // upper right
  }
  else if ( receivedString.equals("2")  )
  {
    TurnTable.SetPosStraight(TABLE_MIN_DEG); // upper left
  }
  else if ( receivedString.equals("3") )
  {
    // x + 30
    int pos = TurnTable.GetTablePos();
    int newPos = pos + 30;
    TurnTable.SetPosStraight( newPos);
  }
  else if ( receivedString.equals("4") )
  {
    // x - 30
    int pos = TurnTable.GetTablePos();
    int newPos = pos - 30;
    TurnTable.SetPosStraight( newPos);
  }
  else if ( receivedString.equals("5") )
  {
    // x + 5
    int pos = TurnTable.GetTablePos();
    int newPos = pos + 5;
    TurnTable.SetPosStraight( newPos);
  }
  else if ( receivedString.equals("6") )
  {
    // x - 5
    int pos = TurnTable.GetTablePos();
    int newPos = pos - 5;
    TurnTable.SetPosStraight( newPos);
  }
  
}
