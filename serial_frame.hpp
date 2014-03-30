#ifndef SERIAL_FRAME_H
#define SERIAL_FRAME_H
#include "serial_channel.hpp"
#include <string>
class SerialFrame;

//Addresses
namespace KopterCMD
{
	enum KopterAddr
	{
		base = 'a',
		flight = 'b',
		nav = 'c',
		mk3 = 'd',
		bl = 'f'
	};
	enum BaseCmd
	{
		debuglabel = 'a',
		sendexctl = 'b',
		requestdisp = 'h',
		requestmenu = 'l',
		version = 'v',
		debugreq = 'd',
		reset = 'R',
		exctl = 'g'
	};
	enum FlightCmd
	{
		sendcompass = 'K',
		enginetest = 't',
		settingsreq = 'q',
		sendsettings = 's',
		ppmchannels = 'p',
		interval3d = 'c',
		mixer = 'n',
		sendmixer = 'm',
		changesetting = 'f',
		serialpoti = 'y',
		blparam = 'u',
		sendblparam = 'w',
	};
	enum NaviCmd
	{
		testserial = 'z',
		errortext = 'e',
		sendtarget = 's',
		sendwaypoint = 'w',
		waypoint = 'x',
		osd = 'o',
		senduartredir = 'u',
		send3dinterval = 'c',
		ncparam = 'j',
		blstatus = 'k',
		time = 't'
	};
	enum Mk3Cmd
	{
		heading = 'w'
	};
};
namespace KopterResp
{
	enum SlaveAddr
	{
		fcslave = 1,
		ncslave,
		mk3slave
	};
	enum BaseResp
	{
		debuglabel = 'A',
		sentexternctl = 'B',
		display = 'H',
		menu = 'L',
		version = 'V',
		debug = 'D',
		externctl = 'G'
	};
	enum FlightResp
	{
		compass = 'k',
		enginetest = 'T',
		settings = 'Q',
		settingwrite = 'S',
		ppmchannels = 'P',
		data3d = 'C',
		mixer = 'N',
		mixerwrite = 'M',
		settingchange = 'F',
		blparam = 'U',
		blwrite = 'W'
	};
	enum NaviResp
	{
		serialtest = 'Z',
		errortest = 'E',
		waypointrec = 'W',
		waypoint = 'X',
		osd = 'O',
		interval3d = 'C',
		ncparam = 'J',
		blstatus = 'K',
		time = 'T'
	};
	enum Mk3Resp
	{
		heading = 'K'
	};
};

bool WriteSerialFrame(SerialChannel *sc, SerialFrame *sf);

class SerialFrame
{
	protected:
	char id;
	char addr;
	unsigned int data_size;	
	char data[256];
	unsigned int encoded_size;
	char encoded_data[256];
	bool is_raw;

	void Decode(unsigned int limit);
	void Encode();
	bool CheckCRC(char* buffer, int crcPtr);
	void SetCRC(char *buffer, int writePtr);
	
	public:
	SerialFrame();
	SerialFrame(const SerialFrame &rhs);
	SerialFrame(char* buffer, int size);
	SerialFrame(std::string str);
	SerialFrame(char i, char a,unsigned int ds, void *d);
	~SerialFrame();
	
	void *GetData(unsigned int &size);
	char *GetStringData();
	void SetData(unsigned int size, void *d);
	char *GetEncodedBuffer(unsigned int &size);

	char GetAddr();
	void SetAddr(char a);

	char GetID();
	void SetID(char i);

	bool IsRaw();


};

#endif
