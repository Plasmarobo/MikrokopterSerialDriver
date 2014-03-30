#ifndef SERIAL_CHANNEL_H
#define SERIAL_CHANNEL_H
#include <stdio.h>
#include <stdlib.h>

class SerialChannel;

class SerialChannel
{
	public:
	bool virtual ReadData(unsigned int size, char *buffer)=0;
	bool virtual WriteData(unsigned int size,const char *buffer)=0;
	bool virtual ReadUntilByte(char delim, int *size, char *buffer)=0;	
	bool virtual ReadNext(unsigned int &size, char *buffer)=0;
	bool virtual Open()=0;
	bool virtual Close()=0;

};

#endif
