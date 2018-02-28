/***************************************************************************
	NAME : EDITOR.C
	DESC : Implementation of Editor window functions for IMS
***************************************************************************/
#include "stdafx.h"
#include <Shellapi.h>                //-----------Linghai-11-14-08---
#include <tchar.h>                       //--------Lingahi-11-14-08----

#include <string>                //------------Linghai-11-12-08----
#include <iostream>
using namespace std;



#include <windows.h>
#include <string.h>

#include <stdio.h>
#include <imslib32\imslib32.h>
#include <jvmath32\jvmath32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "run.h"
#include "editor.h"
#include "devbc.h"
#include "c2of5.h"
#include "c39.h"
#include "c93.h"
#include "c128.h"
#include "cupcean.h"
#include "cdm.h"
#include "winfont.h"
#include "patch.h"
#include "WMTRX.H"
#include "cqr.h"
#include "login.h"
#include "vislamp.h"
#include "clamp.h"
#include "stdloc.h"
#include "cure.h"
#include "cdev.h"
#include "commdlg.h"
#include "resource.h"
#include "preview.h"
/***************************************************************************
	PRIVATE DEFINES
***************************************************************************/
// Control IDs
#define IDC_H 100 // Header window id
#define IDC_V 101 // Vars window id
#define IDC_M 102 // Marks window id
#define IDC_P 103 // Preview window id                      //----------Linghai-11-19-08----

/***************************************************************************
	NON API EXPORT FUNCTION PROTOTYPES
***************************************************************************/
LRESULT WINAPI EDITORWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK KVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK DATEVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK TIMEVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK BASEDVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK SVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK TEXTMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK DEVSYMMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK BITMAPMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK MULTILINEMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK UDHAEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK VISPOSDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

/***************************************************************************
	STATIC FUNCTION PROTOTYPES
***************************************************************************/
static void EditorWM_CREATE(HWND hwnd);
static void EditorWM_COMMAND(HWND hwnd,WPARAM wP,LPARAM lP);
static BOOL DoVarEdit(HWND parent);
static BOOL DoMarkEdit(HWND parent,IMPMARK *mark,BOOL issub);
static BOOL EditHeaderEntry(HWND hwnd,IMPHEADER *h,IMPUDHAS *u,WORD i);
static BOOL DoUDHAEdit(HWND hwnd,IMPUDHA *udha);
static BOOL DoInspectPosEdit(HWND hwnd,IMPHEADER *h,IMPUDHAS *u);
static BOOL DoReadPosEdit(HWND hwnd,IMPHEADER *h,IMPUDHAS *u);
static void EditSelectIMConfig(void);
static void inkjetUpdate(void);
static void NewMark(WORD idm,HWND hwnd);
static IMXYMODE GetXYMODE(char *fixture);
static int raxitdatapass(double passingdata);
static void DoLocate(HWND hdlg,int idcx,int idcy,LPSTR prompt);
void RotBlt(HDC destDC, int srcx1, int srcy1, int srcx2, int srcy2,
  HDC srcDC , int destx1, int desty1 ,int thetaInDegrees ,DWORD mode);

/***************************************************************************
	PUBLIC VARIABLES
***************************************************************************/
IMP  EditorIMP;     // IMP to edit
IMPVALS EditorPreIVals;  // Instance editor preview vals
IMPVALS EditorPreGVals; // Global editor preview vals
IMPMARK EditorMark;   // Copy of mark being editied by dialog
IMPMARK ClipboardMark;   // Mark on fake clipboard
BOOL ClipboardMarkValid=false;
BOOL EditorIsSub=FALSE;
BOOL EditorIJFound=FALSE; // True if editing with attached ij
HWND EditorIJDev;         // Device HWND of inkjet if EditorIJFound
IJINFO EditorIJInfo;      // IJINFO of inkjet if EditorIJFound
BOOL EditHasRadius;
IMXYMODE EditXYMode;
double EditXScale;  // Metric display scaling factor X or 1.0
double EditYScale;  // Metric display scaling factor Y or 1.0
double EditZScale;  // Metric display scaling factor Z or 1.0
double EditWScale;  // Metric display scaling factor W or 1.0---sam 01/06/17

BOOL EditBandEnable; // Band preview enable
COLORREF EditEvenBandColor; // even band background
COLORREF EditOddBandColor;  // odd band background
BOOL EditHasRot;   // No mark rotation flag
BOOL EditHasRotDeg;   // No mark rotation degree flag
BOOL EditHasSlant; // No mark slant flag
IMCORR EditorIMCorr;   // Copy of im correction being editied by dialog

/***************************************************************************
	STATIC VARIABLES
***************************************************************************/
static char EditorClass[]="IMSEDITOR"; // Editor window class name
static HWND HWnd; // Header window
static HWND VWnd; // Vars window
static HWND MWnd; // Marks window
static HWND PWnd; // Preview window                   //----------------Linghai-11-19-08---
static int HCurSel; // Header current selection index
static int VCurSel; // Vars current selection index
static int MCurSel; // Marks current selection index
static WORD FocusID; // ID of child listbox in focus : 0 for marker
static IMPVAR editvar;   // Copy of var being editied by dialog
static IMPUDHA editudha;
static int vncount;     // counter for making var names
static BOOL editisgvar; // Value to set global checkbox ...
                        // ... on entry and exit from var edit dialogs
static BOOL allowsave; // Flag set when it is ok to allow overwrite of an existing file
static HWND VisPosWnd=0; // VisPos dialog if open or 0 if not
static char sBANDVIEW[]="BANDVIEW";
static VISMODE EditorVisMode; // VISMODE in edit vis pos

static WIN32_FIND_DATA fData;                       //---------------------Linghai-11-10-08-------
//HANDLE nxtFile;                                    //---------------------Linghai-11-10-08-------

//static PREVIEWFONTS pf;                              //------------------------Linghai-11-20-08---
//static PREVIEW pv;                                   //-------------------------Linghai-11-20-08--
//static IMP previewIMP;

/***************************************************************************
	PUBLIC FUNCTIONS
***************************************************************************/

void RotBlt(HDC destDC, int srcx1, int srcy1, int srcx2, int srcy2,
  HDC srcDC , int destx1, int desty1 ,int thetaInDegrees ,DWORD mode)
{
  double theta = thetaInDegrees * (3.14159/180);
  //multiply degrees by PI/180 to convert to radians

  //determine width and height of source
  int width = srcx2 - srcx1;
  int height = srcy2 - srcy1;

  //determine centre/pivot point ofsource
  int centreX = int(float(srcx2 + srcx1)/2);
  int centreY = int(float(srcy2 + srcy1)/2);

  //since image is rotated we need to allocate a rectangle
  //which can hold the image in any orientation
  if(width>height)height = width;
  else
    width = height;


  //allocate memory and blah blah
  HDC memDC = CreateCompatibleDC(destDC);
  HBITMAP memBmp = CreateCompatibleBitmap(destDC, width, height);
  RECT rect;
  HBITMAP obmp = (HBITMAP) SelectObject(memDC, memBmp);
  rect.left=0;
  rect.top=0;
  rect.right=width;
  rect.bottom=height;
  FillRect(memDC, &rect, (HBRUSH) CreateSolidBrush(RGB(255,255,255)));

  //pivot point of our mem DC
  int newCentre = int(float(width)/2);

  //hmmm here's the rotation code. X std maths :|
  for(int x = srcx1; x<=srcx2; x++)
    for(int y = srcy1; y<=srcy2; y++)
    {
      COLORREF col = GetPixel(srcDC,x,y);

      int newX = int((x-centreX)*sin(theta)+(y-centreY)*cos(theta));
      int newY = int((x-centreX)*cos(theta)-(y-centreY)*sin(theta));


      SetPixel(memDC , newY + newCentre,newX + newCentre, col);
    }

  //splash onto the destination
  BitBlt(destDC, destx1, desty1, width, height, memDC, 0,0,mode);


  //free mem and blah
  SelectObject(memDC, obmp);

  DeleteDC(memDC);
  DeleteObject(memBmp);
}


/***************************************************************************
	hwnd = CreateEditorWnd(parent,id,x,y,w,h)

	Creates an editor child window.

	HWND hwnd = Handle of new editor window or NULL on error.
	HWND parent = Handle of parent for new editor window.
	WORD id = Child window ID for new editor window.
	int x = Window x coordinate
	int y = Window y coordinate
	int w = Window width
	int h = Window height
***************************************************************************/
HWND CreateEditorWnd(HWND parent,WORD id,int x,int y,int w,int h)
	{
	static BOOL registered=FALSE;
	WNDCLASS wc;

	if(!registered)
		{
		wc.style=CS_VREDRAW|CS_HREDRAW;
		wc.lpfnWndProc=EDITORWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); 
		wc.lpszMenuName=NULL;
		wc.lpszClassName=EditorClass;
		if(!RegisterClass(&wc))
			return NULL;
		registered=TRUE;
		}

	// Get BANDVIEW info from ini
	EditBandEnable=(BOOL)GetPrivateProfileInt(sBANDVIEW,"Enable",1,IMSIni);
	EditEvenBandColor=GetPrivateProfileCOLORREF(sBANDVIEW,"EvenBandColor",
		RGB(255,255,0),IMSIni);
	EditOddBandColor=GetPrivateProfileCOLORREF(sBANDVIEW,"OddBandColor",
		RGB(0,255,255),IMSIni);

	return CreateWindow(EditorClass,NULL,
		WS_CHILD|WS_VISIBLE,
		x,y,w,h,parent,(HMENU)id,IMSInst,NULL);
	}

/******************************************************************************
	DoInsertsPopup(hdlg,hctrl,pudhas,pvars,pgvars)

	Generates and executes the inserts popup menu for the text entry
	control of a mark editor dialog.
******************************************************************************/
void DoInsertsPopup(HWND hdlg,HWND hctrl,
	IMPUDHAS *pudhas,IMPVARS *pvars,IMPVARS *pgvars)
	{
	POINT p;
	//HMENU menu,vmenu,hamenu,svmenu,uvmenu;                    //--------------------Linghai-4-20-09-----
	HMENU menu,vmenu,hamenu,svmenu;                    //--------------------Linghai-4-20-09-----
	int i;

	// Create the vars sub menu
	vmenu=CreatePopupMenu();
	// Globals
	for(i=0;i<pgvars->count;i++)
		{
		wsprintf(tbuf,"%%%s%%",
		(LPSTR)&pgvars->var[i].name);
		AppendMenu(vmenu,MF_STRING|MF_ENABLED,IDM_VMENU+i,tbuf);
		}
	// Instance
	for(i=0;i<pvars->count;i++)
		{
		wsprintf(tbuf,"%%%s%%",(LPSTR)&pvars->var[i].name);
		AppendMenu(vmenu,MF_STRING|MF_ENABLED,IDM_VMENU+i+pgvars->count,tbuf);
		}

	// Create the header attributes sub menu
	hamenu=CreatePopupMenu();
	for(i=0;i<IMPNBHAS;i++) // basic
		{
		wsprintf(tbuf,"%%!%s%%",(LPSTR)IMPBHANames[i]);
		AppendMenu(hamenu,MF_STRING|MF_ENABLED,IDM_HAMENU+i,tbuf);
		}
	for(i=0;i<pudhas->count;i++) // user defined
		{
		wsprintf(tbuf,"%%!%s%%",(LPSTR)&pudhas->udha[i].name);
		AppendMenu(hamenu,MF_STRING|MF_ENABLED,IDM_HAMENU+i+IMPNBHAS,tbuf);
		}

	// Create the system vars sub menu
	svmenu=CreatePopupMenu();
	for(i=0;i<IMPNSYSVARS;i++)
		{
		wsprintf(tbuf,"%%%s%%",(LPSTR)IMPSysVarNames[i]);
		AppendMenu(svmenu,MF_STRING|MF_ENABLED,IDM_SVMENU+i,tbuf);
		}

	// Create the main popup menu

	//Create the uid vars sub menu                                                        //------------Linghai-4-20-09---
	/*uvmenu=CreatePopupMenu();
	for(i = 0; i <IMPUIDVARS;i++)
	{
		wsprintf(tbuf,"%%%s%%",(LPSTR)IMPUIDVarNames[i]);
		AppendMenu(uvmenu,MF_STRING|MF_ENABLED,IDM_UVMENU+i,tbuf);
	}*/

	menu=CreatePopupMenu();
	AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)vmenu,"Variables");
	AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)hamenu,"Header attributes");
	AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)svmenu,"System variables");
	//AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)uvmenu,"UID variables");                 //-----------Linghai-4-20-09---

	// Display the menu at the caret position
	GetCaretPos(&p);
	ClientToScreen(hctrl,&p);
	TrackPopupMenu(menu,0,p.x,p.y,0,hdlg,NULL);
	//SendMessage(hdlg,"This control",NULL,hctrl);
	SetCapture(hctrl);
	// Destroy the menus
	DestroyMenu(menu);
	}

/******************************************************************************
	DoInsertsPopup(hdlg,hctrl,pudhas,pvars,pgvars)

	Generates and executes the inserts popup menu for the text entry
	control of a mark editor dialog.
******************************************************************************/
void DoInsertsPopup(HWND hdlg,HWND hctrl,
	IMPVARS *pvars,IMPVARS *pgvars)
	{
	POINT p;
	//HMENU menu,vmenu,hamenu,svmenu,uvmenu;                    //--------------------Linghai-4-20-09-----
	HMENU menu,vmenu,svmenu;                    //--------------------Linghai-4-20-09-----
	int i;

	// Create the vars sub menu
	vmenu=CreatePopupMenu();
	// Globals
	for(i=0;i<pgvars->count;i++)
		{
		wsprintf(tbuf,"%%%s%%",
		(LPSTR)&pgvars->var[i].name);
		AppendMenu(vmenu,MF_STRING|MF_ENABLED,IDM_VMENU+i,tbuf);
		}
	// Instance
	for(i=0;i<pvars->count;i++)
		{
		wsprintf(tbuf,"%%%s%%",(LPSTR)&pvars->var[i].name);
		AppendMenu(vmenu,MF_STRING|MF_ENABLED,IDM_VMENU+i+pgvars->count,tbuf);
		}

	

	// Create the system vars sub menu
	svmenu=CreatePopupMenu();
	for(i=0;i<IMPNSYSVARS;i++)
		{
		wsprintf(tbuf,"%%%s%%",(LPSTR)IMPSysVarNames[i]);
		AppendMenu(svmenu,MF_STRING|MF_ENABLED,IDM_SVMENU+i,tbuf);
		}

	// Create the main popup menu

	//Create the uid vars sub menu                                                        //------------Linghai-4-20-09---
	/*uvmenu=CreatePopupMenu();
	for(i = 0; i <IMPUIDVARS;i++)
	{
		wsprintf(tbuf,"%%%s%%",(LPSTR)IMPUIDVarNames[i]);
		AppendMenu(uvmenu,MF_STRING|MF_ENABLED,IDM_UVMENU+i,tbuf);
	}*/

	menu=CreatePopupMenu();
	AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)vmenu,"Variables");
	//AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)hamenu,"Header attributes");
	AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)svmenu,"System variables");
	//AppendMenu(menu,MF_POPUP|MF_STRING,(unsigned int)uvmenu,"UID variables");                 //-----------Linghai-4-20-09---

	// Display the menu at the caret position
	GetCaretPos(&p);
	ClientToScreen(hctrl,&p);
	TrackPopupMenu(menu,0,p.x,p.y,0,hdlg,NULL);
	//SendMessage(hdlg,"This control",NULL,hctrl);
	SetCapture(hctrl);
	// Destroy the menus
	DestroyMenu(menu);
	}


/****************************************************************************
	find varable value by varable name
*****************************************************************************/
//void FindValuesByVarName(char[] name)
//{
//	tlen=wsprintf(tbuf,"%s",name);
//tlen=wsprintf(tbuf,"%s:%s",(LPSTR)pvar->name,(LPSTR)pval->k);
//}
/******************************************************************************
	processed=ProcessInsertsPopup(wP,hctrl)

	Processes the WM_COMMAND messages from the InsertsPopup menu.

	BOOL processed = TRUE if message processed.
	WORD wP= wP from WM_COMMAND message.
	HWND hctrl= text control to insert into.	
******************************************************************************/
BOOL ProcessInsertsPopup(WORD wP,HWND hctrl)
	{
	BOOL processed=FALSE;
	WORD id = LOWORD(wP); // Win32 change
	
	if(id & IDM_VMENU)
		{
		int vi;   // var index
		int ivi;  // instance var index

		vi=id^IDM_VMENU;
		ivi=vi-EditorIMP.gvars.count;
		if(0 <= vi && vi < EditorIMP.gvars.count)
			{
			wsprintf(tbuf,"%%%s%%",(LPSTR)&EditorIMP.gvars.var[vi].name);
			SendMessage(hctrl,EM_REPLACESEL,0,(LONG)(LPSTR)tbuf);
			}
		if(0 <= ivi && ivi < EditorIMP.vars.count)
			{
			wsprintf(tbuf,"%%%s%%",(LPSTR)&EditorIMP.vars.var[ivi].name);
			SendMessage(hctrl,EM_REPLACESEL,0,(LONG)(LPSTR)tbuf);
			}
		processed=TRUE;
		}
	else if(id & IDM_HAMENU)
		{
		int i;
		i=id^IDM_HAMENU;
		if(0 <= i && i < IMPNBHAS) // basic ?
			{
			wsprintf(tbuf,"%%!%s%%",(LPSTR)IMPBHANames[i]);
			SendMessage(hctrl,EM_REPLACESEL,0,(LONG)(LPSTR)tbuf);
			}
		else if(IMPNBHAS <= i && i < (EditorIMP.udhas.count+IMPNBHAS))
			{
			wsprintf(tbuf,"%%!%s%%",
				(LPSTR)&EditorIMP.udhas.udha[i-IMPNBHAS].name);
			SendMessage(hctrl,EM_REPLACESEL,0,(LONG)(LPSTR)tbuf);
			}
		processed=TRUE;
		}
	else if(id & IDM_SVMENU)
		{
		int i;
		i=id^IDM_SVMENU;
		if(0 <= i && i < IMPNSYSVARS)
			{
			wsprintf(tbuf,"%%%s%%",
				(LPSTR)IMPSysVarNames[i]);
			SendMessage(hctrl,EM_REPLACESEL,0,(LONG)(LPSTR)tbuf);
			}
		processed=TRUE;
		}
	/*else if(id & IDM_UVMENU)                   //---------------------------Linghai-4-20-09-----
	{
		int i;
		i=id^IDM_UVMENU;
		if(0 <= i && i < IMPUIDVARS)
		{
			wsprintf(tbuf,"%%%s%%",(LPSTR)IMPUIDVarNames[i]);//IMPUIDVarNames
			SendMessage(hctrl,EM_REPLACESEL,0,(LONG)(LPSTR)tbuf);
		}
		processed=TRUE;
	}*/
	return processed;
	}

/******************************************************************************
	EditXYModePrep(hdlg)

	Prepares the XY specific labels of the dialog and the unit scaling
	factors for displayed units (for metric mode)  for the given state of
	EditorXYMode and EditorHasRadius.
******************************************************************************/
void EditXYModePrep(HWND hdlg)
	{
	static int xidc[]=
		{IDC_XLBL,IDC_XRESLBL,IDC_XDOTSLBL,IDC_XSIZELBL,IDC_XYDOTSLBL,0};
	static int yidc[]=
		{IDC_YLBL,IDC_YRESLBL,IDC_YDOTSLBL,IDC_YSIZELBL,IDC_XYDOTSLBL,0};
	static int subnotidc[]=
		{IDC_XLBL,IDC_X,IDC_YLBL,IDC_Y,IDC_ZLBL,IDC_Z,IDC_ROTLBL,IDC_ROT,
		IDC_SLANTLBL,IDC_SLANT,IDC_JUSTLBL,IDC_JUST,IDC_XRESLBL,IDC_XRES,
		IDC_YRESLBL,IDC_YRES,IDC_MSLBL,IDC_MULTISTROKE,IDC_REPLBL,IDC_REPEATS,
		IDC_LOCATE, IDC_DEG,0};
	char c;
	int i;
	char *p;

	// Compute metric scaling factors EditXScale,EditYScale,EditZScale
	if(Metric)
		{
		double f=MetricMM ? 25.4f : 2.54f;
		switch(EditXYMode)
			{
			case imXS:
				EditXScale=f;
				EditYScale=EditHasRadius ? f : 1.0f;
				break;
			case imSY:
				EditXScale=EditHasRadius ? f : 1.0f;
				EditYScale=f;
				break;
			default:
				EditXScale=f;
				EditYScale=f;
			}
		EditZScale=f;
		}
	else
		{
		EditXScale=1.0f;
		EditYScale=1.0f;
		EditZScale=1.0f;
		}

	if(!hdlg)
		return;

	c=EditHasRadius ? 'C' : 'A';
	if(EditXYMode==imXS)
		{
		i=0;
		while(yidc[i])
			{
			if(GetDlgItemText(hdlg,yidc[i],tbuf,TSIZE))
				{
				for(p=tbuf;*p;p++)
					if(*p=='Y')
						*p=c;
				SetDlgItemText(hdlg,yidc[i],tbuf);
				}
			i++;
			}
		}
	else if(EditXYMode==imSY)
		{
		i=0;
		while(xidc[i])
			{
			if(GetDlgItemText(hdlg,xidc[i],tbuf,TSIZE))
				{
				for(p=tbuf;*p;p++)
					if(*p=='X')
						*p=c;
				SetDlgItemText(hdlg,xidc[i],tbuf);
				}
			i++;
			}
		}
	if(EditXYMode==imXY)
		{
		EditHasRot=!HideOptMarkRotation;
		EditHasSlant=!HideOptMarkSlant;
		//EditHasRotDeg=!HideOptRotaryDEG; ///  deg
		}
	else
		{
		EditHasRot=FALSE;
		EditHasSlant=FALSE;
		//EditHasRotDeg=FALSE; ///  deg
		}
	if(!EditHasRot)
		{
		ShowWindow(GetDlgItem(hdlg,IDC_ROTLBL),SW_HIDE);
		ShowWindow(GetDlgItem(hdlg,IDC_ROT),SW_HIDE);
		}
	/*if(!EditHasRotDeg)
		{
		ShowWindow(GetDlgItem(hdlg,IDC_DEGLBL),SW_HIDE);
		ShowWindow(GetDlgItem(hdlg,IDC_DEG),SW_HIDE);
		}*/
	if(!EditHasSlant)
		{
		ShowWindow(GetDlgItem(hdlg,IDC_SLANTLBL),SW_HIDE);
		ShowWindow(GetDlgItem(hdlg,IDC_SLANT),SW_HIDE);
		}
	if(EditorIsSub) // Disable SUBNOT controls if in sub editor
		for(i=0;subnotidc[i];i++)
			{
			HWND w=GetDlgItem(hdlg,subnotidc[i]);
			if(w)
				EnableWindow(w,FALSE);
			}
	}

// DoMarkLocate : Performs interactive mark location using StdLoc
void DoMarkLocate(HWND hdlg,int idcx,int idcy)
	{
	DoLocate(hdlg,idcx,idcy,"Mark Location:");
	}
	
// DoCureLocate : Same as DoMarkLocate except prompt
void DoCureLocate(HWND hdlg,int idcx,int idcy)
	{
	DoLocate(hdlg,idcx,idcy,"Cure Location:");
	}

/******************************************************************************
	fi=EditorIMPMapFontName(fontname)
	Maps a fontname to the fonts index of the EditorIMP, Adds the name to
	the IMPs fonts if it is not found.

	char *fontname = name of font to map
	int fi = IMP font index for fontname or -1 on error
******************************************************************************/
int EditorIMPMapFontName(char *fontname)
	{
	// If blank : error
	if(fontname[0]==0)
		{
		ShowError(IDE_BADFONT);
		return -1;
		}
	// Lock the fonts
	// Search for match in EditorIMP.fonts
	int fi=0;         //--------Linghai-11-4-08
	for(fi=0;fi<EditorIMP.fonts.count;fi++)
		if(!lstrcmp(fontname,EditorIMP.fonts.name[fi]))
			break;
	// If found in EditorIMP.fonts
	if( fi<EditorIMP.fonts.count)
		return fi;
	if(EditorIMP.fonts.count==IMPMAXFONTS) // If no room 
		{
		ShowError(IDE_MAXFONTS);
		return -1;
		}
	// Warn user if new font name not in selij
	if(SendMessage(IMWnd,IMM_FINDFONT,0,(LONG)(LPSTR)fontname)==-1)
		if(MessageBox(GetFocus(),
			"Font not found on selected inkjet!\n"
			"Continue and add the font?",
			IMSName,MB_ICONQUESTION|MB_YESNO)==IDNO)
			return -1;  // Allow for escape
	// Add to fonts
	fi=EditorIMP.fonts.count; // new index
	lstrcpy(EditorIMP.fonts.name[fi],fontname);
	EditorIMP.fonts.count++;
	return fi;
	}

/******************************************************************************
	ok = DoSubMarkEdit(parent,mark)

	Saves the current EditorMark
	then calls DoMarkEdit to run the appropriate mark editor dialog for
	the supplied mark. Restores EditorMark before returning.
	This function is indended to be called from the CDev and CBmp
	editors to edit bitmap sub marks.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	IMPMARK *mark = Mark to edit.
******************************************************************************/
BOOL DoSubMarkEdit(HWND parent,IMPMARK *mark)
	{
	IMPMARK SavedMark=EditorMark;
	BOOL ok=DoMarkEdit(parent,mark,TRUE);
	EditorMark=SavedMark;
	return ok;
	}

/******************************************************************************
	InitNewMark(mark)

	Instanciates the marks type based on the idm and then initializes
	the mark.  This function sets the marks type.

	IMPMARK *mark = mark to initialize.
	WORD idm = idm identifying new mark type (like IDM_NEWWINFONTMARK)
******************************************************************************/
void InitNewMark(IMPMARK *mark,WORD idm)
	{
	// Common initializers
	mark->origin.x=0.0;
	mark->origin.y=0.0;
	mark->rotation=0.0;
	mark->deg.t=0.0;
	mark->slant=0.0;
	mark->zflight=0.0;
	mark->justx=mjxLEFT;
	mark->justy=mjyTOP;
	mark->rep=0;
	switch(idm)
		{
		case IDM_NEWTEXTMARK: // Create default text mark
			mark->type=mtTEXT;
			mark->t.ijfmt.font=-1; // No initial font
			mark->t.ijfmt.ms=1;
			mark->t.ijfmt.xflip=0;
			mark->t.ijfmt.yflip=0;
			mark->t.cw=0.1;
			mark->t.ch=0.15;
			mark->t.text[0]=0;
			break;
		case IDM_NEWDEVBCMARK: // Create DEVBC mark
			DEVBCInit(mark);
			break;
		case IDM_NEWC128MARK: // Create C128 mark
			C128Init(mark);
			break;
		case IDM_NEWCUPCEANMARK: // Create CUPCEAN mark
			CUPCEANInit(mark);
			break;
		case IDM_NEWC2OF5MARK: // Create C2OF5 mark
			C2OF5Init(mark);
			break;
		case IDM_NEWC39MARK: // Create C39 mark
			C39Init(mark);
			break;
		case IDM_NEWC93MARK: // Create C93 mark
			C93Init(mark);
			break;
		case IDM_NEWBITMAPMARK: // Create default bitmap mark
			mark->type=mtBITMAP;
			mark->b.xres=100.0;
			mark->b.yres=100.0;
			mark->b.ms=1;
			mark->b.neg=FALSE;
			mark->b.fn[0]=0;
			break;
		case IDM_NEWMULTILINEMARK: // Create default multiline mark
			mark->type=mtMULTILINE;
			mark->ml.ijfmt.font=-1; // No initial font
			mark->ml.ijfmt.ms=1;
			mark->ml.ijfmt.xflip=0;
			mark->ml.ijfmt.yflip=0;
			mark->ml.cw=0.1;
			mark->ml.ch=0.15;
			mark->ml.text[0]=0;
			break;
		case IDM_NEWCDMMARK: // Create Data Matrix mark
			CDMInit(mark);
			break;
		case IDM_NEWCQRMARK: // Create QR Code mark
			CQRInit(mark);
			break;
		case IDM_NEWUIDMARK:                                    //-----------Linghai-4-9-09--UID
			UIDInit(mark);
			break;
			/*
		case IDM_NEWFORMAT12: //Create UID format12    //-----------------------Linghai-11-21-08-begin-newfomat--
			CDMInit(mark,1);
			break;
		case IDM_NEWFORMAT6:  //Create UID format6
			CDMInit(mark,2);
			break;
		case IDM_NEWFORMAT5:  //Create UID format5
			CDMInit(mark,3);
			break;                             //----------------------Linghai-11-21-08-end-newformat--
			*/
		case IDM_NEWWINFONTMARK: // Create default WinFont mark
			WINFONTInit(mark);
			break;
		case IDM_NEWPATCHMARK:
			PATCHInit(mark);
			break;
		case IDM_NEWWMTRXMARK:
			WMTRXInit(mark);
			break;
		case IDM_NEWCURESTEP: // Create default CureStep
			CURESTEPInit(mark);
			break;
		case IDM_NEWCDEVMARK: // Create default CDev mark
			CDEVInit(mark);
			break;
		case IDM_NEWDEVSYMMARK:
			mark->type=mtDEVSYM;
			mark->devsym.ijfmt.font=-1; // No initial font
			mark->devsym.ijfmt.ms=1;
			mark->devsym.ijfmt.xflip=0;
			mark->devsym.ijfmt.yflip=0;
			mark->devsym.width=0.3f;
			mark->devsym.height=0.3f;
			mark->devsym.text[0]=0;
			break;
		}
	}

/******************************************************************************
	FrePreVals()

	Frees the memory associated with the preview vals lists
	EditorPreIVals and EditorPreGVals.
******************************************************************************/
void FreePreVals()
	{
	/*
	if(EditorPreIVals)
		{
		delete [] EditorPreIVals;
		EditorPreIVals=NULL;
		}
	if(EditorPreGVals)
		{
		delete [] EditorPreGVals;
		EditorPreGVals=NULL;
		}
	*/
	}

/******************************************************************************
	BuildPreVals(pimp)

	Builds the edit preview value list EditPreIVals and EditPreGVals from
	the information in the supplied imp.

	PIMP = the imp supplying the preview var/val info.
******************************************************************************/
BOOL BuildPreVals(PIMP pimp)
	{
	//FreePreVals();

//	int isize=sizeof(IMPVALS)+pimp->vars.count*sizeof(IMPVAL);
	EditorPreIVals=IMPVALS(pimp->vars.count);
	if(!EditorPreIVals.val && pimp->vars.count>0)
		{
		ShowError(IDE_NOMEM);
		return FALSE;
		}
	EditorPreIVals.count=pimp->vars.count;
	if(!IMPInitVals(&EditorPreIVals,&pimp->vars))
		return FALSE;

//	int gsize=sizeof(IMPVALS)+pimp->gvars.count*sizeof(IMPVAL);
	EditorPreGVals=IMPVALS(pimp->gvars.count);
	if(!EditorPreGVals.val && pimp->gvars.count>0)
		{
		ShowError(IDE_NOMEM);
		return FALSE;
		}
	EditorPreGVals.count=pimp->gvars.count;
	if(!IMPInitVals(&EditorPreGVals,&pimp->gvars))
		return FALSE;

	// Copy each vars preview value to EditorPre vals
	for(int i=0;i<EditorPreIVals.count;i++)
		EditorPreIVals.val[i]=pimp->vars.var[i].preval;
	for(int i=0;i<EditorPreGVals.count;i++)
		EditorPreGVals.val[i]=pimp->gvars.var[i].preval;

	return TRUE;
	}

/***************************************************************************
	NON API EXPORT FUNCTIONS
***************************************************************************/
/***************************************************************************
	EDITORWP

	Editor Window Process.
***************************************************************************/
LRESULT WINAPI EDITORWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_CREATE:
			EditorWM_CREATE(hwnd);
			return 0;

		case WM_SIZE:
			{
			WORD w,h,w23,h2;
			w=LOWORD(lP);
			h=HIWORD(lP);
			w23=2*w/3;
			//w23=w/3;
			h2=h/2;

			MoveWindow(HWnd,0,0,w23,h2,TRUE);
			MoveWindow(VWnd,w23,0,w-w23,h2,TRUE);
			//MoveWindow(PWnd,w23+w23,0,w23,h2,TRUE);               //-----------------Linghai-11-19-08---
			MoveWindow(MWnd,0,h2,w,h-h2,TRUE);
			//MoveWindow(MWnd,0,h2,w23,h-h2,TRUE);                    //-----------------Linghai-11-19-08---
			

			return 0;
			}

		case WM_SETFOCUS:
			{
			SetFocus(FocusID ? GetDlgItem(hwnd,FocusID) : IMWnd);
			return 0;
			}

		case WM_COMMAND:
			{
				
			EditorWM_COMMAND(hwnd,wP,lP);
			return 0;
			}

		case WM_INITMENUPOPUP:
			EnableMenuItem((HMENU)wP,IDM_DELETEAE,
				MF_BYCOMMAND|(EditorIMP.aesetup?MF_ENABLED:MF_GRAYED));
			{
			int sel=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
			BOOL focus=FocusID==IDC_M;
			BOOL mselected=focus&&EditorIMP.marks.count;
			BOOL canpaste=focus && ClipboardMarkValid;
			EnableMenuItem((HMENU)wP,IDM_DELETE,MF_BYCOMMAND|(mselected?MF_ENABLED:MF_GRAYED));
			EnableMenuItem((HMENU)wP,IDM_CUT,MF_BYCOMMAND|(mselected?MF_ENABLED:MF_GRAYED));
			EnableMenuItem((HMENU)wP,IDM_COPY,MF_BYCOMMAND|(mselected?MF_ENABLED:MF_GRAYED));
			EnableMenuItem((HMENU)wP,IDM_PASTE,MF_BYCOMMAND|(canpaste?MF_ENABLED:MF_GRAYED));
			EnableMenuItem((HMENU)wP,IDM_PASTEAFTER,MF_BYCOMMAND|(canpaste?MF_ENABLED:MF_GRAYED));
			}
			return 0;

		case WM_QUERYENDSESSION:
			{
			char bfn[MAX_PATH];
			char * pbfn=NULL;
			bfn[0]=0;

			if(EditorIMP.changed)
				{
				if (EditorIMP.fn[0]!=0){
					GetFileTitle(EditorIMP.fn,bfn,MAX_PATH);
					pbfn = &bfn[0];
				}
				//bfn=GetOFSBaseName(&EditorIMP.ofs);
				wsprintf(tbuf,"Save changes to - %s",
					pbfn ? pbfn : (LPSTR)"(untitled)");
				switch(MessageBox(hwnd,tbuf,IMSName,
					MB_YESNOCANCEL|MB_ICONEXCLAMATION))
					{
					case IDYES:
						SendMessage(hwnd,WM_COMMAND,IDM_SAVE,0);
						if(EditorIMP.changed) // if save failed
							return 0;         // don't end session
						break;
					case IDNO:
						break;
					case IDCANCEL:
						return 0; // don't end session
					}
				}
			return 1; // ok to end session
			}

		case WM_DESTROY:
			SendMessage(IMWnd,IMM_CONFIG,0,0);
			SendMessage(IMWnd,IMM_LOCKIJ,0,0);
			return 1;
		}
	return DefWindowProc(hwnd,message,wP,lP);
	}

/***************************************************************************
	KVAREDITDP

	Keyed variable edit dialog process. Allows user to edit the variable
	in editvar.
***************************************************************************/
BOOL CALLBACK KVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	KEDITINFO ki;
	SEDITINFO si;
	HWND hctrl;
	DWORD width;

	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			// NAME
			hctrl=GetDlgItem(hdlg,IDC_NAME);
			ki.xextent=0;
			ki.yextent=0;
			ki.length=IMPLVNAME;
			ki.digits=1;
			ki.lowers=1;
			ki.uppers=1;
			ki.spaces=0;
			ki.others=0;
			KeditSetInfo(hctrl,&ki);
			SendMessage(hctrl,WM_SETTEXT,0,(LONG)(LPSTR)editvar.name);
			// WIDTH
			hctrl=GetDlgItem(hdlg,IDC_WIDTH);
/*zzz 04/14/04 - changed si.xextent (original value is 0) to resize IDC_WIDTH*/	
			si.xextent=30;
			si.yextent=0;
			si.minval=1;
			si.maxval=IMPLKVVAL;
			si.step=1;
			SeditSetInfo(hctrl,&si);
			SeditSetVal(hctrl,editvar.width);

			//DIGITS
			CheckDlgButton(hdlg,IDC_DIGITS,editvar.k.digits);
			//LOWERS
			CheckDlgButton(hdlg,IDC_LOWERS,editvar.k.lowers);
			//SPACE
			CheckDlgButton(hdlg,IDC_SPACE,editvar.k.space);
			//UPPERS
			CheckDlgButton(hdlg,IDC_UPPERS,editvar.k.uppers);
			//OTHERS
			CheckDlgButton(hdlg,IDC_OTHERS,editvar.k.others);

			// Global
			CheckDlgButton(hdlg,IDC_GLOBAL,editisgvar);

			// Unique
			CheckDlgButton(hdlg,IDC_UNIQUE,editvar.unique);

			return TRUE;
			}

		case WM_COMMAND:
			switch(wP)
				{
				case IDOK:
					// Check for invalid name
					GetDlgItemText(hdlg,IDC_NAME,tbuf,IMPLVNAME+1);
					if(!tbuf[0])
						{
						ShowError(IDE_BADVNAME);
						SetFocus(GetDlgItem(hdlg,IDC_NAME));
						return TRUE;      
						}
					// Check for invalid width
					hctrl=GetDlgItem(hdlg,IDC_WIDTH);
					if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&width)))
						{
						ShowError(IDE_BADVWIDTH);
						SetFocus(hctrl);
						return TRUE;
						}
					// Update editvar
					// NAME
					lstrcpy(editvar.name,tbuf);
					// WIDTH
					editvar.width=width;
					// DIGITS
					editvar.k.digits=IsDlgButtonChecked(hdlg,IDC_DIGITS);
					// LOWERS
					editvar.k.lowers=IsDlgButtonChecked(hdlg,IDC_LOWERS);
					// SPACE
					editvar.k.space=IsDlgButtonChecked(hdlg,IDC_SPACE);
					// UPPERS
					editvar.k.uppers=IsDlgButtonChecked(hdlg,IDC_UPPERS);
					// OTHERS
					editvar.k.others=IsDlgButtonChecked(hdlg,IDC_OTHERS);

					// Global
					editisgvar=IsDlgButtonChecked(hdlg,IDC_GLOBAL);

					// Unique
					editvar.unique=IsDlgButtonChecked(hdlg,IDC_UNIQUE);

					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
			break;
		}
	return FALSE;
	}

/***************************************************************************
	DATEVAREDITDP

	DATE Variable editor.
***************************************************************************/
BOOL CALLBACK DATEVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	HWND hctrl;
	switch(message)
		{
		case WM_INITDIALOG:
			{
			BOOL r;
			int d,h;
			long o;
			SetDlgItemText(hdlg,IDC_NAME,editvar.name);
			SendDlgItemMessage(hdlg,IDC_NAME,EM_LIMITTEXT,IMPLVNAME,0);
			SetDlgItemText(hdlg,IDC_FORMAT,editvar.d.format);
			SendDlgItemMessage(hdlg,IDC_FORMAT,EM_LIMITTEXT,IMPLDATEFORMAT,0);
			// Compute retro days and hours
			o=editvar.d.offset;
			r=o<0;
			if(r)
				o=-o;
			d=(int)(o / 86400L);
			h=(int)(o / 3600L % 24L);
			SetDlgItemInt(hdlg,IDC_DAYSOFFSET,d,FALSE);
			SendDlgItemMessage(hdlg,IDC_DAYSOFFSET,EM_LIMITTEXT,5,0);
			SetDlgItemInt(hdlg,IDC_HOURSOFFSET,h,FALSE);
			SendDlgItemMessage(hdlg,IDC_HOURSOFFSET,EM_LIMITTEXT,2,0);
			CheckDlgButton(hdlg,IDC_RETRO,r?1:0);
			CheckDlgButton(hdlg,IDC_CANOVERRIDE,editvar.d.canOverride?1:0);
			CheckDlgButton(hdlg,IDC_GLOBAL,1);
			// BASE
			hctrl=GetDlgItem(hdlg,IDC_BASE);
			IMPAddBasesToCB(hctrl,10,10);
			if(SendMessage(hctrl,CB_SELECTSTRING,-1,
				(LPARAM)(LPSTR)editvar.d.base)<0)
				SendMessage(hctrl,CB_SETCURSEL,0,0);

			return FALSE;
			}

		case WM_COMMAND:
			switch(wP)
				{
				case IDOK:
					{
					int d,h,i;
					BOOL r,xlated;
					// IDC_NAME
					GetDlgItemText(hdlg,IDC_NAME,tbuf,IMPLVNAME+1);
					if(!tbuf[0])
						{
						ShowError(IDE_BADVNAME);
						SetFocus(GetDlgItem(hdlg,IDC_NAME));
						return TRUE;
						}
					lstrcpy(editvar.name,tbuf);

					// IDC_FORMAT
					GetDlgItemText(hdlg,IDC_FORMAT,tbuf,IMPLDATEFORMAT+1);
					lstrcpy(editvar.d.format,tbuf);

					// IDC_DAYSOFFSET
					d=(int)GetDlgItemInt(hdlg,IDC_DAYSOFFSET,&xlated,0);
					if(!xlated)
						{
						ShowError(IDE_BADDAYSOFFSET);
						SetFocus(GetDlgItem(hdlg,IDC_HOURSOFFSET));
						return TRUE;
						}
					// IDC_HOURSOFFSET
					h=(int)GetDlgItemInt(hdlg,IDC_HOURSOFFSET,&xlated,0);
					if(!xlated)
						{
						ShowError(IDE_BADHOURSOFFSET);
						SetFocus(GetDlgItem(hdlg,IDC_HOURSOFFSET));
						return TRUE;
						}
					// IDC_RETRO
					r=IsDlgButtonChecked(hdlg,IDC_RETRO);
					editvar.d.offset=(r ? -1L : 1L)*(d*86400L+h*3600L);

					// IDC_CANOVERRIDE
					editvar.d.canOverride=IsDlgButtonChecked(hdlg,IDC_CANOVERRIDE);

					// BASE
					hctrl=GetDlgItem(hdlg,IDC_BASE);
					i=SendMessage(hctrl,CB_GETCURSEL,0,0);
					SendMessage(hctrl,CB_GETLBTEXT,i,
						(LPARAM)(LPSTR)editvar.d.base);

					// End Dialog
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
					}
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				case IDC_HINTS:
					MessageBox(hdlg,
						"Date format replacements are as follows:\n"
						"    Y     = 1 digit year\n"
						"    YY    = 2 digit year\n"
						"    YYYY  = 4 digit year\n"
						"    M     = 1|2 digit month\n"
						"    MM    = 2 digit month\n"
						"    Mmm   = 3 letter month\n"
						"    MMM   = Mmm all caps\n"
						"    Mmmm  = The month as a word\n"
						"    MMMM  = Mmmm all caps\n"
						"    D     = 1|2 digit day of month\n"
						"    DD    = 2 digit day of month\n"
						"    Ddd   = 3 letter day of week\n"
						"    DDD   = Ddd all caps\n"
						"    Dddd  = The day of the week as a word\n"
						"    DDDD  = Dddd all caps\n"
						"    W     = 1|2 digit week of year\n"
						"    WW    = 2 digit week of year\n"
						"    J     = 1..3 letter julian day of year\n"
						"    JJJ   = 3 letter julian day of year\n"
						"    PPPPPP= Packed MMDDYY 0..9 = A..Z\n"
						"All other characters are passed through.",
						"Date Format Hints",MB_OK|MB_ICONINFORMATION);
					return TRUE;
				}

		}
	return FALSE;
	}

/***************************************************************************
	TIMEVAREDITDP

	TIME Variable editor.
***************************************************************************/
BOOL CALLBACK TIMEVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	long o;
	BOOL r;
	int h,m,s,i;
	BOOL xlated;
	HWND hctrl;

	switch(message)
		{
		case WM_INITDIALOG:
			// Limit text lengths in all edit controls
			SendDlgItemMessage(hdlg,IDC_NAME,EM_LIMITTEXT,IMPLVNAME,0);
			SendDlgItemMessage(hdlg,IDC_HOURSOFFSET,EM_LIMITTEXT,2,0);
			SendDlgItemMessage(hdlg,IDC_MINUTESOFFSET,EM_LIMITTEXT,2,0);
			SendDlgItemMessage(hdlg,IDC_SECONDSOFFSET,EM_LIMITTEXT,2,0);
			SendDlgItemMessage(hdlg,IDC_SEPERATOR,EM_LIMITTEXT,1,0);
			SendDlgItemMessage(hdlg,IDC_AMSUFFIX,EM_LIMITTEXT,
				IMPLTIMESUFFIX,0);
			SendDlgItemMessage(hdlg,IDC_PMSUFFIX,EM_LIMITTEXT,
				IMPLTIMESUFFIX,0);
			// Init
			SetDlgItemText(hdlg,IDC_NAME,editvar.name);
			CheckDlgButton(hdlg,IDC_USENAMES,editvar.t.useNames ? 1 : 0);
			CheckDlgButton(hdlg,IDC_GLOBAL,1);
			// Compute retro h,m,s
			o=editvar.t.offset;
			r=o<0;
			if(r)
				o=-o;
			h=(int)(o / 3600L % 24L);
			m=(int)(o / 60 % 60);
			s=(int)(o % 60);
			SetDlgItemInt(hdlg,IDC_HOURSOFFSET  ,h,FALSE);
			SetDlgItemInt(hdlg,IDC_MINUTESOFFSET,m,FALSE);
			SetDlgItemInt(hdlg,IDC_SECONDSOFFSET,s,FALSE);
			CheckDlgButton(hdlg,IDC_RETRO,r?1:0);
			CheckDlgButton(hdlg,IDC_CANOVERRIDE,editvar.t.canOverride?1:0);
			if(editvar.t.useNames)
				{ // Disable format controls
				EnableWindow(GetDlgItem(hdlg,IDC_IS24),0);
				EnableWindow(GetDlgItem(hdlg,IDC_ISLZ),0);
				EnableWindow(GetDlgItem(hdlg,IDC_SEPERATOR),0);
				EnableWindow(GetDlgItem(hdlg,IDC_AMSUFFIX),0);
				EnableWindow(GetDlgItem(hdlg,IDC_PMSUFFIX),0);
				EnableWindow(GetDlgItem(hdlg,IDC_SHOWSECONDS),0);
				}
			else // Init format controls
				{
				CheckDlgButton(hdlg,IDC_IS24,editvar.t.is24?1:0);
				CheckDlgButton(hdlg,IDC_ISLZ,editvar.t.isLZ?1:0);
				if(editvar.t.seperator)
					{
					wsprintf(tbuf,"%c",editvar.t.seperator);
					SetDlgItemText(hdlg,IDC_SEPERATOR,tbuf);
					}
				if(editvar.t.is24){
					EnableWindow(GetDlgItem(hdlg,IDC_AMSUFFIX),0);
					EnableWindow(GetDlgItem(hdlg,IDC_PMSUFFIX),0);
				}
				else
					SetDlgItemText(hdlg,IDC_AMSUFFIX,editvar.t.amSuffix);
				SetDlgItemText(hdlg,IDC_PMSUFFIX,editvar.t.pmSuffix);
				CheckDlgButton(hdlg,IDC_SHOWSECONDS,editvar.t.showSeconds?1:0);
				}

			// BASE
			hctrl=GetDlgItem(hdlg,IDC_BASE);
			IMPAddBasesToCB(hctrl,10,10);
			if(SendMessage(hctrl,CB_SELECTSTRING,-1,
				(LPARAM)(LPSTR)editvar.t.base)<0)
				SendMessage(hctrl,CB_SETCURSEL,0,0);

			return FALSE;

		case WM_COMMAND:  // Win32 changed
			switch(LOWORD(wP))
				{
				case IDOK: // copy and validate all values
					// IDC_NAME
					GetDlgItemText(hdlg,IDC_NAME,tbuf,IMPLVNAME+1);
					if(!tbuf[0])
						{
						ShowError(IDE_BADVNAME);
						SetFocus(GetDlgItem(hdlg,IDC_NAME));
						return TRUE;
						}
					lstrcpy(editvar.name,tbuf);

					// IDC_HOURSOFFSET
					h=(int)GetDlgItemInt(hdlg,IDC_HOURSOFFSET,&xlated,0);
					if(!xlated)
						{
						ShowError(IDE_BADHOURSOFFSET);
						SetFocus(GetDlgItem(hdlg,IDC_HOURSOFFSET));
						return TRUE;
						}

					// IDC_MINUTESOFFSET
					m=(int)GetDlgItemInt(hdlg,IDC_MINUTESOFFSET,&xlated,0);
					if(!xlated)
						{
						ShowError(IDE_BADMINUTESOFFSET);
						SetFocus(GetDlgItem(hdlg,IDC_MINUTESOFFSET));
						return TRUE;
						}

					// IDC_SECONDSOFFSET
					s=(int)GetDlgItemInt(hdlg,IDC_SECONDSOFFSET,&xlated,0);
					if(!xlated)
						{
						ShowError(IDE_BADSECONDSOFFSET);
						SetFocus(GetDlgItem(hdlg,IDC_SECONDSOFFSET));
						return TRUE;
						}
					// IDC_RETRO
					r=IsDlgButtonChecked(hdlg,IDC_RETRO);
					editvar.t.offset=(r ? -1L : 1L)*(h*3600L+m*60+s);


					editvar.t.useNames=IsDlgButtonChecked(hdlg,IDC_USENAMES);

					if(!editvar.t.useNames)
						{
						editvar.t.is24=IsDlgButtonChecked(hdlg,IDC_IS24);
						editvar.t.isLZ=IsDlgButtonChecked(hdlg,IDC_ISLZ);
						GetDlgItemText(hdlg,IDC_SEPERATOR,tbuf,2);
						editvar.t.seperator=tbuf[0];
						if(!editvar.t.is24)
							GetDlgItemText(hdlg,IDC_AMSUFFIX,
								editvar.t.amSuffix,IMPLTIMESUFFIX+1);
						else
							editvar.t.amSuffix[0]=0;
						GetDlgItemText(hdlg,IDC_PMSUFFIX,
								editvar.t.pmSuffix,IMPLTIMESUFFIX+1);
						editvar.t.showSeconds=IsDlgButtonChecked(hdlg,IDC_SHOWSECONDS);
						}

					// IDC_CANOVERRIDE
					editvar.t.canOverride=IsDlgButtonChecked(hdlg,IDC_CANOVERRIDE);
					// BASE
					hctrl=GetDlgItem(hdlg,IDC_BASE);
					i=SendMessage(hctrl,CB_GETCURSEL,0,0);
					SendMessage(hctrl,CB_GETLBTEXT,i,
						(LPARAM)(LPSTR)editvar.t.base);

					// End Dialog
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;

				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;

				case IDC_USENAMES:
					if(IsDlgButtonChecked(hdlg,IDC_USENAMES))
						{ // if Checked : disable format controls
						EnableWindow(GetDlgItem(hdlg,IDC_IS24),0);
						EnableWindow(GetDlgItem(hdlg,IDC_ISLZ),0);
						EnableWindow(GetDlgItem(hdlg,IDC_SEPERATOR),0);
						EnableWindow(GetDlgItem(hdlg,IDC_AMSUFFIX),0);
						EnableWindow(GetDlgItem(hdlg,IDC_PMSUFFIX),0);
						EnableWindow(GetDlgItem(hdlg,IDC_SHOWSECONDS),0);
						}
					else
						{ // Enable and initialize format controls
						EnableWindow(GetDlgItem(hdlg,IDC_IS24),1);
						EnableWindow(GetDlgItem(hdlg,IDC_ISLZ),1);
						EnableWindow(GetDlgItem(hdlg,IDC_SEPERATOR),1);
						EnableWindow(GetDlgItem(hdlg,IDC_AMSUFFIX),1);
						EnableWindow(GetDlgItem(hdlg,IDC_PMSUFFIX),1);
						EnableWindow(GetDlgItem(hdlg,IDC_SHOWSECONDS),1);
						CheckDlgButton(hdlg,IDC_IS24,0);
						CheckDlgButton(hdlg,IDC_ISLZ,0);
						SetDlgItemText(hdlg,IDC_SEPERATOR,":");
						SetDlgItemText(hdlg,IDC_AMSUFFIX,"AM");
						SetDlgItemText(hdlg,IDC_PMSUFFIX,"PM");
						CheckDlgButton(hdlg,IDC_SHOWSECONDS,1);
						}
					return TRUE;

				case IDC_IS24:
					if(IsDlgButtonChecked(hdlg,IDC_IS24))
						{
						SetDlgItemText(hdlg,IDC_AMSUFFIX,"");
						EnableWindow(GetDlgItem(hdlg,IDC_AMSUFFIX),0);
						SetDlgItemText(hdlg,IDC_PMSUFFIX,"");
						EnableWindow(GetDlgItem(hdlg,IDC_PMSUFFIX),0);
						}
					else
						{
						EnableWindow(GetDlgItem(hdlg,IDC_AMSUFFIX),1);
						EnableWindow(GetDlgItem(hdlg,IDC_PMSUFFIX),1);
						SetDlgItemText(hdlg,IDC_AMSUFFIX,"AM");
						SetDlgItemText(hdlg,IDC_PMSUFFIX,"PM");
						}
				}
		}
	return FALSE;
	}

/***************************************************************************
	BASEDVAREDITDP

	BASED variable edit dialog process. Allows user to edit the variable
	in editvar.
***************************************************************************/
BOOL CALLBACK BASEDVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	KEDITINFO ki;
	SEDITINFO si;
	HWND hctrl;
	DWORD width;
	int i;

	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			// NAME
			hctrl=GetDlgItem(hdlg,IDC_NAME);
			ki.xextent=0;
			ki.yextent=0;
			ki.length=IMPLVNAME;
			ki.digits=1;
			ki.lowers=1;
			ki.uppers=1;
			ki.spaces=0;
			ki.others=0;
			KeditSetInfo(hctrl,&ki);
			SendMessage(hctrl,WM_SETTEXT,0,(LONG)(LPSTR)editvar.name);

			// WIDTH
			hctrl=GetDlgItem(hdlg,IDC_WIDTH);
			//si.xextent=0;
			si.xextent=30;
			si.yextent=0;
			si.minval=1;
			si.maxval=IMPLBASEDVAL;
			si.step=1;
			SeditSetInfo(hctrl,&si);
			SeditSetVal(hctrl,editvar.width);

			// BASE
			hctrl=GetDlgItem(hdlg,IDC_BASE);
			IMPAddBasesToCB(hctrl,0,0);
			if(SendMessage(hctrl,CB_SELECTSTRING,-1,
				(LPARAM)(LPSTR)editvar.b.base)<0)
				SendMessage(hctrl,CB_SETCURSEL,0,0);

			// LOOP
			CheckDlgButton(hdlg,IDC_LOOP,editvar.b.loop);


			// Global
			CheckDlgButton(hdlg,IDC_GLOBAL,editisgvar);

			// Unique
			CheckDlgButton(hdlg,IDC_UNIQUE,editvar.unique);

			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			switch(LOWORD(wP))
				{
				case IDOK:
					// Check for invalid name
					GetDlgItemText(hdlg,IDC_NAME,tbuf,IMPLVNAME+1);
					if(!tbuf[0])
						{
						ShowError(IDE_BADVNAME);
						SetFocus(GetDlgItem(hdlg,IDC_NAME));
						return TRUE;      
						}
					// Check for invalid width
					hctrl=GetDlgItem(hdlg,IDC_WIDTH);
					if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&width)))
						{
						ShowError(IDE_BADVWIDTH);
						SetFocus(hctrl);
						return TRUE;
						}
					// Update editvar
					// NAME
					lstrcpy(editvar.name,tbuf);
					// WIDTH
					editvar.width=width;

					// BASE
					hctrl=GetDlgItem(hdlg,IDC_BASE);
					i=SendMessage(hctrl,CB_GETCURSEL,0,0);
					SendMessage(hctrl,CB_GETLBTEXT,i,(LPARAM)(LPSTR)editvar.b.base);

					// LOOP
					editvar.b.loop=IsDlgButtonChecked(hdlg,IDC_LOOP);

					// Global
					editisgvar=IsDlgButtonChecked(hdlg,IDC_GLOBAL);

					// Unique
					editvar.unique=IsDlgButtonChecked(hdlg,IDC_UNIQUE);

					// End Dialog

					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
			break;
		}
	return FALSE;
	}

/***************************************************************************
	SVAREDITDP

	Serial number variable edit dialog process. Allows user to edit the
	variable in editvar.
***************************************************************************/
BOOL CALLBACK SVAREDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	KEDITINFO ki;
	SEDITINFO si;
	DWORD width,minval,maxval,step,repeats;

	switch(message)
		{
		case WM_INITDIALOG:
			// NAME
			ki.xextent=0;
			ki.yextent=0;
			ki.length=IMPLVNAME;
			ki.digits=1;
			ki.lowers=1;
			ki.uppers=1;
			ki.spaces=0;
			ki.others=0;
			KeditSetInfo(GetDlgItem(hdlg,IDC_NAME),&ki);
			SendDlgItemMessage(hdlg,IDC_NAME,WM_SETTEXT,
				 0,(LONG)(LPSTR)editvar.name);
			// WIDTH
			si.xextent=0;
			si.yextent=0;
			si.minval=1;
			si.maxval=IMPLSVVAL;
			si.step=1;
			SeditSetInfo(GetDlgItem(hdlg,IDC_WIDTH),&si);
			SeditSetVal(GetDlgItem(hdlg,IDC_WIDTH),editvar.width);
			// ZEROS
			if(editvar.lpad!='0') // only alow zero lpading
				editvar.lpad=0;
			CheckDlgButton(hdlg,IDC_ZEROS,editvar.lpad);
			// MIN
			si.xextent=0;
			si.yextent=0;
			si.minval=0;
			si.maxval=IMPMAXSVVAL;
			si.step=1;
			SeditSetInfo(GetDlgItem(hdlg,IDC_MIN),&si);
			SeditSetVal(GetDlgItem(hdlg,IDC_MIN),editvar.s.minval);
			// MAX
			si.xextent=0;
			si.yextent=0;
			si.minval=0;
			si.maxval=IMPMAXSVVAL;
			si.step=1;
			SeditSetInfo(GetDlgItem(hdlg,IDC_MAX),&si);
			SeditSetVal(GetDlgItem(hdlg,IDC_MAX),editvar.s.maxval);
			// LOOP
			CheckDlgButton(hdlg,IDC_LOOP,editvar.s.loop);
			// STEP
			si.xextent=0;
			si.yextent=0;
			si.minval=1;
			si.maxval=IMPMAXSVSTEP;
			si.step=1;
			SeditSetInfo(GetDlgItem(hdlg,IDC_STEP),&si);
			SeditSetVal(GetDlgItem(hdlg,IDC_STEP),editvar.s.step);
			// DOWN
			CheckDlgButton(hdlg,IDC_DOWN,!editvar.s.dirpos);
			// REPEATS
			si.xextent=0;
			si.yextent=0;
			si.minval=0;
			si.maxval=IMPMAXSVREPEAT;
			si.step=1;
			SeditSetInfo(GetDlgItem(hdlg,IDC_REPEATS),&si);
			SeditSetVal(GetDlgItem(hdlg,IDC_REPEATS),editvar.s.repeat);

			// Global
			CheckDlgButton(hdlg,IDC_GLOBAL,editisgvar);
			// Unique
			CheckDlgButton(hdlg,IDC_UNIQUE,editvar.unique);

			return TRUE;

		case WM_COMMAND:     // Command - Win32 changed
			switch(LOWORD(wP))
				{
				case IDOK:
					// Get sedit values for validation
					SeditGetVal(GetDlgItem(hdlg,IDC_WIDTH),&width);
					SeditGetVal(GetDlgItem(hdlg,IDC_MIN),&minval);
					SeditGetVal(GetDlgItem(hdlg,IDC_MAX),&maxval);
					SeditGetVal(GetDlgItem(hdlg,IDC_STEP),&step);
					SeditGetVal(GetDlgItem(hdlg,IDC_REPEATS),&repeats);
					// Invalid if width < maxval's length
					if(width<SeditNlength(maxval))
						{
						ShowError(IDE_VWIDTHTOOSMALL);
						SetFocus(GetDlgItem(hdlg,IDC_WIDTH));
						return TRUE;
						}
					// Invalid if minval not less than maxval
					if(maxval<=minval)
						{
						ShowError(IDE_VMAXTOOSMALL);
						SetFocus(GetDlgItem(hdlg,IDC_MAX));
						return TRUE;
						}
					// Check for invalid step
					if(!SeditValidate(GetDlgItem(hdlg,IDC_STEP)))
						{
						ShowError(IDE_VBADSTEP);
						SetFocus(GetDlgItem(hdlg,IDC_STEP));
						return TRUE;
						}
					// Invalid max is off step
					if((maxval-minval)%step!=0)
						{
						ShowError(IDE_VMAXOFFSTEP);
						SetFocus(GetDlgItem(hdlg,IDC_MAX));
						return TRUE;
						}
					// Invalid if name length is 0
					GetDlgItemText(hdlg,IDC_NAME,tbuf,IMPLVNAME+1);
					if(!tbuf[0])
						{
						ShowError(IDE_BADVNAME);
						SetFocus(GetDlgItem(hdlg,IDC_NAME));
						return TRUE;      
						}
					// Update editvar
					// NAME
					GetDlgItemText(hdlg,IDC_NAME,editvar.name,IMPLVNAME+1);
					// WIDTH
					editvar.width=width;
					// ZEROS
					if(IsDlgButtonChecked(hdlg,IDC_ZEROS))
						editvar.lpad='0';
					else
						editvar.lpad=0;
					// MIN
						editvar.s.minval=minval;
					// MAX
						editvar.s.maxval=maxval;
					// LOOP
					editvar.s.loop=IsDlgButtonChecked(hdlg,IDC_LOOP);
					// STEP
						editvar.s.step=step;
					// DOWN
					editvar.s.dirpos=!IsDlgButtonChecked(hdlg,IDC_DOWN);
					// REPEATS
						editvar.s.repeat=(WORD)repeats;

					// Global
					editisgvar=IsDlgButtonChecked(hdlg,IDC_GLOBAL);

					// Unique
					editvar.unique=IsDlgButtonChecked(hdlg,IDC_UNIQUE);

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
	TEXTMARKEDITDP

	mtTEXT editor dialog process.
***************************************************************************/
BOOL CALLBACK TEXTMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{

	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{


			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
//			IMPFONTS *pfonts;
			HWND hctrl;
			int i;

			// Initilize the controls
			EditXYModePrep(hdlg);

			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);
			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.y*EditYScale);
			// IDC_Z
			hctrl=GetDlgItem(hdlg,IDC_Z);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.zflight*EditZScale);

			// IDC_DEG---testing yaxis
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=CFPP;
				FeditSetInfo(hctrl,&finfo);	
				double passingdata =  EditorMark.deg.t; //sam
				FeditSetVal(hctrl,EditorMark.deg.t*EditYScale);


			// IDC_FONTS
			hctrl=GetDlgItem(hdlg,IDC_FONTS);
			// Limit edit text to maximum font name
			SendMessage(hctrl,CB_LIMITTEXT,sizeof(IJFONTNAME)-1,0);
			// Fill the listbox with the used and available fonts
			if(EditorIJFound)
				{  // Add all fonts from imp not in ij
				for(i=0;i<EditorIMP.fonts.count;i++)
					if(SendMessage(IMWnd,IMM_FINDFONT,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i])==-1)
						SendMessage(hctrl,CB_ADDSTRING,
							0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
				// Add fonts from ij
				SendMessage(IMWnd,IMM_CBADDFONTS,(WPARAM)hctrl,MMCBAF_TEXT);
				}
			else // inkjet not found : Add fonts from imp
				for(i=0;i<EditorIMP.fonts.count;i++)
					SendMessage(hctrl,CB_ADDSTRING,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
			// Select marks current font
			if(EditorMark.t.ijfmt.font==-1) // No current font
				SendMessage(hctrl,CB_SETCURSEL,-1,0); // No current sel
			else
				SelectCurrentFont(hctrl,(LPSTR)EditorIMP.fonts.name[EditorMark.t.ijfmt.font]);
//				SendMessage(hctrl,CB_SELECTSTRING,
//					-1,(LONG)(LPSTR)EditorIMP.fonts.name[EditorMark.t.ijfmt.font]);

			// IDC_ROT
			hctrl=GetDlgItem(hdlg,IDC_ROT);
			if(EditHasRot)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.rotation);
				}
			
			// IDC_SLANT
			hctrl=GetDlgItem(hdlg,IDC_SLANT);
			if(EditHasSlant)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.slant);
				}

			// IDC_JUST
			hctrl=GetDlgItem(hdlg,IDC_JUST);
			// Init it
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Left");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Center");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Right");
			// Select current just
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.justx,0);

			// IDC_WIDTH
			hctrl=GetDlgItem(hdlg,IDC_WIDTH);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=1;    // format 1.0000
			finfo.flen=4;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.t.cw*EditXScale);

			// IDC_HEIGHT
			hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=1;    // format 1.0000
			finfo.flen=4;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.t.ch*EditYScale);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.t.ijfmt.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.t.ijfmt.yflip ? 1: 0);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.t.ijfmt.xflip ? 1: 0);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLTEXT;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.t.text);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);

			return TRUE;
			}

		case WM_COMMAND: // Win32 changed
			{
			HWND hctrl;
			DWORD dw;
			int i;
//			IMPUDHAS *pudhas;
//			IMPVARS *pvars;
//			IMPVARS *pgvars;

			if(lP) // If from control
				{
				switch(LOWORD(wP))
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDOK:
						// Validate each control and retrieve its value

						// IDC_X
						hctrl=GetDlgItem(hdlg,IDC_X);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.x)))
							{
							ShowError(IDE_BADX);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.x/=EditXScale;
						// IDC_Y
						hctrl=GetDlgItem(hdlg,IDC_Y);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.y)))
							{
							ShowError(IDE_BADY);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.y/=EditYScale;
						// IDC_Z
						hctrl=GetDlgItem(hdlg,IDC_Z);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.zflight)||
							(EditorMark.zflight<0.0f)))
							{
							ShowError(IDE_BADZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.zflight/=EditZScale;

						// IDC_DEG
						/*	hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
						EditorMark.deg.t/=EditYScale;*/
						
						// IDC_FONTS
						hctrl=GetDlgItem(hdlg,IDC_FONTS);
						// Get the fontname
						SendMessage(hctrl,WM_GETTEXT,
							sizeof(IJFONTNAME),(LONG)(LPSTR)tbuf);
						// Trim leading and trailing spaces from the font name
						strtrimts(tbuf);
						strtrimls(tbuf);
						i=EditorIMPMapFontName(tbuf);
						if(i<0)
							{
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.t.ijfmt.font=i;

						// IDC_ROT
						if(EditHasRot)
							{
							hctrl=GetDlgItem(hdlg,IDC_ROT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.rotation)))
								{
								ShowError(IDE_BADROT);
								SetFocus(hctrl);
								return TRUE;
								}
							}

						
						// IDC_SLANT
						if(EditHasSlant)
							{
							hctrl=GetDlgItem(hdlg,IDC_SLANT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.slant)))
								{
								ShowError(IDE_BADSLANT);
								SetFocus(hctrl);
								return TRUE;
								}
							}

						// IDC_JUST
						hctrl=GetDlgItem(hdlg,IDC_JUST);
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_WIDTH
						hctrl=GetDlgItem(hdlg,IDC_WIDTH);
						if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&EditorMark.t.cw))||
							(!(EditorMark.t.cw>0.0)))
							{
							ShowError(IDE_BADCW);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.t.cw/=EditXScale;

						// IDC_HEIGHT
						hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.t.ch))||
							(!(EditorMark.t.ch>0.0)))
							{
							ShowError(IDE_BADCH);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.t.ch/=EditYScale;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.t.ijfmt.ms=(WORD)dw;

						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_YFLIP
						EditorMark.t.ijfmt.yflip=IsDlgButtonChecked(hdlg,IDC_YFLIP);

						// IDC_XFLIP
						EditorMark.t.ijfmt.xflip=IsDlgButtonChecked(hdlg,IDC_XFLIP);

						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						if((!KeditValidate(hctrl))||
							(!KeditGetVal(hctrl,(LPSTR)&(EditorMark.t.text)))||
							(!EditorMark.t.text[0]))
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}

						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;

					case IDC_WEXT:
						if(HIWORD(wP)==XN_INS)
							{
							hctrl=GetDlgItem(hdlg,IDC_WEXT);
							DoInsertsPopup(hdlg,hctrl,&EditorIMP.udhas,
								&EditorIMP.vars,&EditorIMP.gvars);
							}
						return TRUE;
					}
				}
			else // From text insert menu
				{
				hctrl=GetDlgItem(hdlg,IDC_WEXT);
				if(ProcessInsertsPopup(wP,hctrl))
					return TRUE;
				}
			break;
			}
		}
	return FALSE;
	}

/***************************************************************************
	DEVSYMMARKEDITDP

	mtDEVSYM editor dialog process.
***************************************************************************/
BOOL CALLBACK DEVSYMMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{

	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{


			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
//			IMPFONTS *pfonts;
			HWND hctrl;
			int i;

			// Initilize the controls
			EditXYModePrep(hdlg);

			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);
			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.y*EditYScale);
			// IDC_Z
			hctrl=GetDlgItem(hdlg,IDC_Z);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.zflight*EditZScale);
			// IDC_DEG
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=CFPP;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t*EditYScale);
			// IDC_FONTS
			hctrl=GetDlgItem(hdlg,IDC_FONTS);
			// Limit edit text to maximum font name
			SendMessage(hctrl,CB_LIMITTEXT,sizeof(IJFONTNAME)-1,0);
			// Fill the listbox with the used and available fonts
			if(EditorIJFound)
				{  // Add all fonts from imp not in ij
				for(i=0;i<EditorIMP.fonts.count;i++)
					if(SendMessage(IMWnd,IMM_FINDFONT,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i])==-1)
						SendMessage(hctrl,CB_ADDSTRING,
							0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
				// Add fonts from ij
				SendMessage(IMWnd,IMM_CBADDFONTS,(WPARAM)hctrl,MMCBAF_SYMBOL);
				}
			else // inkjet not found : Add fonts from imp
				for(i=0;i<EditorIMP.fonts.count;i++)
					SendMessage(hctrl,CB_ADDSTRING,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
			// Select marks current font
			if(EditorMark.devsym.ijfmt.font==-1) // No current font
				SendMessage(hctrl,CB_SETCURSEL,-1,0); // No current sel
			else
				SelectCurrentFont(hctrl,(LPSTR)EditorIMP.fonts.name[EditorMark.devsym.ijfmt.font]);
			//	SendMessage(hctrl,CB_SELECTSTRING,
			//		-1,(LONG)(LPSTR)EditorIMP.fonts.name[EditorMark.devsym.ijfmt.font]);

			// IDC_ROT
			hctrl=GetDlgItem(hdlg,IDC_ROT);
			if(EditHasRot)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.rotation);
				}
			
			// IDC_SLANT
			hctrl=GetDlgItem(hdlg,IDC_SLANT);
			if(EditHasSlant)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.slant);
				}

			// IDC_JUST
			hctrl=GetDlgItem(hdlg,IDC_JUST);
			// Init it
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Left");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Center");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Right");
			// Select current just
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.justx,0);

			// IDC_XRES
			hctrl=GetDlgItem(hdlg,IDC_WIDTH);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.999
			finfo.flen=3;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.devsym.width*EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.999
			finfo.flen=3;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.devsym.height*EditYScale);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=500;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.devsym.xdots);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_YFLIP
	//		CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.devsym.ijfmt.yflip ? 1: 0);

			// IDC_XFLIP
	//		CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.devsym.ijfmt.xflip ? 1: 0);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLDEVSYM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.devsym.text);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);

			return TRUE;
			}

		case WM_COMMAND: // Win32 changed
			{
			HWND hctrl;
			DWORD dw;
			int i;
//			IMPUDHAS *pudhas;
//			IMPVARS *pvars;
//			IMPVARS *pgvars;

			if(lP) // If from control
				{
				switch(LOWORD(wP))
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDOK:
						// Validate each control and retrieve its value

						// IDC_X
						hctrl=GetDlgItem(hdlg,IDC_X);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.x)))
							{
							ShowError(IDE_BADX);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.x/=EditXScale;
						// IDC_Y
						hctrl=GetDlgItem(hdlg,IDC_Y);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.y)))
							{
							ShowError(IDE_BADY);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.y/=EditYScale;
						// IDC_Z
						hctrl=GetDlgItem(hdlg,IDC_Z);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.zflight)||
							(EditorMark.zflight<0.0f)))
							{
							ShowError(IDE_BADZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.zflight/=EditZScale;

						// IDC_DEG
						/*	hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
							EditorMark.deg.t/=EditYScale;
						*/
						// IDC_FONTS
						hctrl=GetDlgItem(hdlg,IDC_FONTS);
						// Get the fontname
						SendMessage(hctrl,WM_GETTEXT,
							sizeof(IJFONTNAME),(LONG)(LPSTR)tbuf);
						// Trim leading and trailing spaces from the font name
						strtrimts(tbuf);
						strtrimls(tbuf);
						i=EditorIMPMapFontName(tbuf);
						if(i<0)
							{
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devsym.ijfmt.font=i;

						// IDC_ROT
						if(EditHasRot)
							{
							hctrl=GetDlgItem(hdlg,IDC_ROT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.rotation)))
								{
								ShowError(IDE_BADROT);
								SetFocus(hctrl);
								return TRUE;
								}
							}

						
						// IDC_SLANT
						if(EditHasSlant)
							{
							hctrl=GetDlgItem(hdlg,IDC_SLANT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.slant)))
								{
								ShowError(IDE_BADSLANT);
								SetFocus(hctrl);
								return TRUE;
								}
							}

						// IDC_JUST
						hctrl=GetDlgItem(hdlg,IDC_JUST);
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_WIDTH
						hctrl=GetDlgItem(hdlg,IDC_WIDTH);
						if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&EditorMark.devsym.width))||
							(!(EditorMark.devsym.width>0.0)))
							{
							ShowError(IDE_BADWIDTH);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devsym.width/=EditXScale;

						// IDC_HEIGHT
						hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.devsym.height))||
							(!(EditorMark.devsym.height>0.0)))
							{
							ShowError(IDE_BADHEIGHT);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devsym.height/=EditYScale;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devsym.xdots=(WORD)dw;

						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_YFLIP
				//		EditorMark.devsym.ijfmt.yflip=IsDlgButtonChecked(hdlg,IDC_YFLIP);

						// IDC_XFLIP
				//		EditorMark.devsym.ijfmt.xflip=IsDlgButtonChecked(hdlg,IDC_XFLIP);

						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						if((!KeditValidate(hctrl))||
							(!KeditGetVal(hctrl,(LPSTR)&(EditorMark.devsym.text)))||
							(!EditorMark.devsym.text[0]))
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}

						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;

					case IDC_WEXT:
						if(HIWORD(wP)==XN_INS)
							{
							hctrl=GetDlgItem(hdlg,IDC_WEXT);
							DoInsertsPopup(hdlg,hctrl,&EditorIMP.udhas,
								&EditorIMP.vars,&EditorIMP.gvars);
							}
						return TRUE;
					}
				}
			else // From text insert menu
				{
				hctrl=GetDlgItem(hdlg,IDC_WEXT);
				if(ProcessInsertsPopup(wP,hctrl))
					return TRUE;
				}
			break;
			}
		}
	return FALSE;
	}

/***************************************************************************
	BITMAPMARKEDITDP

	mtBITMAP editor dialog process.
***************************************************************************/
BOOL CALLBACK BITMAPMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
		
	switch(message)
		{
	/*case ON_CBN_SELENDOK:
		{

			char path[256];                  //--------------------Linghai-11-13-08---begin idc-fn
							char name[256];
							strcpy(path,"C:\\Linghai\\LinghaiProject\\ims5000\\");
							int pathLength = strlen(path);
							int filename = 1;
							filename = GetDlgItemText(hdlg,IDC_FN,name,256);
							int nameLength = strlen(name);
							for(int i = 0; i < nameLength; i++)
							{
								path[pathLength+i] = name[i];
							}//end for loop to add name to path
							path[pathLength+nameLength] = '.';
							path[pathLength+nameLength+1] = 'b';
							path[pathLength+nameLength+2] = 'm';
							path[pathLength+nameLength+3] = 'p';
							path[pathLength+nameLength+4] = '\0';
							

							int openfile = 1;
							HBITMAP bitmap = (HBITMAP)LoadImage(NULL,path,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
							SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap);
                                                                   //----------------Linghai-11-13-08---end idc-fn
							return true;
		}//END CASE on-cbn-selendok
		*/

		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;

			// Initilize the controls
			EditXYModePrep(hdlg);
			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);
			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.y*EditYScale);
			// IDC_Z
			hctrl=GetDlgItem(hdlg,IDC_Z);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.zflight*EditZScale);
			// IDC_DEG
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=CFPP;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t*EditYScale);

			// IDC_ROT
			hctrl=GetDlgItem(hdlg,IDC_ROT);
			if(EditHasRot)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.rotation);
				}
			
			
			// IDC_SLANT
			hctrl=GetDlgItem(hdlg,IDC_SLANT);
			if(EditHasSlant)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.slant);
				}

			// IDC_JUST
			hctrl=GetDlgItem(hdlg,IDC_JUST);
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Left");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Center");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Right");
			// Select current just
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.justx,0);

			// IDC_XRES
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.b.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.b.yres/EditYScale);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.b.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_NEG
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.b.neg);

			// IDC_FN
			hctrl=GetDlgItem(hdlg,IDC_FN);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLBMFN;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.b.fn);

			char name2[256];
			int filename2 = 0;
			filename2 = GetDlgItemText(hdlg,IDC_FN,name2,256);
			int nameLength2 = strlen(name2);
			name2[nameLength2] = '\0';
							
			if(filename2 != 0)                                      //-------------------------Linghai-02-17-09------
			{
			HBITMAP bitmap1 = (HBITMAP)LoadImage(NULL,name2,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
			SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap1);
			}
			else
			{
				HBITMAP bitmap = (HBITMAP)LoadImage(NULL,"c:\\ims5000\\bmp\\logo.bmp",
				IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
				SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap);
			}

			/*char path[256];                  //--------------------Linghai-11-13-08---begin idc-fn
							char name[256];
							strcpy(path,"C:\\ims5000\\bmp\\");
							int pathLength = strlen(path);
							int filename = 1;
							filename = GetDlgItemText(hdlg,IDC_FN,name,256);
							int nameLength = strlen(name);
							for(int i = 0; i < nameLength; i++)
							{
								path[pathLength+i] = name[i];
							}//end for loop to add name to path
							path[pathLength+nameLength] = '\0';
							//path[pathLength+nameLength] = '.';
							//path[pathLength+nameLength+1] = 'b';
							//path[pathLength+nameLength+2] = 'm';
							//path[pathLength+nameLength+3] = 'p';
							//path[pathLength+nameLength+4] = '\0';
							

							int openfile = 1;
							HBITMAP bitmap = (HBITMAP)LoadImage(NULL,path,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
							SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap);
							*/
			

			// IDC_BITMAPFILE           //-----------------------Linghai-11-10-08-----BEGIN
			

			/*WIN32_FIND_DATA findfile;
			HANDLE firstfile;
			BOOL next;
			int length;
			firstfile = FindFirstFile("c:\\ims5000\\bmp\\*.bmp",&fData);
			if(firstfile != INVALID_HANDLE_VALUE)
				{
					do
					{
						length = lstrlen(fData.cFileName)-4;
						char fname[MAX_PATH];

						for(int i = 0; i < length; i++)
							fname[i] = fData.cFileName[i];
						fname[length] = '\0';
					
						SendDlgItemMessage(hdlg,IDC_FN,CB_ADDSTRING,100,(LPARAM)(LPSTR)fname);
						next = FindNextFile(firstfile,&fData);
					}while(next);
				}
				*/
				

			//SetWindowText(hctrl,EditorMark.b.fn);    //------------Linghai-11-10-08-----END IDC-BITMAPFILE

			//IDC_IMAGE                                 //-----------Linghai-11-13-08--begin idc-image
			/*HBITMAP bitmap = (HBITMAP)LoadImage(NULL,"c:\\ims5000\\bmp\\logo.bmp",
				IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
			SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap);
			                                            //-----------Linghai-11-13-08---end idc-image
														*/
														


			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);

			return TRUE;
			}

		case WM_COMMAND: // Win32 changed
			{
			HWND hctrl;
			DWORD dw;

			if(lP) // If from control
				{
				switch(LOWORD(wP))
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDC_SEARCHFILE:     //---Linghai-11-14-08---begin idc-searchfile
						{
							char dirname[MAX_PATH];
							//GetModuleFileName(NULL,dirname,sizeof(dirname));
							strcpy(dirname,"C:\\ims5000\\bmp\\");            //------------------------Linghai-2-9-2009------  

							char *p = strrchr(dirname,'\\');
							if (p) *p = 0;

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
								
								SetDlgItemText(hdlg,IDC_FN,ofn.lpstrFile);
								//SetDlgItemText(hdlg,IDC_FN,noExtName);
							}

							//char path1[256];                  //--------------------Linghai-11-13-08---begin idc-fn
							char name1[256];
							//strcpy(dirname,"C:\\ims5000\\bmp\\");
							//int pathLength1 = strlen(path1);
							int filename1 = 1;
							filename1 = GetDlgItemText(hdlg,IDC_FN,name1,256);
							int nameLength1 = strlen(name1);
							/*for(int i = 0; i < nameLength1; i++)
							{
								path1[pathLength1+i] = name1[i];
							}//end for loop to add name to path
							*/
							//name1[nameLength1] = '\0';
							name1[nameLength1] = '\0';
							//path[pathLength+nameLength] = '.';
							//path[pathLength+nameLength+1] = 'b';
							//path[pathLength+nameLength+2] = 'm';
							//path[pathLength+nameLength+3] = 'p';
							//path[pathLength+nameLength+4] = '\0';
							

							int openfile1 = 1;
							HBITMAP bitmap1 = (HBITMAP)LoadImage(NULL,name1,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
							SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap1);
							return TRUE;
						}                   //---Linghai-11-14-08---end idc-searchfile
					case IDC_BITMAPFILE:          //--------------Linghai-11-12-08--begin idc-bitmapfile
						{
							char path[256];
							char name[256];
							strcpy(path,"start mspaint ");
							int pathLength = strlen(path);
							int filename = 1;
							filename = GetDlgItemText(hdlg,IDC_FN,name,256);
							int nameLength = strlen(name);
							for(int i = 0; i < nameLength; i++)
							{
								path[pathLength+i] = name[i];
							}//end for loop to add name to path
							
							path[pathLength+nameLength] = '\0';
							//char tem[] = " -hideself on";             //----------------------Linghai-02-17-09------
							//strcat(path,tem);
							int openfile = 1;
							openfile = system(path);
							//note system("c:\\ims5000\\bmp\\logo.bmp");
							if(openfile == 0 && filename == 0)
							{
								return TRUE;  
							}else 
								return false;
								

						}                         //--------------Linghai-11-12-08-end idc-bitmapfile
						
					case IDOK:
						// Validate each control and retrieve its value
						// IDC_X
						hctrl=GetDlgItem(hdlg,IDC_X);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.x)))
							{
							ShowError(IDE_BADX);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.x/=EditXScale;
						// IDC_Y
						hctrl=GetDlgItem(hdlg,IDC_Y);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.y)))
							{
							ShowError(IDE_BADY);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.y/=EditYScale;
						// IDC_Z
						hctrl=GetDlgItem(hdlg,IDC_Z);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.zflight)||
							(EditorMark.zflight<0.0f)))
							{
							ShowError(IDE_BADZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.zflight/=EditZScale;

						// IDC_DEG
						/*	hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
						EditorMark.deg.t/=EditYScale;
						*/
						// IDC_ROT
						if(EditHasRot)
							{
							hctrl=GetDlgItem(hdlg,IDC_ROT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.rotation)))
								{
								ShowError(IDE_BADROT);
								SetFocus(hctrl);
								return TRUE;
								}
							}
						
						// IDC_SLANT
						if(EditHasSlant)
							{
							hctrl=GetDlgItem(hdlg,IDC_SLANT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.slant)))
								{
								ShowError(IDE_BADSLANT);
								SetFocus(hctrl);
								return TRUE;
								}
							}

						// IDC_JUST
						hctrl=GetDlgItem(hdlg,IDC_JUST);
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.b.xres))
							||(EditorMark.b.xres < 10.0)
							||(EditorMark.b.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.b.xres*=EditXScale;
						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.b.yres))
							||(EditorMark.b.yres < 10.0)
							||(EditorMark.b.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.b.yres*=EditYScale;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.b.ms=(WORD)dw;

						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_NEG
						EditorMark.b.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

						// IDC_FN
						hctrl=GetDlgItem(hdlg,IDC_FN);
						if((!KeditValidate(hctrl))||
							(!KeditGetVal(hctrl,(LPSTR)&(EditorMark.b.fn)))||
							(!EditorMark.b.fn[0]))
							{
							ShowError(IDE_BADBMFN);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;

					case IDC_FN:
						if(HIWORD(wP)==XN_INS)
							{
							hctrl=GetDlgItem(hdlg,IDC_FN);
							DoInsertsPopup(hdlg,hctrl,&EditorIMP.udhas,
								&EditorIMP.vars,&EditorIMP.gvars);
							}
						if(HIWORD(wP) == CBN_SELENDCANCEL)          //-------------Linghai-11-13-08---
						{
							char path[256];                  //--------------------Linghai-11-13-08---begin idc-fn
							char name[256];
							//strcpy(path,"C:\\ims5000\\bmp");
							//int pathLength = strlen(path);
							int filename = 1;
							filename = GetDlgItemText(hdlg,IDC_FN,name,256);
							int nameLength = strlen(name);
							for(int i = 0; i < nameLength; i++)
							{
								//path[pathLength+i] = name[i];
								path[i] = name[i];
							}//end for loop to add name to path             //------------------Linghai-02-17-09---------
							/*path[pathLength+nameLength] = '.';
							path[pathLength+nameLength+1] = 'b';
							path[pathLength+nameLength+2] = 'm';
							path[pathLength+nameLength+3] = 'p';
							path[pathLength+nameLength+4] = '\0'*/
							path[nameLength] = '\0';             
							

							int openfile = 1;
							HBITMAP bitmap = (HBITMAP)LoadImage(NULL,path,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
							SendDlgItemMessage(hdlg,IDC_IMAGE,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)bitmap);
                                                                   //----------------Linghai-11-13-08---end idc-fn
						}//end cbn-selchange                      //--------------Linghai-11-13-08---
						return TRUE;
					} 
				}
			else // From text insert menu
				{
				hctrl=GetDlgItem(hdlg,IDC_FN);
				if(ProcessInsertsPopup(wP,hctrl))
					return TRUE;
				}
			break;
			}
		}
	return FALSE;
	}

/***************************************************************************
	MULTILINEMARKEDITDP

	mtMULTILINE editor dialog process.
***************************************************************************/
BOOL CALLBACK MULTILINEMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
//			IMPFONTS *pfonts;
			HWND hctrl;
			int i;

			// Initilize the controls
			EditXYModePrep(hdlg);

			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);
			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.y*EditYScale);
			// IDC_Z
			hctrl=GetDlgItem(hdlg,IDC_Z);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.zflight*EditZScale);
			// IDC_DEG
			hctrl=GetDlgItem(hdlg,IDC_DEG);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0;
			finfo.ilen=3;
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.deg.t*EditYScale);
			

			// IDC_FONTS
			hctrl=GetDlgItem(hdlg,IDC_FONTS);
			// Limit edit text to maximum font name
			SendMessage(hctrl,CB_LIMITTEXT,sizeof(IJFONTNAME)-1,0);
			// Fill the listbox with the used and available fonts
			if(EditorIJFound)
				{  // Add all fonts from imp not in ij
				for(i=0;i<EditorIMP.fonts.count;i++)
					if(SendMessage(IMWnd,IMM_FINDFONT,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i])==-1)
						SendMessage(hctrl,CB_ADDSTRING,
							0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
				// Add fonts from ij
				SendMessage(IMWnd,IMM_CBADDFONTS,(WPARAM)hctrl,MMCBAF_MULTILINE);
				}
			else // inkjet not found : Add fonts from imp
				for(i=0;i<EditorIMP.fonts.count;i++)
					SendMessage(hctrl,CB_ADDSTRING,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
			// Select marks current font
			if(EditorMark.ml.ijfmt.font==-1) // No current font
				SendMessage(hctrl,CB_SETCURSEL,-1,0); // No current sel
			else
				SelectCurrentFont(hctrl,(LPSTR)EditorIMP.fonts.name[EditorMark.ml.ijfmt.font]);
				//SendMessage(hctrl,CB_SELECTSTRING,-1,
				//	(LONG)(LPSTR)EditorIMP.fonts.name[EditorMark.ml.ijfmt.font]);

			// IDC_ROT
			hctrl=GetDlgItem(hdlg,IDC_ROT);
			if(EditHasRot)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.rotation);
				}
			// IDC_SLANT
			hctrl=GetDlgItem(hdlg,IDC_SLANT);
			if(EditHasSlant)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.slant);
				}

			// IDC_JUST
			hctrl=GetDlgItem(hdlg,IDC_JUST);
			// Init it
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Left");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Center");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Right");
			// Select current just
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.justx,0);

			// IDC_WIDTH
			hctrl=GetDlgItem(hdlg,IDC_WIDTH);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=1;    // format 1.0000
			finfo.flen=4;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.ml.cw*EditXScale);

			// IDC_HEIGHT
			hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=1;    // format 1.0000
			finfo.flen=4;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.ml.ch*EditYScale);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.ml.ijfmt.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.ml.ijfmt.yflip ? 1: 0);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.ml.ijfmt.xflip ? 1: 0);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLMLTEXT;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.ml.text);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);

			return TRUE;
			}

		case WM_COMMAND: // Win32 changed
			{
			HWND hctrl;
			DWORD dw;
			int i;
//			IMPUDHAS *pudhas;
//			IMPVARS *pvars;
//			IMPVARS *pgvars;

			if(lP) // If from control
				{
				switch(LOWORD(wP))
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDOK:
						// Validate each control and retrieve its value
						// IDC_X
						hctrl=GetDlgItem(hdlg,IDC_X);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.x)))
							{
							ShowError(IDE_BADX);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.x/=EditXScale;
						// IDC_Y
						hctrl=GetDlgItem(hdlg,IDC_Y);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.origin.y)))
							{
							ShowError(IDE_BADY);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.origin.y/=EditYScale;
						// IDC_Z
						hctrl=GetDlgItem(hdlg,IDC_Z);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.zflight)||
							(EditorMark.zflight<0.0f)))
							{
							ShowError(IDE_BADZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.zflight/=EditZScale;

						// IDC_DEG
						/*	hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
						EditorMark.deg.t/=EditYScale;
						*/
						// IDC_FONTS
						hctrl=GetDlgItem(hdlg,IDC_FONTS);
						// Get the fontname
						SendMessage(hctrl,WM_GETTEXT,
							sizeof(IJFONTNAME),(LONG)(LPSTR)tbuf);
						// Trim leading and trailing spaces from the font name
						strtrimts(tbuf);
						strtrimls(tbuf);
						i=EditorIMPMapFontName(tbuf);
						if(i<0)
							{
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.ml.ijfmt.font=i;

						// IDC_ROT
						if(EditHasRot)
							{
							hctrl=GetDlgItem(hdlg,IDC_ROT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.rotation)))
								{
								ShowError(IDE_BADROT);
								SetFocus(hctrl);
								return TRUE;
								}
							}
							
						// IDC_SLANT
						if(EditHasSlant)
							{
							hctrl=GetDlgItem(hdlg,IDC_SLANT);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.slant)))
								{
								ShowError(IDE_BADSLANT);
								SetFocus(hctrl);
								return TRUE;
								}
							}

						// IDC_JUST
						hctrl=GetDlgItem(hdlg,IDC_JUST);
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,CB_GETCURSEL,0,0);
						// IDC_WIDTH
						hctrl=GetDlgItem(hdlg,IDC_WIDTH);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.ml.cw))||
							(!(EditorMark.ml.cw>0.0)))
							{
							ShowError(IDE_BADCW);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.ml.cw/=EditXScale;

						// IDC_HEIGHT
						hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.ml.ch))||
							(!(EditorMark.ml.ch>0.0)))
							{
							ShowError(IDE_BADCH);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.ml.ch/=EditYScale;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.ml.ijfmt.ms=(WORD)dw;

						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_YFLIP
						EditorMark.ml.ijfmt.yflip=IsDlgButtonChecked(hdlg,IDC_YFLIP);
						// IDC_XFLIP
						EditorMark.ml.ijfmt.xflip=IsDlgButtonChecked(hdlg,IDC_XFLIP);
						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.ml.text,IMPLMLTEXT+1);
						if(!EditorMark.ml.text[0])
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;

					case IDC_WEXT:
						switch(HIWORD(wP))
							{
							case XN_INS:
								hctrl=GetDlgItem(hdlg,IDC_WEXT);
								DoInsertsPopup(hdlg,hctrl,&EditorIMP.udhas,
									&EditorIMP.vars,&EditorIMP.gvars);
								return TRUE;
							case XN_TAB:
								{
								HWND hc=GetDlgItem(hdlg,IDC_LOCATE);
								if(!IsWindowVisible(hc))
									hc=GetDlgItem(hdlg,IDOK);
								SetFocus(hc);
								}
								return TRUE;
							case XN_BKTAB:
								SetFocus(GetDlgItem(hdlg,IDC_XFLIP));
								return TRUE;
							case XN_ENTER:
								SendMessage(hdlg,WM_COMMAND,IDOK,1);
								return TRUE;
							}
					}
				}
			else // From text insert menu
				{
				hctrl=GetDlgItem(hdlg,IDC_WEXT);
				if(ProcessInsertsPopup(wP,hctrl))
					return TRUE;
				}
			break;
			}
		}
	return FALSE;
	}

/***************************************************************************
	UDHAEDITDP

	UDHA editor dialog process.
***************************************************************************/
BOOL CALLBACK UDHAEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			HWND hctrl;
			int i;
			char key[5];

			// Initilize the controls
			hctrl=GetDlgItem(hdlg,IDC_NAME); // IDC_NAME
			SendMessage(hctrl,EM_LIMITTEXT,IMPLUDHANAME,0);
			SendMessage(hctrl,WM_SETTEXT,0,(LPARAM)(LPSTR)editudha.name);
			hctrl=GetDlgItem(hdlg,IDC_VALUE); // IDC_VALUE
			SendMessage(hctrl,EM_LIMITTEXT,IMPLUDHAVAL,0);
			SendMessage(hctrl,WM_SETTEXT,0,(LPARAM)(LPSTR)editudha.val);
			CheckDlgButton(hdlg,IDC_REDIT,editudha.redit); // IDC_REDIT
			// Check to see if name is auto included and check
			// IDC_INCLUDE if so.
			for(i=0;i<IMPMAXUDHAS;i++)
				{
				wsprintf(key,"%d",i+1);
				tlen=GetPrivateProfileString("UDHA",key,"",tbuf,
					TSIZE,IMSIni);
				if(tlen<1)
					continue;
				if(!lstrcmp(editudha.name,tbuf[0]==':'
					? (LPSTR)&tbuf[1] : (LPSTR)&tbuf[0]))
					break;
				}
			CheckDlgButton(hdlg,IDC_INCLUDE,i<IMPMAXUDHAS);

			// Initial focus to value if name!="" else to name
			if(editudha.name[0])
				{
				SendDlgItemMessage(hdlg,IDC_VALUE,EM_SETSEL,0,MAKELPARAM(0,-1));
				SetFocus(GetDlgItem(hdlg,IDC_VALUE));
				}
			else
				SetFocus(GetDlgItem(hdlg,IDC_NAME));

			return FALSE;
			}

		case WM_COMMAND: // Win32 changed
			{
			HWND hctrl;
			LRESULT r;
			int i;
			char key[5];

			switch(LOWORD(wP))
				{
				case IDOK:
					// Validate each control and retrieve its value
					//IDC_NAME
					hctrl=GetDlgItem(hdlg,IDC_NAME);
					r=SendMessage(hctrl,WM_GETTEXT,IMPLUDHANAME+1,
						(LPARAM)(LPSTR)editudha.name);
					if(r<1)
						{
						ShowError(IDE_BADUDHANAME);
						return TRUE;
						}
					//IDC_VALUE
					hctrl=GetDlgItem(hdlg,IDC_VALUE);
					SendMessage(hctrl,WM_GETTEXT,IMPLUDHAVAL+1,
						(LPARAM)(LPSTR)editudha.val);
					// IDC_REDIT
					editudha.redit=IsDlgButtonChecked(hdlg,IDC_REDIT);

					//IDC_INCLUDE
					// if include then write to the ini file
					//   if redit then write n=:name else n=name
					// if !include and found then delete
					if(IsDlgButtonChecked(hdlg,IDC_INCLUDE))
						{ // Get index of existing entry or next index
						for(i=0;i<IMPMAXUDHAS;i++)
							{
							wsprintf(key,"%d",i+1);
							tlen=GetPrivateProfileString("UDHA",key,"",tbuf,
								TSIZE,IMSIni);
							if(tlen<1)
								break;
							if(!lstrcmp(editudha.name,tbuf[0]==':'
								? (LPSTR)&tbuf[1] : (LPSTR)&tbuf[0]))
								break;
							}
						if(i==IMPMAXUDHAS)
							{
							ShowError(IDE_IMPMAXINCUDHAS);
							return TRUE;
							}
						wsprintf(tbuf,editudha.redit ? ":%s" : "%s",
							(LPSTR)editudha.name);
						wsprintf(key,"%d",i+1);
						WritePrivateProfileString("UDHA",key,tbuf,IMSIni);
						}
					else // Delete if !include and found
						{
						for(i=0;i<IMPMAXUDHAS;i++)
							{
							wsprintf(key,"%d",i+1);
							tlen=GetPrivateProfileString("UDHA",key,"",tbuf,
								TSIZE,IMSIni);
							if(tlen<1)
								continue;;
							if(!lstrcmp(editudha.name,tbuf[0]==':'
								? (LPSTR)&tbuf[1] : (LPSTR)&tbuf[0]))
								WritePrivateProfileString("UDHA",key,NULL,
									IMSIni);
							}
						}

					EndDialog(hdlg,TRUE);
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
			}
		}
	return FALSE;
	}

/***************************************************************************
	VISPOSDP

	VIS (Inspector/Reader) Position editor dialog process.
***************************************************************************/
BOOL CALLBACK VISPOSDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	static IMPHEADER *h;
	static BOOL allok;     // False is IM VIS or other problem
	static BOOL imfloaded;
	static int nesti; // nest index
	static P3 nestwcs;
	static LPSTR visname;
	static LPP3 vispos;
	static BOOL injog;
	static M3 irtm;
	static double irtz;
	static IMF imf;
	static PIMSTEP movesteps;
	static PIMSTEP prepsteps=0;
	static WORD badpreperror;

	IMFNEST *pnest;
	P3 jogtcs;
	P3 jogncs;
	double v[3];
	LPSTR caption;

	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			allok=TRUE;
			imfloaded=FALSE;
			movesteps=NULL;
			VisPosWnd=hdlg;
			nesti=0; // first nest only for now!
			h=(IMPHEADER *)lP;
			IMClearSelLocator();
			if(prepsteps)
				IMSTEPFree(prepsteps,FALSE);
			prepsteps=0;
			// access fixture
			imfloaded=GetIMF(&imf,h->fixture);
			if(!imfloaded)
				{
				allok=FALSE;
				ShowError(0);
				}

			// get nest info
			if(allok)
				if(!GetNestWCS(&nestwcs,&imf,nesti))
					{
					allok=FALSE;
					ShowError(0);
					}
			if(allok)
				pnest=&(imf.nests.nest[nesti]);
			switch(EditorVisMode)
				{
				case vmINSPECTOR:
					caption="Inspector Position";
					visname=h->inspector;
					vispos=&(h->inspectpos);
					prepsteps=IMSELINSPECTORSTEPNew(visname);
					if(prepsteps)
						IMSTEPAppend(prepsteps,IMPREPINSPECTORSTEPNew(h->inspectprep));
					badpreperror=IDE_BADINSPECTPREP;
					break;
				case vmREADER:
					caption="Reader Position";
					visname=h->reader;
					vispos=&(h->readpos);
					badpreperror=IDE_BADREADPREP;
					prepsteps=IMSELREADERSTEPNew(visname);
					if(prepsteps)
						IMSTEPAppend(prepsteps,IMPREPREADERSTEPNew(h->readprep));
					break;
				default:
					allok=FALSE;
				}
			SetWindowText(hdlg,caption);
			allok=allok && !AEWnd; // Prevent XY usage when AEWnd owns it
			if(allok)
				allok=IMVisIRTM(&irtm,visname,pnest->rotation);
			if(allok)
				allok=IMVisIRTZ(&irtz,visname);
			if(allok)
				SendMessage(IMWnd,IMM_MARK,0,(LPARAM)prepsteps);
			allok=allok && (IMState==IMN_READY || IMState==IMN_PRINTOFF
				|| IMState==IMN_XYONLY || IMState==IMN_WARMUP);
			injog=FALSE;
			//IDC_EDIT <- coorindates
			SendDlgItemMessage(hdlg,IDC_EDIT,EM_LIMITTEXT,30,0);
			v[0]=vispos->x;
			v[1]=vispos->y;
			v[2]=vispos->z;
			MScaleVect((LPDOUBLE)&v,3,FALSE);
			sprintf(tbuf,"%0.3f,%0.3f,%0.3f",v[0],v[1],v[2]);
			SetDlgItemText(hdlg,IDC_EDIT,tbuf);
			return TRUE;

		case WM_COMMAND:  // Win32 changed
			switch(LOWORD(wP))
				{
				case IDOK:
					if(allok) // Turn off live 
						IMVisLive(visname,FALSE);
					if(movesteps)
						IMSTEPFree(movesteps,FALSE);
					movesteps=NULL;
					if(imfloaded)
						IMFFree(&imf);
					// update the position being edited
					GetDlgItemText(hdlg,IDC_EDIT,tbuf,TSIZE);
					ParseVect(tbuf,(LPDOUBLE)vispos,3);
					MScaleVect((LPDOUBLE)vispos,3,TRUE);
					VisPosWnd=0; // mark window closed
					EndDialog(hdlg,TRUE);
					return TRUE;
				case IDCANCEL:
					if(allok) // Turn off live vis
						IMVisLive(visname,FALSE);
					if(movesteps)
						IMSTEPFree(movesteps,FALSE);
					movesteps=NULL;
					if(imfloaded)
						IMFFree(&imf);
					VisPosWnd=0; // mark window closed
					EndDialog(hdlg,FALSE);
					return TRUE;
				case IDC_JOG:
					SendMessage(IMWnd,IMM_JOG,0,0); // start jog mode
					return TRUE;
				case IDC_MOVE:
					if(movesteps)
						IMSTEPFree(movesteps,FALSE);
					movesteps=NULL;
					GetDlgItemText(hdlg,IDC_EDIT,tbuf,TSIZE);
					ParseVect(tbuf,(LPDOUBLE)&VisPos,3);
					MScaleVect((LPDOUBLE)&VisPos,3,TRUE);
					switch(EditorVisMode)
						{
						case vmINSPECTOR:
							movesteps=IMSELINSPECTORSTEPNew(visname);
							break;
						case vmREADER:
							movesteps=IMSELREADERSTEPNew(visname);
							break;
						case vmLOCATOR:
							movesteps=IMSELLOCATORSTEPNew(visname);
							break;
						default:
							movesteps=NULL;
						}
					if(ZEXISTS)
						zvis=VisPos.z+imf.extent.z+h->partSize.z;
					else
						zvis=0;
					IMSTEPAppend(movesteps,IMFCompileVPosMove(&imf,nesti,EditorVisMode));
					SendMessage(IMWnd,IMM_MARK,0,(LPARAM)movesteps);
					return TRUE;
				case IDC_IM:
					switch(HIWORD(wP))
						{
						case IMN_READY:
						case IMN_PRINTOFF:
						case IMN_XYONLY:
						case IMN_WARMUP:
							if(prepsteps)
								{
								if(prepsteps)
									IMSTEPFree(prepsteps,FALSE);
								prepsteps=0;
								allok=IMVisLive(visname,TRUE);
								}
							EnableWindow(hdlg,TRUE);
							SetFocus(GetDlgItem(hdlg,IDOK));
							if(allok)
								{
								EnableWindow(GetDlgItem(hdlg,IDC_JOG),TRUE);
								EnableWindow(GetDlgItem(hdlg,IDC_MOVE),TRUE);
								}
							if(injog)
								{
								// get jogtcs
								SendMessage(IMWnd,IMM_GETXYPOS,0,(LPARAM)(LPDOUBLE)&jogtcs.x);
								if(ZEXISTS)
									SendMessage(IMWnd,IMM_GETZPOS,0,(LPARAM)(LPDOUBLE)&jogtcs.z);
								// compute jogncs from jogtcs
								//jogncs
								LPP3 temp=&jogtcs;
								//zzzLPP2 p2=(LPP2)temp;
								P2 p2=*((LPP2)temp);
								P2 temp2=p2*irtm;
								jogncs.x=temp2.x;
								jogncs.y=temp2.y;
								//P2Xform((LPP2)&jogncs,(LPP2)&jogtcs,&irtm);
								jogncs.x-=nestwcs.x;
								jogncs.y-=nestwcs.y;
								if(ZEXISTS)
									jogncs.z=jogtcs.z+irtz-nestwcs.z-h->partSize.z;
								else
									jogncs.z=0.0f;
								v[0]=jogncs.x;
								v[1]=jogncs.y;
								v[2]=jogncs.z;

								MScaleVect((LPDOUBLE)&v,3,FALSE);
								// update IDC_EDIT
								sprintf(tbuf,"%0.3f,%0.3f,%0.3f",
									v[0],v[1],v[2]);
								SetDlgItemText(hdlg,IDC_EDIT,tbuf);
								injog=FALSE;
								}
							return TRUE;
						case IMN_ERROR:
							if(prepsteps)
								{
								ShowError(badpreperror);
								if(prepsteps)
									IMSTEPFree(prepsteps,FALSE);
								prepsteps=0;
								}
							EnableWindow(hdlg,TRUE);
							EnableWindow(GetDlgItem(hdlg,IDC_JOG),FALSE);
							EnableWindow(GetDlgItem(hdlg,IDC_MOVE),FALSE);
							SetFocus(GetDlgItem(hdlg,IDCANCEL));
							injog=FALSE;
							return TRUE;
						case IMN_JOG:
							EnableWindow(hdlg,FALSE);
							injog=TRUE;
							return TRUE;
						}
				}
		}
	return FALSE;
	}

/***************************************************************************
	STATIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	EditorWM_CREATE(hwnd)

	EDITORWP WM_CREATE handler.

	HWND hwnd = Handle of window receiving the message.
***************************************************************************/
static void EditorWM_CREATE(HWND hwnd)
	{
	LONG i,li;

	// Create and initialize header window
	HWnd=CreateWindow("XLISTBOX","Header",
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_NOTIFY|
		LBS_NOINTEGRALHEIGHT|WS_BORDER|WS_CAPTION,
		0,0,0,0,hwnd,(HMENU)IDC_H,IMSInst,NULL);
//zzz
	// Reallocate udhas,vars,gvars,marks,fonts to max capacity for editing
	EditorIMP.udhas.realloc(IMPMAXUDHAS);
	EditorIMP.vars.realloc(IMPMAXVARS);
	EditorIMP.gvars.realloc(IMPMAXVARS);
	EditorIMP.marks.realloc(IMPMAXMARKS);
	EditorIMP.fonts.realloc(IMPMAXFONTS);



	// Initialize header window
	EditHasRadius=FALSE;
	for(i=0;i<IMPNBHAS+EditorIMP.udhas.count;i++)
		{
		if(IMPHAIsHidden(i)) // skip hidden HAs
			continue;
		IMPHeaderAttributeToTbuf(EditorIMP.pheader,&EditorIMP.udhas,i);
		li=SendMessage(HWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
		SendMessage(HWnd,LB_SETITEMDATA,li,i);
		// Init EditHasRadius based on UDHA RADIUS
		if(i>=IMPNBHAS)
			if(!lstrcmp(EditorIMP.udhas.udha[i-IMPNBHAS].name,"Radius"))
				EditHasRadius=lstrcmp(EditorIMP.udhas.udha[i-IMPNBHAS].val,"0")
					&&lstrcmp(EditorIMP.udhas.udha[i-IMPNBHAS].val,"");
		}
	// Init EditXYMode based on fixture
	EditXYMode=GetXYMODE(EditorIMP.pheader->fixture);

	// Create and initialize vars window
	VWnd=CreateWindow("XLISTBOX","Variables",
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_NOTIFY|
		LBS_NOINTEGRALHEIGHT|WS_BORDER|WS_CAPTION,
		0,0,0,0,
		hwnd,(HMENU)IDC_V,IMSInst,NULL);
	// gvars
	for(i=0;i<EditorIMP.gvars.count;i++)
		{
		IMPVarToTbuf(&EditorIMP.gvars.var[i]);
		SendMessage(VWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
		}
	vncount=EditorIMP.gvars.count;

	// vars
	for(i=0;i<EditorIMP.vars.count;i++)
		{
		IMPVarToTbuf(&EditorIMP.vars.var[i]);
		SendMessage(VWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
		}
	vncount+=EditorIMP.vars.count;

	vncount++;

	// Create and initialize marks window
	MWnd=CreateWindow("XLISTBOX","Marks",
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_NOTIFY|WS_OVERLAPPED|
		LBS_NOINTEGRALHEIGHT|WS_BORDER|WS_CAPTION|WS_CLIPSIBLINGS,
		0,0,0,0,
		hwnd,(HMENU)IDC_M,IMSInst,NULL);
	SendMessage(MWnd,WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT),0);
	for(i=0;i<EditorIMP.marks.count;i++)
		{
		IMPMarkToTbuf(&(EditorIMP.marks.mark[i]),&EditorIMP.fonts);
		SendMessage(MWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
		}

	//Create and initialize preview window   ---------------------Linghai-11-19-08---
	/*PWnd=CreateWindow(WC_DIALOG,"Preview",
		WS_CHILD|WS_VISIBLE|WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_CLIPSIBLINGS,
		0,0,0,0,
		hwnd,(HMENU)IDC_P,IMSInst,NULL);  //------------------Linghai-11-19-08---*/
	//PWnd=CreateDialog(IMSInst,"PREVIEW",hwnd,(DLGPROC)PreWindowDP);
	//ShowWindow(PWnd,SW_SHOW);
	//pIMP(EditorIMP);                                      //-----------Linghai-11-24-08--
	//SetWindowLongPtr(PWnd,DWLP_DLGPROC,(long)PreWindowDP);

	
	//PWnd=PreWindow(IMSInst,EditorIMP,hwnd,IMSIni,IDC_P);                   //------------Linghai-11-21-08---
	//PWnd = Pre(IMSInst, EditorIMP, IMSIni,hwnd);
	//SendMessage(PWnd,WM_INITDIALOG,1,0);                                //-------------Linghai-11-24-08--
	
	
	// Lock the inkjet
	SendMessage(IMWnd,IMM_LOCKIJ,1,0);

	// Update inkjet/fonts knowledge
	inkjetUpdate();

	FocusID=IDC_H;
	HCurSel=0;
	VCurSel=0;
	MCurSel=0;
	}

static void EditorDeleteSelectedMark(HWND hwnd,BOOL warn)
	{
	int i=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
	if(i==-1)
		return;
	if((!warn)||(MessageBox(hwnd,"Delete Mark",IMSName,
		MB_YESNO|MB_ICONQUESTION)==IDYES))
		{
		SendMessage(MWnd,LB_DELETESTRING,i,0L);
		if(i < EditorIMP.marks.count-1)
			{ // If not the last mark
			memmove(            // Close hole
				&EditorIMP.marks.mark[i],
				&EditorIMP.marks.mark[i+1],
				(EditorIMP.marks.count-i-1)*sizeof(IMPMARK));
			}
		else
			{
			i--; // Move selection index to new last mark
			if(i==-1)
				i=0;
			}
		SendMessage(MWnd,LB_SETCURSEL,i,0L);
		EditorIMP.marks.count--;
		IMPPurgeFonts(&EditorIMP);
		EditorIMP.changed=TRUE;
		}
	}

static void EditorCopySelectedMark(HWND hwnd)
	{
	int i=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
	if(i==-1)
		return;
	ClipboardMark=EditorIMP.marks.mark[i];
	ClipboardMarkValid=true;
	}

static void EditorPasteAtIndex(HWND hwnd,int index)
	{
	IMPMarkToTbuf(&ClipboardMark,&EditorIMP.fonts);
	if(index==EditorIMP.marks.count) // Appending
		SendMessage(MWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
	else // Inserting
		{
		SendMessage(MWnd,LB_INSERTSTRING,index,(LONG)(LPSTR)tbuf);
		memmove(            // Open hole
			&EditorIMP.marks.mark[index+1],
			&EditorIMP.marks.mark[index],
			(EditorIMP.marks.count-index)*sizeof(IMPMARK));
		}
	EditorIMP.marks.mark[index]=ClipboardMark;
	EditorIMP.marks.count++;
	IMPPurgeFonts(&EditorIMP); // zzz Killing my font
	EditorIMP.changed=TRUE;
	}

static void EditorPasteAtSelected(HWND hwnd)
	{
	int i=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
	if(i==-1)
		i=0;
	EditorPasteAtIndex(hwnd,i);
	}

static void EditorPasteAfterSelected(HWND hwnd)
	{
	int i=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
	if(i==-1)
		i=0;
	if(EditorIMP.marks.count==0)
		i=0;
	EditorPasteAtIndex(hwnd,i+1);
	}

/***************************************************************************
	EditorWM_COMMAND(hwnd,wP,lP)

	EDITORWP WM_COMMAND handler.

	HWND hwnd = Handle of window receiving the message.
	WORD wP = Word param
	LONG lP = Long param
***************************************************************************/
static void EditorWM_COMMAND(HWND hwnd,WPARAM wP,LPARAM lP)   // Changed for Win32
	{
	WORD i,vi;
	DWORD li;
//	OFSTRUCT ofs,tofs;
	char tofs[MAX_PATH];
	char bfn[MAX_PATH];
	bfn[0]=0;
	char * pbfn=NULL;
	int mi;

	WORD id = LOWORD(wP);
	WORD ncode = HIWORD(wP);
	
	if(!lP) // If from menu
		{
		switch(LOWORD(wP))
			{

					case ID_ACCEL40017:
					{
					int item=SendMessage(MWnd,LB_GETCURSEL,0,0);

					if(item!=LB_ERR && item !=0)
					{
					IMPMARK tmark=EditorIMP.marks.mark[item-1];
					EditorIMP.marks.mark[item-1]=EditorIMP.marks.mark[item];
					EditorIMP.marks.mark[item]=tmark;
					SendMessage(MWnd,LB_RESETCONTENT,0,0);
					for(i=0;i<EditorIMP.marks.count;i++)
					{
						IMPMarkToTbuf(&(EditorIMP.marks.mark[i]),&EditorIMP.fonts);
						SendMessage(MWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
					}
					SendMessage(MWnd,LB_SETCURSEL,item-1,0);
					}

					}
					break;
					case ID_ACCEL40018:
					{
					int item=SendMessage(MWnd,LB_GETCURSEL,0,0);
					int count=SendMessage(MWnd,LB_GETCOUNT,0,0);
					if(item!=count-1 && item!=LB_ERR)
					{
						IMPMARK tmark=EditorIMP.marks.mark[item+1];
						EditorIMP.marks.mark[item+1]=EditorIMP.marks.mark[item];
					    EditorIMP.marks.mark[item]=tmark;
						SendMessage(MWnd,LB_RESETCONTENT,0,0);
						for(i=0;i<EditorIMP.marks.count;i++)
						{
							IMPMarkToTbuf(&(EditorIMP.marks.mark[i]),&EditorIMP.fonts);
							SendMessage(MWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
						}
						SendMessage(MWnd,LB_SETCURSEL,item+1,0);
					}
					}
					break;
			case IDM_QUIT:
				if(EditorIMP.changed&&UserLevel>2)
					{
					if(EditorIMP.fn[0]){
						GetFileTitle(EditorIMP.fn,bfn,MAX_PATH);
						pbfn=&bfn[0];
					}
					//bfn=GetOFSBaseName(&EditorIMP.ofs);
					wsprintf(tbuf,"Save changes to - %s",
						pbfn ? pbfn : (LPSTR)"(untitled)");
						
					switch(MessageBox(hwnd,tbuf,IMSName,
						MB_YESNOCANCEL|MB_ICONEXCLAMATION))
						{
						case IDYES:
							SendMessage(hwnd,WM_COMMAND,IDM_SAVE,0);
							if(EditorIMP.changed) // if save failed
								return;           // don't quit
							break;
						case IDNO:
							break;
						case IDCANCEL:
							return;
						}
					}
				ClampRelease();
				if(EditorIMP.aesetup)
					EditorIMP.aesetup->closeedit();
				IMPFree(&EditorIMP);
				IMFFree(&RunIMF);
				SetMode(mFILES);
				return;

			case IDM_SAVE:
				pbfn=NULL;
				if (EditorIMP.fn[0]){
					GetFileTitle(EditorIMP.fn,bfn,MAX_PATH);
					pbfn=&bfn[0];
				}
//				bfn=GetOFSBaseName(&EditorIMP.ofs);
				if(!pbfn) // If untitled
					{            // Do save as instead
					SendMessage(hwnd,WM_COMMAND,IDM_SAVEAS,0);
					return;
					}
				if(!IMPValid(&EditorIMP))
					{
					ShowError(IDE_INVALIDIMP);
					return;
					}
				IMPSave(&EditorIMP);
				return;

			case IDM_SAVEAS:
				{
					if(!IMPValid(&EditorIMP))
					{
						ShowError(IDE_INVALIDIMP);
						return;
					}
					pbfn=NULL;
					if (EditorIMP.fn[0]){
						GetFileTitle(EditorIMP.fn,bfn,MAX_PATH);
						pbfn=&bfn[0];
					}
					//bfn=GetOFSBaseName(&EditorIMP.ofs);
					if(!DoPrompt(hwnd,"Save As",100,pbfn ? pbfn : ""))
						return;
					strfnextend(tbuf,"IMP");
					// Check for pre-existing file of same name
					HANDLE fh = CreateFile(tbuf,GENERIC_READ,0,NULL,OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,NULL);
					
					if(fh != INVALID_HANDLE_VALUE)
					{
						if(UserLevel>2)
						{
							if(MessageBox(hwnd,"Overwrite existing file?",
								IMSName,MB_YESNO|MB_ICONQUESTION)==IDNO)
								return;
						}
						else 
						{
							MessageBox(hwnd,"File already exist!",
								IMSName,MB_OK|MB_ICONEXCLAMATION);
							return;
						}
						CloseHandle(fh);
					}

					fh = CreateFile(tbuf,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,NULL);
					
					// Create the file
					if(fh==INVALID_HANDLE_VALUE)
					{
						ShowError(IDE_CREATEIMP);
						return;
					}
					
					CloseHandle(fh);
					// Save EditorIMP.ofs in case of save failure
					lstrcpy(tofs,EditorIMP.fn);              // zzz 2/26
					// Update EditorIMP.ofs for save
					lstrcpy(EditorIMP.fn,tbuf);
					// Save the file
					if(IMPSave(&EditorIMP))
					{ // On error
						lstrcpy(EditorIMP.fn,tofs); // Restore the old ofs
						return;
					}
					
					// Update the caption
					char capt[MAX_PATH];
					GetFileTitle(EditorIMP.fn,capt,MAX_PATH);
						wsprintf(tbuf,"%s-ROTARY :EDITOR - %s",
						(LPSTR)IMSName,capt);
					SetWindowText(IMSWnd,tbuf);
					return;
				}

			case IDM_TEST:
				{
				IMFFree(&RunIMF);
				RunIMF.imp=EditorIMP;
				// Exctract fixture name out of the header
				// and optionally build complete path to it.
				if(IMSFixtures[0])
					{
					lstrcpy(tbuf,IMSFixtures);
					lstrcat(tbuf,"\\");
					}
				else
					tbuf[0]=0;

				lstrcat(tbuf,RunIMF.imp.pheader->fixture);
				strfnextend(tbuf,"IMF");

				// Initialize the ofs for the fixture
				HANDLE fh = CreateFile(tbuf,GENERIC_READ,0,NULL,OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,NULL);

				lstrcpy(RunIMF.fn,tbuf); 

				if(fh == INVALID_HANDLE_VALUE)
					{
					ShowError(IDE_NOIMF);
					return;
					}
				CloseHandle(fh);

				// Load the fixture
				if(!IMFLoad(&RunIMF))
					{
					ShowError(0);
					return;
					}
				if(!IMFAllocVals(&RunIMF))
					return;
				if(IMFInitVals(&RunIMF))
					{
					for(i=0;i<RunIMF.nests.count;i++)
						RunIMF.nests.nest[i].flags|=nbfOCCUPIED;
					RunMode=rmTEST;
					RunLinked=FALSE;
					SetMode(mRUN);
					}
				
				return;
				}

			case IDM_DELETE:
				EditorDeleteSelectedMark(hwnd,true);
				return;
			//FAKE CLIPBOARD FOR MARKS
			case IDM_CUT:
				EditorCopySelectedMark(hwnd);
				EditorDeleteSelectedMark(hwnd,false);
				return;
			case IDM_COPY:
				EditorCopySelectedMark(hwnd);
				return;
			case IDM_PASTE:
				EditorPasteAtSelected(hwnd);
				return;
			case IDM_PASTEAFTER:
				EditorPasteAfterSelected(hwnd);
				return;
			case IDM_NEWKVAR:
				if(EditorIMP.vars.count+EditorIMP.gvars.count==IMPMAXVARS)
					{	// If full
					ShowError(IDE_IMPMAXVARS);
					return;
					}

				// Create default keyed var
				wsprintf(editvar.name,"K%d",vncount++);
				editvar.width=IMPLKVVAL;
				editvar.lpad=0;
				editvar.type=vtKEYED;
				editvar.unique=0;
				editvar.k.digits=TRUE;
				editvar.k.lowers=TRUE;
				editvar.k.uppers=TRUE;
				editvar.k.space=TRUE;
				editvar.k.others=TRUE;
				IMPInitVal(&editvar.preval,&editvar);
				// Run the dialog
				editisgvar=FALSE;
				if(DoVarEdit(hwnd))
					{
					if(editisgvar)
						{
						EditorIMP.gvars.var[EditorIMP.gvars.count++]=editvar;
						// Insert in VWnd at end of globals
						SendMessage(VWnd,LB_INSERTSTRING,
							EditorIMP.gvars.count-1,
							(LONG)(LPSTR)editvar.name);
						}
					else // instance var
						{
						EditorIMP.vars.var[EditorIMP.vars.count++]=editvar;
						SendMessage(VWnd,LB_ADDSTRING,0,
							(LONG)(LPSTR)editvar.name);
						}
					EditorIMP.changed=TRUE;
					}
				return;

			case IDM_NEWSVAR:
				if(EditorIMP.vars.count+EditorIMP.gvars.count==IMPMAXVARS)
					{	// If full
					ShowError(IDE_IMPMAXVARS);
					return;
					}
				// Create default serial var
				wsprintf(editvar.name,"S%d",vncount++);
				editvar.width=IMPLSVVAL;
				editvar.lpad=0;
				editvar.type=vtSERIAL;
				editvar.unique=0;
				editvar.s.step=1L;
				editvar.s.dirpos=TRUE;
				editvar.s.repeat=0;
				editvar.s.minval=0L;
				editvar.s.maxval=IMPMAXSVVAL;
				editvar.s.loop=FALSE;
				IMPInitVal(&editvar.preval,&editvar);
				// Run the dialog
				editisgvar=FALSE;
				if(DoVarEdit(hwnd))
					{
					if(editisgvar)
						{
						EditorIMP.gvars.var[EditorIMP.gvars.count++]=editvar;
						// Insert in VWnd at end of globals
						SendMessage(VWnd,LB_INSERTSTRING,
							EditorIMP.gvars.count-1,
							(LONG)(LPSTR)editvar.name);
						}
					else // instance var
						{
						EditorIMP.vars.var[EditorIMP.vars.count++]=editvar;
						SendMessage(VWnd,LB_ADDSTRING,0,
							(LONG)(LPSTR)editvar.name);
						}
					EditorIMP.changed=TRUE;
					}
				return;

			case IDM_NEWDVAR:
				if(EditorIMP.vars.count+EditorIMP.gvars.count==IMPMAXVARS)
					{ // if full
					ShowError(IDE_IMPMAXVARS);
					return;
					}
				// Create default date var
				wsprintf(editvar.name,"D%d",vncount++);
				editvar.width=0;
				editvar.lpad=0;
				editvar.type=vtDATE;
				editvar.unique=0;
				editvar.d.offset=0;
				lstrcpy(editvar.d.format,"M/D/YYYY");
				editvar.d.canOverride=FALSE;
				editvar.d.base[0]=0;
				IMPInitVal(&editvar.preval,&editvar);
				// Run the dialog
				editisgvar=TRUE;
				if(DoVarEdit(hwnd))
					{
					// Always global
					EditorIMP.gvars.var[EditorIMP.gvars.count++]=editvar;
					// Insert in VWnd at end of globals
					SendMessage(VWnd,LB_INSERTSTRING,
						EditorIMP.gvars.count-1,(LONG)(LPSTR)editvar.name);
					EditorIMP.changed=TRUE;
					}
				return;

			case IDM_NEWTVAR:
				if(EditorIMP.vars.count+EditorIMP.gvars.count==IMPMAXVARS)
					{ // if full
					ShowError(IDE_IMPMAXVARS);
					return;
					}
				// Create default time var
				wsprintf(editvar.name,"T%d",vncount++);
				editvar.width=0;
				editvar.lpad=0;
				editvar.type=vtTIME;
				editvar.unique=0;
				editvar.t.offset=0;
				editvar.t.useNames=FALSE;
				editvar.t.is24=FALSE;
				editvar.t.isLZ=FALSE;
				editvar.t.seperator=':';
				lstrcpy(editvar.t.amSuffix,"AM");
				lstrcpy(editvar.t.pmSuffix,"PM");
				editvar.t.showSeconds=TRUE;
				editvar.t.canOverride=FALSE;
				editvar.t.base[0]=0;
				IMPInitVal(&editvar.preval,&editvar);
				// Run the dialog
				editisgvar=TRUE;
				if(DoVarEdit(hwnd))
					{
					// Always global
					EditorIMP.gvars.var[EditorIMP.gvars.count++]=editvar;
					// Insert in VWnd at end of globals
					SendMessage(VWnd,LB_INSERTSTRING,
						EditorIMP.gvars.count-1,(LONG)(LPSTR)editvar.name);
					EditorIMP.changed=TRUE;
					}
				return;

			case IDM_NEWBASEDVAR:
				if(EditorIMP.vars.count+EditorIMP.gvars.count==IMPMAXVARS)
					{ // if full
					ShowError(IDE_IMPMAXVARS);
					return;
					}
				// Create default based var
				wsprintf(editvar.name,"S%d",vncount++);
				editvar.width=IMPLBASEDVAL;
				editvar.lpad=0;
				editvar.type=vtBASED;
				editvar.unique=0;
				editvar.b.base[0]=0;
				editvar.b.loop=TRUE;
				IMPInitVal(&editvar.preval,&editvar);
				// Run the dialog
				editisgvar=FALSE;
				if(DoVarEdit(hwnd))
					{
					if(editisgvar)
						{
						EditorIMP.gvars.var[EditorIMP.gvars.count++]=editvar;
						// Insert in VWnd at end of globals
						SendMessage(VWnd,LB_INSERTSTRING,
							EditorIMP.gvars.count-1,
							(LONG)(LPSTR)editvar.name);
						}
					else // instance var
						{
						EditorIMP.vars.var[EditorIMP.vars.count++]=editvar;
						SendMessage(VWnd,LB_ADDSTRING,0,
							(LONG)(LPSTR)editvar.name);
						}
					EditorIMP.changed=TRUE;
					}
				return;

			case IDM_NEWTEXTMARK:
			case IDM_NEWMULTILINEMARK:
			case IDM_NEWDEVBCMARK:
			case IDM_NEWC2OF5MARK:
			case IDM_NEWC39MARK:
			case IDM_NEWC93MARK:
			case IDM_NEWC128MARK:
			case IDM_NEWCUPCEANMARK:
			case IDM_NEWBITMAPMARK:
			case IDM_NEWUIDMARK:
			case IDM_NEWCDMMARK:
			//case IDM_NEWFORMAT12:                   //----------------Linghai-11-21-08---
			//case IDM_NEWFORMAT6:                    //---------------Linghai-11-21-08---
			//case IDM_NEWFORMAT5:                    //----------------Linghai-11-21-08---
			case IDM_NEWWINFONTMARK:
			case IDM_NEWPATCHMARK:
			case IDM_NEWWMTRXMARK:
			case IDM_NEWCQRMARK:
			case IDM_NEWCURESTEP:
			case IDM_NEWCDEVMARK:
			case IDM_NEWDEVSYMMARK:
				NewMark(wP,hwnd);
				//SendMessage(PWnd,WM_INITDIALOG,wP,0);
				//PWnd=PreWindow(IMSInst,EditorIMP,hwnd,IMSIni,IDC_P);                   //------------Linghai-11-21-08---
				//SendMessage(PWnd,WM_INITDIALOG,1,0);                                //-------------Linghai-11-24-08--
				//SendMessage(PWnd,WM_INITDIALOG,1,0);                  //------Linghai-11-24-08----
				//SendMessage(PWnd,WM_CTLCOLORDLG,1,0);                     //-------------Linghai-11-24-08--
				//SendMessage(PWnd,WM_PAINT,1,0);                          //-------------Linghai-11-24-08--
				//SendMessage(PWnd,WM_DESTROY,1,0);                      //-------------Linghai-11-24-08--
				//PWnd=PreWindow(IMSInst,EditorIMP,hwnd,IMSIni,IDC_P);   //-------------Linghai-11-24-08--
				return;

			case IDM_NEWUDHA:
				if(EditorIMP.udhas.count==IMPMAXUDHAS) // If full
					{
					ShowError(IDE_IMPMAXUDHAS);
					return;
					}
				i=EditorIMP.udhas.count;
				// Create default data
				EditorIMP.udhas.udha[i].name[0]=0;
				EditorIMP.udhas.udha[i].redit=FALSE;
				EditorIMP.udhas.udha[i].val[0]=0;

				// Run the dialog
				if(DoUDHAEdit(hwnd,&(EditorIMP.udhas.udha[i])))
					{
					EditorIMP.udhas.count++;
					IMPHeaderAttributeToTbuf(EditorIMP.pheader,
						&EditorIMP.udhas,i+IMPNBHAS);
					li=SendMessage(HWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
					SendMessage(HWnd,LB_SETITEMDATA,li,i+IMPNBHAS);

					EditorIMP.changed=TRUE;
					}
				return;
				case IDM_PREVIEW:
					{
						/*HWND ok=CreateDialog(IMSInst,"PREVIEW",hwnd,(DLGPROC)PREVIEWDP);
						ShowWindow(ok,SW_SHOW);
						if(!ok)
						{
							MessageBox(IMSWnd,"Error creating preview window","IMSName",MB_ICONEXCLAMATION);
						}*/
						//PREVIEWWINDOW(IMSInst, EditorIMP, IMSIni);
					}
					break;
			case IDM_EDITAE: // Add or Edit AE
				if(!EditorIMP.aesetup)
					{
					EditorIMP.aesetup=new AESETUP();
					EditorIMP.changed=TRUE;
					}
				if(EditorIMP.aesetup)
					{
					IMFFree(&RunIMF);
					RunIMF.imp=EditorIMP;
					if(GetIMF(&RunIMF,EditorIMP.pheader->fixture))
						EditorIMP.aesetup->openedit(hwnd,&IMSADWnd,
							&RunIMF,EditorIMP.pheader->partSize.z,0);
					else
						ShowError(0);
					}
				return;

			case IDM_DELETEAE:
				if(EditorIMP.aesetup)
					{
					if(MessageBox(hwnd,"Delete Alignment Extension",
						IMSName,MB_OKCANCEL|MB_ICONQUESTION)==IDCANCEL)
						return;
					EditorIMP.changed=TRUE;
					EditorIMP.aesetup->closeedit();
					delete EditorIMP.aesetup;
					EditorIMP.aesetup=NULL;
					}
				return;
			case IDM_EDITIMC: // Edit IMC
				{
				mi=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
				if(mi==LB_ERR)
					return;
				IMCORR *p=EditorIMP.imcorrs.find(mi);
				if(!p)
					p=EditorIMP.imcorrs.getempty();
				if(!p)
					return;
				if(p->edit(hwnd,mi,EditXScale,EditYScale))
					EditorIMP.changed=TRUE;
				return;
				}
			case IDM_DELETEIMC:
				{
				mi=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
				if(mi==LB_ERR)
					return;
				if(MessageBox(hwnd,"Delete Correction Values",
					IMSName,MB_OKCANCEL|MB_ICONQUESTION)==IDCANCEL)
					return;
				IMCORR *p=EditorIMP.imcorrs.find(mi);
				if(p)
					{
					p->init();
					EditorIMP.changed=TRUE;
					}
				return;
				}
			}
		return;
		}
	switch(LOWORD(wP)) // Control messages
		{
		case IDC_H:
			switch(HIWORD(wP))
				{
				case LBN_DBLCLK:
					li=(int)SendMessage(HWnd,LB_GETCURSEL,0,0L);
					i=(int)SendMessage(HWnd,LB_GETITEMDATA,li,0);
					if(EditHeaderEntry(hwnd,EditorIMP.pheader,
						&EditorIMP.udhas,i))
						{
						IMPHeaderAttributeToTbuf(EditorIMP.pheader,
							&EditorIMP.udhas,i);
						SendMessage(HWnd,LB_DELETESTRING,li,0L);
						SendMessage(HWnd,LB_INSERTSTRING,li,(LONG)(LPSTR)tbuf);
						SendMessage(HWnd,LB_SETITEMDATA,li,i);
						EditorIMP.changed=TRUE;
						}
					SendMessage(HWnd,LB_SETCURSEL,li,0L);
					return;
				case XN_KILLFOCUS:
					HCurSel=(int)SendMessage(HWnd,LB_GETCURSEL,0,0L);
					if(HCurSel==-1)
						HCurSel=0;
					SendMessage(HWnd,LB_SETCURSEL,-1,0L);
					return;
				case XN_SETFOCUS:
					FocusID=IDC_H;
					if(-1==(int)SendMessage(HWnd,LB_GETCURSEL,0,0L))
						SendMessage(HWnd,LB_SETCURSEL,HCurSel,0L);
					return;
				case XN_DEL:
					li=(int)SendMessage(HWnd,LB_GETCURSEL,0,0L);
					if(li==-1)
						return;
					i=(int)SendMessage(HWnd,LB_GETITEMDATA,li,0);
					if(i<IMPNBHAS) // basic ?
						return;
					i-=IMPNBHAS; // convert to UDHAS index
					wsprintf(tbuf,"Delete Header Attribute %s ?",
						(LPSTR)EditorIMP.udhas.udha[i].name);
					if(MessageBox(hwnd,tbuf,IMSName,
						MB_YESNO|MB_ICONQUESTION)==IDYES)
						{
						if(!lstrcmp(EditorIMP.udhas.udha[i].name,"Radius"))
							EditHasRadius=FALSE;
						SendMessage(HWnd,LB_DELETESTRING,li,0L);
						if(i < EditorIMP.udhas.count-1)
							{ // If not the last data
							memmove(            // Close hole
								&(EditorIMP.udhas.udha[i]),
								&(EditorIMP.udhas.udha[i+1]),
								(EditorIMP.udhas.count-i-1)*sizeof(IMPUDHA));
							}
						else
							{
							li--; // Move selection index to new last uhda
							if(li==-1)
								li=0;
							}
						SendMessage(HWnd,LB_SETCURSEL,li,0L);
						EditorIMP.udhas.count--;
						EditorIMP.changed=TRUE;
						}
					return;

				case XN_TAB:
					SetFocus(VWnd);
					return;
				case XN_BKTAB:
					SetFocus(IMWnd);
					return;
				}
			return;
		case IDC_V:
			switch(HIWORD(wP))
				{
				case LBN_DBLCLK:
					i=(int)SendMessage(VWnd,LB_GETCURSEL,0,0L);
					if(i==-1)
						return;
					if(i<EditorIMP.gvars.count) // If global
						{
						editisgvar=TRUE;
						editvar=EditorIMP.gvars.var[i];
						if(DoVarEdit(hwnd))
							{
							SendMessage(VWnd,LB_DELETESTRING,i,0L);
							if(!editisgvar)
								{
								// delete EditorIMP.gvars.var[i]
								if(i < EditorIMP.gvars.count-1)
									{ // If not the last var
									// Close hole
									memmove(&(EditorIMP.gvars.var[i]),
										&(EditorIMP.gvars.var[i+1]),
										(EditorIMP.gvars.count-i-1)*sizeof(IMPVAR));
									}
								EditorIMP.gvars.count--;
								// add to var
								EditorIMP.vars.var[EditorIMP.vars.count++]=editvar;
								SendMessage(VWnd,LB_ADDSTRING,0,
									(LONG)(LPSTR)editvar.name);
								// i = new selection
								i=EditorIMP.gvars.count+EditorIMP.vars.count-1;
								}
							else
								{
								EditorIMP.gvars.var[i]=editvar;
								SendMessage(VWnd,LB_INSERTSTRING,i,
									(LONG)(LPSTR)editvar.name);
								}
							EditorIMP.changed=TRUE;
							}
						}
					else // else instance
						{
						vi=i-EditorIMP.gvars.count;
						editisgvar=FALSE;
						editvar=EditorIMP.vars.var[vi];
						if(DoVarEdit(hwnd))
							{
							SendMessage(VWnd,LB_DELETESTRING,i,0L);
							if(editisgvar) // Move to gvars
								{
								// delete EditorIMP.vars.var[vi]
								if(vi < EditorIMP.vars.count-1) // If not the last var
									memmove(&(EditorIMP.vars.var[vi]), // Close hole
										&(EditorIMP.vars.var[vi+1]),
										(EditorIMP.vars.count-vi-1)*sizeof(IMPVAR));
								EditorIMP.vars.count--;
								// add to gvar
								EditorIMP.gvars.var[EditorIMP.gvars.count++]=editvar;
								SendMessage(VWnd,LB_INSERTSTRING,
									EditorIMP.gvars.count-1,(LONG)(LPSTR)editvar.name);
								i=EditorIMP.gvars.count-1; // i = new current selection
								}
							else
								{
								EditorIMP.vars.var[i-EditorIMP.gvars.count]=editvar;
								SendMessage(VWnd,LB_INSERTSTRING,i,
									(LONG)(LPSTR)editvar.name);
								}
							EditorIMP.changed=TRUE;
							}
						}
					SendMessage(VWnd,LB_SETCURSEL,i,0L);
					return;

				case XN_DEL:
					{
					IMPVARS *v;
					BOOL glob;
					i=(int)SendMessage(VWnd,LB_GETCURSEL,0,0L);
					if(i==-1)
						return;

					glob=i<EditorIMP.gvars.count;

					// setup v and vi for deleting global or instance vars
					if(glob)
						{
						vi=i;
						v=&EditorIMP.gvars;
						}
					else
						{
						vi=i-EditorIMP.gvars.count;
						v=&EditorIMP.vars;
						}

					wsprintf(tbuf,"Delete %s variable %s ?",
							(LPSTR)(glob ? "global" : "instance"),
							(LPSTR)v->var[vi].name);
					if(MessageBox(hwnd,tbuf,IMSName,
						MB_YESNO|MB_ICONQUESTION)==IDYES)
						{
						SendMessage(VWnd,LB_DELETESTRING,i,0L);
						if(vi < v->count-1) // If not the last var
							memmove(            // Close hole
								&(v->var[vi]),
								&(v->var[vi+1]),
								(v->count-vi-1)*sizeof(IMPVAR));

						// if last in listbox then move selection up
						if(i && i==SendMessage(VWnd,LB_GETCOUNT,0,0L))
							i--; // Move selection index to new last var

						SendMessage(VWnd,LB_SETCURSEL,i,0L);
						v->count--;
						EditorIMP.changed=TRUE;
						}
					return;
					}

				case XN_KILLFOCUS:
					VCurSel=(int)SendMessage(VWnd,LB_GETCURSEL,0,0L);
					if(VCurSel==-1)
						VCurSel=0;
					SendMessage(VWnd,LB_SETCURSEL,-1,0L);
					return;
				case XN_SETFOCUS:
					FocusID=IDC_V;
					if(-1==(int)SendMessage(VWnd,LB_GETCURSEL,0,0L))
						SendMessage(VWnd,LB_SETCURSEL,VCurSel,0L);
					return;
				case XN_TAB:
					SetFocus(MWnd);
					return;
				case XN_BKTAB:
					SetFocus(HWnd);
					return;
				}
			return;
		case IDC_M:
			switch(HIWORD(wP))
				{
				case LBN_DBLCLK:
					i=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
					if(i==-1)
					{
						
						return;
					}
					if(DoMarkEdit(hwnd,&EditorIMP.marks.mark[i],FALSE))
						{
						IMPMarkToTbuf(&EditorIMP.marks.mark[i],
							&EditorIMP.fonts);
						SendMessage(MWnd,LB_DELETESTRING,i,0L);
						SendMessage(MWnd,LB_INSERTSTRING,i,
							(LONG)(LPSTR)tbuf);
						IMPPurgeFonts(&EditorIMP);
						EditorIMP.changed=TRUE;
						//PreWin(IMSInst, EditorIMP, IMSIni);          //--------Linghai-11-24-08---
						}
					SendMessage(MWnd,LB_SETCURSEL,i,0L);
					//SendMessage(PWnd,WM_INITDIALOG,1,0);          //-------------Linghai-11-24-08--
					
					return;

				case XN_DEL:
					EditorDeleteSelectedMark(hwnd,true);
					//PreWin(IMSInst, EditorIMP, IMSIni);          //--------Linghai-11-24-08---
					//SendMessage(PWnd,WM_INITDIALOG,1,0);          //-------------Linghai-11-24-08-
					return;
				case XN_KILLFOCUS:
					MCurSel=(int)SendMessage(MWnd,LB_GETCURSEL,0,0L);
					if(MCurSel==-1)
						MCurSel=0;
					SendMessage(MWnd,LB_SETCURSEL,-1,0L);
					return;
				case XN_SETFOCUS:
					FocusID=IDC_M;
					if(-1==(int)SendMessage(MWnd,LB_GETCURSEL,0,0L))
						SendMessage(MWnd,LB_SETCURSEL,MCurSel,0L);
					return;
				case XN_TAB:
					SetFocus(IMWnd);
					return;
				case XN_BKTAB:
					SetFocus(VWnd);
					return;
				}
			return;
		case IDC_IM:
			switch(HIWORD(wP))
				{
				case IMN_SETFOCUS:
					FocusID=0;
					return;
				case IMN_TAB:
					SetFocus(HWnd);
					return;
				case IMN_BKTAB:
					SetFocus(MWnd);
					return;
				case IMN_ERROR:
					SendMessage(IMWnd,IMM_RESET,0,0);
					break;
				}
			if(AEWnd) // Relay IM notifications to AEWnd if open
				SendMessage(AEWnd,WM_COMMAND,wP,lP);
			else if(VisPosWnd) // Relay IM notifications to VisPosWnd if open
				SendMessage(VisPosWnd,WM_COMMAND,wP,lP);
			return;
		}
	}

/***************************************************************************
	ok = DoVarEdit(parent)

	Runs the appropriate variable editor dialog on editvar.
	Also uses and maintains editisgvar.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	IMPVAR *var = Var to edit.
***************************************************************************/
static BOOL DoVarEdit(HWND parent)
	{
	DLGPROC dp;    // Dialog function instance
	LPSTR dn;      // Dialog name
	BOOL ok;       // Dialog result

	switch(editvar.type)
		{
		case vtKEYED:
			dp=(DLGPROC)KVAREDITDP;
			dn="KVAREDIT";
			break;
		case vtSERIAL:
			dp=(DLGPROC)SVAREDITDP;
			dn="SVAREDIT";
			break;
		case vtDATE:
			dp=(DLGPROC)DATEVAREDITDP;
			dn="DATEVAREDIT";
			break;
		case vtTIME:
			dp=(DLGPROC)TIMEVAREDITDP;
			dn="TIMEVAREDIT";
			break;
		case vtBASED:
			dp=(DLGPROC)BASEDVAREDITDP;
			dn="BASEDVAREDIT";
			break;
		default:
			return 0;
		}
	ok=DialogBox(IMSInst,dn,parent,dp);
	if(ok)
		ok=DoValEdit(parent,&editvar.preval,&editvar);
	return ok;
	}
/***************************************************************************
	ok = DoMarkEdit(parent,mark,issub)

	Runs the appropriate mark editor dialog for the supplied mark.

	BOOL ok = TRUE if user pressed ok.
	HWND parent = Parent window.
	IMPMARK *mark = Mark to edit.
	BOOL issub = True if editing a sub mark
***************************************************************************/
static BOOL DoMarkEdit(HWND parent,IMPMARK *mark,BOOL issub)
	{
	DLGPROC dp;    // Dialog function instance
	LPSTR dn;      // Dialog name
	BOOL ok;       // Dialog result

	EditorIsSub=issub;
	EditorMark=*mark;
	switch(mark->type)
		{
		case mtTEXT:
			dp=(DLGPROC)TEXTMARKEDITDP;
			dn="TEXTMARKEDIT";
			break;
		case mtDEVBC:
			dp=(DLGPROC)DEVBCMARKEDITDP;
			dn="DEVBCMARKEDIT";
			break;
		case mtC2OF5:
			dp=(DLGPROC)C2OF5MARKEDITDP;
			dn="C2OF5MARKEDIT";
			break;
		case mtC39:
			dp=(DLGPROC)C39MARKEDITDP;
			dn="C39MARKEDIT";
			break;
		case mtC93:
			dp=(DLGPROC)C93MARKEDITDP;
			dn="C93MARKEDIT";
			break;
		case mtC128:
			dp=(DLGPROC)C128MARKEDITDP;
			dn="C128MARKEDIT";
			break;
		case mtCUPCEAN:
			dp=(DLGPROC)CUPCEANMARKEDITDP;
			dn="CUPCEANMARKEDIT";
			break;
		case mtBITMAP:
			dp=(DLGPROC)BITMAPMARKEDITDP;
			dn="BITMAPMARKEDIT";
			break;
		case mtMULTILINE:
			dp=(DLGPROC)MULTILINEMARKEDITDP;
			dn="MULTILINEMARKEDIT";
			break;
		case mtCDM:
			if(!DM001IsLinked) // Suppress of DLL not installed.
				return FALSE;
			dp=(DLGPROC)CDMMARKEDITDP;
			dn="CDMMARKEDIT";
			break;
		case mtWINFONT:
			dp=(DLGPROC)WINFONTMARKEDITDP;
			dn="WINFONTMARKEDIT";
			break;
		case mtWMTRX:
			dp=(DLGPROC)WMTRXMARKEDITDP;
			dn="WMTRXMARKEDIT";
			break;
		case mtCQR:
			dp=(DLGPROC)CQRMARKEDITDP;
			dn="CQRMARKEDIT";
			break;
	
		case mtUID:                      //------------------------Linghai-4-9-09----------UID
			if(!DM001IsLinked) // Suppress of DLL not installed.
				return FALSE;
			char f[260];
			char c[260];
			GetPrivateProfileString("UID","FORMAT","",f,260,IMSIni);
			GetPrivateProfileString("UID","CONSTRUCT","",c,260,IMSIni);
			if(!strcmp(f,"06"))
			{
				if(!strcmp(c,"2"))
				{
					dp=(DLGPROC)UID06CON2EDITDP;
					dn="UID06CON2EDIT";
				}
				else if(!strcmp(c,"1"))
				{
					dp=(DLGPROC)UID06CON1EDITDP;
					dn="UID06CON1EDIT";
				}
			}
			else if(!strcmp(f,"12"))
			{
				if(!strcmp(c,"2"))
				{
					dp=(DLGPROC)UID06CON2EDITDP;
					dn="UID06CON2EDIT";
				}
				else if(!strcmp(c,"1"))
				{
					dp=(DLGPROC)UID06CON1EDITDP;
					dn="UID06CON1EDIT";
				}
			}
			else if(!strcmp(f,"05"))
			{
				dp=(DLGPROC)UID05EDITDP;
				dn="UID05EDIT";
			}
			else
			{
				dp=(DLGPROC)CDMMARKEDITDP;
				dn="CDMMARKEDIT";
			}
			break;
			
		case mtPATCH:
			dp=(DLGPROC)PATCHMARKEDITDP;
			dn="PATCHMARKEDIT";
			break;
		case mtCURE:
			dp=(DLGPROC)CURESTEPEDITDP;
			dn="CURESTEPEDIT";
			break;
		case mtCDEV:
			dp=(DLGPROC)CDEVMARKEDITDP;
			dn="CDEVMARKEDIT";
			// Make duplicates of any bsmsbsms
			{			
			for(int i=0;i<EditorMark.cdev.nbsm;i++)
				{
				IMPMARK *m=new IMPMARK;
				*m=*EditorMark.cdev.bsm[i];
				EditorMark.cdev.bsm[i]=m;
				}

			}
			break;
		case mtDEVSYM:
			dp=(DLGPROC)DEVSYMMARKEDITDP;
			dn="DEVSYMMARKEDIT";
			break;
		default:
			return 0;
		}
	if(!issub)
		//BuildPreVals(&EditorIMP);
	ok=DialogBox(IMSInst,dn,parent,(DLGPROC)dp);
	if(!issub)
		//FreePreVals();

	if(EditorMark.type==mtCDEV) // Free unused bsms
		{
		IMPMARK *m = ok ? mark : &EditorMark; // m is the unused mark
		for(int i=0;i<m->cdev.nbsm;i++)
			{
			delete m->cdev.bsm[i];
			m->cdev.bsm[i]=0;
			}
		}
	if(ok)
		*mark=EditorMark;
	return ok;
	}

/***************************************************************************
	ok = EditHeaderEntry(hwnd,h,u,i)

	Displays a dialog to edit the given headers entry at index i.
	If the index is >= IMPNBHAS then the udha's are edited.

	BOOL ok = TRUE if user clicked ok.
	HWND hwnd = Parent window.
	IMPHEADER *h = Header to edit.
	IMPUDHAS  *u = User Defined Header Attributes
	WORD i = index of entry to edit.
***************************************************************************/
static BOOL EditHeaderEntry(HWND hwnd,IMPHEADER *h,IMPUDHAS *u,WORD i)
	{
//	IMPMARKS *m;
	double v[5],r[2];
	switch(i)
		{
		case 0: // Desc
			if(!DoPrompt(hwnd,"Description",IMPLDESC,h->desc))
				return FALSE;
			lstrcpy(h->desc,tbuf);
			return TRUE;

		case 1: // Author
			if(!DoPrompt(hwnd,"Author",IMPLAUTHOR,h->author))
				return FALSE;
			lstrcpy(h->author,tbuf);
			return TRUE;
		case 2: // Fixture
			
			if(!DoFixtureFile(hwnd,"Fixture",100 /*IMPLFIXTURE*/,h->fixture))     //-----------Linghai-11-6-08-----
				return FALSE;
			lstrcpy(h->fixture,tbuf);
			EditXYMode=GetXYMODE(h->fixture);
			return TRUE;
		case 3: // Part Size
			
			r[0]=h->partDiameter.di;
			r[1]=h->partDiameter.hi;
			
			MScaleVect((LPDOUBLE)&r,2,FALSE);
			sprintf(tbuf,"%0.3f",r[0]);
			sprintf(tbuf1,"%0.3f",r[1]);
			if(!DoPromptH(hwnd,"Part Diameter & Height",20,tbuf,tbuf1))
				return FALSE;
			
			h->partSize.x=(h->partDiameter.di)/2;
			h->partSize.y=(h->partDiameter.di)/2;
			h->partSize.z=h->partDiameter.hi;
			ParseVect(tbuf,(LPDOUBLE)&h->partDiameter,2);
			MScaleVect((LPDOUBLE)&h->partDiameter,2,TRUE);
			//ParseVect(tbuf,(LPDOUBLE)&h->partSize,3);
			//MScaleVect((LPDOUBLE)&h->partSize,3,TRUE);
			return TRUE;
			
		case 4: // Inkjet
			if(!DoPrompt(hwnd,"Inkjet",sizeof(IJDEVICENAME)-1,h->inkjet))         //------------Linghai-11-7-08---
				return FALSE;
			lstrcpy(h->inkjet,tbuf);
			inkjetUpdate();
			return TRUE;

		case 5: // origin
			
			v[0]=h->origin.x;
			v[1]=h->origin.y;
			MScaleVect((LPDOUBLE)&v,2,FALSE);
			//sprintf(tbuf,"%0.3f,%0.3f",v[0],v[1]);
			sprintf(tbuf,"%0.3f",v[0]);
			sprintf(tbuf1,"%0.3f",v[1]);
			if(!DoPromptO(hwnd,"Origin",20,tbuf,tbuf1))
				return FALSE;
			ParseVect(tbuf,(LPDOUBLE)&h->origin,2);
			MScaleVect((LPDOUBLE)&h->origin,2,TRUE);
			return TRUE;
		case 6: // rotation
			sprintf(tbuf,"%0.1f",h->rotation);
			if(!DoPrompt(hwnd,"Rotation",20,tbuf))
				return FALSE;
			ParseVect(tbuf,(LPDOUBLE)&h->rotation,1);
			return TRUE;
		case 7: // init
			if(!DoPrompt(hwnd,"Init",IMPLFNBASE,h->initsect))
				return FALSE;
			lstrcpy(h->initsect,tbuf);
			return TRUE;
		case 8: // link
			if(!DoPrompt(hwnd,"Link",IMPLFNBASE,h->link))
				return FALSE;
			lstrcpy(h->link,tbuf);
			return TRUE;
		case 9: // nolinkinc
			h->inconlink=!h->inconlink;
			return TRUE;
		case 10: // Inspector
			if(!DoPrompt(hwnd,"Inspector",IMPLFNBASE,h->inspector))
				return FALSE;
			lstrcpy(h->inspector,tbuf);
			return TRUE;
		case 11: // InspectPos
			return DoInspectPosEdit(hwnd,h,u);
		case 12: // InspectPrep
			if(!DoPrompt(hwnd,"InspectPrep",IMPLVISPREP,h->inspectprep))
				return FALSE;
			lstrcpy(h->inspectprep,tbuf);
			return TRUE;
		case 13: // conveyorstart
			h->conveyorstart=!h->conveyorstart;
			return TRUE;
		case 14: // linkautostart
			h->linkautostart=!h->linkautostart;
			return TRUE;
		case 15: // skiponlinkfail
			h->skiponlinkfail=!h->skiponlinkfail;
			return TRUE;
		case 16: // chainlinkfail
			h->chainlinkfail=!h->chainlinkfail;
			return TRUE;
		case 17: // Reader
			if(!DoPrompt(hwnd,"Reader",IMPLFNBASE,h->reader))
				return FALSE;
			lstrcpy(h->reader,tbuf);
			return TRUE;
		case 18: // ReadPos
			return DoReadPosEdit(hwnd,h,u);
		case 19: // ReadPrep
			if(!DoPrompt(hwnd,"ReadPrep",IMPLVISPREP,h->readprep))
				return FALSE;
			lstrcpy(h->readprep,tbuf);
			return TRUE;
		case 20: // ReadVar
			if(!DoPrompt(hwnd,"ReadVar",IMPLVNAME,h->readvar))
				return FALSE;
			lstrcpy(h->readvar,tbuf);
			return TRUE;
		case 21://Part Diameter
			
			v[0]=h->partDiameter.di;
			v[1]=h->partDiameter.hi;
			
			MScaleVect((LPDOUBLE)&v,2,FALSE);
			sprintf(tbuf,"%0.3f",v[0]);
			sprintf(tbuf1,"%0.3f",v[1]);
			if(!DoPromptH(hwnd,"Part Diameter & Height",20,tbuf,tbuf1))
				return FALSE;

			ParseVect(tbuf,(LPDOUBLE)&h->partDiameter,2);
			MScaleVect((LPDOUBLE)&h->partDiameter,2,TRUE);
			return TRUE;
	
		}
	// Translate BHA index i to UDHA index
	i-=IMPNBHAS;

	// Check range
	if(i>=u->count)
		return FALSE;

	// Edit UDHA i
	return DoUDHAEdit(hwnd,&(u->udha[i]));
	}

/******************************************************************************
	ok = DoUDHAEdit(hwnd,udha)

	Edits a UDHA.

	BOOL ok = True if user ressed OK.
	HWND hwnd = Parent window
	IMPUDHA *udha = IMPUDHA to edit.
******************************************************************************/
static BOOL DoUDHAEdit(HWND hwnd,IMPUDHA *udha)
	{
	DLGPROC dp;    // Dialog function instance
	BOOL ok;       // Dialog result

	editudha=*udha;
	dp=(DLGPROC)UDHAEDITDP;
	ok=DialogBox(IMSInst,"UDHAEDIT",hwnd,(DLGPROC)dp);
	if(ok)
		{
		*udha=editudha;
		if(!lstrcmp(editudha.name,"Part Diameter"))   //sam 1/6/17
		{ 
			char xygDir3[128]; // mps rotary test udha
			strcpy(xygDir3,IMSDir); // mps rotary test udha
			strncat (xygDir3, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRoty","Part Diameter",editudha.val,xygDir3);
			//EditHasRadius=lstrcmp(editudha.val,"0")&&lstrcmp(editudha.val,"");

		}

		if(!lstrcmp(editudha.name,"RotaryGalil")) { // mps rotary test udha
			int mps; // mps rotary test udha
			char xygDir[128]; // mps rotary test udha
			strcpy(xygDir,IMSDir); // mps rotary test udha
			strncat (xygDir, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRoty","RotaryGalil",editudha.val,xygDir);
		}

		if(!lstrcmp(editudha.name,"RotaryGalilJet3Delay")) { // mps rotary test udha
			char xygDir1[128]; // mps rotary test udha
			strcpy(xygDir1,IMSDir); // mps rotary test udha
			strncat (xygDir1, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRoty","RotaryGalilJet3Delay",editudha.val,xygDir1);
		}

		if(!lstrcmp(editudha.name,"RotaryGalilUpsideDown")) { // mps rotary test udha
			char xygDir2[128]; // mps rotary test udha
			strcpy(xygDir2,IMSDir); // mps rotary test udha
			strncat (xygDir2, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRoty","RotaryGalilUpsideDown",editudha.val,xygDir2);
		}

	    if(!lstrcmp(editudha.name,"Rotary")) { // mps rotary test udha
			int Rotary; // mps rotary test udha
			char xygDir7[128]; // mps rotary test udha
			strcpy(xygDir7,IMSDir); // mps rotary test udha
			strncat (xygDir7, "\\\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRottary","Rotary",editudha.val,xygDir7); //sam 06/16
		}

		if(!lstrcmp(editudha.name,"RotarySpeed")) { // mps rotary test udha
			char xygDir4[128]; // mps rotary test udha
			strcpy(xygDir4,IMSDir); // mps rotary test udha
			strncat (xygDir4, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRottary","RotarySpeed",editudha.val,xygDir4);  //sam 06/16
		}

		if(!lstrcmp(editudha.name,"RotaryAcclExtra")) { // mps rotary test udha
			char xygDir5[128]; // mps rotary test udha
			strcpy(xygDir5,IMSDir); // mps rotary test udha
			strncat (xygDir5, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRottary","RotaryAcclExtra",editudha.val,xygDir5);  //sam 06/16
		}

		if(!lstrcmp(editudha.name,"RotaryDcclExtra")) { // mps rotary test udha
			char xygDir6[128]; // mps rotary test udha
			strcpy(xygDir6,IMSDir); // mps rotary test udha
			strncat (xygDir6, "\\XYGRoty.ini", 12); // mps rotary test udha
			WritePrivateProfileString("XYGRottary","RotaryDcclExtra",editudha.val,xygDir6);  //sam 06/16
		}

		}
	return ok;
	}

/******************************************************************************
	ok = DoInspectPosEdit(hwnd,h,u)

	Edits the InspectPos.

	BOOL ok = True if user pressed OK.
	HWND hwnd = Parent window
	IMPHEADER *h = Header of IMP with InspectPos to edit
	IMPUDHAS *u = UDHAS of IMP (for VisionLamp)
******************************************************************************/
static BOOL DoInspectPosEdit(HWND hwnd,IMPHEADER *h,IMPUDHAS *u)
	{
	DLGPROC dp;    // Dialog function instance
	BOOL ok;       // Dialog result
	ClampStart(NULL,u);
	ClampLock();
	VisionLampStart(u);
	dp=(DLGPROC)VISPOSDP;
	EditorVisMode=vmINSPECTOR;
	ok=DialogBoxParam(IMSInst,"VISPOS",hwnd,(DLGPROC)dp,(long)h);
	VisionLampEnd();
	return ok;
	}

/******************************************************************************
	ok = DoReadPosEdit(hwnd,h,u)

	Edits the ReadPos.

	BOOL ok = True if user pressed OK.
	HWND hwnd = Parent window
	IMPHEADER *h = Header of IMP with ReadPos to edit
	IMPUDHAS *u = UDHAS of IMP (for VisionLamp)
******************************************************************************/
static BOOL DoReadPosEdit(HWND hwnd,IMPHEADER *h,IMPUDHAS *u)
	{
	DLGPROC dp;    // Dialog function instance
	BOOL ok;       // Dialog result
	ClampStart(NULL,u);
	ClampLock();
	VisionLampStart(u);
	dp=(DLGPROC)VISPOSDP;
	EditorVisMode=vmREADER;
	ok=DialogBoxParam(IMSInst,"VISPOS",hwnd,dp,(long)h);
	VisionLampEnd();
	return ok;
	}

/******************************************************************************
	EditSelectIMConfig()

	Checks for UDHA IMConfig and if found, attempts IMM_CONFIG
******************************************************************************/
static void EditSelectIMConfig(void)
	{
	for(int i=0;i<EditorIMP.udhas.count;i++)
		{
		IMPUDHA *udha=&(EditorIMP.udhas.udha[i]);
		if(!lstrcmp(udha->name,"IMConfig"))
			{
			SendMessage(IMWnd,IMM_CONFIG,0,(LPARAM)(LPSTR)udha->val);
			break;
			}
		}
	}

/******************************************************************************
	inkjetUpdate()

	Determines if the new named inkjet is attached (EditorIJFound) and
	then checks for unsupported fonts and warns the user.
	This should be called any time the inkjet is changed.
******************************************************************************/
static void inkjetUpdate(void)
{
	EditSelectIMConfig();
	HIJ hij=SendMessage(IMWnd,IMM_FINDIJ,0,(LONG)EditorIMP.pheader->inkjet);
	EditorIJFound=(hij!=-1);
	if(EditorIJFound)
	{
		SendMessage(IMWnd,IMM_SELIJ,hij,0); // select it
		EditorIJDev=(HWND)SendMessage(IMWnd,IMM_GETIJHANDLE,hij,0);
		if(EditorIJDev)
			SendMessage(EditorIJDev,IJM_GETINFO,0,(LPARAM)&EditorIJInfo);
		// Search for unsupported fonts
		int i=EditorIMP.fonts.count;
		while(i--)
			if(SendMessage(IMWnd,IMM_FINDFONT,0,(LONG)(LPSTR)&EditorIMP.fonts.name[i])==-1)
				break;
			if(i>=0)
			{   // If any fonts were not found
				ShowWarning(IDW_NOFONTS,EditorIMP.fonts.name[i]); // Warn user
				for(int m=0;m<EditorIMP.marks.count;m++){
					IMPMARK * mark=&(EditorIMP.marks.mark[m]);
					if (mark->type==mtCDEV){
						ShowWarning(IDW_CDEVBMPFMT,NULL);
						break;
						// Warn about cdev marks as bitmap format may not be valid
					}
				}
			}
	}
	else
		ShowWarning(IDW_NOIJ,EditorIMP.pheader->inkjet); // Warn user inkjet is not attached.
}

/******************************************************************************
	NewMark(idm,hwnd)

	Handles the IDMs for creating new marks.

	WORD idm = Menu id to service
	HWND hwnd = Parent window for dialog.
******************************************************************************/
static void NewMark(WORD idm,HWND hwnd)
	{
	if(EditorIMP.marks.count==IMPMAXMARKS) // If full
		{
		ShowError(IDE_IMPMAXMARKS);
		return;
		}
	IMPMARK *pmark=&(EditorIMP.marks.mark[EditorIMP.marks.count]);
	InitNewMark(pmark,idm);
	// Do the mark's edit dialog
	if(DoMarkEdit(hwnd,pmark,FALSE))
		{
		IMPMarkToTbuf(pmark,&EditorIMP.fonts);
		SendMessage(MWnd,LB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
		EditorIMP.marks.count++;
		IMPPurgeFonts(&EditorIMP); // zzz Killing my font
		EditorIMP.changed=TRUE;

		//SendMessage(PWnd,WM_CLOSE,1,0);               //------Linghai-11-24-08-----
		//PreWin(IMSInst, EditorIMP, IMSIni);          //--------Linghai-11-24-08---
		//PWnd = Pre(IMSInst, EditorIMP, IMSIni,hwnd);
		//EditorWM_CREATE(hwnd);
		//PWnd=PreWindow(IMSInst,EditorIMP,hwnd,IMSIni,IDC_P);                   //------------Linghai-11-21-08---
	    //SendMessage(PWnd,WM_INITDIALOG,1,0);                                //-------------Linghai-11-24-08--
		//SendMessage(PWnd,WM_DESTROY,1,0);
		//SendMessage(PWnd,WM_CTLCOLORDLG,1,0);
		//SendMessage(PWnd,WM_PAINT,1,0);
		//PWnd=PreWindow(IMSInst,EditorIMP,hwnd,IMSIni,IDC_P);                   //------------Linghai-11-21-08---
		//SendMessage(PWnd,WM_INITDIALOG,1,0);                                //-------------Linghai-11-24-08--
		}
	}

/******************************************************************************
	GetXYMODE(fixture)

	Determines the XY Mode based on the fixture file.
	Defaults to imXY if the fixture does not exist.
******************************************************************************/
static IMXYMODE GetXYMODE(char *fixture)
	{
	IMF imf;
	IMXYMODE xymode;

	xymode=imXY; // default
	if(IMSFixtures[0])
		{
		lstrcpy(tbuf,IMSFixtures);
		lstrcat(tbuf,"\\");
		}
	else
		tbuf[0]=0;
	lstrcat(tbuf,fixture);
	strfnextend(tbuf,"IMF");
	// Initialize the ofs for the fixture
	//
	lstrcpy(imf.fn,tbuf);

	if (_access(tbuf,0) != -1)  // If it exists ...
		{
		// Load the fixture
		if(IMFLoad(&imf))
			{
			xymode=imf.xymode;  // Exctract XYMODE
			IMFFree(&imf);
			}
		}
	return xymode;
	}

/******************************************************************************
	DoLocate(hdlg,idcx,idcy,prompt)

	Uses the stdlocator to locate a mark.  Fills in the located coordinate
	to the idcx and idcy controls of the supplied mark dialog.
******************************************************************************/
static void DoLocate(HWND hdlg,int idcx,int idcy,LPSTR prompt)
	{
	if(!StdLoc) return;
	// parse X Y fields to StdLoc->locpos
	if(!GetFloatField(hdlg,idcx,&StdLoc->locpos.x)) return;
	if(!GetFloatField(hdlg,idcy,&StdLoc->locpos.y)) return;
	// Load imf
	IMF imf;
	if(GetIMF(&imf,EditorIMP.pheader->fixture))
		{
		imf.imp=EditorIMP;
		// Compute the frame
		if(GetPWCS(&(StdLoc->locframe),&imf,EditorIMP.pheader,0))
			{
			// Do the stdloc
			StdLoc->locpos.z=imf.extent.z+EditorIMP.pheader->partSize.z;
			StdLocFocusX=GetDlgItem(hdlg,idcx);
			StdLocFocusY=GetDlgItem(hdlg,idcy);
			StdLocFocusZ=0;
			StdLoc->locprompt=prompt;
			StdLoc->open(hdlg,&IMSADWnd); // do the locate
			// StdLoc auto updates idcx and idcy on OK
			}
		IMFFree(&imf);
		}
	else
		ShowError(0);
	}

/******************************************************************************
	SelectCurrentFont(hdlg,idcx,idcy,prompt)

	Sets the selection in the list of fonts for a device mark
******************************************************************************/

void SelectCurrentFont(HWND fontlist, LPSTR fontname){
	IJFONTNAME font1,font2;
	int nfonts = SendMessage(fontlist,CB_GETCOUNT,0,0);
	char *p;
	for (int i=0;i<nfonts;i++){
		SendMessage(fontlist,CB_GETLBTEXT,i,(LPARAM)font1);
		lstrcpy(font2,fontname);
		// Trim off both descs
		p = strchr(font1,':');
		if (p) *p=0;
		p = strchr(font2,':');
		if (p) *p=0;
		// Compare strings and select if equal
		if (!lstrcmp(font1,font2))
			SendMessage(fontlist,CB_SETCURSEL,i,0);
	}
}

