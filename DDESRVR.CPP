/******************************************************************************
	NAME : DDESRVR
	DESC : DDE Server for DDESTEST.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <ddeml.h>
#include <dos.h>
#include <stdlib.h>
#include <direct.h>
#include <IMSLIB32\IMSLIB32.H>
#include <JVMATH32\JVMATH32.H>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "login.h"
#include "run.h"
#include "ddesrvr.h"

/******************************************************************************
	PRIVAATE DEFINES
******************************************************************************/
#define DREQSIZE   8
#define MAXIOS     24
#define MAXIONAME  64

/******************************************************************************
	PRIVATE TYPES
******************************************************************************/
typedef struct
	{
	DDERUNEVENT dre;
	int param;
	char csvbuf[512];
	} DREQENTRY;

typedef struct
	{
	HSZ hsz;
	HBIT hbit;
	char name[MAXIONAME];
	} IOItem;

void InputBit(WORD event); // Bit monitor callback

/******************************************************************************
	PUBLIC VARIABLES
******************************************************************************/
USERLEVEL DDEUserLevel;         // DDE User's access level
char DDEUserName[LUSERNAME+1];  // DDE User's name
char DDEUserID[LUSERID+1];      // DDE User's ID
char DDERunFilename[DDERUNMAXFN+1]; // DDERUN file to open
BOOL Extra1IsDDE=FALSE;         // True when EXTRA1 button is set for DDE

/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static DWORD ddeId=0;    // DDEML instance identifier
static HSZ hszService;   // Service name handle
static HSZ hszLOGIN;     // LOGIN topic name handle
static HSZ hszUSER;      // USER item name handle for LOGIN topic
static HSZ hszAUTHORIZE; // AUTHORIZE item name handle for LOGIN topic
static HSZ hszDDERUN;    // DDERUN Topic
static HSZ hsz_EVENT;    // DDERUN Item
static HSZ hsz_VNAMES;   // DDERUN Item
static HSZ hsz_VSIZES;   // DDERUN Item
static HSZ hsz_VCOUNT;   // DDERUN Item
static HSZ hsz_FILES;    // DDERUN Item
static HSZ hsz_IMPDIR;   // DDERUN Item
static HSZ hsz_IMFDIR;   // DDERUN Item
static HSZ hsz_XYPOS;    // DDERUN Servo XY Position
static HSZ hsz_ZPOS;     // DDERUN Servo Z Position
static HSZ hsz_TPOS;     // DDERUN Servo T Position
static HSZ hsz_NBF;      // DDERUN Item : Nest Bit Flags
static HSZ hsz_IMSTATE;  // DDERUN Item : IM State

static HSZ hszSYSTEM;    // SYSTEM Topic

static UR ur; // User record filled by LOGIN|USER poke

static HWND dderunOwner; // Receiver of DRN notifications

// DDE Run Event Queue (dreq)
static DREQENTRY dreqdata[DREQSIZE];
static int dreqhead=0;
static int dreqlen=0;
static BOOL dreqHasNBFCHANGE=FALSE;
static BOOL NBFChangeData;

// Var Items
static int VarCount;
static int GVarCount; // Count of global vars
static int IVarCount; // Count of instance vars
static HSZ VarItems[IMPMAXVARS];
static int HaCount; // Count of HA+UDHA items
static HSZ HaItems[IMPMAXUDHAS+IMPNBHAS];
static int InpCount=0; // Count of IO items
static IOItem InpItems[MAXIOS];
static int OutpCount=0; // Count of IO items
static IOItem OutpItems[MAXIOS];

static char dbuf[512];
static BOOL ddedebug=FALSE;

/******************************************************************************
	STATIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	len=nbftobuf(buf)

	Creats a CSV string of the current NBF to a buffer

	int len = length of buf
	char *buf = buffer for result
******************************************************************************/
static WORD nbftobuf(char *buf)
	{
	int i;
	WORD len;
	if(IMSMode==mRUN && RunMode==rmDDERUN)
		{ // Build the nbf list
		len=0;
		for(i=0;i<RunIMF.nests.count;i++)
			len+=wsprintf(buf+len,
				(LPSTR)(i ? ",%d" : "%d"),
				RunIMF.nests.nest[i].flags);
		}
	else
		{
		lstrcpy(buf,"none");
		len=4;
		}
	return len;
	}

void xOutputDebugString(LPCSTR s)
	{
	if(!ddedebug)
		return;
	OutputDebugString(s);
	}

/******************************************************************************
	ret = DDERunExecute(hdata)

	Handles the Execute DDERUN dde transaction.

	HDDEDATA ret = DDE_FACK or DDE_FNOTPROCESSED
******************************************************************************/
static HDDEDATA DDERunExecute(HDDEDATA hdata)
	{
	static char cmd[512];
	static char ioname[MAXIONAME];
	LPSTR arg;
	long l;
	int i;

	DdeGetData(hdata,(unsigned char*)dbuf,sizeof(dbuf),0);
	arg=ParseCSV(dbuf,cmd);

	if((!lstrcmp(cmd,"OPEN")) && (arg[0]==','))
		{
		ParseCSV(arg,DDERunFilename);
		strfnextend(DDERunFilename,"IMP");
		PostMessage(IMSWnd,WM_COMMAND,IDM_DDERUNSTART,0);
		return (HDDEDATA)DDE_FACK;
		}
	if((!lstrcmp(cmd,"MAPINPUT")) && (arg[0]==','))
		{
		if(InpCount>=MAXIOS)
			{
			xOutputDebugString("DDEMAPINPUT failed\n");
			return (HDDEDATA)DDE_FACK;
			}
		lstrcpyn(ioname,&arg[1],MAXIONAME);
		i=InpCount;
		while(i--)
			{
			if(!lstrcmpi(ioname,InpItems[i].name))
				{
				xOutputDebugString("DDEMAPINPUT already mapped\n");
				return (HDDEDATA)DDE_FACK;
				}
			}
		InpItems[InpCount].hbit=IMMapInBit(ioname);
		xOutputDebugString("DDEMAPINPUT \"");
		xOutputDebugString(ioname);
		xOutputDebugString("\" ?? ");
		if(InpItems[InpCount].hbit)
			{
			InpItems[InpCount].hsz=DdeCreateStringHandle(ddeId,ioname,CP_WINANSI);
			lstrcpy(InpItems[InpCount].name,ioname);
			IMMonitorInBit(InpItems[InpCount].hbit,IBM01,InpCount,InputBit);
			IMMonitorInBit(InpItems[InpCount].hbit,IBM10,MAXIOS+InpCount,InputBit);
			InpCount++;
			xOutputDebugString("success");
			}
		xOutputDebugString("\n");
		return (HDDEDATA)DDE_FACK;
		}

	if((!lstrcmp(cmd,"MAPOUTPUT")) && (arg[0]==','))
		{
		if(OutpCount>=MAXIOS)
			{
			xOutputDebugString("DDEMAPOUTPUT failed\n");
			return (HDDEDATA)DDE_FACK;
			}
		lstrcpyn(ioname,&arg[1],MAXIONAME);
		i=OutpCount;
		while(i--)
			{
			if(!lstrcmpi(ioname,OutpItems[i].name))
				{
				xOutputDebugString("DDEMAPOUTPUT already mapped\n");
				return (HDDEDATA)DDE_FACK;
				}
			}
		OutpItems[OutpCount].hbit=IMMapOutBit(ioname);
		xOutputDebugString("DDEMAPOUTPUT \"");
		xOutputDebugString(ioname);
		xOutputDebugString("\" ?? ");
		if(OutpItems[OutpCount].hbit)
			{
			OutpItems[OutpCount].hsz=DdeCreateStringHandle(ddeId,ioname,CP_WINANSI);
			lstrcpy(OutpItems[OutpCount].name,ioname);
			OutpCount++;
			xOutputDebugString("success");
			}
		xOutputDebugString("\n");
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"JOG"))
		{
		if(IMSMode==mRUN && RunMode==rmDDERUN)
			PostMessage(dderunOwner,WM_DDERUN,DRM_JOG,0);
		else
			DDERunPostEvent(dreNOTREADY,0);
		return (HDDEDATA)DDE_FACK;
		}

	// All message below this point are only handled in DDERUN mode
	if(!(IMSMode==mRUN && RunMode==rmDDERUN))
		return (HDDEDATA)DDE_FACK;

	if((!lstrcmp(cmd,"SETNLOADED")) && (arg[0]==','))
		{
		ParseLong(arg,&l);
		PostMessage(dderunOwner,WM_DDERUN,DRM_SETNLOADED,l);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"QUIT"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_QUIT,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"LOCK"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCK,rlbALL);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UNLOCK"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCK,rlbALL);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"LOCKNESTS"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCK,rlbNESTS);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"LOCKVARS"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCK,rlbVARS);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"LOCKHEADER"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCK,rlbHEADER);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"LOCKAUX"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCK,rlbAUX);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"LOCKQUIT"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCK,rlbQUIT);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UNLOCKNESTS"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCK,rlbNESTS);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UNLOCKVARS"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCK,rlbVARS);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UNLOCKHEADER"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCK,rlbHEADER);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UNLOCKAUX"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCK,rlbAUX);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UNLOCKQUIT"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCK,rlbQUIT);
		return (HDDEDATA)DDE_FACK;
		}
	if((!lstrcmp(cmd,"LOCKSTART"))||
	   (!lstrcmp(cmd,"STARTLOCK"))) // pre 3.39 command name support
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCKSTART,0);
		return (HDDEDATA)DDE_FACK;
		}
	if((!lstrcmp(cmd,"UNLOCKSTART"))||
	   (!lstrcmp(cmd,"STARTUNLOCK")))  // pre 3.39 command name support
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UNLOCKSTART,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"NEXTVALS"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_NEXTVALS,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"UPDATE"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_UPDATE,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"SETFOCUS"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_SETFOCUS,0);
		return (HDDEDATA)DDE_FACK;
		}
	if((!lstrcmp(cmd,"LOCKNESTSONCHANGE")) && (arg[0]==','))
		{
		ParseLong(arg,&l);
		PostMessage(dderunOwner,WM_DDERUN,DRM_LOCKNESTSONCHANGE,l);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"START"))
		{
		PostMessage(dderunOwner,WM_DDERUN,DRM_START,0);
		return (HDDEDATA)DDE_FACK;
		}
	if((!lstrcmp(cmd,"EXTRA1")) && (arg[0]==','))
		{
		ParseCSV(arg,tbuf);
		if(tbuf[0])
			{
			SendMessage(IMWnd,IMM_EXTRA,1,(LONG)(LPSTR)tbuf);
			Extra1IsDDE=TRUE;
			}
		else
			{
			SendMessage(IMWnd,IMM_EXTRA,1,0);
			Extra1IsDDE=FALSE;
			}
		return (HDDEDATA)DDE_FACK;
		}
	return (HDDEDATA)DDE_FNOTPROCESSED;
	}

/******************************************************************************
	ret = DDESystemExecute(hdata)

	Handles the Execute DDERUN dde transaction.

	HDDEDATA ret = DDE_FACK or DDE_FNOTPROCESSED
******************************************************************************/
static HDDEDATA DDESystemExecute(HDDEDATA hdata)
	{
	static char cmd[512];
	DdeGetData(hdata,(unsigned char*)cmd,sizeof(cmd),0);
	if(!lstrcmp(cmd,"POWERDOWN"))
		{
		PostMessage(IMSWnd,WM_COMMAND,IDM_POWERDOWN,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"PRINTON"))
		{
		PostMessage(IMSWnd,WM_COMMAND,IDM_PRINTON,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"SHUTDOWN"))
		{
		PostMessage(IMSWnd,WM_COMMAND,IDM_SHUTDOWN,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"EXIT"))
		{
		PostMessage(IMSWnd,WM_COMMAND,IDM_EXIT,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"INIT"))
		{
		PostMessage(IMWnd,IMM_INIT,0,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"EnableAbortError"))
		{
		SendMessage(IMWnd,IMM_ENABABTERR,1,0);
		return (HDDEDATA)DDE_FACK;
		}
	if(!lstrcmp(cmd,"DisableAbortError"))
		{
		SendMessage(IMWnd,IMM_ENABABTERR,0,0);
		return (HDDEDATA)DDE_FACK;
		}
	return (HDDEDATA)DDE_FNOTPROCESSED;
	}

/******************************************************************************
	hdata = DDERunRequest_EVENT()

	Handles requests for IMS5000|DDERUN|_EVENT by sending the next
	event or "" if the dreq is empty.

	HDDEDATA hdata = handle event data returned.
******************************************************************************/
static HDDEDATA DDERunRequest_EVENT(void)
	{
	static char ebuf[128];
	WORD ide;

	if(dreqlen)
		{
		switch(dreqdata[dreqhead].dre)
			{
			case dreREADY:
				wsprintf(ebuf,"READY,%d",dreqdata[dreqhead].param);
				break;
			case dreSTART:
				wsprintf(ebuf,"START,%d",dreqdata[dreqhead].param);
				break;
			case dreDONE:
				lstrcpy(ebuf,"DONE");
				break;
			case dreCANCEL:
				lstrcpy(ebuf,"CANCEL");
				break;
			case dreQUIT:
				lstrcpy(ebuf,"QUIT");
				break;
			case dreJOGSTART:
				lstrcpy(ebuf,"JOGSTART");
				break;
			case dreJOGEND:
				lstrcpy(ebuf,"JOGEND");
				break;
			case dreNOTREADY:
				lstrcpy(ebuf,"NOTREADY");
				break;
			case dreNBFCHANGE:
				if(NBFChangeData)
					wsprintf(ebuf,"NBFCHANGE,%s",
						(LPSTR)dreqdata[dreqhead].csvbuf);
				else
					lstrcpy(ebuf,"NBFCHANGE");
				dreqHasNBFCHANGE=FALSE;
				break;
			case dreCYCLEQUIT:
				lstrcpy(ebuf,"CYCLEQUIT");
				break;
			case dreCYCLEPASS:
				lstrcpy(ebuf,"CYCLEPASS");
				break;
			case dreCYCLEFAIL:
				lstrcpy(ebuf,"CYCLEFAIL");
				break;
			case dreCONTINUE:
				lstrcpy(ebuf,"CONTINUE");
				break;
			case drePDONE:
				lstrcpy(ebuf,"PDONE");
				break;
			case dreBUSY:
				lstrcpy(ebuf,"BUSY");
				break;
			case dreREADSTART:
				lstrcpy(ebuf,"READSTART");
				break;
			case dreREADEND:
				lstrcpy(ebuf,"READEND");
				break;
			case dreLOCSTART:
				lstrcpy(ebuf,"LOCATESTART");
				break;
			case dreLOCEND:
				lstrcpy(ebuf,"LOCATEEND");
				break;
			case dreEXTRA1:
				lstrcpy(ebuf,"EXTRA1");
				break;
			case dreERROR:
				ide=(WORD)(dreqdata[dreqhead].param);
				if(ide)
					{
			        if(LoadString(IMSInst,ide,tbuf,TSIZE))
						{
						wsprintf(ebuf,"ERROR,%s",(LPSTR)tbuf);
						break;
						}
					}
				// drop in
			default:
				lstrcpy(ebuf,"ERROR");
				break;
			}
		dreqhead=(dreqhead+1)%DREQSIZE;
		dreqlen--;
		}
	else
		ebuf[0]=0;
	return DdeCreateDataHandle(ddeId,(unsigned char*)ebuf,lstrlen(ebuf)+1,0,
		hsz_EVENT,CF_TEXT,0);
	}

/******************************************************************************
	hdata = DDERunRequest_VNAMES()

	Handles requests for IMS5000|DDERUN|_VNAMES by returning the CSV
	list of variable name.
******************************************************************************/
static HDDEDATA DDERunRequest_VNAMES(void)
	{
	HDDEDATA hdata;
	char nbuf[IMPLVNAME+1];
	int nlen;
	int dlen;
	int i;

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_VNAMES,CF_TEXT,0);
	dlen=0;

	for(i=0;i<RunIMF.imp.gvars.count;i++)
		{
		if(dlen)
			nlen=wsprintf(nbuf,",%s",RunIMF.imp.gvars.var[i].name);
		else
			nlen=wsprintf(nbuf,"%s",RunIMF.imp.gvars.var[i].name);
		DdeAddData(hdata,(unsigned char*)nbuf,nlen,dlen);
		dlen+=nlen;
		}
	for(i=0;i<RunIMF.imp.vars.count;i++)
		{
		if(dlen)
			nlen=wsprintf(nbuf,",%s",RunIMF.imp.vars.var[i].name);
		else
			nlen=wsprintf(nbuf,"%s",RunIMF.imp.vars.var[i].name);
		DdeAddData(hdata,(unsigned char*)nbuf,nlen,dlen);
		dlen+=nlen;
		}
	DdeAddData(hdata,(unsigned char*)"",1,dlen); // Null terminate
	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_VSIZES()

	Handles IMS5000|DDERUN|_VSIZES requests by returning the size of each
	variable in a CSV list.
******************************************************************************/
static HDDEDATA DDERunRequest_VSIZES(void)
	{
	HDDEDATA hdata;
	char sbuf[24];
	int slen;
	int dlen;
	int i;

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_VSIZES,CF_TEXT,0);
	dlen=0;
	for(i=0;i<RunIMF.imp.gvars.count;i++)
		{
		if(dlen)
			slen=wsprintf(sbuf,",%d",RunIMF.imp.gvars.var[i].width);
		else
			slen=wsprintf(sbuf,"%d",RunIMF.imp.gvars.var[i].width);
		DdeAddData(hdata,(unsigned char*)sbuf,slen,dlen);
		dlen+=slen;
		}
	for(i=0;i<RunIMF.imp.vars.count;i++)
		{
		if(dlen)
			slen=wsprintf(sbuf,",%d",RunIMF.imp.vars.var[i].width);
		else
			slen=wsprintf(sbuf,"%d",RunIMF.imp.vars.var[i].width);
		DdeAddData(hdata,(unsigned char*)sbuf,slen,dlen);
		dlen+=slen;
		}
	DdeAddData(hdata,(unsigned char*)"",1,dlen); // Null terminate
	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_VCOUNT()

	Handles IMS5000|DDERUN|_VCOUNT requests by returning the count of
    variables in the current program.
******************************************************************************/
static HDDEDATA DDERunRequest_VCOUNT(void)
	{
	HDDEDATA hdata;
	tlen=wsprintf(tbuf,"%d",VarCount);
	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_VCOUNT,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)tbuf,tlen+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_FILES()

	Handles IMS5000|DDERUN|_FILES requests by returning the name of each
	file in the programs dir in a CSV list.
******************************************************************************/
static HDDEDATA DDERunRequest_FILES(void)
	{
	HDDEDATA hdata;
	char *fn;
	char fbuf[512];
	int flen;
	int dlen;
	char SavedDir[_MAX_PATH];

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_FILES,CF_TEXT,0);

	dlen=0;
	_getdcwd(0,SavedDir,_MAX_PATH); // Save current dir and drive
	chddir(IMSProgs); // Switch to IMS dir and drive
	fn=FindFirstFile("*.*");
	while(fn)
		{
		flen=wsprintf(fbuf,dlen ? ",%s" : "%s",(LPSTR)fn);
		DdeAddData(hdata,(unsigned char*)fbuf,flen,dlen);
		dlen+=flen;
		fn=FindNextFile();
		}
	DdeAddData(hdata,(unsigned char*)"",1,dlen); // Null terminate
	chddir(SavedDir); // Restore saved drive and dir
	return hdata;
	}

/***************************************************************************
	hdata = DDERunRequestVar(vi)

	Handles dde var requests by returning all of the vals for the
	specified var in a CSV list.

	HDDEDATA hdata = CSV list of vals from var
	int vi = vars/vals index requested.
***************************************************************************/
static HDDEDATA DDERunRequestVar(int vi)
	{
	char vbuf[IMPLKVVAL+1];
	int vlen;
	HDDEDATA hdata;
	int i;
	int dlen;
	int ivi; // instance vi = vi-gvars.count

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,VarItems[vi],CF_TEXT,0);

	ivi=vi-RunIMF.imp.gvars.count;
	dlen=0;
	if(vi<RunIMF.imp.gvars.count)
		{
		vlen=ValToBuf(&RunIMF.imp.gvals,vi,&RunIMF.imp.gvars,vbuf);
		vlen++; // Include null
		DdeAddData(hdata,(unsigned char*)vbuf,vlen,dlen);
		dlen+=vlen;
		}
	else
		{
		for(i=0;i<RunIMF.nests.count;i++)
			{
			// zzz - New format
			IMPVALS *pvals = &(RunIMF.pvalslist[i]);
			//IMPVALS *pvals= (IMPVALS*) (RunIMF.pvalslist+i*RunIMF.valssize);
			vlen=ValToBuf(pvals,ivi,&RunIMF.imp.vars,vbuf);
			// ',' instead of \0 except for last
			if(i<(RunIMF.nests.count-1))
				vbuf[vlen]=',';
			vlen++; // Include , or null
			DdeAddData(hdata,(unsigned char*)vbuf,vlen,dlen);
			dlen+=vlen;
			}
		}
	return hdata;
	}

/***************************************************************************
	hdata = DDERunRequestHa(hai)

	Handles dde HA requests by returning the value for the
	specified HA.

	HDDEDATA hdata = Value for HA
	int hai = Header/UDHA index requested.
***************************************************************************/
static HDDEDATA DDERunRequestHa(int hai)
	{
	char habuf[IMPLUDHAVAL+1];
	HDDEDATA hdata=DdeCreateDataHandle(ddeId,NULL,0,0,HaItems[hai],CF_TEXT,0);
	int halen=HAToBuf(RunIMF.imp.pheader,&RunIMF.imp.udhas,hai,habuf);
	DdeAddData(hdata,(unsigned char*)habuf,halen+1,0);
	return hdata;
	}

/***************************************************************************
	hdata = DDERunRequestInput(index)

	Handles dde Input requests by returning the value for the
	specified Input.

	HDDEDATA hdata = Value for the input
	int index = Input index requested.
***************************************************************************/
static HDDEDATA DDERunRequestInput(int index)
	{
	char ioval[10];
	if(IMReadInBit(InpItems[index].hbit))
		lstrcpy(ioval,"1");
	else
		lstrcpy(ioval,"0");
	wsprintf(dbuf,"DDEInput(%d) %s=%s\n",index,InpItems[index].name,ioval);
	xOutputDebugString(dbuf);
	HDDEDATA hdata=DdeCreateDataHandle(ddeId,NULL,0,0,InpItems[index].hsz,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)ioval,2,0);
	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_IMPDIR()

	Handles IMS5000|DDERUN|_IMPDIR requests by string IMSProgs
******************************************************************************/
static HDDEDATA DDERunRequest_IMPDIR(void)
	{
	HDDEDATA hdata;

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_IMPDIR,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)IMSProgs,lstrlen(IMSProgs)+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_IMFDIR()

	Handles IMS5000|DDERUN|_IMFDIR requests by string IMSFixtures
******************************************************************************/
static HDDEDATA DDERunRequest_IMFDIR(void)
	{
	HDDEDATA hdata;

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_IMFDIR,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)IMSFixtures,lstrlen(IMSFixtures)+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_XYPOS()

	Handles IMS5000|DDERUN|_XYPOS requesting the xy position from IM
	and formatting the result into a CSV string.
******************************************************************************/
static HDDEDATA DDERunRequest_XYPOS(void)
	{
	HDDEDATA hdata;
	P2 p;

	SendMessage(IMWnd,IMM_GETXYPOS,0,(LPARAM)(LPP2)&p);
	tlen=VectToString(tbuf,(LPDOUBLE)&p,2,",");
	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_XYPOS,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)tbuf,tlen+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_ZPOS()

	Handles IMS5000|DDERUN|_ZPOS requesting the z position from IM
	and formatting the result into a string.
******************************************************************************/
static HDDEDATA DDERunRequest_ZPOS(void)
	{
	HDDEDATA hdata;
	double p;

	SendMessage(IMWnd,IMM_GETZPOS,0,(LPARAM)(LPDOUBLE)&p);
	tlen=VectToString(tbuf,(LPDOUBLE)&p,1,",");
	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_ZPOS,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)tbuf,tlen+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_TPOS()

	Handles IMS5000|DDERUN|_TPOS requesting the T position from IM
	and formatting the result into a string.
******************************************************************************/
static HDDEDATA DDERunRequest_TPOS(void)
	{
	HDDEDATA hdata;
	double p;

	SendMessage(IMWnd,IMM_GETTPOS,0,(LPARAM)(LPDOUBLE)&p);
	tlen=VectToString(tbuf,(LPDOUBLE)&p,1,",");
	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_TPOS,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)tbuf,tlen+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_NBF()

	Handles IMS5000|DDERUN|_NBF requests by returning a csv list of the
	current nest bit flags or "none" if not in a run mode.
******************************************************************************/
static HDDEDATA DDERunRequest_NBF(void)
	{
	HDDEDATA hdata;

	tlen=nbftobuf(tbuf);
	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_NBF,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)tbuf,tlen+1,0);

	return hdata;
	}

/******************************************************************************
	hdata = DDERunRequest_IMSTATE()

	Handles IMS5000|DDERUN|_IMSTATE requests by returning a string naming
	the current IM state
******************************************************************************/
static HDDEDATA DDERunRequest_IMSTATE(void)
	{
	static char *snames[]={"OFF","INIT","READY","PRINTOFF",
		"MARKING","ERROR","JOG","CLEAN","XYONLY","WARMUP"};
	HDDEDATA hdata;
//	int i;


	// Map IMState to snames
	if(IMState>=IMN_OFF && IMState<=IMN_WARMUP)
		tlen=wsprintf(tbuf,"%s",(LPSTR)snames[IMState-IMN_OFF]);
	else
		tlen=wsprintf(tbuf,"UNKNOWN");

	hdata=DdeCreateDataHandle(ddeId,NULL,0,0,hsz_IMSTATE,CF_TEXT,0);
	DdeAddData(hdata,(unsigned char*)tbuf,tlen+1,0);

	return hdata;
	}

/***************************************************************************
	DDERunRequest(item)

	Handles dde requests by returning the specified item.
***************************************************************************/
static HDDEDATA DDERunRequest(HSZ item)
	{
	int i;

	if(item==hsz_EVENT)
		return DDERunRequest_EVENT();
	if(item==hsz_FILES)
		return DDERunRequest_FILES();
	if(item==hsz_IMPDIR)
		return DDERunRequest_IMPDIR();
	if(item==hsz_IMFDIR)
		return DDERunRequest_IMFDIR();
	if(item==hsz_XYPOS)
		return DDERunRequest_XYPOS();
	if(item==hsz_ZPOS)
		return DDERunRequest_ZPOS();
	if(item==hsz_TPOS)
		return DDERunRequest_TPOS();
	if(item==hsz_NBF)
		return DDERunRequest_NBF();
	if(item==hsz_IMSTATE)
		return DDERunRequest_IMSTATE();

	if(IMSMode==mRUN && RunMode==rmDDERUN)
		{
		if(item==hsz_VNAMES)
			return DDERunRequest_VNAMES();
		if(item==hsz_VSIZES)
			return DDERunRequest_VSIZES();
		if(item==hsz_VCOUNT)
			return DDERunRequest_VCOUNT();
		// Search for matching var item
		i=VarCount;
		while(i--)
			if(item==VarItems[i])
				return DDERunRequestVar(i); // Return requested var 
		// Return requested udha
		i=HaCount;
		while(i--)
			if(item==HaItems[i])
				return DDERunRequestHa(i); // Return requested HA 
		}

	i=InpCount;
	while(i--)
		if(DdeCmpStringHandles(item,InpItems[i].hsz)==0)
			return DDERunRequestInput(i); // Return requested Input

	return DDE_FNOTPROCESSED;
	}

/***************************************************************************
	hdata = DDERunPokeVar(vi,hdata)

	Handles dde var pokes by filling in all the vals associated with the
	given var index with the elements of the CSV list in hdata.

	int vi = vars/vals index requested.
	HDDEDATA hdata = CSV list of vals for var
***************************************************************************/
static HDDEDATA DDERunPokeVar(int vi,HDDEDATA hdata)
	{
	LPSTR pdata;
	char vbuf[IMPLKVVAL+1];
//	int vlen;
	int i;
	int ivi;  // instance var index == vi-gvars.count

	pdata=(LPSTR)DdeAccessData(hdata,NULL);
	if(pdata)
		{
		ivi=vi-RunIMF.imp.gvars.count;
		if(vi<RunIMF.imp.gvars.count)
			{
			pdata=ParseCSV(pdata,vbuf);
			switch(RunIMF.imp.gvars.var[vi].type)
				{
				case vtKEYED:
					lstrcpy(RunIMF.imp.gvals.val[vi].k,vbuf);
					break;
				case vtDATE:
					lstrcpy(RunIMF.imp.gvals.val[vi].d.override,vbuf);
					break;
				case vtTIME:
					lstrcpy(RunIMF.imp.gvals.val[vi].t.override,vbuf);
					break;
				case vtBASED:
					lstrcpy(RunIMF.imp.gvals.val[vi].b,vbuf);
					break;
				case vtSERIAL:
					ParseLong(vbuf,(LPLONG)&(RunIMF.imp.gvals.val[vi].s));
					RunIMF.imp.gvals.val[vi].c=0;
					break;
				}
			}
		else
			{
			for(i=0;i<RunIMF.nests.count;i++)
				{
				// zzz - New format
				IMPVALS * pvals = &(RunIMF.pvalslist[i]);
				//IMPVALS *pvals=(IMPVALS*) (RunIMF.pvalslist+i*RunIMF.valssize);
				// Parse the next value to vbuf
				if(i==0 && *pdata==',') // If first element emty
					vbuf[0]=0;
				else
					pdata=ParseCSV(pdata,vbuf);
					// Copy vbuf to the val
				switch(RunIMF.imp.vars.var[ivi].type)
					{
					case vtKEYED:
						lstrcpy(pvals->val[ivi].k,vbuf);
						break;
					case vtBASED:
						lstrcpy(pvals->val[ivi].b,vbuf);
						break;
					case vtSERIAL:
						ParseLong(vbuf,(LPLONG)&(pvals->val[ivi].s));
						pvals->val[ivi].c=0;
						break;
					}
				}
			}
		DdeUnaccessData(hdata);
		}
	return (HDDEDATA)DDE_FACK;
	}

/***************************************************************************
	hdata = DDERunPokeHA(hi,hdata)

	Handles dde HA pokes by filling in the value associated with the
	given HA index with the value in hdata.

	int hi = Header attributes index requested.
	HDDEDATA hdata = text value for HA
***************************************************************************/
static HDDEDATA DDERunPokeHA(int hi,HDDEDATA hdata)
	{
	LPSTR pdata;
                
	if(hi<IMPNBHAS)
		{
		wsprintf(dbuf,"DDERunPokeHA Invalid HI(%d)\n",hi);
		xOutputDebugString(dbuf);
		return (HDDEDATA) DDE_FACK;
		}
	int ihi=hi-IMPNBHAS;
	if(!RunIMF.imp.udhas.udha[ihi].redit)
		{
		wsprintf(dbuf,"DDERunPokeHA() %s not runtime editable\n",RunIMF.imp.udhas.udha[ihi].name);
		xOutputDebugString(dbuf);
		return (HDDEDATA) DDE_FACK;
		}
	pdata=(LPSTR)DdeAccessData(hdata,NULL);
	if(pdata)
		{   
		lstrcpyn(RunIMF.imp.udhas.udha[ihi].val,pdata,IMPLUDHAVAL);
		DdeUnaccessData(hdata);
		wsprintf(dbuf,"DDERunPokeHA(%d,%s) \"%s\"\n",ihi,pdata,RunIMF.imp.udhas.udha[ihi].name);
		xOutputDebugString(dbuf);
		}
	return (HDDEDATA)DDE_FACK;
	}

/***************************************************************************
	hdata = DDERunPokeOutput(index,hdata)

	Handles dde IO pokes by setting the value associated with the
	given IO index with the value in hdata.

	int index = IO index requested.
	HDDEDATA hdata = binary value for IO
***************************************************************************/
static HDDEDATA DDERunPokeOutput(int index,HDDEDATA hdata)
	{
	LPSTR pdata;
	int ival;
                
	pdata=(LPSTR)DdeAccessData(hdata,NULL);
	if(pdata)
		{   
		ival=atoi(pdata);
		wsprintf(dbuf,"DDEOutput(%d) %s=%s\n",index,OutpItems[index].name,pdata);
		DdeUnaccessData(hdata);
		IMSetOutBit(OutpItems[index].hbit,ival);
		xOutputDebugString(dbuf);
		}
	return (HDDEDATA)DDE_FACK;
	}

/***************************************************************************
	hdata = DDERunPoke_IMPDIR(hdata)

	Handles dde pokes to _IMPDIR

	HDDEDATA hdata = value to poke
***************************************************************************/
static HDDEDATA DDERunPoke_IMPDIR(HDDEDATA hdata)
	{
	LPSTR pdata;

	pdata=(LPSTR)DdeAccessData(hdata,NULL);
	if(pdata)
		{
		lstrcpy(IMSProgs,pdata);
		DdeUnaccessData(hdata);
		}
	else
		IMSProgs[0]=0;
	return (HDDEDATA)DDE_FACK;
	}

/***************************************************************************
	hdata = DDERunPoke_IMFDIR(hdata)

	Handles dde pokes to _IMFDIR

	HDDEDATA hdata = value to poke
***************************************************************************/
static HDDEDATA DDERunPoke_IMFDIR(HDDEDATA hdata)
	{
	LPSTR pdata;

	pdata=(LPSTR)DdeAccessData(hdata,NULL);
	if(pdata)
		{
		lstrcpy(IMSFixtures,pdata);
		DdeUnaccessData(hdata);
		}
	else
		IMSFixtures[0]=0;
	return (HDDEDATA)DDE_FACK;
	}

/***************************************************************************
	hdata = DDERunPoke_NBF(hdata)

	Handles dde pokes to _NBF

	HDDEDATA hdata = value to poke
***************************************************************************/
static HDDEDATA DDERunPoke_NBF(HDDEDATA hdata)
	{
	LPSTR pdata;
	int i;
	long l;

	pdata=(LPSTR)DdeAccessData(hdata,NULL);
	if(pdata)
		{

		if(IMSMode==mRUN && RunMode==rmDDERUN)
			{
			for(i=0;i<RunIMF.nests.count;i++)
				{
				pdata=ParseLong(pdata,&l);
				if(!pdata)
					break;
				RunIMF.nests.nest[i].flags=(int)l;
				}
			PostMessage(dderunOwner,WM_DDERUN,DRM_INVALIMF,0);
			}
		DdeUnaccessData(hdata);
		}
	return (HDDEDATA)DDE_FACK;
	}

/***************************************************************************
	DDERunPoke(item,hdata)

	Handles dde pokes by changing the specified item.
***************************************************************************/
static HDDEDATA DDERunPoke(HSZ item,HDDEDATA hdata)
	{
	int i;

	if(item==hsz_IMPDIR)
		return DDERunPoke_IMPDIR(hdata);
	if(item==hsz_IMFDIR)
		return DDERunPoke_IMFDIR(hdata);
	if(item==hsz_NBF)
		return DDERunPoke_NBF(hdata);
	// Search for matching var item
	i=VarCount;
	while(i--)
		if(item==VarItems[i])
			return DDERunPokeVar(i,hdata); // Poke requested var item
	// Search for matching var item
	i=HaCount;
	while(i--)
		if(item==HaItems[i])
			return DDERunPokeHA(i,hdata); // Poke requested ha item
	// Search for matching output item
	i=OutpCount;
	while(i--)
		if(item==OutpItems[i].hsz)
			return DDERunPokeOutput(i,hdata); // Poke requested ha item

	return DDE_FNOTPROCESSED;
	}

/******************************************************************************
	ret = PokeLoginUser(hdata)

	Handles the poke login user dde transaction.
******************************************************************************/
static HDDEDATA PokeLoginUser(HDDEDATA hdata)
	{
	LPSTR p;
	LPSTR pdata;
	int lid,lacode,llevel; // Lengths of string components
	HDDEDATA ret;
	long l;

	pdata=(LPSTR)DdeAccessData(hdata,NULL);

	// Get lengths of "ID,ACODE,LEVEL"
	p=pdata;

	lid=lacode=llevel=0;
	while(*p && *p!=',')
		{
		lid++;
		p++;
		}
	if(*p)
		{
		p++; // past ,
		while(*p && *p!=',')
			{
			lacode++;
			p++;
			}
		if(*p)
			{
			p++; // past ,
			while(*p && *p!=',')
				{
				llevel++;
				p++;
				}
			}
		}

	// Blank ur
	ur.id[0]=0;
	ur.acode[0]=0;
	ur.level=0;

	// Validate and if valid copy to ur
	if(lid > 0
		&& lid <= LUSERID
		&& lacode > 0
		&& lacode <= LACODE
		&& llevel > 0)
		{
		ParseLong(pdata+lid+lacode+2,&l);
		if(l>0)
			{
			lstrcpyn(ur.id,pdata,lid+1);
			ur.id[lid]=0;
			lstrcpyn(ur.acode,pdata+lid+1,lacode+1);
			ur.acode[lacode]=0;
			ur.level=(USERLEVEL)l;
			ret=(HDDEDATA)DDE_FACK;
			}
		else
			ret=DDE_FNOTPROCESSED;
		}
	else
		ret=DDE_FNOTPROCESSED;

	DdeUnaccessData(hdata);
	return (HDDEDATA)ret;
	}

/******************************************************************************
	ret = RequestLoginAuthorize(void)

	Handles the request login authorize dde transaction.
******************************************************************************/
static HDDEDATA RequestLoginAuthorize(void)
	{
	BOOL pass;
	int lname;

	pass=AuthorizeUser(&ur);
	if(pass)
		{
		lstrcpy(UserID,ur.id);
		lstrcpy(UserName,ur.name);
		UserLevel=ur.level;
		lstrcpy(DDEUserID,ur.id);
		lstrcpy(DDEUserName,ur.name);
		DDEUserLevel=ur.level;
		lname=lstrlen(ur.name);
		return DdeCreateDataHandle(ddeId,(unsigned char*)ur.name,lname+1,0,
			hszAUTHORIZE,CF_TEXT,0);
		}
	return DdeCreateDataHandle(ddeId,(unsigned char*)"",1,0,hszAUTHORIZE,CF_TEXT,0);
	}

/******************************************************************************
	EXPORT FUNCTIONS
******************************************************************************/
/******************************************************************************
	DDEP

	Server DDE Process.
******************************************************************************/
HDDEDATA CALLBACK DDEP(UINT type,UINT fmt,HCONV hconv,
	HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2)
	{
	switch(type)
		{
		case XTYP_ADVREQ:
		case XTYP_REQUEST:
			if(hsz1==hszLOGIN && hsz2==hszAUTHORIZE)
				return RequestLoginAuthorize();
			if(hsz1==hszDDERUN)
				return DDERunRequest(hsz2);
			break;
		case XTYP_POKE:
			if(hsz1==hszLOGIN && hsz2==hszUSER)
				return PokeLoginUser(hData);
			if(hsz1==hszDDERUN)
				return DDERunPoke(hsz2,hData);
			break;
		case XTYP_EXECUTE:
			if(hsz1==hszDDERUN)
				return DDERunExecute(hData);
			if(hsz1==hszSYSTEM)
				return DDESystemExecute(hData);
			break;
		case XTYP_CONNECT:
			if(hsz1==hszLOGIN) // Allow connections to Login topic
				return (HDDEDATA)TRUE;
			if(hsz1==hszDDERUN) // Allow connections to DDERun topic
				return (HDDEDATA)TRUE;
			if(hsz1==hszSYSTEM) // Allow connections to SYSTEM topic
				return (HDDEDATA)TRUE;
			break;
		case XTYP_ADVSTART:
			{
			int i;
			if(hsz1!=hszDDERUN)
				return (HDDEDATA)FALSE;
			if(hsz2==hsz_EVENT || hsz2==hsz_IMSTATE)
				return (HDDEDATA)TRUE;
			i=InpCount;
			while(i--)
				{
				if(hsz2==InpItems[i].hsz)
					return (HDDEDATA)TRUE;
				}
			return (HDDEDATA)FALSE;
//zzz			if((hsz1==hszDDERUN) && (hsz2==hsz_EVENT || hsz2==hsz_IMSTATE))
//				return (HDDEDATA)TRUE;
//			return (HDDEDATA)FALSE;
			}

		case XTYP_ADVSTOP:
			return (HDDEDATA)DDE_FACK;
		}
	return 0;
	}

/******************************************************************************
	API Functions
******************************************************************************/
/******************************************************************************
	ok = DDEServerInit()

	Initializes the DDE Server

	BOOL ok = True on success.
******************************************************************************/
BOOL DDEServerInit(void)
	{
	if(ddeId) // Error if allready open
		return FALSE;

	// Initialize DDEML instance
	if(DdeInitialize(&ddeId,
		(PFNCALLBACK)MakeProcInstance((FARPROC)DDEP,IMSInst),
		APPCMD_FILTERINITS,0L))
		{
		ddeId=0;
		return -1L;
		}

	hszSYSTEM=DdeCreateStringHandle(ddeId,SZDDESYS_TOPIC,CP_WINANSI);

	// Init DDERUN data
	hszDDERUN=DdeCreateStringHandle(ddeId,"DDERUN",CP_WINANSI);
	hsz_EVENT=DdeCreateStringHandle(ddeId,"_EVENT",CP_WINANSI);
	hsz_VNAMES=DdeCreateStringHandle(ddeId,"_VNAMES",CP_WINANSI);
	hsz_VSIZES=DdeCreateStringHandle(ddeId,"_VSIZES",CP_WINANSI);
	hsz_VCOUNT=DdeCreateStringHandle(ddeId,"_VCOUNT",CP_WINANSI);
	hsz_FILES=DdeCreateStringHandle(ddeId,"_FILES",CP_WINANSI);
	hsz_IMPDIR=DdeCreateStringHandle(ddeId,"_IMPDIR",CP_WINANSI);
	hsz_IMFDIR=DdeCreateStringHandle(ddeId,"_IMFDIR",CP_WINANSI);
	hsz_NBF=DdeCreateStringHandle(ddeId,"_NBF",CP_WINANSI);
	hsz_IMSTATE=DdeCreateStringHandle(ddeId,"_IMSTATE",CP_WINANSI);
	hsz_XYPOS=DdeCreateStringHandle(ddeId,"_XYPOS",CP_WINANSI);
	hsz_ZPOS=DdeCreateStringHandle(ddeId,"_ZPOS",CP_WINANSI);
	hsz_TPOS=DdeCreateStringHandle(ddeId,"_TPOS",CP_WINANSI);

	// Init login data
	ur.id[0]=0;
	ur.acode[0]=0;
	ur.name[0]=0;
	ur.level=0;
	DDEUserID[0]=0;
	DDEUserName[0]=0;
	DDEUserLevel=0;
	// Create Login string handles
	hszLOGIN=DdeCreateStringHandle(ddeId,"LOGIN",CP_WINANSI);
	hszUSER=DdeCreateStringHandle(ddeId,"USER",CP_WINANSI);
	hszAUTHORIZE=DdeCreateStringHandle(ddeId,"AUTHORIZE",CP_WINANSI);

	// Create and Register service name
	hszService=DdeCreateStringHandle(ddeId,IMSName,CP_WINANSI);
	DdeNameService(ddeId,hszService,NULL,DNS_REGISTER);

	return TRUE;
	}

/******************************************************************************
	ok = DDEServerShutdown(void)

	Performs cleanup of the DDE Server.

	BOOL ok = True on success.
******************************************************************************/
BOOL DDEServerShutdown(void)
	{
	if(!ddeId) // If not open
		return FALSE;

	// Unregister and free service names
	DdeNameService(ddeId,NULL,NULL,DNS_UNREGISTER);
	DdeFreeStringHandle(ddeId,hszService);

	// DDERUN shutdown

	DdeFreeStringHandle(ddeId,hsz_TPOS);
	DdeFreeStringHandle(ddeId,hsz_ZPOS);
	DdeFreeStringHandle(ddeId,hsz_XYPOS);
	DdeFreeStringHandle(ddeId,hsz_NBF);
	DdeFreeStringHandle(ddeId,hsz_IMSTATE);
	DdeFreeStringHandle(ddeId,hsz_IMFDIR);
	DdeFreeStringHandle(ddeId,hsz_IMPDIR);
	DdeFreeStringHandle(ddeId,hsz_FILES);
	DdeFreeStringHandle(ddeId,hsz_VCOUNT);
	DdeFreeStringHandle(ddeId,hsz_VSIZES);
	DdeFreeStringHandle(ddeId,hsz_VNAMES);
	DdeFreeStringHandle(ddeId,hsz_EVENT);
	DdeFreeStringHandle(ddeId,hszDDERUN);

	// Free Login string handles
	DdeFreeStringHandle(ddeId,hszLOGIN);
	DdeFreeStringHandle(ddeId,hszUSER);
	DdeFreeStringHandle(ddeId,hszAUTHORIZE);

	// Free system string handle
	DdeFreeStringHandle(ddeId,hszSYSTEM);

	for(int i=0;i<InpCount;i++)
		DdeFreeStringHandle(ddeId,InpItems[i].hsz);
	InpCount=0;
	for(int i=0;i<OutpCount;i++)
		DdeFreeStringHandle(ddeId,OutpItems[i].hsz);
	OutpCount=0;

	DdeUninitialize(ddeId);
	ddeId=0;
	return TRUE;
	}

/***************************************************************************
	DDERunInit(hwnd)

	Initializes a DDERun session.  The HSZs for all variables are created
	and the dderunOwner is set.

	HWND hwnd = Owner window handle
***************************************************************************/
void DDERunInit(HWND hwnd)
	{
	int i;

	dderunOwner=hwnd; // Save owner

	GVarCount=RunIMF.imp.gvars.count;
	IVarCount=RunIMF.imp.vars.count;
	VarCount=GVarCount+IVarCount;
	for(i=0;i<GVarCount;i++)
		VarItems[i]=DdeCreateStringHandle(ddeId,
			(RunIMF.imp.gvars.var)[i].name,CP_WINANSI);
	for(;i<VarCount;i++)
		VarItems[i]=DdeCreateStringHandle(ddeId,
			(RunIMF.imp.vars.var)[i-GVarCount].name,CP_WINANSI);
	HaCount=IMPNBHAS+RunIMF.imp.udhas.count;
	for(i=0;i<HaCount;i++)
		{
		char name[IMPLUDHANAME+2];
		if(i<IMPNBHAS)
			wsprintf(name,"!%s",(LPSTR)IMPBHANames[i]);
		else
			wsprintf(name,"!%s",(LPSTR)RunIMF.imp.udhas.udha[i-IMPNBHAS].name);
		HaItems[i]=DdeCreateStringHandle(ddeId,name,CP_WINANSI);
		}
	}

/***************************************************************************
	DDERunShutdown()

	Shuts down a DDERUN session by setting the owner to 0 and freeing
    all of the VarItems[] string handles.
***************************************************************************/
void DDERunShutdown(void)
	{
	int i;

	dderunOwner=0;

	for(i=0;i<VarCount;i++)
		DdeFreeStringHandle(ddeId,VarItems[i]);
	VarCount=0;
	for(i=0;i<HaCount;i++)
		DdeFreeStringHandle(ddeId,HaItems[i]);
	HaCount=0;
	}


/******************************************************************************
	DDERunPostEvent(dre,param)

	Posts a DDERUN event to dreQueue.  Every DDE request for EVENT returns

	BOOL ok = True on success.
******************************************************************************/
void DDERunPostEvent(DDERUNEVENT dre,int param)
	{
	if(dre==dreNBFCHANGE)
		{
		if(dreqHasNBFCHANGE && !NBFChangeData) // skip if in queue and
			return;                            // not adding data
		dreqHasNBFCHANGE=TRUE;
		}
	if(dreqlen<DREQSIZE)
		{
		dreqdata[(dreqhead+dreqlen)%DREQSIZE].dre=dre;
		dreqdata[(dreqhead+dreqlen)%DREQSIZE].param=param;
		if(dre==dreNBFCHANGE && NBFChangeData)
			nbftobuf(dreqdata[(dreqhead+dreqlen)%DREQSIZE].csvbuf);
		dreqlen++;
		if(ddeId)
			DdePostAdvise(ddeId,hszDDERUN,hsz_EVENT);
		}
	}

/******************************************************************************
	DDERunStart(void)

	Handles the OPEN DDERun Transaction (Async)
******************************************************************************/
void DDERunStart(void)
	{
//	IMPHEADER *ph;
	BOOL dderun; // True if state was DDERUN going in

	dderun=(IMSMode==mRUN && RunMode==rmDDERUN);

	if(IMSMode!=mLOGIN
		&& IMSMode!=mFILES 
		&& !dderun)
		{
		DDERunPostEvent(dreERROR,IDE_NOCANDDERUN);
		ShowError(IDE_NOCANDDERUN);
		return;
		}

	// Free old run data if needed
	if(dderun)
		{
		IMPFree(&RunIMF.imp);
		IMFFree(&RunIMF);
		}

	if(!GetIMFIMP(&RunIMF,IMSProgs,DDERunFilename))
		{
		DDERunPostEvent(dreERROR,IDError);
		ShowError(0);
		if(dderun) // failed attempt cancles current dderun
			SetMode(mLOGIN);
		return;
		}

	NBFChangeData=GetPrivateProfileInt(sDDERUN,"NBFChangeData",0,IMSIni);
	ddedebug=GetPrivateProfileInt(sDDERUN,"debug",0,IMSIni);

	if(GetPrivateProfileInt(sDDERUN,"MaximizeOnOpen",0,IMSIni))
		ShowWindow(IMSWnd,SW_SHOWMAXIMIZED);
	else
		ShowWindow(IMSWnd,SW_RESTORE);

	RunMode=rmDDERUN;
	RunLinked=FALSE;
	SetMode(mRUN);
	}

/******************************************************************************
	DDERunIMStateAdvise()

	Causes a dde advise for the the IMState DDERUN item so that a hotlinked
	client will get state changes automatically.
******************************************************************************/
void DDERunIMStateAdvise(void)
	{
	if(ddeId)
		DdePostAdvise(ddeId,hszDDERUN,hsz_IMSTATE);
	}

/******************************************************************************
	Reload the DDE INI settings

******************************************************************************/
void ReloadDDESettings(){
	NBFChangeData=GetPrivateProfileInt(sDDERUN,"NBFChangeData",0,IMSIni);
}

void InputBit(WORD event) // Bit monitor callback
	{
	wsprintf(dbuf,"InputBit(event=%d)\n",event); //zzzzz
	xOutputDebugString(dbuf);  //zzzzz
	int index;
	BOOL h2l=FALSE;
	index=event;
	if(index>=MAXIOS)
		{
		index-=MAXIOS;
		h2l=TRUE;
		}
	if(index<InpCount)
		{
		if(InpItems[index].hsz&&ddeId)
			{
			DdePostAdvise(ddeId,hszDDERUN,InpItems[index].hsz);
			xOutputDebugString("post advise \"");  //zzzzz
			xOutputDebugString(InpItems[index].name);  //zzzzz
			xOutputDebugString("\"\n");  //zzzzz
			return;
			}
		}
	}
