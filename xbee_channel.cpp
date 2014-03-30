#include <xbee.h>
#include "xbee_channel.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char XbeeChannel::model[] = "xbee5";
XbeeChannel::XbeeChannel(char *filename, unsigned int baud)
{
	port = new char[strlen(filename)+1];
	strcpy(filename, port);
	port[strlen(filename)] = '\0';
	baudrate = baud;
	timeout = 10;
}
XbeeChannel::XbeeChannel()
{
#ifdef WIN32
	port = new char[5];
	sprintf(port, "COM1");
#else
	port = NULL;
#endif
	baudrate = 57600;
	timeout = 10;
}
XbeeChannel::XbeeChannel(const XbeeChannel &rhs)
{
	port = new char[strlen(rhs.port)+1];
	strcpy(rhs.port, port);
	port[strlen(rhs.port)] = '\0';
	baudrate = rhs.baudrate;
	timeout = rhs.timeout;
}

XbeeChannel::~XbeeChannel()
{
	delete [] port;
}

void XbeeChannel::SetTimeout(unsigned int t)
{
	timeout = t;
}

bool XbeeChannel::Open()
{
#ifdef OSX
	system("ulimit -n 1024");
#endif	
	xbee_err ret;
	if((ret = xbee_setup(&xbee, model, port, baudrate)) != XBEE_ENONE)
	{
		printf("XbeeChannel::Open: xbee_setup: %d - %s\n", ret, xbee_errorToStr(ret));
		return false;
	}
	return true;
}

bool XbeeChannel::ReadData(unsigned int size, char *buffer)
{
	struct xbee_pkt *pkt;
	xbee_err ret;
	unsigned int i;
	if((ret = xbee_conRx(con, &pkt, NULL)) != XBEE_ENONE)
	{
		printf("XbeeChannel::ReadData: xbee_conRx: %s\n", xbee_errorToStr(ret));
		return false;
	}else{
		printf("XbeeChannel::ReadData: Recieved %d bytes\n", pkt->dataLen);
		for(i = 0; i < (unsigned int)(pkt->dataLen); ++i)
		{
			if(i < size)
			{
				buffer[i] = pkt->data[i];
			}else{
				return true;
			}
		}
	}
	for(i = i; i < size; ++i)
	{
		buffer[i] = '\0';
	}	
	return true;
}

bool XbeeChannel::ReadNext(unsigned int &size, char *buffer)
{
	struct xbee_pkt *pkt;
	xbee_err ret;
	for(unsigned int counter = 0; counter < timeout; ++counter)
	{
		if((ret = xbee_conRxWait(con, &pkt, NULL)) != XBEE_ENONE)
		{
			printf("XbeeChannel::ReadNext: ReadFailed\n");
		}else{
			printf("XbeeChannel::ReadNext: Recieved %d bytes\n", pkt->dataLen);
			buffer = new char[pkt->dataLen];
			memcpy(buffer, pkt->data, pkt->dataLen);
			size = pkt->dataLen;
			return true;
		}
	}
	printf("XbeeChannel::ReadNext: Timed out\n");
	return false;
}

bool XbeeChannel::WriteData(unsigned int size, char *buffer)
{
	xbee_err ret;
	unsigned char txRet;
	ret = xbee_conTx(con, &txRet, buffer, size);
	printf("XbeeChannel::WriteData: txRet: %d\n", txRet);
	if(ret){
		printf("XBeeChannel::WriteData: xbee_conTx: %d - %s\n", ret, xbee_errorToStr(ret));
		return false;	
	}
	return true;
}

bool XbeeChannel::ReadUntilByte(char delim, int *size, char *buffer)
{
	return false;
}

bool XbeeChannel::Close()
{
	xbee_err ret;
	if((ret = xbee_conEnd(con)) != XBEE_ENONE)
	{
		xbee_log(xbee, -1, "xbee_condEnd() returned: %d", ret);
		printf("XbeeChannel::Close: xbee_conEnd: %d\n", ret);
		return false;
	}
	xbee_shutdown(xbee);
	return true;
}
