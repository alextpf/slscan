#include "PacketReader.h"
#include "Configuration.h"

//#define DEBUG_SERIAL
//==========================================================================
PacketReader::PacketReader()
    : m_DesiredMotorSpeed( MAX_ABS_SPEED )
    , m_IsPacketRead( false )
{}

//==========================================================================
bool PacketReader::ReadPacket()
{
    const byte numBytes = 14; // 12 byte data + 2 bytes sync markers
    static bool inSync = false; // true: ready; false: not ready
    static byte counter = 0;
    
    byte startMarker = 0x7F;

    byte tmp;
    bool ret = false;
    
    if ( Serial.available() > 0 )
    {
        // We rotate the Buffer (we could implement a ring buffer in future)
        for( int i = numBytes - 1 ; i > 0 ; i-- )
        {
            m_Buffer[i] = m_Buffer[i - 1];
        }
#ifdef DEBUG_SERIAL        
        //debug
        int i = Serial.available();
        Serial.print("# available=");
        Serial.println(i);
#endif
        
        m_Buffer[0] = Serial.read();

#ifdef DEBUG_SERIAL        
        Serial.print("m_Buffer[0] = ");
        Serial.println(m_Buffer[0]);
#endif
        
        // We look for a  message start like "AA" to sync packets
        if( ( m_Buffer[0] == startMarker ) && ( m_Buffer[1] == startMarker ) )
        {
            if( inSync )
            {
                Serial.println( "S ERR" );
            }
            
            inSync = true;
            counter = numBytes-2;
        }
        else if( inSync )
        {
#ifdef DEBUG_SERIAL          
            Serial.print("counter=");
            Serial.println(counter);
#endif
            
            counter--;   // Until we complete the packet
            
            if( counter <= 0 )   // packet complete!!
            {
#ifdef DEBUG_SERIAL              
                Serial.println("done");
#endif                
              
                // Extract parameters
                m_DesiredTablePos = ExtractParamInt( 2 );
                m_DesiredMotorSpeed = ExtractParamInt( 0 );
                
                inSync = false;
                m_IsPacketRead = true;

                ret = true;
            }
        }
    }    
    return ret;
} // ReadPacket

//==========================================================================
uint16_t PacketReader::ExtractParamInt(uint8_t pos)
{
  union{
    byte Buff[2];
    uint16_t d;
  }
  u;

  u.Buff[0] = m_Buffer[pos];
  u.Buff[1] = m_Buffer[pos+1];
  
  return (u.d); 
} // ExtractParamInt

void PacketReader::showNewData() 
{
    if (m_IsPacketRead == true) 
    {
        Serial.print("This just in (HEX values)... ");
        for (byte n = 0; n < 12; n++) {
            Serial.print(m_Buffer[n], HEX);
            Serial.print(' ');
        }
        Serial.println();
        m_IsPacketRead = false;
    }
}//showNewData
