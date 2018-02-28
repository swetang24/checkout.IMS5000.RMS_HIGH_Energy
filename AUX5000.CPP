/***************************************************************************
	Name : AUX4001.C
	Desc : Definitions for AUX5000 system.
***************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <direct.h>
#include <imslib32\imslib32.h>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "run.h"
#include "aux5000.h"
#include "commdlg.h"

/***************************************************************************
	PRIVATE DEFINES
***************************************************************************/
#define MAXAUXCOUNT 4 // Maximum number of AUXes supported
#define LBTEXT     20 // Maximum length of an AUXes button text
#define LAUXNAME    8 // Maximum length of an AUX name
#define MAXPATH   120 // Maximum length of a path name

// IDCs
#define IDC_AUXBTN 100 // +n where n==[0..MAXAUXCOUNT-1]
                // 101 reserved IDC_AUXBTN+1
                // 102 reserved IDC_AUXBTN+2
                // 103 reserved IDC_AUXBTN+3

/***************************************************************************
	PRIVATE TYPES
***************************************************************************/
typedef struct
	{
	char btext[LBTEXT+1];
	char name[LAUXNAME+1];
	} AUXLISTENTRY;

typedef struct
	{
	int count;     // Number of used entries.
	int sel;       // Index of last selected entry or -1
	AUXLISTENTRY entry[MAXAUXCOUNT];
	} AUXLIST;

/******************************************************************************
	NON API EXPORT FUNCTION PROTOTYPES
******************************************************************************/
LRESULT CALLBACK AUXWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);

/***************************************************************************
	STATIC FUNCTION PROTOTYPES
***************************************************************************/
static void BuildAUXList(void);
static void DoAUX(HWND hwnd);
static BOOL WriteCMDFile(LPSTR auxfile,LPSTR cmdfile,LPSTR keys,LPSTR name,
	LPSTR section);
static BOOL WriteCSVFile(LPSTR auxfile,LPSTR csvfile,LPSTR name,LPSTR section);

/***************************************************************************
	STATIC VARIABLES
***************************************************************************/
static char AUXClass[]="AUX4001"; // AUX button window class name
static AUXLIST AUXList;           // The AUX List
static BOOL AUXInitialized=FALSE; // AUXInit has been called?
static WORD FocusID;              // ID of child in focus

/***************************************************************************
	PUBLIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	count = AUXInit()

	Initializes the AUX system.  Causes AUXTable to be built.
	After initialization, AUXInit can be called again to
	get AUXList.count.

	WORD count = Number of AUX devices found, -1 on error
***************************************************************************/
int AUXInit(void)
	{
	WNDCLASS wc;

	if(!AUXInitialized)
		{
		wc.style=CS_VREDRAW|CS_HREDRAW;
		wc.lpfnWndProc=AUXWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); 
		wc.lpszMenuName=NULL;
		wc.lpszClassName=AUXClass;
		if(!RegisterClass(&wc))
			return -1;
		BuildAUXList();
		AUXInitialized=TRUE;
		}
	return AUXList.count;
	}

/***************************************************************************
	hwnd = AUXCreate(parent,id,x,y,w,h)

	Creates a new AUX button window.  A button is placed
	in the AUX window for each AUX device listed in the
	AUX section of IMSIni.

	hwnd = Handle of new aux window or NULL on error.
	HWND parent = Parent window handle.
	WORD id = AUX window id.
	int x = initial x coordinate.
	int y = initial y coordinate.
	int w = initial width.
	int h = initial height.
***************************************************************************/
HWND AUXCreate(HWND parent,WORD id,int x,int y,int w,int h)
	{
	HWND hwnd;

	if(!AUXInitialized)
		return 0;

	hwnd=CreateWindow(AUXClass,NULL,
		WS_CHILD|WS_VISIBLE|WS_BORDER,
		x,y,w,h,parent,(HMENU)id,IMSInst,NULL);

	return hwnd;
	}

/***************************************************************************
	NON API EXPORT FUNCTIONS
***************************************************************************/
/***************************************************************************
	AUXWP

	AUX buttons window procedure.
***************************************************************************/
LRESULT CALLBACK AUXWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_CREATE:
			{
			int i;
			for(i=0;i<AUXList.count;i++)
				CreateWindow("XBUTTON",AUXList.entry[i].btext,
					WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
					0,0,0,0,hwnd,(HMENU)(IDC_AUXBTN+i),IMSInst,NULL);
			FocusID=IDC_AUXBTN;
			return 0;
			}

		case WM_SIZE:
			{
			int xs,ys,bxs,bys;
			int i,tex;
			HDC hdc;
			SIZE te;
			LPSTR btext;

			xs=LOWORD(lP);
			ys=HIWORD(lP);

			// Compute bxs = widest text extent x + 8
			bxs=0;
			i=AUXList.count;
			hdc=GetDC(NULL); // Get display DC
			while(i--)
				{
				btext=(LPSTR)&AUXList.entry[i].btext;
				GetTextExtentPoint(hdc,btext,lstrlen(btext),&te);
				tex=te.cx;
				bxs=max(tex,bxs);
				}
			ReleaseDC(NULL,hdc);
			bxs+=5*sftm.tmAveCharWidth;

			bys=ys+2;

			// Size all the buttons
			i=AUXList.count;
			while(i--)
				MoveWindow(GetDlgItem(hwnd,IDC_AUXBTN+i),
					xs-((AUXList.count-i)*bxs),-1,bxs,bys,TRUE);
			return 0;
			}

		case WM_SETFOCUS:
			Notify(hwnd,AN_SETFOCUS);
			SetFocus(GetDlgItem(hwnd,FocusID));
			return 0;

		case WM_COMMAND://Win32 changed
			{
			int i;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			i=wP-IDC_AUXBTN; // i = AUXList index
			if(i<0 || i>=AUXList.count)
				return 0;
			switch(ncode)
				{
				case XN_TAB:
					i++;
					if(i<AUXList.count)
						SetFocus(GetDlgItem(hwnd,IDC_AUXBTN+i));
					else
						{
						Notify(hwnd,AN_TAB);
						FocusID=IDC_AUXBTN;
						}
					return 0;
				case XN_BKTAB:
					if(i)
						SetFocus(GetDlgItem(hwnd,IDC_AUXBTN+i-1));
					else
						{
						Notify(hwnd,AN_BKTAB);
						FocusID=IDC_AUXBTN+AUXList.count-1;
						}
					return 0;
				case XN_SETFOCUS:
					FocusID=IDC_AUXBTN+i;
					return 0;
				case BN_CLICKED:
					AUXList.sel=i;
					Notify(hwnd,AN_CLICKED);
					return 0;
				}
			return 0;
			}

		case AM_DOAUX:
			DoAUX(hwnd);
			return 0;
		}
	return DefWindowProc(hwnd,message,wP,lP);
	}

/***************************************************************************
	STATIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	BuildAUXList()

	Builds AUXList from the AUX section of IMSIni.
***************************************************************************/
static void BuildAUXList(void)
	{
	static char sect[]="AUX";
	char keys[128];
	char *key;
	char *p;
	int nlen;
	int blen;

	AUXList.count=0;

	if(!GetPrivateProfileString(sect,NULL,"",keys,sizeof(keys),IMSIni))
		return;

	key=keys;
	while(*key)
		{
		GetPrivateProfileString(sect,key,"",tbuf,TSIZE,IMSIni);
		// p = & of ','
		p=&(tbuf[0]);
		while((*p)!=',')
			{
			if(!*p)
				break;
			p++;
			}
		if(!*p)
			{
			key+=lstrlen(key)+1;
			continue;
			}
		*p=0; // change , to null
		p++; // advance to start of button text
		nlen=lstrlen(tbuf);
		blen=lstrlen(p);
		if(nlen<1 || nlen > LAUXNAME || nlen<1)
			{
			key+=lstrlen(key)+1;
			continue;
			}
		blen=min(blen,LBTEXT);
		p[blen]=0;

		// Initialize entry
		lstrcpy(AUXList.entry[AUXList.count].name,tbuf);
		lstrcpy(AUXList.entry[AUXList.count].btext,p);

		// Count it.
		AUXList.count++;

		if(AUXList.count==MAXAUXCOUNT)
			break;

		// next key
		key+=lstrlen(key)+1;
		}
	AUXList.sel=-1;
	}

/***************************************************************************
	DoAUX(hwnd)

	Executes the current AUXList selection with the data in 
	RunFN and RunIMF.imp.

	HWND hwnd = parent AUX window.
***************************************************************************/
static void DoAUX(HWND hwnd)
	{
	char SavedDir[MAXPATH];
	char auxfile[MAXPATH];
	char cmdfile[13];
	char csvfile[13];
	char cmdline[MAXPATH];
	char defprog[32];
	char keys[128];
	char *name;
	char section[MAX_PATH];
	BOOL ok;
	BOOL prmpt; // Flag to prompt for LQTY
	BOOL minimize; // run command minized flag
	int i;

	// skip if no selection
	if(AUXList.sel==-1)
		return;

	// Save current dir and drive
	_getdcwd(0,SavedDir,MAXPATH);

	// Switch to IMS dir and drive
	chddir(IMSDir);

	// Get easy access to selected name
	name=&AUXList.entry[AUXList.sel].name[0];

	// Build complete path to aux file
	wsprintf(auxfile,"%s\\%s.AUX",(LPSTR)IMSDir,(LPSTR)name);

	GetPrivateProfileString(name,"DEFAULT","",defprog,sizeof(defprog),auxfile);
	// Determine section and get keys
	GetFileTitle(RunIMF.imp.fn,section,MAX_PATH);
	if(!GetPrivateProfileString(section,NULL,"",keys,sizeof(keys),auxfile))
		{
		lstrcpy(section,defprog);
//		section=&defprog[0];  // zzz 2/26
		if(!GetPrivateProfileString(section,NULL,"",keys,sizeof(keys),auxfile))
			{
			chddir(SavedDir);
			return;
			}
		}


	GetPrivateProfileString(name,"CMDFILE","",cmdfile,sizeof(cmdfile),auxfile);

	// Get csvfile with section override
	GetPrivateProfileString(section,"CSVFILE","",csvfile,
		sizeof(csvfile),auxfile);
	if(!csvfile[0])
		GetPrivateProfileString(name,"CSVFILE","",csvfile,
			sizeof(csvfile),auxfile);
	if(csvfile[0]) // csv sections must have the CSV= entry
		{
		if(!GetPrivateProfileString(section,"CSV","",tbuf,TSIZE,auxfile))
			{
			chddir(SavedDir);
			return;
			}
		}

	AuxLQty=1; // Default
	// prompt for LQTY option (with section override)
	i=GetPrivateProfileInt(section,"PromptForLQTY",1000,auxfile);
	if(i!=1000)
		prmpt=(BOOL)i;
	else
		prmpt=GetPrivateProfileInt(name,"PromptForLQTY",0,auxfile);
	if(prmpt)
		{
		lstrcpy(tbuf,"1");
		if(!DoPrompt(hwnd,"How many labels do you want printed?",4,tbuf))
			{ // Cancled by user
			chddir(SavedDir);
			return;
			}
		ParseInt(tbuf,(LPINT)&AuxLQty);
		AuxLQty=max(AuxLQty,1);
		}

	// Get cmdline (look in section first then in name)
	GetPrivateProfileString(section,"CMDLINE","",cmdline,
		sizeof(cmdline),auxfile);
	if(!cmdline[0])
		GetPrivateProfileString(name,"CMDLINE","",cmdline,
			sizeof(cmdline),auxfile);

	// Get minimize (look in section first then in name)
	// prompt for LQTY option (with section override)
	i=GetPrivateProfileInt(section,"Minimize",1000,auxfile);
	if(i!=1000)
		minimize=(BOOL)i;
	else
		minimize=GetPrivateProfileInt(name,"Minimize",0,auxfile);
		

	if(csvfile[0])
		ok=WriteCSVFile(auxfile,csvfile,name,section);
	else
		ok=WriteCMDFile(auxfile,cmdfile,keys,name,section);

	if(ok) // Execute cmdline
		WinExec(cmdline,minimize ? SW_SHOWMINNOACTIVE : SW_SHOWNORMAL);

   // Restore saved drive and dir
   chddir(SavedDir);
   }

/******************************************************************************
	ok = WriteCMDFile(auxfile,cmdfile,keys,name,section)

	Writes out a .INI style key=value cmd file.

	BOOL ok = TRUE on success.
	LPSTR auxfile = name of .AUX file to read
	LPSTR cmdfile = name of .CMD file to write
	LPSTR keys = \0\0 terminaled list of \0 terminaded key names
	LPSTR name = AUX name.
	LPSTR section = ini section name to read from .AUX file
******************************************************************************/
static BOOL WriteCMDFile(LPSTR auxfile,LPSTR cmdfile,LPSTR keys,LPSTR name,
	LPSTR section)
	{
	char ebuf[80]; // expression buffer
	int elen;
	LPSTR key;
//	OFSTRUCT ofs;
	HANDLE fh;
	IMPVALS *pvals;

	// Create the cmd file
	fh= CreateFile(cmdfile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE)
		return FALSE;        

	tlen=wsprintf(tbuf,"[%s]\r\n",(LPSTR)name); // Write section name
	_lwrite32(fh,tbuf,tlen);

	// zzz - New format
	// valslist[0] is the only one aux works on
	pvals=&(RunIMF.pvalslist[0]);
	// Enumerate through keys
	key=keys;
	while(*key)
		{
		GetPrivateProfileString(section,key,"",ebuf, // get expression
			sizeof(ebuf),auxfile);
		if(!IMPExpandTextToTbuf(ebuf,&RunIMF.imp.vars,pvals,
			&RunIMF.imp.gvars,&RunIMF.imp.gvals,RunIMF.imp.pheader,
			&RunIMF.imp.udhas))
			{
			CloseHandle(fh);
			return FALSE;
			}
		elen=wsprintf(ebuf,"%s=%s\r\n",(LPSTR)key,(LPSTR)tbuf);
		_lwrite32(fh,ebuf,elen);
		key+=lstrlen(key)+1;
		}
	CloseHandle(fh);
	return TRUE;
	}

/******************************************************************************
	ok = WriteCSVFile(auxfile,csvfile,name,section)

	Writes out a .CSV style file for the expression at CSV= in the AUX file.

	BOOL ok = TRUE on success.
	LPSTR auxfile = name of .AUX file to read
	LPSTR csvfile = name of CSV file to write
	LPSTR name = AUX name.
	LPSTR section = ini section name to read from .AUX file
******************************************************************************/
static BOOL WriteCSVFile(LPSTR auxfile,LPSTR csvfile,LPSTR name,LPSTR section)
	{
	char ebuf[128]; // expression buffer
	//OFSTRUCT ofs;
	HANDLE fh;
	IMPVALS *pvals;

	// Create the csv file
	fh=CreateFile(csvfile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE)
		return FALSE;        

	// zzz - New format
	// valslist[0] is the only one aux works on
	pvals=&(RunIMF.pvalslist[0]);
	GetPrivateProfileString(section,"CSV","",ebuf,sizeof(ebuf),auxfile);
	if(!IMPExpandTextToTbuf(ebuf,&RunIMF.imp.vars,pvals,&RunIMF.imp.gvars,
		&RunIMF.imp.gvals,RunIMF.imp.pheader,&RunIMF.imp.udhas))
		{
		CloseHandle(fh);
		return FALSE;
		}
	lstrcat(tbuf,"\r\n");
	tlen=lstrlen(tbuf);
	_lwrite32(fh,tbuf,tlen);
	CloseHandle(fh);
	return TRUE;
	}
