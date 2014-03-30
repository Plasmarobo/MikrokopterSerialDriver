#include "serial_channel.hpp"
#include "raw_channel.hpp"
#include "serial_frame.hpp"
#include <iostream>
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("No port specified, exiting\n");
		exit(0);
	}
	SerialChannel *r = new RawChannel(argv[1], 0);
	r->Open();
	char *d = new char[5];
	d[0] = 100;
	d[1] = 200;
	d[2] = 7;
	d[3] = 5;
	d[4] = 13;
	SerialFrame *s = new SerialFrame('a', FC_ADDR, 5, d);
	unsigned int size = 5;
	for(int i = 1; i < 600; ++i)
	{
		s->Send(r); 
		sleep(1);
	}
	r->Close();
	delete s;
	delete [] d;
	delete r;
}
