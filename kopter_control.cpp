#include "kopter_control.hpp"
#include "raw_channel.hpp"
#include "serial_frame.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

SerialEvent::SerialEvent()
{
	event_type = SerialEventCodes::se_error;
	memset(buffer, 0, 256);
	size = 0;
}

SerialEvent::SerialEvent(int t, char *b, long s)
{
	event_type = t;
	memcpy(buffer, b, s);
	size = s;
}

bool WaitMutex(HANDLE mutex)
{
	DWORD result = WaitForSingleObject(mutex, INFINITE);
	switch(result)
	{
	case WAIT_OBJECT_0:
		return true;
	case WAIT_ABANDONED:
		return false;
	default:
		return false;
	}
}

DWORD WINAPI Read_Thread(LPVOID lpParam)
{
	Comm_Data *params = (Comm_Data*)lpParam;
	char c;
	char buffer[256];
	int readPtr;
	bool read;
	std::stringstream s;
#ifdef SERIAL_LOGGER_ON
	std::ofstream serial_log;
#endif
	char filename[32];
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(filename, 32, "serial_log_%S%M%H%d%m%Y.txt", timeinfo);
	serial_log.open(filename, std::ios::out);
	while(true)
	{
		c = 0;
		if(!WaitMutex(params->mutex)) continue;
		if(params->stop)
		{
			ReleaseMutex(params->mutex);
			return 0;
		}
		read = params->scp->ReadData(1, &c);
		ReleaseMutex(params->mutex);
		if(c == '#')
		{
			printf("\nIgnored:\n%s\n", s.str());
			s.str(std::string(""));
			ZeroMemory(buffer, 256);
			SerialEvent se(SerialEventCodes::se_read_start, buffer, 0);
			if(!WaitMutex(params->mutex)) continue;
			params->seqp->push(se);
			ReleaseMutex(params->mutex);
			readPtr = 0;
			buffer[readPtr++] = c;
#ifdef SERIAL_LOGGER_ON
			serial_log << c;
#endif
			while(c != '\r')
			{
				if(!WaitMutex(params->mutex)) continue;
				params->scp->ReadData(1,&c);
				ReleaseMutex(params->mutex);
#ifdef SERIAL_LOGGER_ON
				serial_log << c;
#endif
				buffer[readPtr++] = c;
			}
#ifdef SERIAL_LOGGER_ON
			serial_log << '\r';
#endif
			//Issue Event
			SerialEvent re(SerialEventCodes::se_read_end, buffer, readPtr);
			if(!WaitMutex(params->mutex)) continue;
			printf("\nSerialRead:\n#%s\n", buffer);
			params->seqp->push(re);
			ReleaseMutex(params->mutex);
		}
		else if (read)
		{
#ifdef SERIAL_LOGGER_ON
			serial_log << c;
#endif
			s << c;
		}
	}
#ifdef SERIAL_LOGGER_ON
	serial_log.close();
#endif
	return 0;
}

DWORD WINAPI Update_Thread(LPVOID lpParam)
{
	KopterController *k = (KopterController*) lpParam;
	while (!k->IsStopped())
	{
		k->RenewDebugSubscription();
		k->HandleSerialEvents();
	}
	return 0;
}
KopterController::KopterController(char *port)
{
	sc = new RawChannel(port, 57600);
	sc->Open();
	debug_ready = false;
	debug_rate = 0;
	debug_file.open("data.csv", std::ios::out | std::ios::trunc);
	//SerialFrame: Aquire Debug
	comm_data.scp = sc;
	comm_data.seqp = &(read_event_queue);
	comm_data.stop = false;
	thread_mutex = CreateMutex(NULL, FALSE, NULL);
	self_mutex = CreateMutex(NULL, FALSE, NULL);
	comm_data.mutex = thread_mutex;
	read_thread_handle = CreateThread(NULL, 0, Read_Thread, &comm_data, 0, &read_thread_id);
	update_thread_handle = CreateThread(NULL, 0, Update_Thread, this, 0, &update_thread_id);
	ZeroMemory(analog_debug_names, 32 * 17);
	if(read_thread_handle == NULL || update_thread_handle == NULL)
		printf("Error spawning child thread\n");
	debug_index = 0;
	waypoint_ptr = 0;
}

KopterController::~KopterController()
{
	WaitMutex(self_mutex);
	WaitMutex(thread_mutex);
	comm_data.stop = true;
	CloseHandle(read_thread_handle);
	CloseHandle(update_thread_handle);
	ReleaseMutex(thread_mutex);
	ReleaseMutex(self_mutex);
	CloseHandle(thread_mutex);
	CloseHandle(self_mutex);
	debug_file.close();
	sc->Close();
} 

bool KopterController::IsStopped()
{
	if (!WaitMutex(self_mutex))
		return false;
	bool result = false; 
	if (!WaitMutex(comm_data.mutex))
		result = comm_data.stop;
	ReleaseMutex(self_mutex);
	ReleaseMutex(comm_data.mutex);
	return result;
}

void KopterController::Init()
{
	unsigned char dat=0;
	short echo_pattern = 1024;
	char *buffer; unsigned int bytes_read=16;
	//SerialFrame sf(KopterCMD::testserial, KopterCMD::KopterAddr::base, 2, &echo_pattern);

	SerialFrame sf(KopterCMD::BaseCmd::version, KopterCMD::KopterAddr::base, 0, NULL);
	//WriteSerialFrame(sc, &sf);
	//Sleep(1000);
	EnqueueWrite(&sf);
	for(int i = 0; i < 32; ++i)
	{
		dat = i;
		sf.SetAddr(KopterCMD::nav);
		sf.SetID(KopterCMD::debuglabel);
		sf.SetData(1, &dat);
		EnqueueWrite(&sf);
	}
	
}

void KopterController::HandleSerialEvents()
{
	//Dispatch Writes
	if (WaitMutex(self_mutex))
	{

		while (!write_event_queue.empty())
		{
			SerialEvent e = write_event_queue.front();
			write_event_queue.pop();
			if (e.event_type == SerialEventCodes::se_write_start)
			{
				if (!sc->WriteData(e.size, e.buffer)){ write_event_queue.push(e); };
			}
			Sleep(10);
		}
		ReleaseMutex(self_mutex);
	}
	while (!read_event_queue.empty())
	{
		if (!WaitMutex(thread_mutex))
			return;
		SerialEvent e = read_event_queue.front();
		read_event_queue.pop();
		ReleaseMutex(thread_mutex);
		switch (e.event_type)
		{
		case SerialEventCodes::se_read_end:
		{
											  char *data = NULL;
											  SerialFrame sf(e.buffer, e.size);
											  data = sf.GetStringData();
											  switch (sf.GetID())
											  {
											  case KopterResp::BaseResp::debuglabel:
												  memcpy(&(analog_debug_names[data[0]][0]), &(data[1]), 16);
												  analog_debug_names[data[0]][16] = '\0';
												  printf("Debug Index %i recieved: %s\n", (unsigned char) data[0], analog_debug_names[data[0]]);
												  if (data[0] == 31)
													  debug_ready = true;
												  break;
											  case KopterResp::BaseResp::sentexternctl:
												  printf("ExternControl Confirmation: %c\n", data[0]);
												  break;
											  case KopterResp::BaseResp::display:
												  data[80] = '\0';
												  printf("Display Text Recieved: %s\n", data);
												  break;
											  case KopterResp::BaseResp::menu:
												  printf("Menu item %i/%i Recieved: %s\n", (unsigned int) data[0], (unsigned int) data[1], &(data[2]));
												  break;
											  case KopterResp::BaseResp::version:
												  /*version.SWMajor = (unsigned char) data[0];
												  version.SWMinor = (unsigned char) data[1];
												  version.ProtoMajor = (unsigned char) data[2];
												  version.ProtoMinor = (unsigned char) data[3];
												  version.SWPatch = (unsigned char) data[4];
												  for (int i = 0; i < 5; ++i)
													  version.HardwareError[i] = (unsigned char) data[5 + i];
													  */
												  version = *((VersionData*) data);
												  printf("Software: %i.%i\nProto: %i.%i\n Patch: %i\nError State: 0x%x\n", version.SWMajor, version.SWMinor, version.ProtoMajor, version.ProtoMinor, *((short*) version.HardwareError));
												  break;
											  case KopterResp::BaseResp::debug:
												  printf("DebugStructure:\n");
												  //ZeroMemory(&(debugdata), sizeof(DebugOut));
												  //debugdata = *((DebugOut*) data);
												  parseDebugOut(data);
												  debug_file << debug_index;
												  for (int i = 0; i < 32; ++i)
												  {
													  if (analog_debug_names[i][0] != '\0')
													  {
														  debug_file << "," << debugdata.Analog[i];
														  printf("%s: %i\n", analog_debug_names[i], debugdata.Analog[i]);
													  }
												  }
												  debug_file << std::endl;
												  debug_index++;
												  break;
											  case KopterResp::BaseResp::externctl:
												  ctrl.Digital[0] = (unsigned char) data[0];
												  ctrl.Digital[1] = (unsigned char) data[1];
												  ctrl.RemoteButtons = (unsigned char) data[2];
												  ctrl.Nick = data[3];
												  ctrl.Roll = data[4];
												  ctrl.Gier = data[5];
												  ctrl.Gas = (unsigned char) data[6];
												  ctrl.Height = data[7];
												  ctrl.free = data[8];
												  ctrl.Frame = data[9];
												  ctrl.Config = data[10];
												  break;
											  case KopterResp::FlightResp::compass:
												  break;
											  case KopterResp::FlightResp::enginetest:
												  //case KopterResp::NaviResp::time:
												  if (sf.GetAddr() == KopterCMD::KopterAddr::flight)
												  {
												  }
												  else if (sf.GetAddr() == KopterCMD::KopterAddr::nav)
												  {
												  }
												  break;
												  break;
											  case KopterResp::FlightResp::settings:
												  break;
											  case KopterResp::FlightResp::settingwrite:
												  break;
											  case KopterResp::FlightResp::ppmchannels:
												  break;
											  case KopterResp::FlightResp::data3d:
												  //case KopterResp::NaviResp::interval3d:
												  if (sf.GetAddr() == KopterCMD::KopterAddr::flight)
												  {
												  }
												  else if (sf.GetAddr() == KopterCMD::KopterAddr::nav)
												  {
												  }
												  break;
											  case KopterResp::FlightResp::mixer:
												  break;
											  case KopterResp::FlightResp::mixerwrite:
												  break;
											  case KopterResp::FlightResp::settingchange:
												  break;
											  case KopterResp::FlightResp::blparam:
												  break;
											  case KopterResp::FlightResp::blwrite:
												  //case KopterResp::NaviResp::waypointrec:
												  if (sf.GetAddr() == KopterCMD::KopterAddr::flight)
												  {
												  }
												  else if (sf.GetAddr() == KopterCMD::KopterAddr::nav)
												  {
												  }
												  break;
											  case KopterResp::NaviResp::serialtest:
											  {
																					   short recieved_pattern = *((short*) data);
																					   if (recieved_pattern != test_pattern)
																						   printf("BAD TEST PATTERN\n");
																					   else
																						   printf("Serial Test Successful\n");
											  }

												  break;
											  case KopterResp::NaviResp::errortest:
												  break;
											  case KopterResp::NaviResp::waypoint:
												  break;
											  case KopterResp::NaviResp::osd:
												  break;
											  case KopterResp::ncparam:
												  break;
											  case KopterResp::NaviResp::blstatus:
												  break;
											  default:
												  break;
											  }
											  delete[] data;
		}
			break;
		default:
			break;
		}
	}

}

int KopterController::parseSignedInt(char a, char b)
{
	int res = (int) ((b << 8) | a);
	if ((res&(1 << 15)) != 0)
		return -(res&(0xFFFF - 1)) ^ (0xFFFF - 1);
	else
		return res;

}


unsigned int KopterController::parseUnsignedInt(char a, char b)
{
	return (int) ((b << 8) | a);
}

void KopterController::parseDebugOut(char *buffer)
{
	debugdata.Status[0] = buffer[0];
	debugdata.Status[1] = buffer[1];
	for (int i = 0; i < 32; ++i)
	{
		debugdata.Analog[i] = parseSignedInt(buffer[2 + i * 2], buffer[3 + i * 2]);
	}
}

void KopterController::EnqueueWrite(SerialFrame *sf)
{
	unsigned int size;
	char *buffer = sf->GetEncodedBuffer(size);
	if(WaitMutex(self_mutex))
		write_event_queue.push(SerialEvent(SerialEventCodes::se_write_start,buffer,size));
	ReleaseMutex(self_mutex);
	delete [] buffer;
}


void KopterController::EngineTest(int ms, unsigned char *power)
{
	printf("Initializing Engine Test\n");
	SerialFrame sf(KopterCMD::enginetest, KopterCMD::flight, 16, power);
	//for(int i = 0; i < ms/10; ++i)
	//{
	EnqueueWrite(&sf);
	Sleep(ms);
	for(int i = 0; i < 16; ++i)
		power[i] = 0x0;
	sf.SetData(16, &power);
	EnqueueWrite(&sf);
	printf("Test Complete\n");
}

void KopterController::SerialTest(short test_p)
{
	test_pattern = test_p;
	SerialFrame sf(KopterCMD::testserial, KopterCMD::nav, 2, &test_pattern);
	EnqueueWrite(&sf);
	printf("Sent Test Pattern\n");
}

void KopterController::AssumeControl()
{
	ctrl.Config = 1;
	ctrl.Gas = 0x88;
	ctrl.Frame = 31;
	ctrl.Digital[0] = 0;
	ctrl.Digital[1] = 0;
	ctrl.RemoteButtons = 0;
	ctrl.Nick = 0;
	ctrl.Roll = 0;
	ctrl.Gier = 0;
	ctrl.Height = 1;
	ctrl.free = 0;
		
	SerialFrame sf(KopterCMD::sendexctl, KopterCMD::flight, sizeof(ExternControl), &ctrl);
	EnqueueWrite(&sf);
}

void KopterController::RelinquishControl()
{
	ctrl.Config = 0;
	ctrl.Gas = 0x0;
	ctrl.Digital[0] = 0;
	ctrl.Digital[1] = 1;
	ctrl.RemoteButtons = 0;
	ctrl.Nick = 0;
	ctrl.Roll = 0;
	ctrl.Gier = 0;
	ctrl.Height = 1;
	ctrl.free = 0;

	SerialFrame sf(KopterCMD::sendexctl, KopterCMD::flight, sizeof(ExternControl), &ctrl);
	EnqueueWrite(&sf);
}

void KopterController::GetMixer()
{
	SerialFrame sf(KopterCMD::mixer, KopterCMD::flight, 0, 0);
	EnqueueWrite(&sf);
}

void KopterController::GetBLParameter()
{
	//Stub
}

void KopterController::SetTarget(Waypoint target)
{
	//SerialFrame sf(KopterCMD::waypoint, KopterCMD::nav, sizeof(Waypoint), &target);
	//EnqueueWrite(&sf);
}

void KopterController::IssueDebugRequest(unsigned int ms)
{
	unsigned char send_interval = ms/10;
	debug_rate = send_interval;
	debug_index = 0;
	SerialFrame sf(KopterCMD::debugreq, KopterCMD::base, 1, &send_interval);
	while (!debug_ready)
	{
		Sleep(500);
	}
	EnqueueWrite(&sf);
	debug_file << "Index";
	for (int i = 0; i < 32; ++i)
	{
		debug_file << "," << analog_debug_names[i];
	}
	debug_file << std::endl;
	last_sub_time = time(NULL);
}
void KopterController::RenewDebugSubscription()
{
	if ((time(NULL) - last_sub_time > 4) && debug_ready)
	{
		SerialFrame sf(KopterCMD::debugreq, KopterCMD::base, 1, &debug_rate);
		EnqueueWrite(&sf);
		last_sub_time = time(NULL);
	}
}

void KopterController::UpdateNaviData()
{
	unsigned char interval = 0;
	SerialFrame sf(KopterCMD::osd,KopterCMD::nav, 1, &interval);
	EnqueueWrite(&sf);
}

void KopterController::UpdateParamset()
{
	unsigned char index = 1;
	SerialFrame sf(KopterCMD::osd, KopterCMD::flight, 1, &index);
	EnqueueWrite(&sf);
	
}

void KopterController::EnqueueWaypoint(Waypoint target)
{
	waypoints[waypoint_ptr++] = target;
	SerialFrame sf(KopterCMD::sendwaypoint, KopterCMD::waypoint, sizeof(Waypoint), &target);
	EnqueueWrite(&sf);
}

void KopterController::ClearWaypoints()
{
	//Stub
}

DebugOut *KopterController::GetDebugOut(unsigned int &size)
{
	size = 1;
	return &debugdata;
}

