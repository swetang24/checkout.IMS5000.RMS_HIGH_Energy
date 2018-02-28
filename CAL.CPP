/******************************************************************************
	Name : CAL
	Desc : Calibrators for IMS5000
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <imslib32/imslib32.h>
#include <im5000/im5000.h>
#include "ims5000.h"
#include "login.h"
#include "imp.h"
#include "imf.h"
#include "vislamp.h"
#include "cal.h"

HWND CALWnd=0;
CALC *TheCal;
BOOL CalMarked;

static char SavedDir[128];
static BOOL CalWinPosLocked=FALSE;
static char dbuf[512];
static BOOL caldebug=FALSE;

CALC::CALC()
	{
	memset(this,0,sizeof(CALC));
	CalMarked=TRUE;
	locator[0]=0;
	}

CALC::~CALC()
	{
	if(dp)
		FreeProcInstance(dp);
	}

// Opens the Cal dialog
HWND CALC::open(HWND parent,HWND *padwnd)
	{
	if(!hwnd)
		{
		pADWnd=padwnd;
		hparent=parent;
		hwnd=CreateDialogParam(IMSInst,"CAL",0,(DLGPROC)CALDP,(LPARAM)this);
		CALWnd=hwnd;
		return hwnd;
		}
	BringWindowToTop(hwnd);
	return hwnd;
	}

// Closes the dialog for this cal if its open
void CALC::close(void)
	{
	if(hwnd)
		{
		if(marksteps)
			IMSTEPFree(marksteps,FALSE);
		marksteps=NULL;
		if(locsteps)
			{
			SendMessage(IMWnd,IMM_CANCEL,0,0);
			destroy=TRUE;
			return;
			}
		DestroyWindow(hwnd);
		hwnd=0;
		CALWnd=0;
		VisionLampEnd();
		}

	}

void CALC::imnlocate(LPP3 ptcs)
	{
	if(caldebug)
		{
		sprintf(dbuf,"CALC::imnlocate() {%0.4f,%0.4f,%0.4f}\n",ptcs->x,ptcs->y,ptcs->z);
		OutputDebugString(dbuf);
		}
	// Compute ploc for ptcs using irtm irtz
	//P3 tempp3=*ptcs;
	//zzzLPP2 a=(LPP2)ptcs;
	P2 a=*((LPP2)ptcs);
	P2 tempx=a*irtm; //P2Xform((LPP2)&locpos,(LPP2)ptcs,&irtm);
	locpos.x=tempx.x;
	locpos.y=tempx.y;
	if(ZEXISTS)
		locpos.z=ptcs->z+irtz;
	else
		locpos.z=0.0f;
	if(caldebug)
		{
		sprintf(dbuf,"      locpos= {%0.4f,%0.4f,%0.4f}\n",locpos.x,locpos.y,locpos.z);
		OutputDebugString(dbuf);
		}
	}

/*****************************************************************************
	CAL private
******************************************************************************/
void CALC::init(LPSTR calname)
	{
	calok=FALSE;
	if(!GetPrivateProfileString(calname,"locator","",locator,sizeof(locator),CalIni))
		return;
	GetPrivateProfileString(calname,"prep","",prep,sizeof(prep),CalIni);

	// Get calibration type
	GetPrivateProfileString(calname,"caltype","",tbuf,sizeof(tbuf),CalIni);
	if(!lstrcmpi(tbuf,"THETA"))
		caltype=calTHETA;
	else
		caltype=calOFFSET; // default

	// Attempt to get geometry from calimp
	BOOL gotgeometry=FALSE;
	// Attempt to get geometry from calimf
	char name[32];
	if((!gotgeometry)&&GetPrivateProfileString(calname,"calimf","",name,sizeof(name),CalIni))
		{
		IMF imf;
		getddir(SavedDir);
		chddir(IMSProgs);
		BOOL ok=GetIMF(&imf,name);
		chddir(SavedDir);
		if(!ok)
			return;

		if(imf.nests.count<1)
			{
			IMFFree(&imf);
			return;
			}
		IMFNEST *pnest=&(imf.nests.nest[0]);
		M3 fcs,ncs,frame;
		GetFCS(&fcs,&imf,FALSE);
		GetNCS(&ncs,&imf,pnest,0,FALSE);
		frame=ncs*fcs;//M3Mul(&frame,&ncs,&fcs);
		calpos.x=frame.m3[2][0];
		calpos.y=frame.m3[2][1];
		calpos.z=imf.extent.z;
		caltheta=atan2(-frame.m3[1][0],frame.m3[1][1])/DEG;
		if(caltheta<0.0f)
			caltheta+=360.0f;
		IMFFree(&imf);
		gotgeometry=TRUE;
		}

	// Get optional CALIMP and CALDIR
	calimp[0]=0;
	if(GetPrivateProfileString(calname,"calimp","",tbuf,sizeof(tbuf),CalIni))
		{
		// Determine caldir and calimp
		char *p=strrchr(tbuf,'\\'); // p is start of imp portion
		if(p)
			p++;
		else
			p=tbuf;
		// Prefix caldir with IMSProgs if relative path
		if(!(tbuf[0]=='\\' || strchr(tbuf,':')))
			{
			lstrcpy(caldir,IMSProgs);
			lstrcat(caldir,"\\");
			}
		// Copy imp portion to calimp
		lstrcpy(calimp,p);
		// Cat remainder of path by truncating tbuf at start of imp first
		*p=0;
		lstrcat(caldir,tbuf);

		IMF imf;
		getddir(SavedDir);
		BOOL ok=GetIMFIMP(&imf,caldir,calimp);
		chddir(SavedDir);
		if(!ok)
			return;

		// Get geometry from calimp if not defined by imf allready
		if(!gotgeometry)
			{
			M3 frame; // Mark to World CS Transform
			double z;  // Mark WCS Z
			if(!GetMWCSZ(&frame,&z,&imf,0,0))
				{
				IMPFree(&imf.imp);
				IMFFree(&imf);
				return;
				}
			calpos.x=frame.m3[2][0];
			calpos.y=frame.m3[2][1];
			calpos.z=z;
			caltheta=atan2(-frame.m3[1][0],frame.m3[1][1])/DEG;
			if(caltheta<0.0f)
				caltheta+=360.0f;
			gotgeometry=TRUE;
			}
		IMPFree(&imf.imp);
		IMFFree(&imf);
		}

	// Get calpos and caltheta if geometry not defined in imp or imf
	if(!gotgeometry)
		{
		if(!GetPrivateProfileVect(calname,"caltheta",&caltheta,1,CalIni))
			caltheta=90;
		if(!GetPrivateProfileVect(calname,"calpos",(LPDOUBLE)&calpos,3,CalIni))
			return;
		gotgeometry=TRUE;
		}

	// Get updatez flag (enables zbase update)
	updatez=(BOOL)GetPrivateProfileInt(calname,"updatez",0,CalIni);
	calvlen=updatez ? 3 : 2;

	vislamp=(BOOL)GetPrivateProfileInt(calname,"VisLamp",0,CalIni);
	if(vislamp)
		VisionLampOn();
	else
		VisionLampEnd();
	locpos=calpos;
	destkey[0]=0;
	if(!GetPrivateProfileString(calname,"destsect","",destsect,sizeof(destsect),CalIni))
		return;
	if(!GetPrivateProfileString(calname,"destkey","",destkey,sizeof(destkey),CalIni))
		return;
	isvis=!lstrcmp(destsect,"VISCONFIG");
	updatelevel=GetPrivateProfileInt(calname,"updatelevel",5,CalIni);
	GetPrivateProfileString(calname,"instructions","",instructions,sizeof(instructions),CalIni);
	calok=TRUE;
	}

// CAL::CALDP(...) Win level dialog proc for CAL
// STATIC
BOOL CALLBACK CALC::CALDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	if(msg==WM_INITDIALOG) // Save this pointer in properties on init
		{
		SetProp(hdlg,"THISH",(HANDLE)HIWORD(lP));
		SetProp(hdlg,"THISL",(HANDLE)LOWORD(lP));
		}
	HANDLE th=GetProp(hdlg,"THISH"); // Get this ponter from properties
	HANDLE tl=GetProp(hdlg,"THISL");
	CALC *t=(CALC *)MAKELONG(tl,th);
	if(!t)
		return FALSE;
	return t->DP(hdlg,msg,wP,lP); // Displatch to instance
	}

// DP(...) Instance level dialog proc for CAL
BOOL CALLBACK CALC::DP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	switch(msg)
		{
		case WM_INITDIALOG:
			{
			caldebug=GetPrivateProfileInt("CAL","debug",0,CalIni);
			hwnd=hdlg;
			if(!loadcallist(GetDlgItem(hdlg,IDC_CALLIST)))
				{
				ShowError(IDE_NOCALS);
				PostMessage(hdlg,WM_CLOSE,0,0);
				return TRUE;
				}
			// Load autocal sequence or hide autocal button if none
			if(!loadautocal())
				ShowWindow(GetDlgItem(hdlg,IDC_AUTOCAL),SW_HIDE);
			autocal=FALSE;

			// Restore window position from [CAL]CalWinPos in CAL.INI
			POINT p;
			if(GetPrivateProfileIntVect("CAL","CalWinPos",(int*)&(p.x),2,CalIni))
				{
				RECT r;
				GetWindowRect(hdlg,&r);
				MoveWindow(hdlg,p.x,p.y,r.right-r.left,r.bottom-r.top,FALSE);
				}
			CalWinPosLocked=(BOOL)GetPrivateProfileInt("CAL","CalWinPosLocked",0,CalIni);

REINIT:
			char cname[64];
			SendDlgItemMessage(hdlg,IDC_CALLIST,CB_GETLBTEXT,cursel,(LPARAM)(LPSTR)cname);
			IMVisLive(locator,FALSE);
			init(cname);
			imok=IMState==IMN_READY||IMState==IMN_PRINTOFF||IMState==IMN_XYONLY||IMState==IMN_WARMUP;
			
			//char buf[512];
			//wsprintf(buf,"CAL IMSTATE = %d\n",IMState);
			//OutputDebugString(buf);


			// Pre prep selected locator so live works correctly
			if(prepsteps)
				IMSTEPFree(prepsteps,FALSE);
			prepsteps=IMSELLOCATORSTEPNew(locator);
			IMSTEPAppend(prepsteps,IMPREPLOCATORSTEPNew(prep));
			if(prepsteps)
				SendMessage(IMWnd,IMM_MARK,0,(LPARAM)prepsteps);
			Sleep(100);
			visok=IMVisLive(locator,TRUE);
			if(visok)
				visok=IMVisIRTM(&irtm,locator,isvis ? caltheta : 0.0f);
			if(visok)
				visok=IMVisIRTZ(&irtz,locator);

			if(caldebug)
				{
				OutputDebugString("CALC::init(");
				OutputDebugString(cname);
				sprintf(dbuf,") calpos= {%0.4f,%0.4f,%0.4f}\n  caltheta=",calpos.x,calpos.y,calpos.z);
				OutputDebugString(dbuf);
				sprintf(dbuf,"%0.4f\n  ",caltheta);
				OutputDebugString(dbuf);
				
				OutputDebugString("irtm=\n    ");
				VectToString(dbuf,(LPDOUBLE)&irtm.m3[0],3,",");
				OutputDebugString(dbuf);
				OutputDebugString("\n    ");
				VectToString(dbuf,(LPDOUBLE)&irtm.m3[1],3,",");
				OutputDebugString(dbuf);
				OutputDebugString("\n    ");
				VectToString(dbuf,(LPDOUBLE)&irtm.m3[2],3,",");
				OutputDebugString(dbuf);
				OutputDebugString("\n");
				}

			SkipM1=FALSE;
			if(calok)
				{
				SetDlgItemText(hdlg,IDC_CALINST,instructions);
				// CALPOS
				P3 p=calpos;
				MScaleVect((LPDOUBLE)&p,3,FALSE);
				VectToString(tbuf,(LPDOUBLE)&p,calvlen,",");
				SetDlgItemText(hdlg,IDC_CALPOS,tbuf);

				// LOCPOS
				p=locpos;
				MScaleVect((LPDOUBLE)&p,3,FALSE);
				VectToString(tbuf,(LPDOUBLE)&p,calvlen,",");
				SetDlgItemText(hdlg,IDC_LOCPOS,tbuf);

				// DELTAPOS
				p.x=locpos.x-calpos.x;
				p.y=locpos.y-calpos.y;
				p.z=locpos.z-calpos.z;
				MScaleVect((LPDOUBLE)&p,3,FALSE);
				VectToString(tbuf,(LPDOUBLE)&p,calvlen,",");
				SetDlgItemText(hdlg,IDC_DELTAPOS,tbuf);
				}
			else
				{
				SetDlgItemText(hdlg,IDC_CALINST,"CAL Invalid");
				SetDlgItemText(hdlg,IDC_CALPOS,"");
				SetDlgItemText(hdlg,IDC_LOCPOS,"");
				SetDlgItemText(hdlg,IDC_DELTAPOS,"");
				}


			SetFocus(GetDlgItem(hdlg,IDC_CALLIST));
			destroy=FALSE;

			updateenables(hdlg);
			if(autocal)  
				PostMessage(hdlg,WM_COMMAND,MAKELONG(IDC_LOCATE,BN_CLICKED),0); // Win32 changed
			return TRUE;
			}

		case WM_ACTIVATE:
			if(pADWnd) // Allow app to to IsDialogMessage
				*pADWnd=wP ? hdlg : 0;
			return FALSE;

///NEW
		case WM_MOVE:
			if(!CalWinPosLocked)
				{
				RECT r;
				GetWindowRect(hdlg,&r);
				wsprintf(tbuf,"%d,%d",r.left,r.top);
				WritePrivateProfileString("CAL","CalWinPos",tbuf,CalIni);
				}
			break;
///ENDNEW

		case WM_CLOSE:
			close();
			return FALSE;

		case WM_COMMAND:  // Win32 changed
			{
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			//WORD ncode=HIWORD(lP);
			switch(id)
				{
				case IDCANCEL:
					close();
					return FALSE;
				case IDC_CALLIST:
					if(ncode==CBN_SELCHANGE)
						{
						cursel=SendDlgItemMessage(hdlg,IDC_CALLIST,CB_GETCURSEL,0,0);
						goto REINIT;
						}
					return TRUE;

				case IDC_AUTOCAL:
					autocal=TRUE;
					cursel=acseq[0];
					SendDlgItemMessage(hwnd,IDC_CALLIST,CB_SETCURSEL,cursel,0);
					goto REINIT;

				case IDC_CALMARK:
					SetFocus(GetDlgItem(hdlg,IDC_CALPOS));
					mark();
					return TRUE;

				case IDC_LOCATE:
					{
					SetFocus(GetDlgItem(hdlg,IDC_CALPOS));
					if(locsteps)
						IMSTEPFree(locsteps,FALSE);
					locsteps=CALCompileLocate();
					SkipM1=FALSE;
					if(locsteps)
						SendMessage(IMWnd,IMM_MARK,0,(LPARAM)locsteps);
					return TRUE;
					}

				case IDC_UPDATE:
					update(hdlg);
					if(autocal)
						if(autocalnext())
							goto REINIT;
					return TRUE;

				case IDC_IM:
					switch(ncode)
						{
						case IMN_READY:
						case IMN_XYONLY:
						case IMN_PRINTOFF:
						case IMN_WARMUP:
							if(prepsteps)
								{
								IMSTEPFree(prepsteps,FALSE);
								prepsteps=NULL;
								}
							if(marksteps)
								{
								CalMarked=TRUE;
								IMSTEPFree(marksteps,FALSE);
								marksteps=NULL;
								}
							if(locsteps)
								{
								P3 p=locpos;
								MScaleVect((LPDOUBLE)&p,3,FALSE);
								VectToString(tbuf,(LPDOUBLE)&p,calvlen,",");
								SetDlgItemText(hdlg,IDC_LOCPOS,tbuf);
								IMSTEPFree(locsteps,FALSE);
								locsteps=NULL;
								// DELTAPOS
								p.x=locpos.x-calpos.x;
								p.y=locpos.y-calpos.y;
								p.z=locpos.z-calpos.z;
								MScaleVect((LPDOUBLE)&p,3,FALSE);
								VectToString(tbuf,(LPDOUBLE)&p,calvlen,",");
								SetDlgItemText(hdlg,IDC_DELTAPOS,tbuf);
								if(autocal)
									{
									if(calok) // CAL HAS UPDATE
										PostMessage(hdlg,WM_COMMAND,MAKELONG(IDC_UPDATE,BN_CLICKED),0); //changed Win32
									else
										if(autocalnext())
											goto REINIT;
									}
								}
							break;
						case IMN_ERROR:
							if(marksteps)
								IMSTEPFree(marksteps,FALSE);
							marksteps=NULL;
							if(locsteps)
								{
								IMSTEPFree(locsteps,FALSE);
								locsteps=NULL;
								SkipM1=TRUE; // Retry on next ready
								updateenables(hdlg);
								return FALSE;
								}
							autocal=FALSE;
							// Mode Wnd handles error after ret
							break;
						case IMN_CANCEL:
							if(destroy)
								{
								DestroyWindow(hwnd);
								hwnd=0;
								CALWnd=0;
								return FALSE;
								}
							break;
						}
					imok=IMState==IMN_READY || IMState==IMN_PRINTOFF || IMState==IMN_XYONLY|| IMState==IMN_WARMUP;
					updateenables(hdlg);
					return FALSE;
				}
			return FALSE;
			}
		case WM_DESTROY:
			IMVisLive(locator,FALSE);
			if(marksteps)
				IMSTEPFree(marksteps,FALSE);
			marksteps=NULL;
			if(locsteps)
				IMSTEPFree(locsteps,FALSE);
			locsteps=NULL;
			// Unreg callback unless needed by mRUN
			dp=NULL;
			return 0;

		}
	return FALSE;
	}

// updateenables : updates the enabled state of controls on the cal
// based on visok and imok
void CALC::updateenables(HWND hdlg)
	{
	if(!hdlg)
		return;
	EnableWindow(GetDlgItem(hdlg,IDC_CALMARK),imok&&visok&&calok&&calimp[0]);
	EnableWindow(GetDlgItem(hdlg,IDC_LOCATE),imok&&visok&&calok);
	EnableWindow(GetDlgItem(hdlg,IDC_UPDATE),imok&&visok&&calok
		&&(isvis || CalMarked)
		&&(updatelevel<=UserLevel));
	SetDlgItemText(hdlg,IDC_LOCATE,SkipM1 ? "*&Locate" : "&Locate");
	}

/******************************************************************************
	steps = CALCompileLocate()

	Compiles the locate sequence for the CAL

	PIMSTEPS steps = The compiled step seqence or NULL on error.
******************************************************************************/
PIMSTEP CALC::CALCompileLocate(void)
	{
	PIMSTEP steps,pstep;

	// Compute zsafe
	zsafe=calpos.z+0.25;
	zsafe=max(zsafe,StartPoint.z);
	//zzzzz
	if(DebugMode)
		{
		sprintf(dbuf,"CALCompileLocate() zsafe=%0.4f\n",zsafe);
		OutputDebugString(dbuf);
		}
	// Select locator
	steps=IMSELLOCATORSTEPNew(locator);
	IMSTEPAppend(steps,IMMSGSTEPNew("LOCATING"));
	
	if(ZEXISTS && !SkipM1) // Move Z to start position, z first
		IMSTEPAppend(steps,IMZMOVESTEPNew(zsafe,xymcsTCS));

	pstep=compile();
	if(!pstep)
		{
		IMSTEPFree(steps,TRUE);
		steps=NULL;
		goto EXIT;
		}
	IMSTEPAppend(steps,pstep);
EXIT:
	return IMOptimizeSteps(steps,0,0,0,0,0,0);
	}

PIMSTEP CALC::compile()
	{
	// Prep locator
	PIMSTEP steps=IMPREPLOCATORSTEPNew(prep);
	if(!steps)
		{
		ShowError(IDE_BADLOCATEPREP);
		return NULL;
		}
	zvis=calpos.z;

	// Must XYMOVE to 0,0 in rotated frame at calpos
	//M3 r;
	double theta=isvis ? caltheta : 0.0f;
	//M3Rotate(&r,theta*DEG);
	M3 r(ROTATE,0,0,theta*DEG);
	r.m3[2][0]=calpos.x;
	r.m3[2][1]=calpos.y;
	IMSTEPAppend(steps,IMPUSHSTEPNew(&r));
	P2 org;
	org.x=0.0f;
	org.y=0.0f;

	if(!SkipM1)
		IMSTEPAppend(steps,IMXYMOVESTEPNew((LPP2)&org,zvis,xymcsLOCATOR));
	IMSTEPAppend(steps,IMLOCATESTEPNew(0,0));
	IMSTEPAppend(steps,IMPOPSTEPNew());

	return steps;
	}

// ok = loadcallist(cb) - loads the combobox cb with the cal names from cal.ini
// that are at or below the current user level.  Returns false if none are found.
BOOL CALC::loadcallist(HWND cb)
	{
	// Load IDC_CALLIST from cal ini
	int ci=0;
	char sci[8];
	wsprintf(sci,"%d",ci);
	BOOL ok=FALSE;
	while(GetPrivateProfileString("CAL",sci,"",tbuf,TSIZE,CalIni))
		{
		SendMessage(cb,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
		ok=TRUE;
		ci++;
		wsprintf(sci,"%d",ci);
		}
	if(ok)
		SendMessage(cb,CB_SETCURSEL,cursel,0);
	return ok;
	}

void CALC::savecalhist(void)
	{
	if(GetPrivateProfileString(destsect,destkey,"",tbuf,TSIZE,IMIni))
		{
		char key[30];
		GetGMT(key);
		char sect[64];
		wsprintf(sect,"%s.%s.hist",(LPSTR)destsect,(LPSTR)destkey);
		WritePrivateProfileString(sect,key,tbuf,CalHistIni);
		}
	}

void CALC::writeprofilevect(LPSTR sect,LPSTR key,LPDOUBLE v,int vlen,LPSTR ini)
	{
	char fmt[64];
	lstrcpy(fmt,"%0.4f");
	for(int i=1;i<vlen;i++)
		lstrcat(fmt,",%0.4f");
	sprintf(tbuf,fmt,v[0],v[1],v[2],v[3],v[4],v[5]);
	WritePrivateProfileString(sect,key,tbuf,ini);
	}

// Loads the acseq from AUTOCAL in the ini file. returns TRUE on success.
BOOL CALC::loadautocal(void)
	{
	GetPrivateProfileString("CAL","AUTOCAL","",tbuf,TSIZE,CalIni);
	int acl=GetPrivateProfileInt("CAL","AutoCalLevel",0,CalIni);
	if(UserLevel<acl)
		return FALSE;
	LPSTR p=tbuf;
	int i;
	acseqlen=0;
	while((p=ParseInt(p,&i)) && acseqlen<ACSEQMAX)
		acseq[acseqlen++]=i;
	return (BOOL)acseqlen;
	}

// Advances to the next cal in acseq or return FALSE if none
BOOL CALC::autocalnext(void)
	{
		int si;
	for(si=0;si<acseqlen;si++)
		if(acseq[si]==cursel)
			break;
	if(si>=acseqlen-1)
		{
		MessageBox(hwnd,"Auto Calibration Complete",IMSName,MB_OK);
		autocal=FALSE;
		close();
		return FALSE;
		}
	cursel=acseq[si+1];
	SendDlgItemMessage(hwnd,IDC_CALLIST,CB_SETCURSEL,cursel,0);
	return TRUE;
	}

// Starts the calimp marking sequence
void CALC::mark(void)
	{
	if(marksteps) // No reentrancy
		return;

	IMF imf;
	getddir(SavedDir);
	BOOL ok=GetIMFIMP(&imf,caldir,calimp);
	chddir(SavedDir);
	if(!ok)
		return;

	// Load VisPos and VisOn for inspector prior to mark/inspect compile
	VisOn=imf.imp.pheader->inspector[0]!=0;
	VisPos=imf.imp.pheader->inspectpos;

	// Prepare NBF
	for(int i=0;i<imf.nests.count;i++)
		{
		IMFNEST *pnest=&(imf.nests.nest[i]);
		pnest->flags=nbfOCCUPIED;
		}

	// Compile
	marksteps=IMFCompile(&imf);
	IMPFree(&imf.imp);
	IMFFree(&imf);
	if(!marksteps)
		return;
	IMClearRTSPoint(TRUE);
	SendMessage(IMWnd,IMM_MARK,0,(LPARAM)marksteps);
	}

// Performs the update action for the given calibration
void CALC::update(HWND hdlg)
	{
	P3 deltapos;
	deltapos.x=locpos.x-calpos.x;
	deltapos.y=locpos.y-calpos.y;
	deltapos.z=locpos.z-calpos.z;

	// Get the config vector (cv) one of:
	// HRx,HRy
	// HRx,HRy,ZBASE
	// HTx,HTy,TRx,TRy
	// HTx,HTy,TRx,TRy,ZBASE
	GetPrivateProfileString(destsect,destkey,"",tbuf,TSIZE,IMIni);
	LPSTR p=tbuf;
	double cv[5];
	int cvlen=0;
	while(cvlen<5) // determine size of cv and load it
		{
		if(!(p=ParseDouble(p,&(cv[cvlen]))))
			break;
		cvlen++;
		}
	int cvix=cvlen>3?2:0; // cv index of TRx or HRx

	if(!isvis) // IJ Update?
		CalMarked=FALSE; // Require mark before next ij update

	switch(caltype)
		{
		case calTHETA:
			// Extend cv if HT0 not in current config
			if(cvlen<4)
				{
				int i=cvlen;
				while(i--)
					cv[i+2]=cv[i];
				cv[0]=0.0f;
				cv[1]=0.0f;;
				cvlen+=2;
				cvix=2;
				}
			if(isvis) // VIS THETA Updates -deltapos
				{
				deltapos.x=-deltapos.x;
				deltapos.y=-deltapos.y;
				}
			// THETA Update
			correcttheta(&(cv[0]),&deltapos);
			break;
		case calOFFSET:
		default:
			if(isvis) // VIS OFFSET Update
				{
				cv[cvix]+=deltapos.x;
				cv[cvix+1]+=deltapos.y;
				if(updatez&&(cvlen==3||cvlen==5))
					cv[cvix+2]-=deltapos.z;
				}
			else // IJ OFFSET Update
				{
				cv[cvix]-=deltapos.x;
				cv[cvix+1]-=deltapos.y;
				// (updatez) not valid for ij offsets
				}
			break;
		}

	writeprofilevect(destsect,destkey,(LPDOUBLE)&cv,cvlen,IMIni);

	SendMessage(IMWnd,IMM_CFGCHANGE,0,0); // Tell IM of change

	visok=IMVisIRTM(&irtm,locator,isvis ? caltheta : 0.0f);
	if(visok)
		visok=IMVisIRTZ(&irtz,locator);

	// ZERO DELTAPOS
	deltapos.x=0.0f;
	deltapos.y=0.0f;
	deltapos.z=0.0f;
	VectToString(tbuf,(LPDOUBLE)&deltapos.x,calvlen,",");
	SetDlgItemText(hdlg,IDC_DELTAPOS,tbuf);
	savecalhist();
	}

// See 4001\DOC\THETACAL
void CALC::correcttheta(LPDOUBLE cv,LPP3 e)
	{
	double cosa=cos(caltheta*DEG);
	double sina=sin(caltheta*DEG);
	P2 hta;
	hta.x=cv[0]*cosa-cv[1]*sina;
	hta.y=cv[0]*sina+cv[1]*cosa;

	P2 d;
	// d=e+ht0-hta
	d.x=e->x+cv[0]-hta.x;
	d.y=e->y+cv[1]-hta.y;

	cv[2]+=cv[0]; // Add old offset to TR to save 0 deg offset
	cv[3]+=cv[1];

	double a=caltheta/2.0f;
	double h=sqrt(d.x*d.x+d.y*d.y)/(2*sin(a*DEG));
	double b=90-a;
	double c=atan2(d.y,d.x)/DEG;
	double bc=b+c;
	cv[0]=h*cos(bc*DEG); // Update HT0
	cv[1]=h*sin(bc*DEG);
	cv[2]-=cv[0]; // Sub new offset from TR to restore 0 deg offset
	cv[3]-=cv[1];
	}

