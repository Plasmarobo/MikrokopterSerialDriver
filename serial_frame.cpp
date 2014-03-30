#include "serial_frame.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

bool WriteSerialFrame(SerialChannel *sc, SerialFrame *sf)
{
	unsigned int sz;
	char *buffer;
	if (!(sf->IsRaw()))
		buffer = sf->GetEncodedBuffer(sz);
	bool res = sc->WriteData(sz, buffer);
	delete [] buffer;
	return res;
}
SerialFrame::SerialFrame()
{
	id = 0;
	addr = 0;
	data_size = 0;
	encoded_size = 0;
	is_raw = false;

}
SerialFrame::SerialFrame(const SerialFrame &rhs)
{
	id = rhs.id;
	addr = rhs.addr;
	SetData(rhs.data_size, data);
	encoded_size = 0;

}
SerialFrame::SerialFrame(std::string str)
{
	unsigned char i = 0;
	is_raw = true;
	char c = data[0];
	do
	{
		c = str[i];
		data[i++] = c;
	} while ((i < 256) && (c != '\r'));
}

SerialFrame::SerialFrame(char *buffer, int size)
{
	//printf("SerialFrame::Parsing\n");
	unsigned char crc1;
	unsigned char crc2;
	unsigned int readPtr = 0;
	unsigned int i;
	if(size >= 6)
	{	
		encoded_size = size-6;
		addr = buffer[1];
		id = buffer[2];
		for(i = 0; i < encoded_size; ++i)
		{
			encoded_data[i] = buffer[3+i];
		}
		if(!CheckCRC(buffer, 3+i))
		{
			printf("Failed CRC check\n");
		}
		Decode(size-3);
		//printf("Decoded data:\n%s\n", GetStringData());
	}
}

SerialFrame::SerialFrame(char i, char a, unsigned int ds, void *d)
{
	id = i;
	addr = a;
	SetData(ds, d);
	encoded_size = 0;

}
SerialFrame::~SerialFrame()
{
}

bool SerialFrame::CheckCRC(char *buffer, int crcPtr)
{
	unsigned int tmpCRC = 0;
	for(int i = 0; i < crcPtr; ++i)
	{
		tmpCRC += buffer[i];
	}
	tmpCRC %= 4096;
	char calc_A = '=' + (tmpCRC/64);
	char calc_B = '=' + (tmpCRC%64);
	return (calc_A == buffer[crcPtr]) && (calc_B == buffer[crcPtr+1]);


}

void SerialFrame::SetCRC(char *buffer, int writePtr)
{
	unsigned int tmpCRC = 0;
	for(int i = 0; i < writePtr; ++i)
	{
		tmpCRC += buffer[i];
	}
	tmpCRC %= 4096;
	buffer[writePtr] = '=' + (tmpCRC/64);
	buffer[writePtr+1] = '=' + (tmpCRC%64);
}


void SerialFrame::Decode(unsigned int limit)
{
	if(encoded_size == 0)
		return;
	//printf("SerialFrame::Decode: Starting\n");
	unsigned char a,b,c,d;
	unsigned char x,y,z;
	unsigned char ptrIn=0;
	unsigned char ptrOut=0;
	unsigned char len = encoded_size;
	while(len)
	{
		a = encoded_data[ptrIn++]-'=';
		b = encoded_data[ptrIn++]-'=';
		c = encoded_data[ptrIn++]-'=';
		d = encoded_data[ptrIn++]-'=';

		if(ptrIn > limit -2) break;

		x = (a << 2) | (b >> 4);
		y = ((b & 0x0f) << 4) | (c >> 2);
		z = ((c & 0x03) << 6) | d;

		if(len--) data[ptrOut++] = x; else break;
		if(len--) data[ptrOut++] = y; else break;
		if(len--) data[ptrOut++] = z; else break;
	}
	data_size = ptrOut;
	//printf("SerialFrame::Decode: Finished\n");
	//Data is now decoded in place and may be coerced into a structure
}
void SerialFrame::Encode()
{
	if(data_size == 0)
	{
		encoded_size = 0;
		return;
	}
	unsigned char len = data_size;
	unsigned char a, b, c;
	unsigned char ptrIn = 0;
	unsigned char ptrOut = 0;
	
	while(len)
	{
		if(len){a = data[ptrOut++];len--; }else a = 0;
		if(len){b = data[ptrOut++];len--; }else b = 0;
		if(len){c = data[ptrOut++];len--; }else c = 0;

		encoded_data[ptrIn++] = '=' + (a >> 2);
		encoded_data[ptrIn++] = '=' + (((a & 0x03) << 4) | ((b & 0xf0) >> 4));
		encoded_data[ptrIn++] = '=' + (((b & 0x0f) << 2) | ((c & 0xc0) >> 6));
		encoded_data[ptrIn++] = '=' + (c & 0x3f);
	}
	encoded_size = ptrIn;
}


void *SerialFrame::GetData(unsigned int &size)
{
	size = data_size;
	char *d = new char[data_size];
	memcpy(d, data, data_size);
	return (void*)d;
}

char *SerialFrame::GetStringData()
{
	char *d = new char[data_size+1];
	memcpy(d, data, data_size);
	d[data_size] = '\0';
	return d;
}

void SerialFrame::SetData(unsigned int size, void *d)
{
	data_size = size;
	if(size > 0)
		memcpy(data, d, size);
}

char *SerialFrame::GetEncodedBuffer(unsigned int &size)
{
	Encode();
	size = 6 + encoded_size;
	char *buffer = new char[size];
	buffer[0] = '#';
	buffer[1] = addr;
	buffer[2] = id;
	unsigned int i;
	for(i = 0; i < encoded_size; ++i)
	{
		buffer[3+i] = encoded_data[i];
	}
	SetCRC(buffer, 3+i);
	buffer[5+i] = '\r'; //buffer[5+i] = 13; 
	return buffer;
}

char SerialFrame::GetAddr()
{
	return addr;
}

void SerialFrame::SetAddr(char a)
{
	addr = a;
}

char SerialFrame::GetID()
{
	return id;
}

void SerialFrame::SetID(char i)
{
	id = i;
}

bool SerialFrame::IsRaw()
{
	return is_raw;
}