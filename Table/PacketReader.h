//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////
#ifndef PACKET_READER_H
#define PACKET_READER_H

#include "Arduino.h"
#include "Point2D.h"

typedef Point2D<int> Point2I;   // 16 bit
typedef Point2D<long> Point2L;  // 32 bit
typedef Point2I RobotPos;       // alias

class PacketReader
{
public:
    PacketReader();
    bool      ReadPacket();
    uint16_t  ExtractParamInt(uint8_t pos);
    
    RobotPos  GetDesiredBotPos()
    {
      return m_DesiredBotPos;
    }
    
    RobotPos  GetDetectedBotPos()
    {
      return m_DetectedBotPos;
    }
    
    int       GetDesiredXMotorSpeed()
    {
      return m_DesiredXMotorSpeed;
    }
    
    int       GetDesiredYMotorSpeed()
    {
      return m_DesiredYMotorSpeed;
    }
    
    void showNewData();
    
private:

    byte      m_Buffer[14];    
    bool      m_IsPacketRead;
    RobotPos  m_DesiredBotPos;
    RobotPos  m_DetectedBotPos;
    int       m_DesiredXMotorSpeed;
    int       m_DesiredYMotorSpeed;
}; // PacketReader
#endif
