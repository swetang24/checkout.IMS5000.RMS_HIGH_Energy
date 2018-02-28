// ims5000.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

//zzzzzzzzzz added for debugging
//#define CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//zzzzzzzzzzzz
//#include <vld.h>
//#include "files.h"
#include "resource.h"
#include "IMSConfigIni.h"
#include <stdio.h>
#include <process.h>
#include <shellapi.h>
#include "commdlg.h"
#include "cdev.h"

#include <iostream>           //-------------Linghai-11-10-08---------
#include <fstream>            //-------------Linghai-11-10-08---------
#include <string>              //-------------Linghai-11-10-08---------
using namespace std;              //-------------Linghai-11-10-08--------- 


#include <Shellapi.h>                //-----------Linghai-11-14-08---
#include <tchar.h>                       //--------Lingahi-11-14-08----


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow);


/******************************************************************************
	NAME : IMS5000.C
	DESC : Implementation of IMS5000
	       Inkjet Marking System 5000. © 2005-2016 JETEC Corporation
******************************************************************************/
#include <windows.h>

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define NICONS 6
#define LTIMENAME    30
#define MAXTIMENAMES 10

/******************************************************************************
	PRIVATE TYPES
******************************************************************************/
typedef struct
	{
	long time;
	char name[LTIMENAME+1];
	} TIMENAME;

typedef struct
	{
	LPSTR qbuf;     // pointer to the beginning of the history buffer
	WORD qsize;     // Number of values to allow
	WORD curr;      // Current buffer pointer offset
	WORD nque;      // Number of values currently in the queue
	} UNIQUEHIST;

/******************************************************************************
	WINMAIN AND NON API CALLBACK FUNCTION PROTOTYPES
******************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow);
LRESULT CALLBACK IMSWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK ABOUTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK PROMPTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK PROMPTODP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK PROMPTHDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK PROMPTPDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
//BOOL CALLBACK SERINPUTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK FIXTUREFILEDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);  //------------Linghai-11-6-08-----
BOOL CALLBACK ADDLINKDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);  //------------Linghai-11-6-08-----
BOOL CALLBACK OPENLINKDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);  //------------Linghai-11-6-08-----

/***************************************************************************
	STATIC FUNCTION PROTOTYPES
***************************************************************************/
static BOOL IMSInit(void);
static BOOL IMSInstInit(int nCmdShow);
static void About(void);
static void Shell(void);
static void LoadTimeNames(void);
static int GetTimeName(LPSTR buf,time_t t);
static void MapDigits(LPSTR s,LPCSTR map);
static void ReloadIniSettings();
void				WriteLink(char buf[],int buflen);
void				ReadLink();
void				OpenFileLink(int readorwrite);
static void DoAutoStarts();

/***************************************************************************
	STATIC VARIABLES
***************************************************************************/
fstream myfileLink;
static HWND ModeWnd;       // Mode window
static int  WinMode;       // Window mode: SIZENORMAL|SIZEICONIC|SIZEFULLSCREEN
static int iniIcon;        // Icon number
static char *Icons[NICONS]=  // Icon resource names
	{"IMS","IMW","SMC","SMS","SMA","IMA"};
static HICON IMSIcon;      // Current applicaton icon
//static struct _find_t FindT; // Used by FindFirstFile and FindNextFile
static WIN32_FIND_DATA fData;
HANDLE nxtFile;
static char sHideOpt[]="HideOpt";
static float screen_cx = static_cast<float>( GetSystemMetrics(SM_CXSCREEN) );
static float screen_cy = static_cast<float>( GetSystemMetrics(SM_CYSCREEN) );
// Centre of screen
static float screen_centre_x = screen_cx / 2.0f;
static float screen_centre_y = screen_cy / 2.0f;


// Prompt dialog
static WORD promptl;
static char promptn[64];

//Fixturefile dialog      --------------------Linghai-11-6-08
static WORD fixturefilel;
static char fixturefilen[64];

// CIMple support
static int CIMplePort;
static char CIMpleMark[128];
static char sCIMple[]="CIMPLE"; // ini seciton

// Unique variable history queue
static UNIQUEHIST histque;

// INI Configuration
IMSConfigIni IMSConfig;

/***************************************************************************
	PUBLIC VARIABLES
***************************************************************************/
// Version management
HINSTANCE IMSInst;            // IMS application instance handle
HWND IMSWnd;               // IMS application window
HWND IMWnd;                // IM child window
char IMSName[]="IMS5000";  // IMS application name
char IMSMILSTD[]="MIL-STD-130 REV : N COMPLIANT, Win 7";  // IMS application MIL Standard e
IMSMODE IMSMode;           // Current application mode
HWND IMSADWnd=0;           // Handle of the Active Dialog (for IsDialogMessage)

char IMSDir[128];          // IMS directory
char IMSIni[128];          // IMS application .INI file name.
char CalIni[128];          // IMS calibration .INI file name.
char CalHistIni[128];      // IMS calibration history .INI file name.
char IMIni[128];           // IM5000 INI file name.
char IMSUsr[128];          // IMS application .USR file name.
char IMSProgs[128];        // IMS programs directory
char IMSFixtures[128];     // IMS programs directory
char BMPDir[128];          // IMS bitmaps directory
char IMSVisObjects[128];   // IMS Vision Object directory
BOOL IMSIsShell;           // true if this instance is the shell
BOOL IMSIsActive;          // true if IMS is the active app
HACCEL IMSAccel;           // Accelerators

char var1[TSIZE];
char var2[TSIZE];
char var1val[TSIZE];
char var2val[TSIZE];
int var1len;
int var2len;
int var1vallen;
int var2vallen;

char tbuf[TSIZE];          // General purpose text buffer
int tlen;                 // Length of tbuf
char tbuf1[TSIZE];
int tlen1;
char tbuf2[TSIZE];
int tlen2;
TEXTMETRIC sftm;           // System font text metric
TEXTMETRIC sfftm;          // System fixed font text metric
P3 RehomePoint;            // Head position prior to re-home
P3 StartPoint;             // Head position for start and end of marking
P1 rotrehomepoint;            // Head position prior to re-home   ---sam---
P1 rotstartpoint;             // Head position for start and end of marking  ----sam----

BOOL ZStartOnly;           // Use StartPoint.z only flag
BOOL ReturnToStart;        // Return to Starting position after marking?
BOOL UpSpray;              // Draw IMF in upspray mode flag
BOOL VisEnable;            // True if vision can be enabled via DDE
BOOL VisInterleave;        // True if acquire interleaved with marking
BOOL ReplaceZeros;
BOOL WarnOnBlankVars;      // Enables blank variables on start button warn
BOOL Metric;               // True if metric (cm) instead of inch mode
BOOL MetricMM;             // If Metric and MetricMM use mm instead of cm
BOOL NoFontMarks;   // Disable all font based marks from edit new menu
BOOL NoBitmapMarks; // Disable all bitmap based marks from edit new menu
BOOL NoCal;         // Disable calibration system
BOOL NoAlign;       // Disable alignment user interface
IMINFO IMInfo;      // Info about the IM

WORD IMState;              // The current IM State IMN_ 
WORD UniqueOverrideLevel;  // The unique override user level

// HideOpt
BOOL HideOptRotation;
BOOL HideOptInspect;
BOOL HideOptRead;
BOOL HideOptConveyor;
BOOL HideOptInit;
BOOL HideOptLink;
BOOL HideOptRotaryIMF;
BOOL HideOptRotaryDEG;  //rotary deg
BOOL HideOptModeMenu;
BOOL HideOptMarkRotation;
BOOL HideOptMarkSlant;

// Vision DDE vars
BOOL VisOn=FALSE;    // True only if vision enabled for mark+acquire cycle
BOOL updateVar=FALSE;
P3 VisPos;           // Servo position for camera relative to nest origin

// LQTY system variable for AUX system
UINT AuxLQty=1; // %@LQTY% loaded by PromptForLQty=1 in .AUX file.

// Error handling
WORD IDError;              // Error ID
int iErrorLine;            // Line number ref for internal errors
char *iErrorFile;          // Filename ref for internal errors


TIMENAME TimeNames[MAXTIMENAMES]; // Time names
int nTimeNames; // Count of TimeNames used

// Conveyor
BOOL ConvOpen=FALSE;
BOOL ConvSimplimatic; // True if new style PIP PR PC
int	 ConvType;
int	 ConvDelay=10;	// in milliseconds
// New style ConvSimplimatic bits
HBIT bitConvRun;   // Output enables conveyor operation
HBIT bitConvReset; // Output resets conveyor
HBIT bitConvReady; // Input indicates conveyor ready
HBIT bitConvError; // Input indicates conveyor error
HBIT bitConvPIP1;  // Input indicates Part In Position 1
HBIT bitConvPR1;   // Output Part Request 1
HBIT bitConvPC1;   // Output Part Complete 1
HBIT bitConvPIP2;  // Input indicates Part In Position 2
HBIT bitConvPR2;   // Output Part Request 2
HBIT bitConvPC2;   // Output Part Complete 2
HBIT bitConvPIP3;  // Input indicates Part In Position 3
HBIT bitConvPR3;   // Output Part Request 3
HBIT bitConvPC3;   // Output Part Complete 3

char sConvRun[]="ConvRun";
char sConvReset[]="ConvReset";
char sConvReady[]="ConvReady";
char sConvError[]="ConvError";
char sConvPIP1[]="ConvPIP1";
char sConvPR1[]="ConvPR1";
char sConvPC1[]="ConvPC1";
char sConvPIP2[]="ConvPIP2";
char sConvPR2[]="ConvPR2";
char sConvPC2[]="ConvPC2";
char sConvPIP3[]="ConvPIP3";
char sConvPR3[]="ConvPR3";
char sConvPC3[]="ConvPC3";

// Old style !ConvSimplimatic bits
HBIT bitConvPIP; // Input signals part in position
HBIT bitConvPC;  // Output signals part complete/request
HBIT bitConvPS;  // Output signals PASSED status on 0->1 PC
HBIT bitThruConvPIP;
HBIT bitThruConvPC;
HBIT bitThruConvPC1;
HBIT bitThruConvPC2;
HBIT bitThruConvPC3;
HBIT bitConvOutGate;
char sConvPIP[]="ConvPIP"; // bit names
char sConvPC[]="ConvPC";
char sConvPS[]="ConvPS";
char sThruConvPIP[]="ThruConvPIP";
char sThruConvPIP1[]="ThruConvPIP1";
char sThruConvPIP2[]="ThruConvPIP2";
char sThruConvPIP3[]="ThruConvPIP3";
char sThruConvPC[]="ThruConvPC";
char sThruConvPC1[]="ThruConvPC1";
char sThruConvPC2[]="ThruConvPC2";
char sThruConvPC3[]="ThruConvPC3";
char sConvOutGate[]="ConvOutGate";

BOOL SuppressConveyor; // Controlled by menu entry
BOOL LinkFail;       // True if linking IMP is reporting FAIL
BOOL CycleComplete;  // TRUE if the parts are done
BOOL CycleFailed;    // TRUE if not all parts passed
BOOL CycleStarted;    // TRUE if any parts have started

// RedGreen Lamps
BOOL RedGreenOpen=FALSE;
HBIT bitRedLamp;
HBIT bitGreenLamp;
char sRedLamp[]="RedLamp";
char sGreenLamp[]="GreenLamp";
BOOL RedLampTrapError=FALSE; // If set, the next ShowError turns on the red lamp

// CIMple support
BOOL CIMpleIsOpen=FALSE;
BOOL CIMpleTrapError=FALSE; // If set the next ShowError send a CIMple error

char sDDERUN[]="DDERUN";
//JETDBO32 * dbObj=NULL;
BOOL DebugMode=FALSE;

#define MODAD  4462
//zzz#define MODAD  4461
bool timeLimit(void);

VOID CALLBACK CloseWinProc(HWND hwnd, UINT uMsg, 
						   UINT_PTR idEvent, DWORD dwTime);
void hLock_showerror(WORD err);

/******************************************************************************
	WINMAIN AND NON API CALLBACK FUNCTIONS
******************************************************************************/
/***************************************************************************
	WinMain(hInstance,hPrevInstance,lpszCmdLine,nCmdShow)

	IMS application main function.  Program entry/exit point.
	Message loop.
***************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
	{
//zzz	_CrtMemState ms1, ms2, ms3;
//zzz	_CrtMemCheckpoint( &ms1 );
	
	MSG msg;
	int qsize;

	IMSInst=(HINSTANCE)hInstance; // Save application instance handle.

	HWND prewnd=FindWindow(IMSName,NULL);
	if(prewnd)
		{
		BringWindowToTop(prewnd);  // Bring it to the top
		if(WinMode==SIZEICONIC)    // If its an icon
			ShowWindow(IMSWnd,SW_RESTORE); //Restore it
		ShowError(IDE_IMSRUNNING); // Show error on top of it.
		return FALSE;
		}
	
	if (!timeLimit())	// Check internal time limits violations  ..... ddd
	    return FALSE;

	// Allocate larger app message queue (12-16 entries instead of 8)
	for(qsize=16;!SetMessageQueue(qsize);qsize--)
		if(qsize<12)
			{
			IERROR; // Unable to allocate minimum queue size
			ShowError(0);
			return FALSE;
			}
	
	if(!IMSInit())     // Initialize the application
		{
		ShowError(IDE_IMSINIT);
		return FALSE;
		}
	if(!IMSInstInit(SW_SHOW)) // Initialize the instance
		{
		ShowError(IDE_IMSINSTINIT);
		return FALSE;
		}

	// Message loop
	while(GetMessage(&msg,NULL,0,0))
		{
		if(IMSADWnd)
			if(IsDialogMessage(IMSADWnd,&msg))
				continue;
		if(TranslateAccelerator(IMSWnd,IMSAccel,&msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}

/*	_CrtMemCheckpoint( &ms2 );

	OutputDebugString("\n\n***Debug Memory Info 1\n");
	_CrtMemDumpStatistics( &ms1 );
	OutputDebugString("\n***Debug Memory Info 2\n");
	_CrtMemDumpStatistics( &ms2 );
	if ( _CrtMemDifference( &ms3, &ms1, &ms2) )
		{
		OutputDebugString("\n***Debug Memory Difference\n");
		_CrtMemDumpStatistics( &ms3 );
		}
	OutputDebugString("\n***Debug Memory Leaks\n");
	_CrtDumpMemoryLeaks();
*/
	return msg.wParam; // Return WM_QUIT wParam
	
	}

/***************************************************************
bool timeLimit(void)

****************************************************************/
bool timeLimit(void)
     {
		char tbuf[256] = "Please contact:\tJETEC Corporation © 2017\n\t\t949.477.6161\n\t\twww.Jetec.com",
			cbuf[256] = {NULL},
			dbuf[256] = "IMS5000 - Rotary Edition Software License is Valid.";

		bool License = false;

		// get and print the current time
		time_t currentTime;
		time (&currentTime); // fill now with the current time

		struct tm * ptm= localtime(&currentTime);
		int month	= 05		// Set value '0' to disable. D. Ford set to 0 on 8/8/16
			, day	= 04	// Always default 28 days this accounts for Feb.
			, year	= 2027;	// Set value '0' to disable D. Ford set to 0 on 8/8/16
					
		//GetPrivateProfileString(IMSName,"License","",tbuf,TSIZE,IMSIni);

		//strcmp 

		if ((year-1900) > ptm->tm_year)			// Is it less than the year?
			License=true;						// Good
		else if ((year-1900) == ptm->tm_year)	// Is it within the year?
				{								// Check month
				if ((month-1) > ptm->tm_mon)	// Is it less than the month?
					License=true;				// Good
				else if ((month-1) == ptm->tm_mon) // Is it within the month?
					{							// Check day
					if (day >= ptm->tm_mday)	// Is it NOT passed the day?
						License=true;			// Good
					}
				}

		// if license is close to ending...just days within current month and current year...
		if ( (day > ptm->tm_mday)&& ((month-1) == ptm->tm_mon)&& ((year-1900) == ptm->tm_year))
			{
			int dnum = day - ptm->tm_mday;
			if ((dnum <= day)&&(dnum >1))
				wsprintf(dbuf,"<<< NOTICE >>> This IMS5000 Rotary Edition Software License will expire in %d days.",dnum);
			else
				{
				wsprintf(dbuf,"<<<WARNING>>> This IMS5000 Software License will expire after today.");
				}
			MessageBox(NULL,tbuf,dbuf,MB_OK);
			}

/* 		wsprintf(tbuf,"Valid for this Date and Time - %s \n Until %d/%d/%d"
					 ,ctime(&currentTime)
					 , month
					 , day
					 , year
					 );
					 //,((ptm->tm_mon)+1)
					 //,ptm->tm_mday
					 //,(ptm->tm_year)+1900)
					 //;*/
		if (!License)
			{
				wsprintf(dbuf,"IMS5000 Rotary Edition Software License is invalid.");
				MessageBox(NULL,tbuf,dbuf,MB_OK);
			}
		return License;
	}

/***************************************************************************
	IMSWP(hwnd,message,wP,lP)

	IMS window process.
***************************************************************************/
LRESULT CALLBACK IMSWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_CREATE:
			{

			RECT r;     // Client rect
			WORD w,h;   // Width and Height of client rect
			WORD mh;    // Height of marker rect

			// Get client rect and compute upper and lower window extents
			GetClientRect(hwnd,&r);
			w=(WORD)r.right+1;
			h=(WORD)r.bottom+1;
			mh=(WORD)(7*sftm.tmHeight>>2)+4;

			if(IMSLIB32GetAPIVersion()!=IMSLIB32APIVER) // check IMSLIB1 API ver
				{
				ShowError(IDE_BADIMSLIB1API);
				return -1L;
				}

			if(IMGetAPIVersion()!=IM5000APIVER) // check IM API version
				{
				ShowError(IDE_BADIMAPI);
				return -1L;
				}
			IMWnd=IMOpen(hwnd,IDC_IM,0,h-mh,w,mh,&IMSADWnd);
			if(!IMWnd)
				{
				ShowError(IDE_CREATEIM);
				return -1L;
				}
			SendMessage(IMWnd,IMM_GETINFO,0,(LONG)(LPIMINFO)&IMInfo);

			if(!DDEServerInit())
				return -1L;
			DM001Link();
			QRCoderCheck();
			RefEdgeInit();
			ConveyorInit();
			RedGreenInit();
			CIMpleInit();
			VisionLampInit();
			ClampInit();
			PC2ClampInit();
			IMPLoadBases();
			UHISTInit();
			StdLocInit();
			CDevSysInit();

			IMRegisterNotifyCallback(IMNotifyCallback);

			//old see autostart for new way
			// Load load=
			GetPrivateProfileString(IMSName,"load","",tbuf,TSIZE,IMSIni);
			if(lstrlen(tbuf))
				WinExec(tbuf,SW_SHOWMINNOACTIVE);

			DebugMode=GetPrivateProfileInt(IMSName,"Debug",0,IMSIni);
			// Run run= after UpdateWindow
//			dbObj=new JETDBO32("IMS DEGUG");

			DoAutoStarts();

			return 0L;
			}

		case WM_SIZE:
			{
			WORD w,h;   // Width and Height of client rect
			WORD mh;    // Height of marker rect

			// Don't bother to process SIZEZOOMHIDE | SIZEZOOMSHOW
			if(!(wP==SIZEICONIC
				||wP==SIZENORMAL
				||wP==SIZEFULLSCREEN))
				return 0;
			// Keep track of WinMode
			WinMode=wP; // test this
			// Compute w h and mh
			w=LOWORD(lP);
			h=HIWORD(lP);
			mh=(WORD)(7*sftm.tmHeight>>2)+4;
			
			// Move them
			MoveWindow(ModeWnd,0,0,w,h-mh,TRUE);
			MoveWindow(IMWnd,0,h-mh,w,mh,TRUE);
			return 0;
			}

		case WM_ACTIVATEAPP:
			IMSIsActive=(BOOL)wP;
			return 0;

		case WM_ACTIVATE:
			//zzz
//			OutputDebugString("IMS5000  WM_ACTIVATE\n");
			IMSADWnd=0; // old style windows never use adwnd
			// IMSADWnd=wP ? hwnd : 0;
			return 0;

		case WM_CLOSE: // Disable WM_CLOSE
			return 0;

		case WM_COMMAND:  // Changed Win32
			{
				WORD id = LOWORD(wP);
				WORD ncode = HIWORD(wP);
				
				if(!lP) // Menu or acceleraotr
				{
					switch(LOWORD(wP)) // Global menu processing
					{

					case IDM_CAL:
						{

					    /*char tbuf[512],tbuf2[512];
 
						SetCursorPos(screen_centre_x,screen_centre_y);
INPUT *key;
//if(windowHandle == NULL)
 //cout << "not found";
//SetForegroundWindow(windowHandle);
//Sleep(1000);

key = new INPUT;
key->type = INPUT_KEYBOARD;
key->ki.wVk = VK_F10;
key->ki.dwFlags = 0;
key->ki.time = 0;
key->ki.wScan = 0;
key->ki.dwExtraInfo = 0;

SendInput(1,key,sizeof(INPUT));
					    //_spawnlp(_P_NOWAIT,(const char*)"C:\\Program Files\\CrossHair\\CrossHair.exe",
						//(const char*)tbuf,NULL);
					    _spawnlp(_P_NOWAIT,(const char*)"C:\\Program Files\\ATI Multimedia\\main\\ATIMMC.exe",
						(const char*)tbuf2,NULL);
//_spawnlp(_P_NOWAIT,(const char*)"C:\\Program Files\\TextPad 5\\TextPad.exe",
						//(const char*)tbuf2,NULL);
							*/
						if(!TheCal)
							TheCal=new CALC();
						if(TheCal)
							TheCal->open(hwnd,&IMSADWnd);
						return 0;
						}
					case IDM_ABOUT:
						About();
						return 0;
					case IDM_HELP:
						{
						char tbuf[512],tbuf2[512];
					    int len=GetModuleFileName(IMSInst,tbuf,sizeof(tbuf));
						tbuf[len-11]=0;
						lstrcat(tbuf,"help\\imshelp.htm");
						sprintf(tbuf2,"iexplore.exe %s",tbuf);
					    _spawnlp(_P_NOWAIT,(const char*)"c:\\program files\\internet explorer\\iexplore.exe",
						(const char*)tbuf2,NULL);
						return 0;
					}
					case IDM_EXIT:
						if(IMSIsShell)
							ExitWindows(0,0);
						else
							DestroyWindow(IMSWnd);
						return 0;
					case IDM_SHUTDOWN:
						SendMessage(IMWnd,IMM_SHUTDOWN,0,0);
						return 0;
					case IDM_POWERDOWN:
						SendMessage(IMWnd,IMM_POWERDOWN,0,0);
						return 0;
					case IDM_PRINTON:
						SendMessage(IMWnd,IMM_PRINTON,0,0);
						return 0;
					case IDM_CLEAN:
						SendMessage(IMWnd,IMM_CLEAN,0,0);
						return 0;
					case IDM_SHELL:
						Shell();
						return 0;
					case IDM_CFGSAVE:
						{
							RECT r;
							// Save window rect 
							// only if WinMode==SIZENORMAL
							if(WinMode==SIZENORMAL)
							{
								GetWindowRect(IMSWnd,&r);
								wsprintf(tbuf,"%d,%d,%d,%d",
									r.left,r.top,r.right,r.bottom);
								WritePrivateProfileString(IMSName,"WinRect",
									tbuf,IMSIni);
							}
							// Save WinMode
							wsprintf(tbuf,"%d",WinMode);
							WritePrivateProfileString(IMSName,"WinMode",
								tbuf,IMSIni);
							return 0;
						}
					case IDM_COM:                               //---------------------Linghai-7-16-09--
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						
							BOOL res = IMSConfig.initializeCOM(IMSWnd,IMSInst,"Company",IMSIni);
							if (res){
								// Update the settings
								if (IMSConfig.NeedsRestart())
									MessageBox(IMSWnd,"In order for some of the changes to take effect, the program must be restarted",IMSName,MB_ICONEXCLAMATION);
								ReloadIniSettings();
								MessageBox(IMSWnd,"Company's information changed.",IMSName,MB_OK);
					
							}
							
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
						
							return 0;
						}
					case IDM_CUID06:                          //-----------------Linghai-4-14-09---------
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						
							BOOL res = IMSConfig.initializeUID06(IMSWnd,IMSInst,"UID06",IMSIni);
							if (res){
								// Update the settings
								if (IMSConfig.NeedsRestart())
									MessageBox(IMSWnd,"In order for some of the changes to take effect, the program must be restarted",IMSName,MB_ICONEXCLAMATION);
								ReloadIniSettings();
								MessageBox(IMSWnd,"UID settings changed.",IMSName,MB_OK);
					
							}
							
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
						
							return 0;
						}
					case IDM_CUID05:                          //-----------------Linghai-4-14-09---------
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						
							BOOL res = IMSConfig.initializeUID05(IMSWnd,IMSInst,"UID05",IMSIni);
							//BOOL res = IMSConfig.initializeUID(IMSWnd,IMSInst,"UID",IMSIni);
							if (res){
								// Update the settings
								if (IMSConfig.NeedsRestart())
									MessageBox(IMSWnd,"In order for some of the changes to take effect, the program must be restarted",IMSName,MB_ICONEXCLAMATION);
								ReloadIniSettings();
								MessageBox(IMSWnd,"UID settings changed.",IMSName,MB_OK);
					
							}
							
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
						
							return 0;
						}
				case IDM_CUID12:                          //-----------------Linghai-4-14-09---------
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						
							BOOL res = IMSConfig.initializeUID12(IMSWnd,IMSInst,"UID12",IMSIni);
							if (res){
								// Update the settings
								if (IMSConfig.NeedsRestart())
									MessageBox(IMSWnd,"In order for some of the changes to take effect, the program must be restarted",IMSName,MB_ICONEXCLAMATION);
								ReloadIniSettings();
								MessageBox(IMSWnd,"UID settings changed.",IMSName,MB_OK);
					
							}
							
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
						
							return 0;
						}
				case IDM_CUIDALL:                          //-----------------Linghai-4-14-09---------
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						
							BOOL res = IMSConfig.initializeUID(IMSWnd,IMSInst,"UID",IMSIni);
							if (res){
								// Update the settings
								if (IMSConfig.NeedsRestart())
									MessageBox(IMSWnd,"In order for some of the changes to take effect, the program must be restarted",IMSName,MB_ICONEXCLAMATION);
								ReloadIniSettings();
								MessageBox(IMSWnd,"UID settings changed.",IMSName,MB_OK);
					
							}
							
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
						
							return 0;
						}

					case IDM_CFGIMS:
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						
							BOOL res = IMSConfig.initialize(IMSWnd,IMSInst,IMSName,IMSIni);
							if (res){
								// Update the settings
								if (IMSConfig.NeedsRestart())
									MessageBox(IMSWnd,"In order for some of the changes to take effect, the program must be restarted",IMSName,MB_ICONEXCLAMATION);
								ReloadIniSettings();
								MessageBox(IMSWnd,"IMS5000 settings changed.",IMSName,MB_OK);
					
							}
							
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
						
							return 0;
						}
					case IDM_CFGIM:
						{
							char dir[MAX_PATH];
							GetCurrentDirectory(sizeof(dir),dir);
						//	MessageBox(NULL,dir,"dir",MB_OK);
							if (IMConfigIni(IMSWnd))
								MessageBox(IMSWnd,"IM5000 settings changed.",IMSName,MB_OK);
							// Config may have changed the directory, so set it back
							SetCurrentDirectory(dir);
							return 0;
						}
					case IDM_CFGSHELL:
						GetPrivateProfileString(IMSName,"Shell","",
							tbuf,TSIZE,IMSIni);
						if(DoPrompt(hwnd,"Shell",TSIZE,tbuf))
							WritePrivateProfileString(IMSName,"Shell",
							tbuf,IMSIni);
						return 0;
					case IDM_APROCEDURE:
						//ok=DialogBox(IMSInst,"FIXTUREFILEB",parent,(DLGPROC)FIXTUREFILEDP);
						DialogBox(IMSInst,"ADPROCEDURE",hwnd,(DLGPROC)ADDLINKDP);
						return 0;
					case IDM_OPROCEDURE:
						DialogBox(IMSInst,"OPPROCEDURE",hwnd,(DLGPROC)OPENLINKDP);
						return 0;
					//////////////////////////////////////////////////////////////////////////////////

					
					case IDM_NEWNOTES:
						char filename[MAX_PATH];
						char dir[MAX_PATH];
						char thispath[MAX_PATH];
						strcpy(thispath,"c:\\ims5000\\Notes");
						CreateDirectory(thispath,NULL);
						int len;
						GetFileTitle(IMSMode==mEDIT?EditorIMP.fn:RunIMF.imp.fn,filename,MAX_PATH);
						len = strlen(filename);
						filename[len-3]='t';
						filename[len-2]='x';
						filename[len-1]='t';
						filename[len]='\0';
						strcat(thispath,"\\");
						strcat(thispath,filename);
						len = strlen(thispath);
						thispath[len] = '\0';
						FILE *file ;
						file = fopen(thispath,"a");
						fclose(file);

						strcpy(dir,"start Notepad ");
						strcat(dir,thispath);
						len = strlen(dir);
						dir[len] = '\0';
						system(dir);
						return 0;
					//case IDM_OPENNOTES:
						//system("explorer c:\\ims5000\\Notes");
						//WinExec("Explorer.exe c:\\ims5000", SW_NORMAL);
						//return 0;
					

					case IDM_DDERUNSTART: // posted by DDERunExecute
						DDERunStart();
						return 0;
					}
					// Global accelerators
					if(id==IDA_INIT)
					{
						SendMessage(IMWnd,IMM_INIT,0,0);
						return 0;
					}
					if(id==IDA_CLAMPREL)
					{
						ClampToggle();
						return 0;
					}
				}
				else // from a control
				{
					if(id==IDC_IM) 	// Keep track of IM state
						if(ncode<IMN_TAB) // if its a state change
						{
							IMState=HIWORD(wP);
//							char buf[512];
//							wsprintf(buf,"IMSTATE = %d\n",IMState);
//							ShowDebugString(buf);
							DDERunIMStateAdvise();
						}
						if(CALWnd) // Send a copy to CAL if open (no ret)
							SendMessage(CALWnd,message,wP,lP);
						if(StdLoc&&StdLoc->GetHwnd()) // Send a copy to StdLoc if open (no ret)
							SendMessage(StdLoc->GetHwnd(),message,wP,lP);
				}
				// Relay to mode window
				return SendMessage(ModeWnd,message,wP,lP);
			}							

		case WM_QUERYENDSESSION:
		case WM_INITMENU:
		case WM_INITMENUPOPUP:
			return SendMessage(ModeWnd,message,wP,lP); // Relay to mode


		case WM_SETFOCUS:
			SetFocus(ModeWnd);
			return 0;

		case WM_DESTROY:
//			delete dbObj;
			CDevSysQuit();
			StdLocQuit();
			FilesShutdown();
			DM001UnLink();
			DDEServerShutdown();
			CIMpleQuit();
			ConveyorQuit();
			UHISTFree();
			IMPFreeBases();
			IMRegisterNotifyCallback(NULL);
			PostQuitMessage(0);
			return 0;
		}
	return DefWindowProc(hwnd,message,wP,lP);
	}

/***************************************************************************
	ABOUTDP

	About Dialog Process.
***************************************************************************/
BOOL CALLBACK ABOUTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			lstrcpy(tbuf,"");

			// Version
			LPSTR   lps;
			LPSTR   lpVersion;
			DWORD   dwVerInfoSize;
			DWORD   dwVerHnd;
			WORD    wVersionLen;
			WORD    wRootLen;
			BOOL    bRetCode;
			char    szGetName[256];

			char file[256];
			HINSTANCE hinst=(HINSTANCE)GetWindowLong(hdlg,GWL_HINSTANCE);
			GetModuleFileName(hinst,file,sizeof(file));
			dwVerInfoSize = GetFileVersionInfoSize(file,&dwVerHnd);
			if(dwVerInfoSize)
				{
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;

				// allocate memory to hold the verinfo block
				hMem=GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = (LPSTR)GlobalLock(hMem);
				GetFileVersionInfo(file,dwVerHnd,dwVerInfoSize,lpstrVffInfo);

				lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\");
				wRootLen=lstrlen(szGetName);

				lstrcpy(&szGetName[wRootLen],"FileVersion");
				wVersionLen=0;
				lpVersion=NULL;
				bRetCode=VerQueryValue((LPVOID)lpstrVffInfo,(LPSTR)szGetName,
					(void FAR* FAR*)&lpVersion,(UINT FAR *)&wVersionLen);
				if(bRetCode&&wVersionLen&&lpVersion)
					{
					lstrcpy(tbuf,"Version ");
					lps=lpVersion;
					int i=lstrlen(tbuf);
					int ncommas=0;
					while(*lps)
						{
						if(*lps==',')
							{
							ncommas++;
							if(ncommas==1)
								tbuf[i++]='.';
							else if(ncommas==3)
								i+=wsprintf(&tbuf[i],"    Build ");
							}
						else if(*lps!=' ')
							tbuf[i++]=*lps;
						lps++;
						}
					tbuf[i]=0;
					}

				GlobalUnlock(hMem);
				GlobalFree(hMem);
				}

			SendDlgItemMessage(hdlg,IDC_VERSION,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			// SerialNo
			tlen=wsprintf(tbuf,"SN : ");
			tlen+=GetPrivateProfileString(IMSName,"SerialNo","",
				(LPSTR)&tbuf+tlen,TSIZE-tlen,IMSIni);
			SendDlgItemMessage(hdlg,IDC_SN,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			// Licensee
			tlen=wsprintf(tbuf,"Licensed to : ");
			tlen+=GetPrivateProfileString(IMSName,"Licensee","",
				(LPSTR)&tbuf+tlen,TSIZE-tlen,IMSIni);
			SendDlgItemMessage(hdlg,IDC_LICENSEE,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			// IMS LIC EXP INFO --sam--04/30/17
			tlen=wsprintf(tbuf,"Licensed Exp. Date: ");
			tlen+=GetPrivateProfileString(IMSName,"LicExp","",
				(LPSTR)&tbuf+tlen,TSIZE-tlen,IMSIni);
			SendDlgItemMessage(hdlg,IDC_IMLICEXP,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			
			// IM API & Version Number -- Version and API number 9-28-16
			if(IMInfo.ver&0xf000)//Version Control
				wsprintf(tbuf,"IM Version %d.%02dD%d",
					(IMInfo.ver&0x0fe)>>8,IMInfo.ver&0xfe,IMInfo.ver>>12);
			else
			wsprintf(tbuf,"IM Version %d.%02d",
				IMInfo.ver<<8,IMInfo.ver&0xfe);

			SendDlgItemMessage(hdlg,IDC_IMVERSION,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);

			if(IMInfo.apiver&0xf000)//API Ver Control
				wsprintf(tbuf,"IM API %d.%02dD%d",
					(IMInfo.apiver)/100,IMInfo.apiver&0xff,
					IMInfo.apiver>>12);
			else
				wsprintf(tbuf,"IM API %d.%02d",
					IMInfo.apiver/100,IMInfo.apiver&0xfed);
			SendDlgItemMessage(hdlg,IDC_IMAPIVER,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);

			// Icon
			SendDlgItemMessage(hdlg,IDC_APPICON,STM_SETICON,
				(WPARAM)IMSIcon,0L);
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			if(LOWORD(wP)==IDOK) // OK button
				{
				EndDialog(hdlg,TRUE);  // End the dialog
				return TRUE;
				}
			if(LOWORD(wP)==IDCANCEL) // OK button
				{
				EndDialog(hdlg,TRUE);  // End the dialog
				return TRUE;
				}
			break;
		}
	return FALSE;            // Message not handled
	}

/***************************************************************************
	PROMPTDP

	Prompt Dialog Process.
***************************************************************************/
BOOL CALLBACK PROMPTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			SendMessage(hdlg,WM_SETTEXT,0,(LONG)(LPSTR)promptn);
			SendDlgItemMessage(hdlg,IDC_EDIT,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDIT,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			switch(LOWORD(wP))
				{
				case IDOK:
					tlen=GetDlgItemText(hdlg,IDC_EDIT,tbuf,promptl+1);
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
		}
	return FALSE;            // Message not handled
	}
/************************************************************************
SERINPUTDP
SIEMENS FILE SER INPUT PROCESS
*************************************************************************/
/*BOOL CALLBACK SERINPUTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
				SetWindowText(hdlg,"Input Window");
			//SendMessage(hdlg,WM_SETTEXT,0,(LONG)(LPSTR)promptn);
			//SendDlgItemMessage(hdlg,IDC_EDITX,EM_LIMITTEXT,promptl,0L);
			//SendDlgItemMessage(hdlg,IDC_EDITY,EM_LIMITTEXT,promptl,0L);
			//SendDlgItemMessage(hdlg,IDC_EDITX,WM_SETTEXT,0,
			//	(LONG)(LPSTR)tbuf);
			//SendDlgItemMessage(hdlg,IDC_EDITY,WM_SETTEXT,0,
			//	(LONG)(LPSTR)tbuf1);
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			
			switch(LOWORD(wP))
				{
//case IDC_EXITPG:
//					return TRUE;
//				case IDC_QUITPG:
//					return TRUE;
				case IDOK:
					//tlen=GetDlgItemText(hdlg,IDC_EDITX,tbuf,promptl+1);
					//strcat(tbuf,"&");
					//tlen+=1;
					//tlen1=GetDlgItemText(hdlg,IDC_EDITY,tbuf1,promptl+1);
					//strcat(tbuf,tbuf1);
					//tlen+=tlen1;
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDC_WIDTH:
					PostMessage(IMSWnd,WM_COMMAND,IDM_QUIT,0);
					EndDialog(hdlg,FALSE);
					return TRUE;
				case IDC_SEARCHFILE:
					if(UserLevel>2)
					{
					PostMessage(IMSWnd,WM_COMMAND,IDM_EXIT,0);
					EndDialog(hdlg,FALSE);
					}
					return TRUE;
				case IDCANCEL:
					if(UserLevel == 4)
					{
					EndDialog(hdlg,FALSE);
					}
					return TRUE;
			}
		}
	return FALSE;            // Message not handled
	}

*/
/***************************************************************************
	PROMPTODP

	PromptO Dialog Process.
***************************************************************************/
BOOL CALLBACK PROMPTODP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
		switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			SendMessage(hdlg,WM_SETTEXT,0,(LONG)(LPSTR)promptn);
			SendDlgItemMessage(hdlg,IDC_EDITX,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITY,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITX,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			SendDlgItemMessage(hdlg,IDC_EDITY,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf1);
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			switch(LOWORD(wP))
				{
				case IDOK:
					tlen=GetDlgItemText(hdlg,IDC_EDITX,tbuf,promptl+1);
					strcat(tbuf,"&");
					tlen+=1;
					tlen1=GetDlgItemText(hdlg,IDC_EDITY,tbuf1,promptl+1);
					strcat(tbuf,tbuf1);
					tlen+=tlen1;
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
						}
				}
				return FALSE;            // Message not handled	
	
}
/***************************************************************************
	PROMPTODP

	PromptO Dialog Process.
***************************************************************************/
BOOL CALLBACK PROMPTHDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			SendMessage(hdlg,WM_SETTEXT,0,(LONG)(LPSTR)promptn);
			SendDlgItemMessage(hdlg,IDC_EDITDIA,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITHIG,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITDIA,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			SendDlgItemMessage(hdlg,IDC_EDITHIG,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf1);
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			switch(LOWORD(wP))
				{
				case IDOK:
					tlen=GetDlgItemText(hdlg,IDC_EDITDIA,tbuf,promptl+1);
					strcat(tbuf,"&");
					tlen+=1;
					tlen1=GetDlgItemText(hdlg,IDC_EDITHIG,tbuf1,promptl+1);
					strcat(tbuf,tbuf1);
					tlen+=tlen1;
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
						}
				}
	return FALSE;            // Message not handled
	
}

/***************************************************************************
	PROMPTPDP

	PromptO Dialog Process.
***************************************************************************/
BOOL CALLBACK PROMPTPDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			SendMessage(hdlg,WM_SETTEXT,0,(LONG)(LPSTR)promptn);
			SendDlgItemMessage(hdlg,IDC_EDITL,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITW,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITH,EM_LIMITTEXT,promptl,0L);
			SendDlgItemMessage(hdlg,IDC_EDITL,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf);
			SendDlgItemMessage(hdlg,IDC_EDITW,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf1);
			SendDlgItemMessage(hdlg,IDC_EDITH,WM_SETTEXT,0,
				(LONG)(LPSTR)tbuf2);
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			switch(LOWORD(wP))
				{
				case IDOK:
					tlen=GetDlgItemText(hdlg,IDC_EDITL,tbuf,promptl+1);
					strcat(tbuf,"x");
					tlen+=1;
					tlen1=GetDlgItemText(hdlg,IDC_EDITW,tbuf1,promptl+1);
					strcat(tbuf,tbuf1);
					tlen+=tlen1;
					strcat(tbuf,"x");
					tlen+=1;
					tlen1=GetDlgItemText(hdlg,IDC_EDITH,tbuf2,promptl+1);
					strcat(tbuf,tbuf2);
					tlen+=tlen1;
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
		}
	return FALSE;            // Message not handled
	}

void OpenFileLink(int readorwrite)
{
	//char filename[MAX_PATH];
	//filename[0] = '\0';
	char *filename;
	filename = new char[MAX_PATH];
	char *thispath;
	thispath = new char[MAX_PATH];
						//char thispath[MAX_PATH];
						//thispath[0] = '\0';
						//GetCurrentDirectory(sizeof(thispath),thispath);
						strcpy(thispath,"c:\\ims5000\\Procedure");
						CreateDirectory(thispath,NULL);
						int len;
						GetFileTitle(EditorIMP.fn,filename,MAX_PATH);
						len = strlen(filename);
						filename[len-3]='t';
						filename[len-2]='x';
						filename[len-1]='t';
						filename[len]='\0';
						strcat(thispath,"\\");
						strcat(thispath,filename);
						len = strlen(thispath);
						thispath[len] = '\0';
						/*FILE *file ;
						file = fopen(thispath,"a");
						fclose(file);*/
	myfileLink.open(thispath,ios::app | ios::out | ios::in);
	delete[] filename;
	delete[] thispath;
}

void WriteLink(char buf[], int buflen)
{
	//fwrite(buf,1,buflen,file);
	//fwrite("\n",1,1,file);
	myfileLink.write(buf,buflen);
	myfileLink.write("\n",strlen("\n"));
	//myfileLink.flush();
}

BOOL CALLBACK OPENLINKDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
{
	char *tem;
	tem = new char[TSIZE];
	int temlen = 0;
	switch (message)
	{
	case WM_INITDIALOG:	
		OpenFileLink(1);
			
			while(myfileLink.getline(tem,TSIZE))
			{
				
				SendDlgItemMessage(hdlg,IDC_LINKDISPLAY,CB_ADDSTRING,0,(LPARAM)(LPSTR)tem);
			}
SendDlgItemMessage(hdlg,IDC_LINKDISPLAY,CB_SETCURSEL,0,0);
			myfileLink.flush();
			myfileLink.clear();
			myfileLink.close();
			delete[] tem;
		
		return TRUE;
	case WM_COMMAND:
		{
			if(LOWORD(wP) == IDOK)
			{
				temlen = GetDlgItemText(hdlg,IDC_LINKDISPLAY,tem,TSIZE+1);
ShellExecute(
        //hdlg, //Parent window
		NULL,
        "open",       //Operation to perform
        tem,       //Path to program
        NULL,         //Parameters
        NULL,         //Default directory
        SW_SHOW);     //How to open
				EndDialog(hdlg, LOWORD(wP));
				delete[] tem;
				return TRUE;
			}
			if ( LOWORD(wP) == IDCANCEL)
			{
				delete[] tem;
				EndDialog(hdlg, LOWORD(wP));
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK ADDLINKDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
{
	//UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		OpenFileLink(0);
		return TRUE;

	case WM_COMMAND:
		if(LOWORD(wP) == IDC_BROWSELINK)
		{
OPENFILENAME ofn; // common dialog box structure
					char pathname[MAX_PATH]; // buffer for file name
					char title[MAX_PATH]; // buffer for file title
					//		HWND hwnd; // owner window
					//		HANDLE hf; // file handle

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hdlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;
					//
					// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
					// use the contents of szFile to initialize itself.
					//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					ofn.lpstrFilter = "All Files\0*.*\0\0";
					ofn.nFilterIndex = 0;
					
					ofn.lpstrInitialDir = initdir;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					if (GetOpenFileName(&ofn) && ofn.lpstrFile)
					{
						SetDlgItemText(hdlg,IDC_EDIT1LINK,ofn.lpstrFile); 
					}	
			/*char dirname[TSIZE];
			int dirnamelen;
			//dirnamelen = GetCurrentDirectory(TSIZE+1,dirname);
			//char initdir[MAX_PATH];
					lstrcpy(dirname,IMSDir);
					lstrcat(dirname,"\\");
			OPENFILENAME ofn; 
			char pathname[TSIZE]; // buffer for file name
			char title[TSIZE]; // buffer for file title
				
					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hdlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;

					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					//ofn.lpstrFilter = "Dynamic Link Libraries (.dll)\0*.DLL\0";
					ofn.lpstrFilter = "All Files\0*.*\0\0";
					ofn.nFilterIndex = 0;
				
					ofn.lpstrInitialDir = dirname;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				
				// Display the Open dialog box. 
					if (GetOpenFileName(&ofn) && ofn.lpstrFile)
					{
						SetDlgItemText(hdlg,IDC_EDIT1LINK,ofn.lpstrFile); 
					}	
					//SetCurrentDirectory(dirname);
					*/
					
		}
		if(LOWORD(wP) == IDOK)
		{
			tlen = GetDlgItemText(hdlg,IDC_EDIT1LINK,tbuf,TSIZE+1);
			
			WriteLink(tbuf,tlen);
			//SetDlgItemText(hDlg,IDC_EDIT1,buf);
			EndDialog(hdlg, LOWORD(wP));
			//fclose(file);
			myfileLink.flush();
			myfileLink.close();
			tlen = 0;
			tbuf[0] = '\0';
			return TRUE;
		}
		else if (LOWORD(wP) == IDCANCEL)
		{
			EndDialog(hdlg, LOWORD(wP));
			//fclose(file);
			myfileLink.flush();
			myfileLink.close();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/****************************************************
-------------------Linghai-11-7-08----
CharMatch

Function to find fixture files
*****************************************************/

/***************************************************************************
---------------Linghai-11-6-08---------------
	FIXTUREFILEDP

	Fixturefile Dialog Process.
***************************************************************************/
BOOL CALLBACK FIXTUREFILEDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			SendMessage(hdlg,WM_SETTEXT,0,(LONG)(LPSTR)fixturefilen);
			HANDLE firstfile;
			BOOL next;
			int length;
			int id;
			if(lstrcmp(fixturefilen,"Fixture") == 0)
				id = 2;
			else if(lstrcmp(fixturefilen,"Inkjet") == 0)
				id = 4;
			switch(id)
			{
			case 2:       //Fixture
				firstfile = FindFirstFile("c:\\ims5000\\fixtures\\*.IMF",&fData);
				break;
			case 4:       //Inkjet
				firstfile = FindFirstFile("c:\\ims5000\\BIN\\IM5000.ini",&fData);     //----------Linghai-11-10-08------
				break;

			}
			if(id ==2)
			{
				if(firstfile != INVALID_HANDLE_VALUE)
				{
					do
					{
						length = lstrlen(fData.cFileName)-4;
						char fname[MAX_PATH];

						for(int i = 0; i < length; i++)
							fname[i] = fData.cFileName[i];
						fname[length] = '\0';
					
						SendDlgItemMessage(hdlg,IDC_FIXTUREFILE,CB_ADDSTRING,fixturefilel,(LPARAM)(LPSTR)fname);
						next = FindNextFile(firstfile,&fData);
					}while(next);
SendDlgItemMessage(hdlg,IDC_FIXTUREFILE,CB_SETCURSEL,0,0);
				}
			}//display fixtrue file name

			if(id == 4)                                                    //----------Linghai-11-10-08------begin if(id ==4)
			{
				if(firstfile != INVALID_HANDLE_VALUE)
				{
					fstream myfile("c:\\ims5000\\BIN\\IM5000.ini",ios::in);
					if(myfile.is_open())
					{
						char line[2000];
						int begin = 0;
						int over = 0;
						while(!myfile.eof())
						{
							myfile.getline(line,2000);
							if(begin == 1 )
							{
								if(strcmp(line,"") == 0)
								{
									over = 1;
								}
								if(over == 0)
								{
									//cout<<"this is what we want"<<endl;
									char *name;
									name = strtok(line,",");
									int i = 1;
									while (name != NULL)
									{
										if(i == 2)
										{
											SendDlgItemMessage(hdlg,IDC_FIXTUREFILE,CB_ADDSTRING,fixturefilel,(LPARAM)(LPSTR)name);
											//cout<<name<<endl;
											break;
										}
										name = strtok(NULL,",");
										i++;
									}
SendDlgItemMessage(hdlg,IDC_FIXTUREFILE,CB_SETCURSEL,0,0);
								}
								else if(over == 1)
								{
									break;
								}
							}
							if(strcmp(line,"[IJ]") == 0)
							{
								begin = 1;
							}
						}
						myfile.close();
					}
					else cout<<"Unable to open file";
				}
			}//display inkjet name                                       //----------Linghai-11-10-08------ end if(id ==4)
			
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
			switch(LOWORD(wP))
				{
				case IDOK:
					tlen=GetDlgItemText(hdlg,IDC_FIXTUREFILE,tbuf,fixturefilel+1);
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDC_SEARCHFILE:                //-----------Linghai-11-18-08---begin-idc-searchfile
OPENFILENAME ofn; // common dialog box structure
					char pathname[MAX_PATH]; // buffer for file name
					char title[MAX_PATH]; // buffer for file title
					//		HWND hwnd; // owner window
					//		HANDLE hf; // file handle

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hdlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;
					//
					// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
					// use the contents of szFile to initialize itself.
					//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					ofn.lpstrFilter = "All Files\0*.*\0\0";
					ofn.nFilterIndex = 0;
					
					ofn.lpstrInitialDir = initdir;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if (GetOpenFileName(&ofn) && ofn.lpstrFileTitle)
					{
						SetDlgItemText(hdlg,IDC_FIXTUREFILE,ofn.lpstrFileTitle); 
					}
					/*char dirname[MAX_PATH];
					//GetModuleFileName(IMSInst,dirname,sizeof(dirname));
					strcpy(dirname,"C:\\ims5000\\fixtures\\"); 
					p = strrchr(dirname,'\\');
					if (p) 
						*p = 0;

					OPENFILENAME ofn; // common dialog box structure
					char pathname[MAX_PATH]; // buffer for file name
					char title[MAX_PATH]; // buffer for file title
			//		HWND hwnd; // owner window
			//		HANDLE hf; // file handle
				
					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hdlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;
				//
				// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
				// use the contents of szFile to initialize itself.
				//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					//ofn.lpstrFilter = "Dynamic Link Libraries (.dll)\0*.DLL\0";
					ofn.lpstrFilter = "All Files\0*.*\0\0";
					ofn.nFilterIndex = 0;
				
					ofn.lpstrInitialDir = dirname;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				
				// Display the Open dialog box. 
					if (GetOpenFileName(&ofn) && ofn.lpstrFileTitle)
					{
						SetDlgItemText(hdlg,IDC_FIXTUREFILE,ofn.lpstrFileTitle); 
					}	
					//ShellExecute(NULL,"open","c:\\","","",SW_SHOWNA);*/
						return TRUE;                 //--------------Linghai-11-18-08-----end-idc-searchfile
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
		}
	return FALSE;            // Message not handled
	}

/***************************************************************************
	PUBLIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	FAR PUBLIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	void SetMode(newmode)

	Changes the application mode.  The correct mode window is placed
	in the mode area, the correct menu is loaded, and the window
	caption is set.

	MODE newmode = New application mode.
***************************************************************************/
void SetMode(IMSMODE newmode)
	{
	RECT r;
	LONG w,h;
	HMENU m;
	char *mname; // menu name
	char *pbasefn = NULL; // base filename

	// Destroy existing mode window
	if(ModeWnd)
		DestroyWindow(ModeWnd),ModeWnd=0;

	// Destroy old menu
	m=GetMenu(IMSWnd);
	if(m)
		DestroyMenu(m);

	// Calculate mode window size
	GetClientRect(IMSWnd,&r);
	w=r.right;
	h=r.bottom-((7*sftm.tmHeight>>2)+4);
	//h=r.bottom-((7*sftm.tmHeight>>2));

	switch(newmode)
		{
		case mLOGIN:
			// Create window
			ModeWnd=CreateLoginWnd(IMSWnd,IDC_MODE,0,0,w,h);
			// Update menu
			m=LoadMenu(IMSInst,"LOGIN");
			SetMenu(IMSWnd,m);
			// Update caption
			wsprintf(tbuf,"%s-ROTARY :LOGIN               %s"
					,(LPSTR)IMSName
					,(LPSTR)IMSMILSTD
					);
			SetWindowText(IMSWnd,tbuf);
			break;
		//case mFILES1:
			//CreateInputWnd(IMSWnd);
			//break;
		case mFILES:
			// Create window
			ModeWnd=CreateFilesWnd(IMSWnd,IDC_MODE,0,0,w,h);


			// Update menu
			if(UserLevel==4)
				mname="FILESD";
			else if(UserLevel==3)
				mname="FILESC";
			else if(UserLevel==2)
				mname="FILESB";
			else
				mname="FILESA";
			m=LoadMenu(IMSInst,mname);
			
			//InsertMenuItem(m,ID_RECENTFILES,FALSE,"TEST");
/*			char tbuf[512];
			int i;
			for(i=0;i<4;i++)
			{
				sprintf(tbuf,"%d",(i+1));
				GetPrivateProfileString("recent",tbuf,"NA",
					tbuf,sizeof(tbuf),IMSIni);
				if(strcmp(tbuf,"NA"))
				{
					InsertMenu(m,ID_RFILE1,MF_BYCOMMAND,ID_RECENTF1+i,tbuf);
				}
			}*/
			//SetDlgItemText(IMSWnd,ID_RFILE1,"sdfd");
			SetMenu(IMSWnd,m);
			if(NoCal)
				{
				DeleteMenu(m,IDM_CAL,MF_BYCOMMAND);
				DrawMenuBar(IMSWnd);
				}
			// Update caption
			wsprintf(tbuf,"%s-ROTARY :FILES               %s"
					,(LPSTR)IMSName
					,(LPSTR)IMSMILSTD
					);
			SetWindowText(IMSWnd,tbuf);
			//DialogBox(IMSInst,"FIXTUREFILE",IMSWnd,(DLGPROC)SERINPUTDP);
			IMSMode=newmode; // CodeRay Modifified 9/29/2010

///
break;/// CodeRay Mod 3/12/2012 skip the input window
//			CreateInputWnd(IMSWnd);
//			return; // CodeRay Modified 9/29/2010
			// since CreateInputWnd is modal, the break
			// here would cause IMSMode to be set to mFILES
			// after a recursive call to this function from
			// within CreateInputWnd context.
			// return instead of break skips this.
//this is the original code (before the bug)
			//DestroyWindow(ModeWnd);
			//DialogBox(IMSInst,"FILESERINPUT",IMSWnd,(DLGPROC)SERINPUTDP);//---------------------siemens input window---Linghai-4/6/10--
//CR		break;

		case mEDIT:
			// Create window
			ModeWnd=CreateEditorWnd(IMSWnd,IDC_MODE,0,0,w,h);
			// Update menu
			mname="EDITOR";
			m=LoadMenu((HINSTANCE)IMSInst,mname);
			SetMenu(IMSWnd,m);
			// Grey IDC_SAVE if UserLevel <3
			if(UserLevel<3)
				EnableMenuItem(m,IDM_SAVE,MF_GRAYED|MF_BYCOMMAND);
			if(!QRCoderIsAvailable) // Remove QR if no QRCODER.EXE
				DeleteMenu(m,IDM_NEWCQRMARK,MF_BYCOMMAND);
			if(!DM001IsLinked) // Remove DM001 add menu item if no DLL
			{
				DeleteMenu(m,IDM_NEWCDMMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWUIDMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWFORMAT12,MF_BYCOMMAND);           //-------------Linghai-11-21-08--
				DeleteMenu(m,IDM_NEWFORMAT6,MF_BYCOMMAND);           //-------------Linghai-11-21-08--
				DeleteMenu(m,IDM_NEWFORMAT5,MF_BYCOMMAND);           //-------------Linghai-11-21-08--
			}
			if(NoFontMarks)
				{
				DeleteMenu(m,IDM_NEWTEXTMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWDEVBCMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWDEVSYMMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWMULTILINEMARK,MF_BYCOMMAND);
				}
			if(NoBitmapMarks)
				{
				DeleteMenu(m,IDM_NEWC93MARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWC128MARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWCUPCEANMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWCDMMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWUIDMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWFORMAT12,MF_BYCOMMAND);           //-------------Linghai-11-21-08--
				DeleteMenu(m,IDM_NEWFORMAT6,MF_BYCOMMAND);           //-------------Linghai-11-21-08--
				DeleteMenu(m,IDM_NEWFORMAT5,MF_BYCOMMAND);           //-------------Linghai-11-21-08--
				DeleteMenu(m,IDM_NEWBITMAPMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWWINFONTMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWPATCHMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWWMTRXMARK,MF_BYCOMMAND);
				DeleteMenu(m,IDM_NEWCQRMARK,MF_BYCOMMAND);
				}
			if(NoAlign)
				DeleteMenu(m,3,MF_BYPOSITION);					// removes POPUP "A&lign" in this position see IMS5000.rc
			if(NoCal)
				DeleteMenu(m,IDM_CAL,MF_BYCOMMAND);
			if(!IMCureExists())
				DeleteMenu(m,IDM_NEWCURESTEP,MF_BYCOMMAND);
			if(NoBitmapMarks || NoFontMarks || NoAlign || NoCal)
				DrawMenuBar(IMSWnd);
			// Update caption

			char capt[MAX_PATH];
			capt[0]=0;
			// zzz 2/26
			if (EditorIMP.fn[0]!=0){
				GetFileTitle(EditorIMP.fn,capt,MAX_PATH);
				pbasefn=&capt[0];
			}

			wsprintf(tbuf,"%s-ROTARY :EDITOR - %s          %s"
					,(LPSTR)IMSName
					,pbasefn ? pbasefn : (LPSTR)"(untitled)"
					,(LPSTR)IMSMILSTD
					);
			SetWindowText(IMSWnd,tbuf);
			break;

		case mRUN:
			// Disable VisOn if not starting DDERUN
			if(RunMode!=rmDDERUN)
				VisOn=0;
			// Update menu
			if(RunMode==rmDDERUN)
				mname="DDERUN";
			else
				mname=UserLevel>1 ?  "RUN2" : "RUN1";
			m=LoadMenu((HINSTANCE)IMSInst,mname);
			SetMenu(IMSWnd,m);
			// Delete menu entries for hidden options
			if(UserLevel>1)
				{
				if(HideOptModeMenu)
					DeleteMenu(m,1,MF_BYPOSITION);
				else
					{
					if(HideOptLink)
						{
						DeleteMenu(m,IDM_SUPPRESSLINK,MF_BYCOMMAND);
						DeleteMenu(m,IDM_SUPPRESSLINKAUTOSTART,MF_BYCOMMAND);
						}
					if(HideOptConveyor)
						DeleteMenu(m,IDM_SUPPRESSCONVEYOR,MF_BYCOMMAND);
					}
				}
			if(NoCal)
				DeleteMenu(m,IDM_CAL,MF_BYCOMMAND);
			if(UserLevel>1 || NoCal)
				DrawMenuBar(IMSWnd);
			// Create window
			ModeWnd=CreateRunWnd(IMSWnd,IDC_MODE,0,0,w,h);
			if(updateVar)
			{
				UpdateVar(IMSWnd);
				updateVar = FALSE;
			}
			// Update caption
//			pbasefn=GetOFSBaseName(&RunIMF.imp.ofs);  // zzz 2/26
			//char capt[MAX_PATH];
			capt[0]=0;
			// zzz 2/26
			if (RunIMF.imp.fn[0]!=0){
				GetFileTitle(RunIMF.imp.fn,capt,MAX_PATH);
				pbasefn=&capt[0];
			}

			wsprintf(tbuf,"%s:%s - %s          %s"
					,(LPSTR)IMSName
					,RunMode==rmTEST ? (LPSTR)"-ROTARY TEST" :
									RunMode==rmDDERUN ? (LPSTR)"DDERUN" : (LPSTR)"RUN"
					,pbasefn ? pbasefn : (LPSTR)"(untitled)"
					,(LPSTR)IMSMILSTD
					);
			SetWindowText(IMSWnd,tbuf);
			break;

		case mIMFEDIT:
			// Create window
			ModeWnd=IMFOpenEdit(IMSWnd,IDC_MODE,0,0,w,h);
			// Update menu
			mname="IMFEDIT";
			m=LoadMenu(IMSInst,mname);
			SetMenu(IMSWnd,m);
			if(NoAlign)
				DeleteMenu(m,1,MF_BYPOSITION);
			if(NoCal)
				DeleteMenu(m,IDM_CAL,MF_BYCOMMAND);
			if(NoAlign||NoCal)
				DrawMenuBar(IMSWnd);
			// Update caption
			//char capt[MAX_PATH];
			capt[0]=0;
			// zzz 2/26
			if (EditIMF.fn[0]!=0){
				GetFileTitle(EditIMF.fn,capt,MAX_PATH);
				pbasefn=&capt[0];
			}
			//GetFileTitle(EditIMF.fn,pbasefn,MAX_PATH);  //zzz 2/26
			wsprintf(tbuf,"%s-ROTARY :FIXTURE EDITOR - %s           %s"
					,(LPSTR)IMSName
					,pbasefn ? pbasefn : (LPSTR)"(untitled)"
					,(LPSTR)IMSMILSTD
					);
			SetWindowText(IMSWnd,tbuf);
			break;
		}
	IMSMode=newmode;      // Save the mode
	SetFocus(ModeWnd); // Set focus to it.
	}

/***************************************************************************
	error = chddir(ddir)

	Changes the current drive and directory.  Simmilar to _chdir except
	that the drive is also changed.

	BOOL error = TRUE if error occured.
	char *ddir = drive and directory (must have drive letter)
***************************************************************************/
BOOL chddir(LPSTR ddir)
	{
	char driveletter;
	char nddir[128]; // near copy of ddir (for _chdir)

	driveletter=ddir[0];
	driveletter=(BYTE)(DWORD)AnsiUpper((LPSTR)(DWORD)driveletter);
	if(_chdrive(driveletter-'A'+1))
		return 1;
	lstrcpy(nddir,ddir);
	return _chdir(nddir)!=0;
	}

/***************************************************************************
	getddir(ddir)

	Gets the current drive and directory.  Simmilar to _getdcwd except that
	this can be called from far segs.

	char *ddir = drive and directory
***************************************************************************/
void getddir(LPSTR ddir)
	{
	char nddir[128]; // near processing buffer
	_getdcwd(0,nddir,128); // Get current dir and drive
	lstrcpy(ddir,nddir);
	}

/***************************************************************************
	br = _lread32(hFile,lpBuf,dwBytes)

	32-bit implemenation of _lread()

	br = Number of bytes read

    hFile = Handle to a file
	lpBuf = Buffer for read
	dwBytes = Number of bytes to read
***************************************************************************/
DWORD _lread32(HANDLE hFile,LPVOID lpBuf,DWORD dwBytes)
	{
	DWORD br;
	ReadFile(hFile,lpBuf,dwBytes,&br,NULL);
	return br;
	}

/***************************************************************************
	bw = _lwrite32(hFile,lpBuf,dwBytes)

	32-bit implemenation of _lwrite()

	bw = Number of bytes written

    hFile = Handle to a file
	lpBuf = Buffer for write
	dwBytes = Number of bytes to write
***************************************************************************/
DWORD _lwrite32(HANDLE hFile,LPCSTR lpBuf,DWORD dwBytes)
	{
	DWORD bw;
	WriteFile(hFile,lpBuf,dwBytes,&bw,NULL);
	return bw;
	}

/***************************************************************************
	fp = _llseek32(hFile,lpBuf,dwBytes)

	32-bit implemenation of _llseek()

	fp = New offset of file pointer

	hFile = Handle to a file
	lOffset = Number of bytes to move 
	iOrigin = starting poisition: FILE_BEGIN, FILE_CURRENT, FILE_END 
***************************************************************************/
DWORD _llseek32(HANDLE hFile,LONG lOffset,DWORD iOrigin){
	if (!iOrigin){
		iOrigin = FILE_BEGIN;
	}
	return SetFilePointer(hFile,lOffset,NULL,iOrigin);  
}

/********************************************************************
Function to do Inkjet process
********************************************************************/
/***************************************************************************
	fn = FindFirstFile(pattern)

	Finds the first file with the given pattern in the current dir.
	Can be called from far segments.

	char *fn = name of first found file or NULL if no files found.
	char *pattern = DOS wildcard pattern.
***************************************************************************/
char * FindFirstFile(char *pattern)
	{
	
	nxtFile=FindFirstFile(pattern,(LPWIN32_FIND_DATA) &fData);
	if(!nxtFile)
	{
		return NULL;
	}
	//if(_dos_findfirst(pattern,_A_NORMAL,&FindT))
	//	return NULL;
	//return FindT.name;
	return fData.cFileName;
	}

/***************************************************************************
	fn = FindNextFile()

	Finds the next filein the current dir.
	Must only be called after FindFirstFile().
	Can be called from far segments.

	char *fn = name of next found file or NULL if no files found.
***************************************************************************/
char * FindNextFile(void)
	{
	if(FindNextFile(nxtFile,(LPWIN32_FIND_DATA) &fData))
		return NULL;
	return fData.cFileName;
	}

/***************************************************************************
	ok = DoPrompt(parent,name,length,def)

	Runs the prompt dialog.  Copies entered text to tbuf.
	Copies the length to tlen.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	LPSTR name= Prompt name.
	WORD length = Maximum length of input.
	LPSTR def = Default string.
***************************************************************************/
BOOL DoPrompt(HWND parent,LPSTR name,WORD length,LPSTR def)
	{
	BOOL ok;

	lstrcpy(tbuf,def);
	promptl=length;
	lstrcpy(promptn,name);
	ok=DialogBox(IMSInst,"PROMPT",parent,(DLGPROC)PROMPTDP);

	return ok;
	}

/***************************************************************************
	ok = DoPromptO(parent,name,length,def)

	Runs the prompt1 dialog.  Copies entered text to tbuf.
	Copies the length to tlen.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	LPSTR name= Prompt name.
	WORD length = Maximum length of input.
	LPSTR def = Default string.
***************************************************************************/
BOOL DoPromptO(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1)
	{
	BOOL ok;

	lstrcpy(tbuf,def);
	lstrcpy(tbuf1,def1);
	promptl=length/2;
	lstrcpy(promptn,name);
	ok=DialogBox(IMSInst,"PROMPTO",parent,(DLGPROC)PROMPTODP);

	return ok;
	}

/***************************************************************************
	ok = DoPromptH(parent,name,length,def)

	Runs the prompt1 dialog.  Copies entered text to tbuf.
	Copies the length to tlen.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	LPSTR name= Prompt name.
	WORD length = Maximum length of input.
	LPSTR def = Default string.
***************************************************************************/
BOOL DoPromptH(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1)
	{
	BOOL ok;

	lstrcpy(tbuf,def);
	lstrcpy(tbuf1,def1);
	promptl=length/2;
	lstrcpy(promptn,name);
	ok=DialogBox(IMSInst,"PROMPTH",parent,(DLGPROC)PROMPTHDP);

	return ok;
	}
/***************************************************************************
	ok = DoPromptP(parent,name,length,def)

	Runs the prompt1 dialog.  Copies entered text to tbuf.
	Copies the length to tlen.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	LPSTR name= Prompt name.
	WORD length = Maximum length of input.
	LPSTR def = Default string.
***************************************************************************/
BOOL DoPromptP(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1,LPSTR def2)
	{
	BOOL ok;

	lstrcpy(tbuf,def);
	lstrcpy(tbuf1,def1);
lstrcpy(tbuf2,def2);
	promptl=length/3;
	lstrcpy(promptn,name);
	ok=DialogBox(IMSInst,"PROMPTP",parent,(DLGPROC)PROMPTPDP);

	return ok;
	}

/***************************************************************************
	ok = DoPromptPD(parent,name,length,def)

	Runs the prompt1 dialog.  Copies entered text to tbuf.
	Copies the length to tlen.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	LPSTR name= Prompt name.
	WORD length = Maximum length of input.
	LPSTR def = Default string.
***************************************************************************/
BOOL DoPromptPD(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1)
	{
	BOOL ok;

	lstrcpy(tbuf,def);
	lstrcpy(tbuf1,def1);
	promptl=length/3;
	lstrcpy(promptn,name);
	ok=DialogBox(IMSInst,"PROMPTP",parent,(DLGPROC)PROMPTPDP);

	return ok;
	}
/*********************************************************************
------------Linghai-11-6-08-----
DisplayFixtureFile(parent,name,length,def)
New function for edit window, if user click Fixture, there is a 
Combobox window pop up and show all fixture files.
***********************************************************************/
BOOL DoFixtureFile(HWND parent,LPSTR name, WORD length, LPSTR def)
{
	BOOL ok;

    lstrcpy(tbuf,def);
	fixturefilel=length;
	lstrcpy(fixturefilen,name);
	if(lstrcmp(name,"Inkjet") == 0)
		ok=DialogBox(IMSInst,"FIXTUREFILEB",parent,(DLGPROC)FIXTUREFILEDP);
	else
		ok=DialogBox(IMSInst,"FIXTUREFILE",parent,(DLGPROC)FIXTUREFILEDP);

	return ok;
}


/***************************************************************************
	ShowError(ide)

	Displays an error message from the resource file in a message
	box along with the error ide number.  If the resource string
	cannot be loaded, the box still displays the ide number.
	If ide is 0 then IDError is used.
	If ide is IDE_INTERNAL then an internal error is displayed.

	WORD ide = Error ID or 0.
***************************************************************************/
void ShowError(WORD ide)
	{
	char ebuf[81];

	if(!ide)
		ide=IDError;

	if(ide==IDE_VALIDATOR)
		{
		if(ValidatorGetError())
			lstrcpy(tbuf,ValidatorGetError());
		else
			lstrcpy(tbuf,"Unknown validator error!");
		}
	else if(ide==IDE_INTERNAL)
		{
		wsprintf(tbuf,"Internal error at %d in %s",
			iErrorLine,(LPSTR)iErrorFile);
		//IMSLog(IMSName,0,tbuf);
		wsprintf(tbuf,"Internal Error\n%d:%s",iErrorLine,(LPSTR)iErrorFile);
		// for CIMple
		if(CIMpleTrapError)
			{
			wsprintf(ebuf,"Internal %d:%s",iErrorLine,(LPSTR)iErrorFile);
			CIMpleSendError(ide,IMSName,ebuf);
			}
		}
	else
		{
		if(!LoadString((HINSTANCE)IMSInst,ide,ebuf,sizeof(ebuf)))
			lstrcpy(ebuf,"Unknown");
		DWORD err=GetLastError();
		tlen=wsprintf(tbuf,"Error : %d\n%s",ide,(LPSTR)ebuf);
		//IMSLog(IMSName,ide,ebuf);
		// CIMple support
		if(CIMpleTrapError)
			CIMpleSendError(ide,IMSName,ebuf);
		}

	CIMpleTrapError=FALSE;
	// RedLamp support
	if(RedLampTrapError)
		{
		SetLamps(REDLAMP);
		RedLampTrapError=FALSE;
		}

	MessageBox(GetActiveWindow(),tbuf,IMSName,
		MB_OK|MB_ICONEXCLAMATION);
	}

/***************************************************************************
	ShowWarning(idw)

	Displays an warning message from the resource file in a message box.

	WORD idw = Warning ID.
***************************************************************************/
void ShowWarning(WORD idw,LPCSTR s)
	{
	if(!LoadString(IMSInst,idw,tbuf,TSIZE))
		return;
	if(s)
		{
		lstrcat(tbuf,"\n");
		lstrcat(tbuf,s);
		}
	MessageBox(GetActiveWindow(),tbuf,IMSName,MB_OK|MB_ICONINFORMATION);
//	IDSMessageBox(idw,MB_OK|MB_ICONINFORMATION);
	}

/***************************************************************************
	ret = IDSMessageBox(WORD ids,WORD flags)

	Displays a message box with the message comming from the stringtable.

	int ret = MessageBox return value
	WORD ids = String ID.
	WORD flags = MessageBox flags
***************************************************************************/
int IDSMessageBox(WORD ids,WORD flags)
	{
	if(!LoadString(IMSInst,ids,tbuf,TSIZE))
		return 0;
	return MessageBox(GetActiveWindow(),tbuf,IMSName,flags);
	}

/***************************************************************************
	len = GetDate(buf,fmt,ofs,base)

	Makes a string representation of the current date+offset using
	the format specification in fmt.

	Date format replacements are as follows:
	    Y     = 1 digit year
	    YY    = 2 digit year
	    YYYY  = 4 digit year
	    M     = 1|2 digit month
	    MM    = 2 digit month
	    Mmm   = 3 letter month
	    MMM   = Mmm all caps
	    Mmmm  = The month as a word
	    MMMM  = Mmmm all caps
	    D     = 1|2 digit day of month
	    DD    = 2 digit day of month
	    Ddd   = 3 letter day of week
	    DDD   = Ddd all caps
	    Dddd  = The day of the week as a word
	    DDDD  = Dddd all caps
	    W     = 1|2 digit week of year
	    WW    = 2 digit week of year
	    J     = 1..3 letter julian day of year
	    JJJ   = 3 letter julian day of year
	    PPPPPP= Packed MMDDYY 0..9 = A..Z
	All other characters are passed through.

	int len = length of date in buf.
	LPSTR buf = buffer to receive date.
	LPSTR fmt = Formatting string.
	long ofs = clock offset
	LPSTR base = base conversion name or NULL for none
***************************************************************************/
int GetDate(LPSTR buf,LPSTR fmt,long ofs,LPSTR base)
	{
	time_t t;
	struct tm *tm;
	int len;
	LPSTR pb,pf;  // pbuf and pfmt;
	int i;
	char pbuf[7];
	LPCSTR bset;

	static char *sMmmm[]=
		{"January","February","March","April","May","June",
		"July","August","September","October","November","December"};
	static char *sMMMM[]=
		{"JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE",
		"JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER"};
	static char *sMmm[]=
		{"Jan","Feb","Mar","Apr","May","Jun",
		"Jul","Aug","Sep","Oct","Nov","Dec"};
	static char *sMMM[]=
		{"JAN","FEB","MAR","APR","MAY","JUN",
		"JUL","AUG","SEP","OCT","NOV","DEC"};
	static char *sDddd[]=
		{"Sunday","Monday","Tuesday","Wednesday",
		"Thursday","Friday","Saturday"};
	static char *sDDDD[]=
		{"SUNDAY","MONDAY","TUESDAY","WEDNESDAY",
		"THURSDAY","FRIDAY","SATURDAY"};
	static char *sDdd[]=
		{"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	static char *sDDD[]=
		{"SUN","MON","TUE","WED","THU","FRI","SAT"};

	static int dim[]={ // Days In Month for Julian calculation
		31,28,31,30,31,30,31,31,30,31,30,31};


	t=time(NULL)+ofs;
	tm=localtime(&t);

	pf=fmt;
	len=0;
	while(*pf)
		{
		pb=buf+len;
		//char tbuf[6];
		//CopyMemory(tbuf,pf,6);
		if(!memcmp(pf,"PPPPPP",6)) // Packed MMDDYY 0..9 = A..Z
		{
			wsprintf(pbuf,"%02d%02d%02d",
				tm->tm_mon+1,tm->tm_mday,tm->tm_year%100);
			for(i=0;i<6;i++)
				pb[i]=pbuf[i]-'0'+'A';
			len+=6;
			pf+=6;
			continue;
			}
		if(!memcmp(pf,"YYYY",4)) // 4 digit year
			{
			len+=wsprintf(pb,"%04d",tm->tm_year+1900);
			pf+=4;
			continue;
			}
		if(!memcmp(pf,"Mmmm",4)) // Month as a word
			{
			len+=wsprintf(pb,"%s",(LPSTR)sMmmm[tm->tm_mon]);
			pf+=4;
			continue;
			}
		if(!memcmp(pf,"MMMM",4)) // Month as a word all caps
			{
			len+=wsprintf(pb,"%s",(LPSTR)sMMMM[tm->tm_mon]);
			pf+=4;
			continue;
			}
		if(!memcmp(pf,"Dddd",4)) // Day of week as word
			{
			len+=wsprintf(pb,"%s",(LPSTR)sDddd[tm->tm_wday]);
			pf+=4;
			continue;
			}
		if(!memcmp(pf,"DDDD",4)) // Day of week as word all caps
			{
			len+=wsprintf(pb,"%s",(LPSTR)sDDDD[tm->tm_wday]);
			pf+=4;
			continue;
			}
		if(!memcmp(pf,"Mmm",3)) // 3 letter month
			{
			len+=wsprintf(pb,"%s",(LPSTR)sMmm[tm->tm_mon]);
			pf+=3;
			continue;
			}
		if(!memcmp(pf,"MMM",3)) // 3 letter month all caps
			{
			len+=wsprintf(pb,"%s",(LPSTR)sMMM[tm->tm_mon]);
			pf+=3;
			continue;
			}
		if(!memcmp(pf,"Ddd",3)) // 3 letter day of week
			{
			len+=wsprintf(pb,"%s",(LPSTR)sDdd[tm->tm_wday]);
			pf+=3;
			continue;
			}
		if(!memcmp(pf,"DDD",3)) // 3 letter day of week all caps
			{
			len+=wsprintf(pb,"%s",(LPSTR)sDDD[tm->tm_wday]);
			pf+=3;
			continue;
			}
		if(!memcmp(pf,"JJJ",3)) // 3 letter julian day of year
			{
			len+=wsprintf(pb,"%03d",tm->tm_yday+1);
			pf+=3;
			continue;
			}
		if(!memcmp(pf,"YY",2)) // 2 digit year
			{
			len+=wsprintf(pb,"%02d",tm->tm_year%100);
			pf+=2;
			continue;
			}
		if(!memcmp(pf,"MM",2)) // 2 digit month
			{
			len+=wsprintf(pb,"%02d",tm->tm_mon+1);
			pf+=2;
			continue;
			}
		if(!memcmp(pf,"DD",2)) // 2 digit day of month
			{
			len+=wsprintf(pb,"%02d",tm->tm_mday);
			pf+=2;
			continue;
			}
		if(!memcmp(pf,"WW",2)) // 2 digit week of year
			{
			len+=wsprintf(pb,"%02d",tm->tm_yday/7+1);
			pf+=2;
			continue;
			}
		if(*pf=='Y') // 1 digit year
			{
			len+=wsprintf(pb,"%d",tm->tm_year%10);
			pf++;
			continue;
			}
		if(*pf=='M') // 1|2 digit month
			{
			len+=wsprintf(pb,"%d",tm->tm_mon+1);
			pf++;
			continue;
			}
		if(*pf=='D') // 1|2 digit day of month
			{
			len+=wsprintf(pb,"%d",tm->tm_mday);
			pf++;
			continue;
			}
		if(*pf=='W') // 1|2 digit week of year
			{
			len+=wsprintf(pb,"%d",tm->tm_yday/7+1);
			pf+=1;
			continue;
			}
		if(*pf=='J') // 1..3 letter julian day of year
			{
			len+=wsprintf(pb,"%d",tm->tm_yday+1);
			pf+=1;
			continue;
			}
		// others
		*pb=*pf;
		len++;
		pf++;
		}
	buf[len]=0;
	if(base)
		{
		bset=IMPGetBasedSet(base);
		if(bset)
			if(lstrlen(bset)==10)
				MapDigits(buf,bset);
		}
	return len;
	}

/***************************************************************************
	len = GetTime(buf,useNames,is24,isLZ,seperator,
		amSuffix,pmSuffix,showSeconds,offset,base)

	Makes a string representation of the current time.

	int len = length of time in buf.
	LPSTR buf = buffer to receive time.
	BOOL useNames=Flag to use TimeNames if true, other formatting is ignored
	BOOL is24=Flag for 24 hour clock
	BOOL isLZ=Flag for leading zero on hours
	char seperator=Seperator character or 0 if none
	LPSTR amSuffix=AM time suffix if(!is24) or NULL if none
	LPSTR pmSuffix=PM or 24hour time suffix or NULL if none
	BOOL showSeconds=Flag for seconds display
	long offset = clock offset
	LPSTR base = base conversion name or NULL for none
***************************************************************************/
int GetTime(LPSTR buf,BOOL useNames,BOOL is24,
	BOOL isLZ,char seperator,LPSTR amSuffix,LPSTR pmSuffix,
	BOOL showSeconds,long offset,LPSTR base)
	{
	time_t t;
	struct tm *tm;
	int len;
	int h;
	LPCSTR bset;

	t=time(NULL)+offset;
	if(useNames)
		return GetTimeName(buf,t);
	tm=localtime(&t);
	// HH
	h=tm->tm_hour;
	if(!is24)
		{
		h%=12;
		if(h==0)
			h=12;
		}
	len=wsprintf(buf,isLZ ? "%02d" : "%d",h);
	// :
	if(seperator)
		buf[len++]=seperator;
	// MM
	len+=wsprintf(buf+len,"%02d",tm->tm_min);
	if(showSeconds)
		{
		// :
		if(seperator)
			buf[len++]=seperator;
		// SS
		len+=wsprintf(buf+len,"%02d",tm->tm_sec);
		}
	// Suffix
	if(!is24)
		{

		if(tm->tm_hour<12) // AM
			{
			if(amSuffix && lstrlen(amSuffix))
				len+=wsprintf(buf+len," %s",amSuffix);
			}
		else // PM
			{
			if(pmSuffix && lstrlen(pmSuffix))
				len+=wsprintf(buf+len," %s",pmSuffix);
			}
		}

	if(base)
		{
		bset=IMPGetBasedSet(base);
		if(bset)
			if(lstrlen(bset)==10)
				MapDigits(buf,bset);
		}
	return len;
	}

/***************************************************************************
	GetGMT(buf)

	Makes a string representation of the current GNT date and time of the
	form YYYYMMDDHHMMSS.

	LPSTR buf = buffer to receive time.
***************************************************************************/
void GetGMT(LPSTR buf)
	{
	time_t t;
	struct tm *tm;

	t=time(NULL);
	tm=gmtime(&t);
	wsprintf(buf,"%04d%02d%02d%02d%02d%02d",
		tm->tm_year+1900,
		tm->tm_mon+1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);
	}

/***************************************************************************
	l = VectToString(s,v,n,sep)

	Generates a string representation of a floating point vector.

	WORD l = length of resultant string.
	LPSTR s = string to receive result.
	LPDOUBLE v = vector to convert.
	int n = length of vector.
	LPSTR sep = seperator string.
***************************************************************************/
WORD VectToString(LPSTR s,LPDOUBLE v,int n,LPSTR sep)
	{
	static char fbuf[32];
	int i,l,lsep;
	double nv; // Near value

	l=0;
	lsep=lstrlen(sep);
	for(i=0;i<n;i++)
		{
		nv=*(v+i);
		sprintf(fbuf,"%0.4f",nv);
		if(i)
			{
			lstrcpy(s+l,sep);
			l+=lsep;
			}
		lstrcpy(s+l,fbuf);
		l+=lstrlen(fbuf);
		}
	s[l]=0;
	return l;
	}

/******************************************************************************
	MScaleVect(v,n,inv)

	Scales a vector to metric or from metric.
	If !Metric then no scaling occures.

	LPDOUBLE v = vector to scale
	int n=size of vector
	BOOL inv=if true scales metric to normal (1/2.54)
******************************************************************************/
void MScaleVect(LPDOUBLE v,int n,BOOL inv)
	{
	double scale;
	if(Metric)
		{
		double f=MetricMM ? 25.4f : 2.54f;
		if(inv)
			scale=1.0f/f;
		else
			scale=f;
		while(n--)
			v[n]*=scale;
		}
	}

/******************************************************************************
	XYMScaleVect(ans,v)

	Scales to metric (if metric) the x and y units at the mark level.
	does not scale angular rotary units in XS or SY xymodes
******************************************************************************/
void XYMScaleVect(LPDOUBLE ans,LPDOUBLE v)
	{
	EditXYModePrep(0);
	ans[0]=v[0]*EditXScale;
	ans[1]=v[1]*EditYScale;
	}

/******************************************************************************
	XMScaleVect(ans,v)

	Scales to metric (if metric) the X unit at the mark level.
	does not scale angular rotary units in SY xy mode
******************************************************************************/
void XMScaleVect(LPDOUBLE ans,LPDOUBLE v)
	{
	EditXYModePrep(0);
	ans[0]=v[0]*EditXScale;
	}

/******************************************************************************
	YMScaleVect(ans,v)

	Scales to metric (if metric) the Y unit at the mark level.
	does not scale angular rotary units in XS xy mode
******************************************************************************/
void YMScaleVect(LPDOUBLE ans,LPDOUBLE v)
	{
	EditXYModePrep(0);
	ans[0]=v[0]*EditYScale;
	}

/***************************************************************************
	IMSGetDefDirs()
	Loads IMSProgs and IMSFixtures and BMPDir from programs= and fixtures=
	and BMPDir= respectivly from ims5000.ini.  if there is no fixtures=
	then IMSFixtures="" which means use the current dir.
***************************************************************************/
void IMSGetDefDirs(void)
	{
	// Get IMSProgs from programs= in ini
	GetPrivateProfileString(IMSName,"programs",IMSDir,
		IMSProgs,sizeof(IMSProgs),IMSIni);

	// IMSFixtures : default = "" = use dir of current IMP
	GetPrivateProfileString(IMSName,"fixtures","",
		IMSFixtures,sizeof(IMSFixtures),IMSIni);

	// IMSVisObjects : default =
	GetPrivateProfileString(IMSName,"VisObjects","",
		IMSVisObjects,sizeof(IMSVisObjects),IMSIni);

	// BMPDir : default = ""
	int l=GetPrivateProfileString(IMSName,"BMPDir","",
		BMPDir,sizeof(BMPDir),IMSIni);
	if(l)
		if(BMPDir[l-1]!='\\')
			lstrcat(BMPDir,"\\");
	}

/******************************************************************************
	RedGreenInit()

	Called during app init to initialize the optional RedGreen lamp support.
******************************************************************************/
void RedGreenInit(void)
	{
	RedGreenOpen=FALSE;

	// Map out bits
	if(!(bitRedLamp=IMMapOutBit(sRedLamp)))
		return;
	if(!(bitGreenLamp=IMMapOutBit(sGreenLamp)))
		return;
	RedGreenOpen=TRUE;
	}

/******************************************************************************
	SetLamps(lampcode)
	
	Sets the state of the status lamps if they are installed.

	int lampcode = NOLAMP|GREENLAMP|REDLAMP
******************************************************************************/
void SetLamps(int lampcode)
	{
	if(!RedGreenOpen)
		return;
	if(RunMode==rmTEST)
		return;
	switch(lampcode)
		{
		case NOLAMP:
			IMSetOutBit(bitRedLamp,FALSE);
			IMSetOutBit(bitGreenLamp,FALSE);
			break;
		case GREENLAMP:
			IMSetOutBit(bitRedLamp,FALSE);
			IMSetOutBit(bitGreenLamp,TRUE);;
			break;
		case REDLAMP:
			IMSetOutBit(bitRedLamp,TRUE);
			IMSetOutBit(bitGreenLamp,FALSE);;
			break;
		}
	}	

/******************************************************************************
******************************************************************************/
void CIMpleInit(void)
	{
	static char comn[]="COMnnn"; // Comm device name (when n is filled in)
	DCB  dcb; // Comm device control block

	if(CIMpleIsOpen)
		return;

	// Load the CIMpleMark
	if(!GetPrivateProfileString(sCIMple,"Mark","",
		CIMpleMark,sizeof(CIMpleMark),IMSIni))
		return;

	// Load the comm port def from the ini file into tbuf
	if(!GetPrivateProfileString(sCIMple,"Port","",
		tbuf,sizeof(tbuf),IMSIni))
		return;

	if(BuildCommDCB(tbuf,&dcb)) // Build the initial DCB
		return;
//zzz	wsprintf(&comn[3],"%d",dcb.Id+1);

	// Open the port
//zzz	CIMplePort=OpenComm(comn,100,200);
	if(CIMplePort<0)
		return;

//zzz	if(SetCommState(&dcb))    // Set its state to match dcb
		{
//zzz		CloseComm(CIMplePort);
		return;
		}
	CIMpleIsOpen=TRUE;
	}

/******************************************************************************
******************************************************************************/
void CIMpleQuit(void)
	{
	if(!CIMpleIsOpen)
		return;

//zzz	FlushComm(CIMplePort,0);              // Flush the transmit queue
//zzz	FlushComm(CIMplePort,1);              // Flush the receive queue
//zzz	CloseComm(CIMplePort);

	CIMpleIsOpen=FALSE;
	}

/******************************************************************************
******************************************************************************/
void CIMpleSendError(WORD ide,LPSTR module,LPSTR text)
	{
	char buf[80];
	int len;

	if(!CIMpleIsOpen)
		return;
	len=wsprintf(buf,"\002""ERROR: %d in %s %s\r",ide,module,text);
//zzz	WriteComm(CIMplePort,buf,len);
	}

/******************************************************************************
******************************************************************************/
void CIMpleSendMark(void)
	{
	int i;

	if(!CIMpleIsOpen)
		return;

	//zzz - New format
	IMPVALS *pvals=&(RunIMF.pvalslist[0]); // 1st val only

	// Search for UDHA CIMplePart
	for(i=0;i<RunIMF.imp.udhas.count;i++)                    // found
		if(!lstrcmp(RunIMF.imp.udhas.udha[i].name,"CIMple")) //  If CIMple defined
			if(lstrcmp(RunIMF.imp.udhas.udha[i].val,"0"))    //  and not 0
				break;

	if(i<RunIMF.imp.udhas.count) // If found
		{               // Compile and output
		if(IMPExpandTextToTbuf(CIMpleMark,
			&RunIMF.imp.vars,pvals,
			&RunIMF.imp.gvars,
			&RunIMF.imp.gvals,
			RunIMF.imp.pheader,
			&RunIMF.imp.udhas))
			{
//zzz			WriteComm(CIMplePort,"\002",1);
//zzz			WriteComm(CIMplePort,tbuf,lstrlen(tbuf));
//zzz			WriteComm(CIMplePort,"\r",1);
			}
		}
	}

/***************************************************************************
	UHISTInit()
***************************************************************************/
BOOL UHISTInit(void)
	{
	UniqueOverrideLevel=(WORD)GetPrivateProfileInt(IMSName,"UniqueOverrideLevel",0,IMSIni);
	histque.qbuf=NULL;
	histque.curr=0;
	histque.nque=0;
	histque.qsize=(WORD)GetPrivateProfileInt(IMSName,"UniqueHistorySize",0,IMSIni);
	if(!histque.qsize)
		return TRUE;
	histque.qbuf=new char[histque.qsize*UHISTVALSIZE];


	if(!histque.qbuf)
		{
		histque.qsize=0;
		ShowError(IDE_NOMEM);
		return FALSE;
		}
	return TRUE;
	}

/***************************************************************************
	UHISTFree()
***************************************************************************/
void UHISTFree(void)
	{
	if(histque.qbuf)
		{
		delete [] histque.qbuf;
		histque.qbuf=NULL;
		}
	histque.curr=0;
	histque.nque=0;
	histque.qsize=0;
	}

/***************************************************************************
	UHISTAdd()
***************************************************************************/
BOOL UHISTAdd(LPSTR s)
	{
	if(!histque.qbuf)
		return FALSE;
	if(UHISTFind(s))
		return FALSE;
	lstrcpyn(&histque.qbuf[histque.curr*UHISTVALSIZE],s,UHISTVALSIZE);
	histque.nque++;
	histque.nque%=histque.qsize;
	histque.curr++;
	histque.curr%=histque.qsize;
	return TRUE;
	}

/***************************************************************************
	UHISTFind()
***************************************************************************/
BOOL UHISTFind(LPSTR s)
	{
	WORD i;
	DWORD bi;
	if(!histque.qbuf)
		return FALSE;
	for(i=0;i<histque.nque;i++)
		{
		bi=i*UHISTVALSIZE;
		if(!lstrcmp(&histque.qbuf[bi],s))
			return TRUE;
		}
	return FALSE;
	}

/***************************************************************************
	UHISTClear()
***************************************************************************/
void UHISTClear(void)
	{
	histque.nque=0;
	histque.curr=0;
	}

/***************************************************************************
	SetFloatField
	Sets an edit control to a double value
***************************************************************************/
void SetFloatField(HWND hdlg,int idc,double f)
	{
	char buf[32];
	VectToString(buf,(LPDOUBLE)&f,1,"");
	SendDlgItemMessage(hdlg,idc,EM_LIMITTEXT,32,0L);
	SendDlgItemMessage(hdlg,idc,WM_SETTEXT,0,(LONG)(LPSTR)buf);
	}

/***************************************************************************
	ok = GetFloatField
	Gets a double from an edit control. If invalid, an error is shown and
	the focus is set to the offending control.
***************************************************************************/
BOOL GetFloatField(HWND hdlg,int idc,LPDOUBLE f)
	{
	char buf[32];
	int len=GetDlgItemText(hdlg,idc,buf,33);
	if(!ParseDouble(buf,(double*)f))
		{
		ShowError(IDE_BADFIELD);
		SetFocus(GetDlgItem(hdlg,idc));
		return FALSE;
		}
	return TRUE;
	}

/***************************************************************************
	STATIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	ok = IMSInit()

	Initializes the IMS application.
	Initializes IMSIsShell
	Registers the AppClass window class.

	BOOL ok = TRUE on success.
***************************************************************************/
static BOOL IMSInit(void)
	{
	WNDCLASS wc;
	LPSTR p;

//zzz	IMSIsShell=(GetNumTasks()==1);

	// Create IMSIni and IMSUsr from Module Filename
	tlen=GetModuleFileName(IMSInst,tbuf,TSIZE); // tbuf=.exe name
	p=tbuf;
	while(*p!='\\' && *p!=NULL)
	{
		p++;
	}
	p=strrchr(tbuf,'\\'); // p = & of last backslash
	if(!p)
		p=tbuf;
	else
		p++;
	*p=0;
	lstrcpy(IMSIni,tbuf);
	lstrcat(IMSIni,"IMS5000.INI");
	lstrcpy(IMSUsr,tbuf);
	lstrcat(IMSUsr,"IMS5000.USR");
	lstrcpy(CalIni,tbuf);
	lstrcat(CalIni,"CAL.INI");
	lstrcpy(CalHistIni,tbuf);
	lstrcat(CalHistIni,"CALHIST.INI");
	lstrcpy(IMIni,tbuf);
	lstrcat(IMIni,"IM5000.INI");

	// Get iniIcon from Icon= in ini
	iniIcon=GetPrivateProfileInt(IMSName,"Icon",0,IMSIni);
	iniIcon=iniIcon%NICONS;

	IMSIcon=LoadIcon((HINSTANCE)IMSInst,Icons[iniIcon]);

	// Fill in the IMS window class
	wc.style=CS_VREDRAW|CS_HREDRAW|CS_NOCLOSE;
	wc.lpfnWndProc=(WNDPROC)IMSWP;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=(HINSTANCE)IMSInst;
	wc.hIcon=IMSIcon;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.lpszClassName=IMSName;

	// Register the IMS window class
	if(!RegisterClass(&wc))
		return FALSE;

	_tzset();

	LoadTimeNames();

	HideOptRotation=(BOOL)GetPrivateProfileInt(sHideOpt,"Rotation",0,IMSIni);
	HideOptInspect=(BOOL)GetPrivateProfileInt(sHideOpt,"Inspect",0,IMSIni);
	HideOptRead=(BOOL)GetPrivateProfileInt(sHideOpt,"Read",0,IMSIni);
	HideOptConveyor=(BOOL)GetPrivateProfileInt(sHideOpt,"Conveyor",0,IMSIni);
	HideOptInit=(BOOL)GetPrivateProfileInt(sHideOpt,"Init",0,IMSIni);
	HideOptLink=(BOOL)GetPrivateProfileInt(sHideOpt,"Link",0,IMSIni);
	HideOptRotaryIMF=(BOOL)GetPrivateProfileInt(sHideOpt,"RotaryIMF",0,IMSIni);
	HideOptRotaryDEG=(BOOL)GetPrivateProfileInt(sHideOpt,"RotaryDEG",0,IMSIni);
	HideOptModeMenu=(BOOL)GetPrivateProfileInt(sHideOpt,"ModeMenu",0,IMSIni);
	HideOptMarkRotation=TRUE;
	HideOptMarkSlant=TRUE;
	if(!HideOptRotation)
		{
		HideOptMarkRotation=(BOOL)GetPrivateProfileInt(sHideOpt,"MarkRotation",0,IMSIni);
		if(!HideOptMarkRotation)
			HideOptMarkSlant=(BOOL)GetPrivateProfileInt(sHideOpt,"MarkSlant",0,IMSIni);
		}

	return TRUE; // ok
	}

/***************************************************************************
	ok = IMSInstInit(nCmdShow)

	Initializes the only instance of the application.
	Creates AppIni from the module name.
	Creates AppUsr from the module name.
	Gets variables from the INI file.
	Creates the IMS application window using WinRect for size and position.
	Loads the accelerators resource

	BOOL ok = TRUE on success.
	int nCmdShow = WinMain nCmdShow
***************************************************************************/
static BOOL IMSInstInit(int nCmdShow)
	{
	RECT wr;       // WinRect
	BOOL wrok;     // WinRect valid flag
	LONG n;        // n to ParseLong into
	int x,y,w,h;   // Window X, Y, Width, and Height
	HDC hdc;       // DISPLAY device context
	LPSTR p;

	// Create IMSDir from Module Filename
	GetModuleFileName((HINSTANCE)IMSInst,IMSDir,sizeof(IMSDir));
	p=IMSDir;
	while(*p!='\\' && *p!=NULL)
	{
		p++;
	}
	p=strrchr(IMSDir,'\\'); // p = & of last backslash
	if(!p)
		return FALSE;
	*p=0;

	// Read IMSProgs and IMSfixtures
	IMSGetDefDirs();

	// Get sftm
	hdc=GetDC(NULL);
	SaveDC(hdc);
	GetTextMetrics(hdc,&sftm);
	SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));
	GetTextMetrics(hdc,&sfftm);
	RestoreDC(hdc,-1);
	ReleaseDC(NULL,hdc);

	// Get WinRect from IMSIni to tbuf
	tlen=GetPrivateProfileString(IMSName,"WinRect","",
		tbuf,TSIZE,IMSIni);
	//MessageBox(NULL,tbuf,NULL,MB_OK);
	wrok=(tlen>0);
	// Convert tbuf into wr
	p=&tbuf[0];
	if(wrok)
		{
		p=ParseLong(p,&n);
		wrok=(p!=(LPSTR)0L);
		wr.left=n;
		}
	if(wrok)
		{
		p=ParseLong(p,&n);
		wrok=(p!=(LPSTR)0L);
		wr.top=n;
		}
	if(wrok)
		{
		p=ParseLong(p,&n);
		wrok=(p!=(LPSTR)0L);
		wr.right=n;
		}
	if(wrok)
		{
		p=ParseLong(p,&n);
		wrok=(p!=(LPSTR)0L);
		wr.bottom=n;
		}

	// Load x,y,w,h from wr or if !wrok set x,y,w,h to default
	if(wrok)
		{
		x=wr.left;
		y=wr.top;
		w=wr.right-wr.left;
		h=wr.bottom-wr.top;
		}
	else
		x=y=w=h=CW_USEDEFAULT;

	// Get WinMode from IMSIni
	tlen=GetPrivateProfileString(IMSName,"WinMode","0",
		tbuf,TSIZE,IMSIni);
	
	if(!ParseLong(tbuf,&n))
		n=0;
	WinMode=n;
	if(!(WinMode==SIZEFULLSCREEN
		||WinMode==SIZEICONIC
		||WinMode==SIZENORMAL))
		WinMode=SIZENORMAL;

	// Adjust nCmdShow if WinMode!=SIZENORMAL
	switch(WinMode)
		{
		case SIZEFULLSCREEN:
			nCmdShow=SW_MAXIMIZE;
			break;
		case SIZEICONIC:
			nCmdShow=SW_MINIMIZE;
		}


	// Create IMS application window
	IMSWnd=CreateWindow(IMSName,IMSName,WS_OVERLAPPEDWINDOW,x,y,w,h,
		NULL,NULL,(HINSTANCE)IMSInst,NULL);
	if(!IMSWnd)
		return FALSE;

	if(HideOptRotaryIMF)
	{
	// Get RehomePoint using IMWnd to create defaults if not found
	if(!GetPrivateProfileVect(IMSName,"RehomePoint",
		(double*)&RehomePoint,3,IMSIni))
		{
		// If not found compute default as top center
		RehomePoint.x=IMInfo.extent.x/2;
		RehomePoint.y=IMInfo.extent.y/200;
		RehomePoint.z=IMInfo.extent.z;
		}

	// Get StartPoint using IMWnd to create defaults if not found
	if(!GetPrivateProfileVect(IMSName,"StartPoint",
		(double*)&StartPoint,3,IMSIni))
		{
		// If not found compute default as top center
		StartPoint.x=IMInfo.extent.x/2;
		StartPoint.y=IMInfo.extent.y/2;
		StartPoint.z=IMInfo.extent.z;
		}
	}

	else    ///--------sam-------
	{
	// Get RehomePoint using IMWnd to create defaults if not found
	if(!GetPrivateProfileVect(IMSName,"RehomePoint",
		(double*)&rotrehomepoint,3,IMSIni))
		{
		// If not found compute default as top center
			rotrehomepoint.di=IMInfo.extentrot.di/2;
			rotrehomepoint.hi=IMInfo.extentrot.hi/200;
		}

	// Get StartPoint using IMWnd to create defaults if not found
	if(!GetPrivateProfileVect(IMSName,"StartPoint",
		(double*)&rotstartpoint,3,IMSIni))
		{
		// If not found compute default as top center
			rotstartpoint.di=IMInfo.extentrot.di/2;
			rotstartpoint.hi=IMInfo.extentrot.hi/2;
		}	
	}

	ZStartOnly=(BOOL)GetPrivateProfileInt(IMSName,"ZStartOnly",0,IMSIni);
	ReturnToStart=(BOOL)GetPrivateProfileInt(IMSName,"ReturnToStart",0,IMSIni);
	if(ZStartOnly) // ZStart Only overrides RTS
		ReturnToStart=FALSE;

	UpSpray=(BOOL)GetPrivateProfileInt(IMSName,"UpSpray",0,IMSIni);

	VisEnable=(BOOL)GetPrivateProfileInt(IMSName,"VisEnable",1,IMSIni);
	VisInterleave=(BOOL)GetPrivateProfileInt(IMSName,"VisInterleave",1,IMSIni);

	ReplaceZeros=(BOOL)GetPrivateProfileInt(IMSName,"ReplaceZeros",0,IMSIni);
	WarnOnBlankVars=(BOOL)GetPrivateProfileInt(IMSName,
		"WarnOnBlankVars",1,IMSIni);

	Metric=(BOOL)GetPrivateProfileInt(IMSName,"Metric",0,IMSIni);
	MetricMM=(BOOL)GetPrivateProfileInt(IMSName,"MetricMM",0,IMSIni);

	FilesInit();

	SetMode(mLOGIN); // Initial Set mode to LOGIN
	
	ShowWindow(IMSWnd,nCmdShow);
	UpdateWindow(IMSWnd);

	IMSAccel=LoadAccelerators(IMSInst,IMSName);

	// Run run= from ini file
	GetPrivateProfileString(IMSName,"run","",tbuf,TSIZE,IMSIni);
	if(lstrlen(tbuf))
		WinExec(tbuf,SW_SHOWNORMAL);
	
	NoFontMarks=(BOOL)GetPrivateProfileInt(IMSName,"NoFontMarks",0,IMSIni);
	NoBitmapMarks=(BOOL)GetPrivateProfileInt(IMSName,"NoBitmapMarks",0,IMSIni);
	NoAlign=(BOOL)GetPrivateProfileInt(IMSName,"NoAlign",0,IMSIni);
	NoCal=(BOOL)GetPrivateProfileInt(IMSName,"NoCal",0,IMSIni);

	return TRUE;
	}

/***************************************************************************
	About()

	Displays the About dialog box.
***************************************************************************/
static void About(void)
	{

	DialogBox((HINSTANCE)IMSInst,"ABOUT",IMSWnd,(DLGPROC)ABOUTDP);
	}

/***************************************************************************
	Shell()

	Starts the shell defined in iniShell.
***************************************************************************/
static void Shell(void)
	{
	GetPrivateProfileString(IMSName,"Shell","",
		tbuf,TSIZE,IMSIni);

	if(WinExec(tbuf,SW_SHOWNORMAL)<32)
		ShowError(IDE_SHELLFAILED);
	}

/***************************************************************************
	LoadTimeNames()

	Loads the time names from the ini or build the default table if there
	are none.
***************************************************************************/
static void LoadTimeNames(void)
	{
	char *p;
	char key[20];
	int i,l;
	// Load time names keys
	GetPrivateProfileString("TimeNames",NULL,"",tbuf,TSIZE,IMSIni);
	p=tbuf;
	i=0;
	while(*p) // for each key
		{
		if(i>=MAXTIMENAMES)
			break;
		l=lstrlen(p);
		lstrcpy(key,p);

		ParseLong(key,&(TimeNames[i].time));
		GetPrivateProfileString("TimeNames",key,"",TimeNames[i].name,
			LTIMENAME+1,IMSIni);
		p+=l+1;
		i++;
		}
	if(i==0) // If no names supplied build default table
		{
		TimeNames[i].time=21600L;
		lstrcpy(TimeNames[i].name,"Day");
		i++;
		TimeNames[i].time=32400L;
		lstrcpy(TimeNames[i].name,"Night");
		i++;
		}
	nTimeNames=i;
	while(i<MAXTIMENAMES) // Zero remaining entries
		TimeNames[i++].name[0]=0;
	}

/***************************************************************************
	len=GetTimeName(buf,t)

	Loads a buffer with the time name for time t.

	int len = length of time name as stored in buf or 0 on error.
	time_t = current time.
***************************************************************************/
static int GetTimeName(LPSTR buf,time_t t)
	{
	int i,n;

	if(nTimeNames<1)
		return 0;
	t-=_timezone;   // Adjust for local time zone
	t%=86400L;      // Normalize to 1 day
	n=nTimeNames-1; // Intialilly set to last time
	for(i=0;i<nTimeNames;i++)
		{
		if(t>=TimeNames[i].time)
			n=i;
		}
	lstrcpy(buf,TimeNames[n].name);
	return lstrlen(buf);
	}

/***************************************************************************
	MapDigits(s,map)

	Maps the digits of a string to differnt characters.
	a digit with value n is mapped to map[n].
	Assumes that the map length is 10

	LPSTR s = string to map.
	LPCSTR map = 10 character mapping string.
***************************************************************************/
static void MapDigits(LPSTR s,LPCSTR map)
	{
	while(*s)
		{
		if(isdigit(*s))
			*s=map[*s-'0'];
		s++;
		}
	}
/***************************************************************************
	ReloadIniSettings()

	Reloads the INI variables after they have been changed by IMSConfigIni

***************************************************************************/
static void ReloadIniSettings(){
	/*-------- Marking ----------*/

	// Get RehomePoint using IMWnd to create defaults if not found
	if(!GetPrivateProfileVect(IMSName,"RehomePoint",
		(double*)&RehomePoint,3,IMSIni))
		{
		// If not found compute default as top center
		RehomePoint.x=IMInfo.extent.x/2;
		RehomePoint.y=IMInfo.extent.y/200;
		RehomePoint.z=IMInfo.extent.z;
		}

	if(!GetPrivateProfileVect(IMSName,"StartPoint",
		(double*)&StartPoint,3,IMSIni))
	{
		StartPoint.x=IMInfo.extent.x/2;
		StartPoint.y=IMInfo.extent.y/2;
		StartPoint.z=IMInfo.extent.z;
	}
	
	ReloadRunSettings();
	
	ZStartOnly=(BOOL)GetPrivateProfileInt(IMSName,"ZStartOnly",0,IMSIni);
	ReturnToStart=(BOOL)GetPrivateProfileInt(IMSName,"ReturnToStart",0,IMSIni);
	if(ZStartOnly)
		ReturnToStart=FALSE;
	WarnOnBlankVars = GetPrivateProfileInt(IMSName,"WarnOnBlankVars",1,IMSIni);
	ReplaceZeros = GetPrivateProfileInt(IMSName,"ReplaceZeros",0,IMSIni);
	
	MetricMM = GetPrivateProfileInt(IMSName,"MetricMM",0,IMSIni);
	Metric = GetPrivateProfileInt(IMSName,"Metric",0,IMSIni);

	histque.qsize=(WORD)GetPrivateProfileInt(IMSName,"UniqueHistorySize",0,IMSIni);
	UniqueOverrideLevel=(WORD)GetPrivateProfileInt(IMSName,"UniqueOverrideLevel",0,IMSIni);
	
	/*-------- System -----------*/	
	UpSpray = GetPrivateProfileInt(IMSName,"Upspray",0,IMSIni);
	LoadTimeNames();

	ReloadDDESettings();
	
	NoFontMarks = GetPrivateProfileInt(IMSName,"NoFontMarks",0,IMSIni);
	NoBitmapMarks = GetPrivateProfileInt(IMSName,"NoBitmapMarks",0,IMSIni);
	NoAlign = GetPrivateProfileInt(IMSName,"NoAlign",0,IMSIni);
	NoCal = GetPrivateProfileInt(IMSName,"NoCal",0,IMSIni);

	HideOptConveyor = GetPrivateProfileInt(sHideOpt,"Conveyor",0,IMSIni);
	HideOptInit = GetPrivateProfileInt(sHideOpt,"Init",0,IMSIni);
	HideOptInspect = GetPrivateProfileInt(sHideOpt,"Inspect",0,IMSIni);
	HideOptLink = GetPrivateProfileInt(sHideOpt,"Link",0,IMSIni);
	HideOptMarkRotation = GetPrivateProfileInt(sHideOpt,"MarkRotation",0,IMSIni);
	HideOptMarkSlant = GetPrivateProfileInt(sHideOpt,"MarkSlant",0,IMSIni);
	HideOptModeMenu = GetPrivateProfileInt(sHideOpt,"ModeMenu",0,IMSIni);
	HideOptRead = GetPrivateProfileInt(sHideOpt,"Read",0,IMSIni);
	HideOptRotaryIMF = GetPrivateProfileInt(sHideOpt,"RotaryIMF",0,IMSIni);
	HideOptRotaryDEG = GetPrivateProfileInt(sHideOpt,"RotaryDEG",0,IMSIni);
	HideOptRotation = GetPrivateProfileInt(sHideOpt,"Rotation",0,IMSIni);
	
	/*--------- Files/dirs --------*/
//	GetPrivateProfileString(IMSName,"run","",tbuf,TSIZE,IMSIni);
//	if(lstrlen(tbuf))
//		WinExec(tbuf,SW_SHOWNORMAL);
	// Load load=
//	GetPrivateProfileString(IMSName,"load","",tbuf,TSIZE,IMSIni);
//	if(lstrlen(tbuf))
//		WinExec(tbuf,SW_SHOWMINNOACTIVE);
	
	ReloadFileOptions();
	IMSGetDefDirs();

	/*--------- Colors/graphics --------*/
	RefEdgeInit();
	CDevReloadSysSettings();

	/*--------- Vision settings --------*/
	VisInterleave=(BOOL)GetPrivateProfileInt(IMSName,"VisInterleave",1,IMSIni);
	ReloadLocator();
};

BOOL CALLBACK MARKINGDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.MARKINGDP(hDlg,msg,wP,lp);
};

BOOL CALLBACK UID06CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.UID06CON1DP(hDlg,msg,wP,lp);
};
BOOL CALLBACK UID06CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.UID06CON2DP(hDlg,msg,wP,lp);
};
BOOL CALLBACK UID05DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.UID05DP(hDlg,msg,wP,lp);
};

BOOL CALLBACK UID12CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.UID12CON1DP(hDlg,msg,wP,lp);
};
BOOL CALLBACK UID12CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.UID12CON2DP(hDlg,msg,wP,lp);
};

BOOL CALLBACK COMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.COMDP(hDlg,msg,wP,lp);
};

BOOL CALLBACK COLORSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.COLORSDP(hDlg,msg,wP,lp);
};

BOOL CALLBACK SYSTEMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.SYSTEMDP(hDlg,msg,wP,lp);
};

BOOL CALLBACK EDITTIMEDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.EDITTIMEDP(hDlg,msg,wP,lp);
};

BOOL CALLBACK FILESDIRSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.FILESDIRSDP(hDlg,msg,wP,lp);
};

BOOL CALLBACK LOCATORDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp){
	return IMSConfig.LOCATORDP(hDlg,msg,wP,lp);
};

//AutoStart System =======================================================================
#define minmax(a,i,b) (((i)<(a))?(a):((i)>(b))?(b):(i))
static char sAutoStart[]="AutoStart";
static char sAutoStartMinimized[]="AutoStartMinimized";
static char sAutoStartPosDelay[]="AutoStartPosDelay";
static char sAutoStartCmdDelay[]="AutoStartCmdDelay";
static char sAutoStartCaptions[]="AutoStartCaptions";
static char sAutoStartPositions[]="AutoStartPositions";
static char sAutoStartCommands[]="AutoStartCommands";
static HANDLE hAutoStartThread=0;

static void _DoAutoStartPositions()
	{	
	char keys[2048];
	char cap[256];
	GetPrivateProfileString(sAutoStartPositions,NULL,NULL,keys,sizeof(keys),IMSIni);
	for(char* key=keys;*key;)
		{
		int lcap=GetPrivateProfileString(sAutoStartCaptions,key,"",cap,sizeof(cap),IMSIni);
		HWND hfind;
		if(cap && (hfind=FindWindow(NULL,cap)))
			{
			POINT pos;
			if(GetPrivateProfileIntVect(sAutoStartPositions,key,(int *)&pos,2,IMSIni))
				SetWindowPos(hfind,0,pos.x,pos.y,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
			}
		while(*key++);
		}
	}

// Sends WM_COMMAND idm 0 messages if specified in ini
static void _DoAutoStartCommands()
	{
	char keys[2048];
	char cap[256];
	GetPrivateProfileString(sAutoStartCommands,NULL,NULL,keys,sizeof(keys),IMSIni);
	for(char* key=keys;*key;)
		{
		int lcap=GetPrivateProfileString(sAutoStartCaptions,key,"",cap,sizeof(cap),IMSIni);
		HWND hfind;
		if(cap && (hfind=FindWindow(NULL,cap)))
			{
			int idm=GetPrivateProfileInt(sAutoStartCommands,key,0,IMSIni);
			if(idm>0)
				PostMessage(hfind,WM_COMMAND,idm,0);
			}
		while(*key++);
		}
	}

static DWORD WINAPI AutoStartThreadProc(LPVOID lpParam)
	{

	UINT PosDelay=GetPrivateProfileInt(IMSName,sAutoStartPosDelay,5,IMSIni);
	PosDelay=minmax(0,PosDelay,60);
	if(PosDelay)
		Sleep(PosDelay*1000);

	_DoAutoStartPositions();

	UINT CmdDelay=GetPrivateProfileInt(IMSName,sAutoStartCmdDelay,0,IMSIni);
	CmdDelay=minmax(0,CmdDelay,60);
	if(CmdDelay)
		Sleep(CmdDelay*1000);

	_DoAutoStartCommands();

	return 0;
	}

static void StartAutoStartThread()
	{
	DWORD id;
	if(!hAutoStartThread)
		hAutoStartThread=CreateThread(NULL,0,AutoStartThreadProc,NULL,0,&id);
	}

static void _DoAutoStarts(char* sect,int nShowCmd)
	{
	char keys[2048];
	char cmd[256];
	char cap[256];
	GetPrivateProfileString(sect,NULL,NULL,keys,sizeof(keys),IMSIni);
	for(char* key=keys;*key;)
		{
		int lcmd=GetPrivateProfileString(sect,key,"",cmd,sizeof(cmd),IMSIni);
		int lcap=GetPrivateProfileString(sAutoStartCaptions,key,"",cap,sizeof(cap),IMSIni);
		if(!(cap && FindWindow(NULL,cap)))
			ShellExecute(NULL,"open",cmd,NULL,IMSDir,nShowCmd);
		while(*key++);
		}
	}

static void DoAutoStarts()
	{
	_DoAutoStarts(sAutoStart,SW_SHOWNORMAL);
	_DoAutoStarts(sAutoStartMinimized,SW_SHOWMINNOACTIVE);
	StartAutoStartThread();
	}
