#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <Windows.h>

#include "raw_channel.hpp"
#include "kopter_control.hpp"

int main(int argc, const char* argv[])
{
	char *port = NULL;
	char *filename = NULL;
	unsigned int baud = 57600;
	unsigned int timeout = 5;
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-p") == 0)
		{
			++i;
			port = new char[strlen(argv[i]) + 1];
			memcpy(port, argv[i], strlen(argv[i]));
			port[strlen(argv[i])] = '\0';
		}
		if (strcmp(argv[i], "-b") == 0)
		{
			++i;
			baud = atoi(argv[i]);
		}
		if (strcmp(argv[i], "-t") == 0)
		{
			++i;
			timeout = atoi(argv[i]);
		}
		if (strcmp(argv[i], "-f") == 0)
		{
			++i;
			filename = new char[strlen(argv[i]) + 1];
			memcpy(filename, argv[i], strlen(argv[i]));
			filename[strlen(argv[i])] = '\0';
		}
	}
	if (port == NULL)
	{
		printf("No port given, please specify a serial port\n");
		return -1;
	}
	printf("Attempting use of %s with baud = %d\n", port, baud);

	std::ifstream playback;
	playback.open(filename, std::ios::in);
	if (!playback.is_open())
	{
		printf("Could not find file\nPress Enter to Exit\n");
		std::cin.get();
		return 0;
	}
	std::string buffer;
	int size = 0;
	char c;
	SerialChannel *sc = new RawChannel(port, 57600);
	sc->Open();
	printf("Press Enter to start playback of %s\n", filename);
	std::cin.get();
	while (!playback.eof())
	{
		std::getline(playback, buffer, '\r');
		if (buffer.length() > 3)
		{
			if (buffer.substr(3) == "#cD")
			{
				printf("Sending %s \n", buffer.c_str());
				sc->WriteData(buffer.length(), buffer.c_str());
				Sleep(1000);
			}
			else{
				sc->WriteData(buffer.length(), buffer.c_str());
				Sleep(500);
			}
		}

	}
	sc->Close();


	return 0;
}
