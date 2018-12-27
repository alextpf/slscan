//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////
#ifndef PACKET_READER_H
#define PACKET_READER_H

#include "Arduino.h"

class PacketReader
{
public:
    PacketReader();
    bool      ReadPacket();
    uint16_t  ExtractParamInt(uint8_t pos);
    
    int  GetDesiredTablePos()
    {
      return m_DesiredTablePos;
    }
    
    int       GetDesiredMotorSpeed()
    {
      return m_DesiredMotorSpeed;
    }
    
    void showNewData();
    
private:

    byte      m_Buffer[4];    
    bool      m_IsPacketRead;
    int       m_DesiredTablePos; // deg
    int       m_DesiredMotorSpeed;
}; // PacketReader
#endif
