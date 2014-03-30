#ifndef XBEE_CHANNEL_H
#define XBEE_CHANNEL_H

#include <xbee.h>
#include "serial_channel.hpp"

class XbeeChannel : public SerialChannel
{
	protected:
	struct xbee *xbee;
	struct xbee_con *con;
	char *port;
	static const char model[]; //= "xbee5";
	unsigned int baudrate;
	unsigned int timeout; //Timeout in seconds
	public:
	XbeeChannel(char *filename, unsigned int baud);
	XbeeChannel();
	XbeeChannel(const XbeeChannel &rhs);
	~XbeeChannel();
	bool Open();
	bool Close();
	void SetTimeout(unsigned int t);
	bool ReadData(unsigned int size, char *buffer);
	bool WriteData(unsigned int size, char *buffer);
	bool ReadUntilByte(char delim, int *size, char *buffer);
	bool ReadNext(unsigned int &size, char *buffer);
};

#endif
