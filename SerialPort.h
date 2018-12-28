////////////////////////////////////////////
// Original Author: Manash Kumar Mandal
// Modified by Alex Chen
////////////////////////////////////////////
#pragma once

#define ARDUINO_WAIT_TIME 2000
#define MAX_DATA_LENGTH 255

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

class SerialPort
{
public:

    SerialPort(char *portName);
    ~SerialPort();

    template <typename TYPE>
    int		ReadSerialPort( TYPE *buffer, unsigned int buf_size );

    template <typename TYPE>
    bool	WriteSerialPort( TYPE *buffer, unsigned int buf_size );
    bool	IsConnected();

private:
	HANDLE		m_Handle;
	bool		m_Connected;
	COMSTAT		m_Status;
	DWORD		m_Errors;
};
