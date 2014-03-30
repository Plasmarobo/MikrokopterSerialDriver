#ifndef RAW_CHANNEL_H
#define RAW_CHANNEL_H

#include "serial_channel.hpp"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif



class AsyncSerial
{
	protected:
	HANDLE hComm;
	DCB dcb;
	DWORD event_mask;
	OVERLAPPED ov;
	
};
#endif
