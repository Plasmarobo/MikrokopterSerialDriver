#ifndef RAW_CHANNEL_H
#define RAW_CHANNEL_H

#include "serial_channel.hpp"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif



class RawChannel : public SerialChannel
{
	protected:
	#ifdef WIN32
	HANDLE hComm;
	DCB dcb;
	DWORD event_mask;
	OVERLAPPED ov;
	BOOL wait_read;
	
	#else
	int sp;
	struct termios tty;
	#endif
	
	unsigned int baud_rate;
	unsigned int timeout;
	char *port;
	
	public:
	RawChannel();
	RawChannel(char* p, unsigned int baud);
	~RawChannel();
	bool Open();
	bool Close();
	void SetTimeout(unsigned int time);
	bool ReadData(unsigned int size, char *buffer);
	bool WriteData(unsigned int size, const char *buffer);
	bool ReadUntilByte(char delim, int *size, char *buffer);
	bool ReadNext(unsigned int &size, char *buffer);
};


#endif
