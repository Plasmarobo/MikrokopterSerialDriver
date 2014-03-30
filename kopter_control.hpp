#ifndef KOPTER_CONTROL_H
#define KOPTER_CONTROL_H

#include "serial_channel.hpp"
#include "serial_frame.hpp"
#include <queue>
#include <fstream>
#ifdef WIN32
#include <Windows.h>
#include <strsafe.h>
#endif

#define SERIAL_LOGGER_ON

struct SerialEvent;
class KopterController;

namespace SerialEventCodes
{
	enum SerialEventCodes{
	se_error = -1,
	se_waiting = 0,
	se_read_start,
	se_read_end,
	se_write_start,
	se_write_end
	};
};

struct SerialEvent
{
	int event_type;
	char buffer[256];
	long size;
	SerialEvent();
	SerialEvent(int t, char *b, long s);
};

#ifdef WIN32
struct Comm_Data
{
	SerialChannel *scp;
	std::queue<SerialEvent> *seqp;
	bool stop;
	HANDLE mutex;
};

DWORD WINAPI Read_Thread(LPVOID lpParam);
DWORD WINAPI Update_Thread(LPVOID lpParam);
bool WaitMutex(HANDLE mutex);
#endif

typedef struct ExternControl
{
	unsigned char Digital[2]; //Unused
	unsigned char RemoteButtons; //LCD menu
	char Nick;
	char Roll;
	char Gier;
	unsigned char Gas;
	char Height; //Barometric-Height sensor
	unsigned char free; //Unused
	unsigned char Frame; //FC confirmation
	unsigned char Config; //If 1, extern control is set active
};

// bitmask for HardwareError[0]
#define FC_ERROR0_GYRO_NICK             0x01
#define FC_ERROR0_GYRO_ROLL             0x02
#define FC_ERROR0_GYRO_YAW              0x04
#define FC_ERROR0_ACC_NICK              0x08
#define FC_ERROR0_ACC_ROLL              0x10
#define FC_ERROR0_ACC_TOP               0x20
#define FC_ERROR0_PRESSURE              0x40
#define FC_ERROR0_CAREFREE              0x80

// bitmask for HardwareError[1]
#define FC_ERROR1_I2C                   0x01
#define FC_ERROR1_BL_MISSING            0x02
#define FC_ERROR1_SPI_RX                0x04
#define FC_ERROR1_PPM                   0x08
#define FC_ERROR1_MIXER                 0x10
#define FC_ERROR1_RES1                  0x20
#define FC_ERROR1_RES2                  0x40
#define FC_ERROR1_RES3                  0x80

struct VersionData
{
	unsigned char SWMajor;
	unsigned char SWMinor;
	unsigned char ProtoMajor;
	unsigned char ProtoMinor;
	unsigned char SWPatch;
	unsigned char HardwareError[5];
};

struct DebugOut
{
	unsigned char Status[2];
	signed int Analog[32];
};

void ParseDebugStructure(void *buffer, unsigned int size, DebugOut *ds);

typedef struct BLData
{
	unsigned char index;
	unsigned char current;
	unsigned char temp;
	unsigned char MaxPWM;
	unsigned char status;
};
// status 
#define INVALID         0x00
#define NEWDATA         0x01
#define PROCESSED       0x02

typedef struct GPSPosDev
{ 
	unsigned short distance;
	short bearing;
};

typedef struct GPSPos
{
	int longitude;
	int latitude;
	int altitude;
	unsigned char status;
};

// ------- NCFlags -------------------------------------
#define NC_FLAG_FREE                            0x01
#define NC_FLAG_PH                              0x02
#define NC_FLAG_CH                              0x04
#define NC_FLAG_RANGE_LIMIT                     0x08
#define NC_FLAG_NOSERIALLINK                    0x10
#define NC_FLAG_TARGET_REACHED                  0x20
#define NC_FLAG_MANUAL                          0x40
#define NC_FLAG_GPS_OK                          0x80

// ------- FCStatusFlags -------------------------------
#define FC_STATUS_MOTOR_RUN                     0x01
#define FC_STATUS_FLY                           0x02
#define FC_STATUS_CALIBRATE                     0x04
#define FC_STATUS_START                         0x08
#define FC_STATUS_EMERGENCY_LANDING             0x10
#define FC_STATUS_LOWBAT                        0x20
#define FC_STATUS_VARIO_TRIM_UP                 0x40
#define FC_STATUS_VARIO_TRIM_DOWN               0x80

// ------- FCStatusFlags2 ------------------------------
#define FC_STATUS2_CAREFREE_ACTIVE              0x01
#define FC_STATUS2_ALTITUDE_CONTROL_ACTIVE      0x02
#define FC_STATUS2_FAILSAFE_ACTIVE              0x04
#define FC_STATUS2_OUT1                         0x08
#define FC_STATUS2_OUT2                         0x10
#define FC_STATUS2_RES1                         0x20
#define FC_STATUS2_RES2                         0x40
#define FC_STATUS2_RES3                         0x80

typedef struct NaviData
{
        unsigned char Version; 
        GPSPos CurrentPosition;
        GPSPos TargetPosition;
        GPSPosDev TargetPositionDeviation;
        GPSPos HomePosition;
        GPSPosDev HomePositionDeviation;
        unsigned char  WaypointIndex;
        unsigned char  WaypointNumber;
        unsigned char  SatsInUse;    
        short Altimeter;            
        short Variometer;          
        unsigned short FlyingTime;
        unsigned char  UBat;     
        unsigned short GroundSpeed;
        short Heading;            
        short CompassHeading;    
        char  AngleNick;        
        char  AngleRoll;        
        unsigned char  RC_Quality;
        unsigned char  FCStatusFlags;
        unsigned char  NCFlags;     
        unsigned char  Errorcode;  
        unsigned char  OperatingRadius;
        short TopSpeed;               
        unsigned char  TargetHoldTime;
        unsigned char  FCStatusFlags2;
        short SetpointAltitude;       
        unsigned char  Gas;          
        unsigned short Current;     
        unsigned short UsedCapacity; 
};

typedef struct MixerTable
{
    unsigned char Revision;
    char Name[12];
    char Motor[16][4];
    unsigned char crc;
};

typedef struct Paramset
{
        unsigned char Revision;
        unsigned char Parameters[12];       // GAS[0], GIER[1],NICK[2], ROLL[3], POTI1, POTI2, POTI3
        unsigned char GlobalConfig;           // 0x01=Höhenregler aktiv,0x02=Kompass aktiv, 0x04=GPS aktiv, 0x08=Heading Hold aktiv
        unsigned char Hoehe_MinGas;           // Wert : 0-100
        unsigned char Luftdruck_D;            // Wert : 0-250
        unsigned char MaxHoehe;               // Wert : 0-32
        unsigned char Hoehe_P;                // Wert : 0-32
        unsigned char Hoehe_Verstaerkung;     // Wert : 0-50
        unsigned char Hoehe_ACC_Wirkung;      // Wert : 0-250
        unsigned char Hoehe_HoverBand;        // Wert : 0-250
        unsigned char Hoehe_GPS_Z;            // Wert : 0-250
        unsigned char Hoehe_StickNeutralPoint;// Wert : 0-250
        unsigned char Stick_P;                // Wert : 1-6
        unsigned char Stick_D;                // Wert : 0-64
        unsigned char StickGier_P;            // Wert : 1-20
        unsigned char Gas_Min;                // Wert : 0-32
        unsigned char Gas_Max;                // Wert : 33-250
        unsigned char GyroAccFaktor;          // Wert : 1-64
        unsigned char KompassWirkung;         // Wert : 0-32
        unsigned char Gyro_P;                 // Wert : 10-250
        unsigned char Gyro_I;                 // Wert : 0-250
        unsigned char Gyro_D;                 // Wert : 0-250
        unsigned char Gyro_Gier_P;            // Wert : 10-250
        unsigned char Gyro_Gier_I;            // Wert : 0-250
        unsigned char Gyro_Stability;         // Wert : 0-16
        unsigned char UnterspannungsWarnung;  // Wert : 0-250
        unsigned char NotGas;                 // Wert : 0-250     //Gaswert bei Empängsverlust
        unsigned char NotGasZeit;             // Wert : 0-250     // Zeitbis auf NotGas geschaltet wird, wg. Rx-Problemen
        unsigned char Receiver;                   // 0= Summensignal, 1= Spektrum, 2 =Jeti, 3=ACT DSL, 4=ACT S3D
        unsigned char I_Faktor;               // Wert : 0-250
        unsigned char UserParam1;             // Wert : 0-250
        unsigned char UserParam2;             // Wert : 0-250
        unsigned char UserParam3;             // Wert : 0-250
        unsigned char UserParam4;             // Wert : 0-250
        unsigned char ServoNickControl;       // Wert : 0-250     // Stellung des Servos
        unsigned char ServoNickComp;          // Wert : 0-250     // Einfluss Gyro/Servo
        unsigned char ServoNickMin;           // Wert : 0-250     // Anschlag
        unsigned char ServoNickMax;           // Wert : 0-250     // Anschlag
        //--- Seit V0.75
        unsigned char ServoRollControl;       // Wert : 0-250     // Stellung des Servos
        unsigned char ServoRollComp;          // Wert : 0-250
        unsigned char ServoRollMin;           // Wert : 0-250
        unsigned char ServoRollMax;           // Wert : 0-250
        //---
        unsigned char ServoNickRefresh;       // Speed of the Servo
    unsigned char ServoManualControlSpeed;//
    unsigned char CamOrientation;         //
        unsigned char Servo3;                    // Value or mapping of the Servo Output
        unsigned char Servo4;                            // Value or mapping of the Servo Output
        unsigned char Servo5;                            // Value or mapping of the Servo Output
        unsigned char LoopGasLimit;           // Wert: 0-250  max. Gas während Looping
        unsigned char LoopThreshold;          // Wert: 0-250  Schwelle für Stickausschlag
        unsigned char LoopHysterese;          // Wert: 0-250  Hysterese für Stickausschlag
        unsigned char AchsKopplung1;          // Wert: 0-250  Faktor, mit dem Gier die Achsen Roll und Nick koppelt (NickRollMitkopplung)
        unsigned char AchsKopplung2;          // Wert: 0-250  Faktor, mit dem Nick und Roll verkoppelt werden
        unsigned char CouplingYawCorrection;  // Wert: 0-250  Faktor, mit dem Nick und Roll verkoppelt werden
        unsigned char OffsetNick;     // Wert: 0-250  180°-Punkt
        unsigned char OffsetRoll;     // Wert: 0-250  180°-Punkt
        unsigned char GyroAccAngle;        // 1/k  (Koppel_ACC_Wirkung)
        unsigned char Drift;
        unsigned char DynamicStability;
        unsigned char UserParam5;             // Wert : 0-250
        unsigned char UserParam6;             // Wert : 0-250
        unsigned char UserParam7;             // Wert : 0-250
        unsigned char UserParam8;             // Wert : 0-250
        //---Output ---------------------------------------------
        unsigned char J16Bitmask;             // for the J16 Output
        unsigned char J16Timing;              // for the J16 Output
        unsigned char J17Bitmask;             // for the J17 Output
        unsigned char J17Timing;              // for the J17 Output
        // seit version V0.75c
        unsigned char WARN_J16_Bitmask;       // for the J16 Output
        unsigned char WARN_J17_Bitmask;       // for the J17 Output
        //---NaviCtrl---------------------------------------------
        unsigned char NaviGpsModeControl;     // Parameters for the Naviboard
        unsigned char NaviGpsGain;
        unsigned char NaviGpsP;
        unsigned char NaviGpsI;
        unsigned char NaviGpsD;
        unsigned char NaviGpsPLimit;
        unsigned char NaviGpsILimit;
        unsigned char NaviGpsDLimit;
        unsigned char NaviGpsACC;
        unsigned char NaviGpsMinSat;
        unsigned char NaviStickThreshold;
        unsigned char NaviWindCorrection;
        unsigned char NaviAccCompensation;    // New since 0.86 -> was: SpeedCompensation
        unsigned char NaviOperatingRadius;
        unsigned char NaviAngleLimitation;
        unsigned char NaviPH_LoginTime;
        //---Ext.Ctrl---------------------------------------------
        unsigned char ExternalControl;         // for serial Control
        //---CareFree---------------------------------------------
        unsigned char OrientationAngle;        // Where is the front-direction?
        unsigned char CareFreeModeControl;         // switch for CareFree
    unsigned char MotorSafetySwitch;
    unsigned char MotorSmooth;
    unsigned char ComingHomeAltitude;
    unsigned char FailSafeTime;
    unsigned char MaxAltitude;
        unsigned char FailsafeChannel;         // if the value of this channel is > 100, the MK reports "RC-Lost"
        unsigned char ServoFilterNick;  
        unsigned char ServoFilterRoll;  
        //------------------------------------------------
        unsigned char BitConfig;          // (war Loop-Cfg) Bitcodiert: 0x01=oben, 0x02=unten, 0x04=links, 0x08=rechts / wird getrennt behandelt
        unsigned char ServoCompInvert;    // //  0x01 = Nick, 0x02 = Roll   0 oder 1  // WICHTIG!!! am Ende lassen
        unsigned char ExtraConfig;        // bitcodiert
        unsigned char GlobalConfig3;      // bitcodiert
        char Name[12];
        unsigned char crc;                                // must be the last byte!
};

#define POINT_TYPE_INVALID 255
#define POINT_TYPE_WP   0
#define POINT_TYPE_POI  1

typedef struct Waypoint
{
	GPSPos Position;             
        short Heading;                    
        unsigned char  ToleranceRadius;            
        unsigned char  HoldTime;                   
        unsigned char  Event_Flag;                 
        unsigned char  Index;                      
        unsigned char  Type;                       
        unsigned char  WP_EventChannelValue;       
        unsigned char  AltitudeRate;               
        unsigned char  Speed;                      
        unsigned char  CamAngle;                   
        unsigned char  Name[4];                    
    	unsigned char  reserve[2];                     
};

class KopterController
{
	protected:
	char analog_debug_names[32][17];
	char display_buffer[80];
	ExternControl ctrl;
	VersionData version;
	BLData bldata[8];
	DebugOut debugdata;
	unsigned int debugdata_size;
	unsigned int debug_index;
	NaviData navdata;
	Waypoint waypoints[128];
	unsigned char waypoint_ptr;
	MixerTable mt;
	Paramset ps;	
	short test_pattern;
	std::ofstream debug_file;
	bool debug_ready;
	unsigned long last_sub_time;
	unsigned int debug_rate;

	HANDLE read_thread_handle;
	HANDLE update_thread_handle;
	HANDLE thread_mutex;
	HANDLE self_mutex;
	DWORD read_thread_id;
	DWORD update_thread_id;
	SerialChannel *sc;
	Comm_Data comm_data;
	std::queue<SerialEvent> read_event_queue;
	std::queue<SerialEvent> write_event_queue;

	int parseSignedInt(char a, char b);
	unsigned int parseUnsignedInt(char a, char b);
	void parseDebugOut(char *buffer);

	public:
	KopterController(char *port);
	~KopterController();

	//Threading and updating
	bool IsStopped();

	void Init();
	void HandleSerialEvents();
	void EnqueueWrite(SerialFrame *sf);

	//API
	void EngineTest(int ms, unsigned char *power);
	void SerialTest(short test_p);
	void AssumeControl();
	void RelinquishControl();
	void GetMixer();
	void GetBLParameter();
	void SetTarget(Waypoint target);
	void IssueDebugRequest(unsigned int ms);
	void RenewDebugSubscription();
	void UpdateNaviData();
	void UpdateParamset();
	void EnqueueWaypoint(Waypoint target);
	void ClearWaypoints();
	DebugOut *GetDebugOut(unsigned int &size);

};

#endif
