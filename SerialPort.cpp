////////////////////////////////////////////
// Original Author: Manash Kumar Mandal
// Modified by Alex Chen
////////////////////////////////////////////

#include "SerialPort.h"

SerialPort::SerialPort(char *portName)
{
    m_Connected = false;

	m_Handle = CreateFileA( static_cast<LPCSTR>( portName ),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

	if ( m_Handle == INVALID_HANDLE_VALUE )
	{
		if ( GetLastError() == ERROR_FILE_NOT_FOUND )
		{
			printf( "ERROR: Handle was not attached. Reason: %s not available\n", portName );
		}
		else
		{
			printf( "ERROR!!!" );
		}
	}
	else
	{
		DCB dcbSerialParameters = { 0 };

		if ( !GetCommState( m_Handle, &dcbSerialParameters ) )
		{
			printf( "failed to get current serial parameters" );
		}
		else
		{
			dcbSerialParameters.BaudRate = CBR_115200;
			dcbSerialParameters.ByteSize = 8;
			dcbSerialParameters.StopBits = ONESTOPBIT;
			dcbSerialParameters.Parity = NOPARITY;
			dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

			if ( !SetCommState( m_Handle, &dcbSerialParameters ) )
			{
				printf( "ALERT: could not set Serial port parameters\n" );
			}
			else
			{
				m_Connected = true;
				PurgeComm( m_Handle, PURGE_RXCLEAR | PURGE_TXCLEAR );
				Sleep( ARDUINO_WAIT_TIME );
			}
		}
	}
} // SerialPort::SerialPort(char *portName)

//=======================================================================
SerialPort::~SerialPort()
{
	if ( m_Connected )
	{
		m_Connected = false;
		CloseHandle( m_Handle );
	}
} // SerialPort::~SerialPort()

//=======================================================================
template <typename TYPE>
int SerialPort::ReadSerialPort( TYPE *buffer, unsigned int buf_size )
{
	DWORD bytesRead;
	unsigned int toRead = 0;

	ClearCommError( m_Handle, &m_Errors, &m_Status );

	if ( m_Status.cbInQue > 0 )
	{
		if ( m_Status.cbInQue > buf_size )
		{
			toRead = buf_size;
		}
		else
		{
			toRead = m_Status.cbInQue;
		}
	}

	if ( ReadFile( m_Handle, buffer, toRead, &bytesRead, NULL ) )
	{
		return bytesRead;
	}

	return 0;
} // ReadSerialPort

//=======================================================================
template <typename TYPE>
bool SerialPort::WriteSerialPort( TYPE *buffer, unsigned int buf_size )
{
	DWORD bytesSend;

	if ( !WriteFile( m_Handle, (void*)buffer, buf_size, &bytesSend, 0 ) )
	{
		ClearCommError( m_Handle, &m_Errors, &m_Status );
		return false;
	}

	return true;

} // WriteSerialPort

//=======================================================================
bool SerialPort::IsConnected()
{
	return m_Connected;
}

template bool SerialPort::WriteSerialPort<BYTE>( BYTE *buffer, unsigned int buf_size );
template bool SerialPort::WriteSerialPort<char>( char *buffer, unsigned int buf_size );

template int SerialPort::ReadSerialPort<BYTE>( BYTE *buffer, unsigned int buf_size );
template int SerialPort::ReadSerialPort<char>( char *buffer, unsigned int buf_size );