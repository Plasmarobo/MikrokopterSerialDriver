#include "raw_channel.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <string.h>
#endif




RawChannel::RawChannel(char *p, unsigned int baud)
{
	int size = strlen(p)+1;
	port = new char[size];
	port[size] = '\0';
	strcpy(port, p);
#ifndef WIN32
	baud_rate = B57600;
#else
	dcb.BaudRate = baud;
	dcb.ByteSize = 8;
#endif
}

RawChannel::RawChannel()
{
	port = new char[5];
#ifndef WIN32
	baud_rate = B57600;
#endif
}

bool RawChannel::Open()
{
	#ifndef WIN32
		sp = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
		fcntl(sp, F_SETFL, 0);
		//fcntl(sp, F_SETFL, FNDELAY);
		if(sp < 0)
		{
			printf("RawChannel::Open: open(): %s: %d: %s\n", port, errno,  strerror(errno));
			return false;
		}
		if(tcgetattr(sp, &tty) != 0)
		{
			printf("RawChannel::Open: Tcgetattr: %d: %s\n", errno, strerror(errno));
			return false;
		}
		cfsetospeed(&tty, baud_rate);
		cfsetispeed(&tty, baud_rate);
		
		tty.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);

		tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
		tty.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK| INPCK| ISTRIP | IXON| IXOFF | IXANY);
		
		tty.c_cflag &= ~(PARENB | CSIZE);
		tty.c_cflag |= CS8;
		
		tty.c_cc[VMIN] = 0;
		tty.c_cc[VTIME] =50;
		if(tcsetattr(sp, TCSANOW, &tty) !=0)
		{
			printf("RawChannel::Open: tcsetattr: %d: %s\n", errno, strerror(errno));
			return false;
		}
		printf("RawChannel::Open: %s opened\n", port);
	#else
		hComm = ::CreateFile(port, GENERIC_READ|GENERIC_WRITE, 0,NULL,OPEN_EXISTING, 0, NULL);
		if(hComm == INVALID_HANDLE_VALUE)
		{
			printf("Error opening serial port: %s\n", port);
			return false;
		}
		if(!::GetCommState(hComm, &dcb))
		{
			printf("Could not setup comm port: %s\n", port);
			return false;
		}
		dcb.BaudRate = CBR_57600;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.fRtsControl = ONESTOPBIT;
		//dcb.EofChar = '\r';
		if(!::SetCommState(hComm, &dcb))
		{
			printf("Could not setup comm port: %s\n", port);
			return false;
		}
		COMMTIMEOUTS cto;
		cto.ReadIntervalTimeout = 100000;
		cto.ReadTotalTimeoutConstant = 100;
		cto.ReadTotalTimeoutMultiplier = 256;
		cto.WriteTotalTimeoutConstant = 0;
		cto.WriteTotalTimeoutMultiplier = 0;
		::SetCommTimeouts(hComm, &cto);
		return true;
	#endif
		return true;
}

RawChannel::~RawChannel()
{
	delete [] port;
}

bool RawChannel::Close()
{
#ifndef WIN32
	close(sp);
#else
	::CloseHandle(hComm);
#endif
	return true;
}

void RawChannel::SetTimeout(unsigned int time)
{
	timeout = time;
}
bool RawChannel::ReadData(unsigned int size, char *buffer)
{
#ifndef WIN32
	int d = read(sp, buffer, size);	
	if(d == -1)
	{
		//printf("RawChannel::ReadData: %d: %s\n", errno, strerror(errno));
		return false;
	}
	return d > 0;
#else
	DWORD read_bytes = 1;
	ReadFile(hComm, buffer, size, &read_bytes, NULL);
	return read_bytes > 0;
#endif
}

bool RawChannel::WriteData(unsigned int size, const char *buffer)
{
#ifndef WIN32
	int d = write(sp, buffer, size);
	return d == size;
#else
	DWORD written_bytes = 0;
	if(!WriteFile(hComm, buffer, size, &written_bytes, NULL))
	{
		printf( "RawChannel::WriteData:Failed, Reason:%d\n",GetLastError());
		return false;
	}else
		return true;
#endif
}

bool RawChannel::ReadUntilByte(char delim, int *size, char *buffer)
{
	return false;
}

bool RawChannel::ReadNext(unsigned int &size, char *buffer)
{
#ifndef WIN32
	int d = read(sp, buffer, size);
	if(d == -1)
	{
		//printf("RawChannel::ReadNext: %d: %s\n", errno, strerror(errno));
		return false;
	}
	size = d;
	return size > 0;
#else
	DWORD read_bytes = 0;
	ReadFile(hComm, buffer, size, &read_bytes, NULL);
	size = read_bytes;
	return read_bytes > 0;
#endif
}

