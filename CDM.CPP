/******************************************************************************
	NAME : CDM.C
	DESC : Code UPC and EAN barcode implementation for IMS5002.
******************************************************************************/
#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <memory.h>
#include <imslib32\imslib32.h>
#include <jvmath32\jvmath32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "editor.h"
#include "cdm.h"
#include "lbmp.h"
#include "stdloc.h"

// Check API Compatability
#if (DM_API!=0x3000)  //changed from 0x2000
#error code specific to DM_API 0x3000
#endif

/***************************************************************************
	PUBLIC VARIABLES
***************************************************************************/
BOOL DM001IsLinked=FALSE; // Indicates DM001 is dynalinked
int ISNEWZERO=0;
char textBuf[256];
int lenBuf = 0;

/******************************************************************************
	STATIC VARS
******************************************************************************/
// Dynalinked DM001 API if(DM001IsLinked)
static pfDMRenderBMP DMRenderBMP;
static pfDMGetErrorString DMGetErrorString;
static pfDMGetStyleName DMGetStyleName;
static pfDMGetECCName DMGetECCName;
static pfDMGetFormatName DMGetFormatName;
static HINSTANCE hDM001Lib=0; // DM001 lib handle
/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/***************************************************************************
	linked = DM001Link()

	Attempts to link to DM001.
***************************************************************************/
BOOL DM001Link(void)
	{
//	OFSTRUCT ofs;

	if(DM001IsLinked)
		return TRUE;

	// Construct DLL name in tbuf
	wsprintf(tbuf,"%s\\DM00132.DLL",(LPSTR)IMSDir);

	HANDLE fh;
	fh=CreateFile(tbuf,GENERIC_READ,0,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	// Test open to prevent windows DLL load error if DLL not present
	if(fh==INVALID_HANDLE_VALUE)
		return FALSE;

	CloseHandle(fh);


	hDM001Lib=LoadLibrary(tbuf);
	if((int)hDM001Lib<32)
		{
		hDM001Lib=0;
		DWORD err = GetLastError();
		return FALSE;
		}
	
	// Get API addresses
	DMRenderBMP=(pfDMRenderBMP)GetProcAddress(hDM001Lib,"DMRenderBMP");
	DMRenderBMP=(pfDMRenderBMP)GetProcAddress(hDM001Lib,"DMRenderBMP");
	DMGetErrorString=(pfDMGetErrorString)GetProcAddress(hDM001Lib,
		"DMGetErrorString");
	DMGetStyleName=(pfDMGetStyleName)GetProcAddress(hDM001Lib,
		"DMGetStyleName");
	DMGetECCName=(pfDMGetECCName)GetProcAddress(hDM001Lib,"DMGetECCName");
	DMGetFormatName=(pfDMGetFormatName)GetProcAddress(hDM001Lib,
		"DMGetFormatName");
	DM001IsLinked=TRUE;
	return TRUE;
	}


/***************************************************************************
	DM001UnLink()

	Unlinks the DM001 DLL.
***************************************************************************/
void DM001UnLink(void)
	{
	if(hDM001Lib)
		FreeLibrary(hDM001Lib);
	hDM001Lib=0;
	DM001IsLinked=FALSE;
	}

/******************************************************************************
	hbmp = CDMRenderBitmap(m,t,showerr)

	Renders the supplied text to a bitmap image of its DM barcode
	following the format described in the mark.
	If the input data matches the cache then the cache bitmap itself
	is returned (NOT A COPY).  You should call CDMClearCache() if you
	want to be sure to get a new bitmap.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
	BOOL showerr = TRUE to display erros, FALSE to suppress them
******************************************************************************/
HBITMAP CDMRenderBitmap(IMPMARK *m,LPSTR t,BOOL showerr)
	{
	DWORD err;
	HBITMAP hbmp;
	LPSTR dsc;
	WORD ldsc;

	if(!DM001IsLinked)
		return 0;

	// compute dsc ldsc
	ldsc=5+sizeof(DMINFO)+lstrlen(t)+1;
	dsc=new char[ldsc];
	lstrcpy(dsc,"[CDM]");                        // 5
	memcpy(dsc+5,&(m->cdm.dminfo),sizeof(DMINFO)); // sizeof(DMINFO)
	lstrcpy(dsc+5+sizeof(DMINFO),t);          // lstrlen(t)+1

	// Check for cache hit
	hbmp=LoadedBitmapsFind(dsc,ldsc);
	if(hbmp)
		{
		delete [] dsc;
		return hbmp;
		}

	// Compute new bitmap
	m->cdm.dminfo.data=t;                    // Link ctext
	hbmp=(*DMRenderBMP)(&(m->cdm.dminfo),&err); // DLL Render
	m->cdm.dminfo.data=NULL;                 // Unlink ctext
	if(!hbmp)                    // Handle error
		{
		if(showerr)
			{ // Get error message from DLL
			if(!(*DMGetErrorString)(err,tbuf,TSIZE))
				wsprintf(tbuf,"DM001 : %ld",err); // or make one if it fails
			MessageBox(GetActiveWindow(),tbuf,"DM001",
				MB_OK|MB_ICONEXCLAMATION);
			}
		delete [] dsc;
		return 0;
		}

	// Copy to cache
	LoadedBitmapsAdd(dsc,ldsc,hbmp);

	delete [] dsc;

	return hbmp;
	}
/******************************************************************************
	UIDInit(m)                                                                           -------------Linghai-7-9-09

	Initializes a new IMPUIDMARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void UIDInit(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->rep=0;
	m->type=mtUID;
	
	m->uid.xres=100.0;
	m->uid.yres=100.0;
	m->uid.ms=1;
	m->uid.dminfo.neg=FALSE;
	m->uid.dminfo.xmul=1;
	m->uid.dminfo.ymul=1;
	m->uid.dminfo.xqz=0;
	m->uid.dminfo.yqz=0;
	m->uid.dminfo.format=5;
	m->uid.dminfo.eccval=DM_ECC_200;
	m->uid.dminfo.xflip=FALSE;
	m->uid.dminfo.yflip=FALSE;
	m->uid.dminfo.rows=0;
	m->uid.dminfo.cols=0;
	m->uid.dminfo.style=DM_NORMAL_IMAGE;          
	m->uid.dminfo.border=1;                 // Standard border
	m->uid.dminfo.encrypt=DM_NO_ENCRYPTION; // No encryption
	m->uid.dminfo.data=NULL;                // No compiled data yet.
	m->uid.text[0]=0;                       // Blank source text
	}

/******************************************************************************
	CDMInit(m)

	Initializes a new IMPCDMMARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void CDMInit(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->rep=0;
	m->type=mtCDM;
	m->cdm.xres=100.0;
	m->cdm.yres=100.0;
	m->cdm.ms=1;
	m->cdm.dminfo.neg=FALSE;
	m->cdm.dminfo.xmul=1;
	m->cdm.dminfo.ymul=1;
	m->cdm.dminfo.xqz=0;
	m->cdm.dminfo.yqz=0;
	m->cdm.dminfo.format=5;
	m->cdm.dminfo.eccval=DM_ECC_200;
	m->cdm.dminfo.xflip=FALSE;
	m->cdm.dminfo.yflip=FALSE;
	m->cdm.dminfo.rows=0;
	m->cdm.dminfo.cols=0;
	m->cdm.dminfo.style=DM_NORMAL_IMAGE;          
	m->cdm.dminfo.border=1;                 // Standard border
	m->cdm.dminfo.encrypt=DM_NO_ENCRYPTION; // No encryption
	m->cdm.dminfo.data=NULL;                // No compiled data yet.
	m->cdm.text[0]=0;                       // Blank source text
	}

/******************************************************************************
	CDMMARKEDITDP

	mtCDM editor dialog process.
******************************************************************************/
BOOL CALLBACK CDMMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
		
			int i;
			/*
			char tA[] = "abcdefg";  //---------Linghai-11-20-08
			char textArea[MAX_PATH];
			for(i = 0; i < sizeof(textArea)/sizeof(char); i++)
			{
				textArea[i] = tA[i];
			}//end for loop to get textarea neam
			textArea[i] = '\0';                                                      //-----------Linghai-11-20-08---
			*/
		

			// Initilize the controls
			EditXYModePrep(hdlg);
			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);

			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
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

			// IDC_DEG	--sam	10/7/16
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t);
		
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
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cdm.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cdm.yres/EditYScale);

			// IDC_DMCOLS
			hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.dminfo.cols);

			// IDC_DMROWS
			hctrl=GetDlgItem(hdlg,IDC_DMROWS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.dminfo.rows);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.dminfo.xmul);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.dminfo.ymul);

			// IDC_XQZ
			hctrl=GetDlgItem(hdlg,IDC_XQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.dminfo.xqz);

			// IDC_YQZ
			hctrl=GetDlgItem(hdlg,IDC_YQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.dminfo.yqz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdm.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_DMFORMAT
			hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=DM_MIN_FORMAT;i<=DM_MAX_FORMAT;i++)
			{
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetFormatName)(i));
				
			}
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.cdm.dminfo.format-1,0);

			// IDC_DMECC
			hctrl=GetDlgItem(hdlg,IDC_DMECC);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_ECCS;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetECCName)((DM_ECC_Value)i));
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.cdm.dminfo.eccval,0);

			// IDC_DMSTYLE
			hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_STYLES;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetStyleName)((DM_Style)i));
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.cdm.dminfo.style,0);

			// IDC_DMBORDER
			hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=1;i<=15;i++)
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.cdm.dminfo.border-1,0);

			// IDC_NEG
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.cdm.dminfo.neg);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.cdm.dminfo.xflip);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.cdm.dminfo.yflip);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.cdm.text);
			//SetDlgItemText(hdlg,IDC_WEXT,textArea);                                   
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;

			if(hcontrol) // If from control
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					//case IDC_HINTDMM:                 //------------Linghai-11-17-08--idc_hintdmm begin
					//	MessageBox(hdlg,
					//		"This is Data Matrix Mark Hint:\n"
					//		"It will be updated later.\n"
					//		".....\n","Data Matrix Mark Hint",
					//		MB_OK|MB_ICONINFORMATION);
					//	return TRUE;                   //------------Linghai-11-17-08--idc_hintdmm end
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

						// IDC_DEG	--sam	10/7/16
						/*	hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
						*/
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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.cdm.xres))
							||(EditorMark.cdm.xres < 10.0)
							||(EditorMark.cdm.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.cdm.yres))
							||(EditorMark.cdm.yres < 10.0)
							||(EditorMark.cdm.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.yres*=EditYScale;

						// IDC_DMCOLS
						hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMCOLS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.dminfo.cols=(WORD)dw;

						// IDC_DMROWS
						hctrl=GetDlgItem(hdlg,IDC_DMROWS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMROWS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.dminfo.rows=(WORD)dw;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.dminfo.xmul=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.dminfo.ymul=(WORD)dw;

						// IDC_XQZ
						hctrl=GetDlgItem(hdlg,IDC_XQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.dminfo.xqz=(WORD)dw;

						// IDC_YQZ
						hctrl=GetDlgItem(hdlg,IDC_YQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.dminfo.yqz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdm.ms=(WORD)dw;

						// IDC_DMFORMAT
						hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
						EditorMark.cdm.dminfo.format=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_DMECC
						hctrl=GetDlgItem(hdlg,IDC_DMECC);
						EditorMark.cdm.dminfo.eccval=(DM_ECC_Value)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMSTYLE
						hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
						EditorMark.cdm.dminfo.style=(DM_Style)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMBORDER
						hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
						EditorMark.cdm.dminfo.border=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

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
						EditorMark.cdm.dminfo.neg=IsDlgButtonChecked(
							hdlg,IDC_NEG);

						// IDC_XFLIP
						EditorMark.cdm.dminfo.xflip=IsDlgButtonChecked(
							hdlg,IDC_XFLIP);

						// IDC_YFLIP
						EditorMark.cdm.dminfo.yflip=IsDlgButtonChecked(
							hdlg,IDC_YFLIP);

						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.cdm.text,IMPLCDM+1);
						if(!EditorMark.cdm.text[0])
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDC_WEXT:
						switch(ncode)
							{
							case XN_INS:
								DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
									&EditorIMP.udhas,
									&EditorIMP.vars,
									&EditorIMP.gvars);
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
								SetFocus(GetDlgItem(hdlg,IDC_NEG));
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

/*******************************************************************************
	preview uid text
*******************************************************************************/
void UIDPreText(char text[])
{
	char allText[256];
	lenBuf=0;
	strcpy(allText,"");
	strcpy(textBuf,"");
	bool ISVARIABLE = false;
	int indexofalltext = 0;         //alltext index
	int indexofvar = 0;           //varname index
	
	char varName[256];
	char vartype = 'v';

	if(strcmp(text,""))
	{
		int len = strlen(text);
		for(int i = 0; i < len; i++)
		{
			if(text[i] == '%')        //character is %
			{
				
				if(ISVARIABLE)
				{
					ISVARIABLE = false;
					//varName[indexofvar] = '\0';
					FindValuesByVarName(varName,vartype);
					strcat(allText,textBuf);
					indexofalltext += lenBuf;
					//indexofalltext += 1;
					vartype = 'v';
				}
				else
				{
					ISVARIABLE = true;
					indexofvar = 0;
									
				}

				
			}
			else if(text[i] == '!' && ISVARIABLE)
			{
				vartype = 'u';
			}
			else if(text[i] == '@' && ISVARIABLE)
			{
				vartype = 'g';
			}
			else 
			{
				if(ISVARIABLE)
				{
					varName[indexofvar] = text[i];
					indexofvar += 1;
					varName[indexofvar] = '\0';
				}
				else 
				{
					allText[indexofalltext] = text[i];
					indexofalltext += 1;
					allText[indexofalltext] = '\0';
				}
			}
		}
		//allText[indexofalltext] = '\0';
		//lenBuf=wsprintf(textBuf,"%s",allText);
		strcpy(textBuf,allText);
	}

}

/*******************************************************************************
	copy values to tbuf by variable's name
*******************************************************************************/
void FindValuesByVarName(char name[],char t)
{
	int i = 0;
	int length = 0;
	char varName[256];
	strcpy(varName,name);;

	switch(t)
	{
		case 'u':
			{
				//udhas
				//length = EditorIMP.udhas.count;
				for(i = 0; i < EditorIMP.udhas.count; i++)
				{
					if(!strcmp(varName,EditorIMP.udhas.udha[i].name))
					{
						lenBuf = wsprintf(textBuf,"%s",EditorIMP.udhas.udha[i].val);
						break;
					}
					
				}
			}
			break;

		case 'v':
			{
				//vars
				//length = EditorIMP.vars.count;
				for(i = 0; i < EditorIMP.vars.count; i++)
				{
					if(!strcmp(varName,EditorIMP.vars.var[i].name))
					{
						switch(EditorIMP.vars.var[i].type)
						{
						case vtKEYED:
							lenBuf=wsprintf(textBuf,"%s",EditorIMP.vars.var[i].preval.k);
							//strcpy(textBuf,EditorIMP.vars.var[i].preval.k);
							break;
						case vtSERIAL:
							lenBuf=wsprintf(textBuf,"%lu",EditorIMP.vars.var[i].preval.s);
							//strcpy(textBuf,EditorIMP.vars.var[i].preval.s);
							break;
						case vtDATE:
							{
								lenBuf = GetDate(textBuf,"MM/DD/YY",0,NULL);
								/*
								if(EditorIMP.vars.var[i].preval.d.override[0])
									lenBuf = wsprintf(textBuf,"%s",EditorIMP.vars.var[i].preval.d.override);

								else
									//lenBuf = GetDate(textBuf,EditorIMP.vars.var[i].d.format,EditorIMP.vars.var[i].d.offset,
									//		EditorIMP.vars.var[i].d.base);
								lenBuf = GetDate(textBuf,"MM/DD/YY",0,NULL);
								*/
							}
							break;
						case vtTIME:
							lenBuf=wsprintf(textBuf,"%s",EditorIMP.vars.var[i].preval.t.override);
							//strcpy(textBuf,EditorIMP.vars.var[i].preval.t);
							break;
						case vtBASED:
							lenBuf=wsprintf(textBuf,"%s",EditorIMP.vars.var[i].preval.b);
							//strcpy(textBuf,EditorIMP.vars.var[i].preval.b);
							break;
						}
					}
					//break;
				}
			}
			break;

		case 'g':
			{
				//gvars
				if(!lstrcmp(name,"DATE")) // Date in default format
		lenBuf=GetDate(textBuf,"MM/DD/YY",0,NULL);
	else if(!lstrcmp(name,"TIME")) // Time in default format
		lenBuf=GetTime(textBuf,FALSE,FALSE,FALSE,':',NULL,NULL,FALSE,0,NULL);
	else if(!lstrcmp(name,"USERID"))
		lenBuf=wsprintf(textBuf,"%s",(LPSTR)UserID);
	else if(!lstrcmp(name,"USERNAME"))
		lenBuf=wsprintf(textBuf,"%s",(LPSTR)UserName);
	else if(!lstrcmp(name,"LQTY"))
		lenBuf=wsprintf(textBuf,"%u",AuxLQty);
	else if(!lstrcmp(name,"CYCLESTATUS"))
		lenBuf=wsprintf(textBuf,"%s",CycleFailed ? (LPSTR)"Fail" : (LPSTR)"Pass");
	else if(!lstrcmp(name,"UIDFORMAT5"))
		lenBuf=wsprintf(textBuf,"[)>\x1e""05\x1d");
	else if(!lstrcmp(name,"UIDFORMAT6"))
		lenBuf=wsprintf(textBuf,"[)>\x1e""06\x1d");
	else if(!lstrcmp(name,"UIDSEPARATOR"))
		lenBuf=wsprintf(textBuf,"\x1d");
	else if(!lstrcmp(name,"UIDEND"))
		lenBuf=wsprintf(textBuf,"\x04");
			}
			break;
	}
}

/******************************************************************************
	uidMARKEDITDP

	uid editor dialog process.
******************************************************************************/
BOOL CALLBACK UIDMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
		char SectionCName[] = "COMPANY";
		char SectionUName[] = "UID";
		//char IniFileName[260];
		//strcpy(IniFileName,IMSIni);
	//wsprintf(tbuf,"%s\\DM00132.DLL",(LPSTR)IMSDir);
		//strcpy(IniFileName,"IMS5000.ini");
		//GetCurrentDirectory(260,IniFileName);
		//strcpy(IniFileName,IMSDir);
		//strcat(IniFileName,"\\IMS5000.ini");
		//char IniFileName[] = "C:\\NEW TEST\\ims5000V2\\Debug\\IMS5000.ini";
		//char IniFileName[] = "IMS5000.ini";
		char begin[] = "UID Format";
		char end[]   = "RsEOT";
		char groupSep[] = "Gs";
		char recordSet[] = "Rs";
		char rsV[] = "%30%";
		char gsV[] = "%29%";
		char endV[] = "%4%";
		char first[1024];
		char tem[265];
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
		
			int i;
			
			/*char tA[] = "Try uid code";  //---------Linghai-11-20-08
			char textArea[MAX_PATH];
			for(i = 0; i < sizeof(textArea)/sizeof(char); i++)
			{
				textArea[i] = tA[i];
			}//end for loop to get textarea neam
			textArea[i] = '\0';                                                      //-----------Linghai-11-20-08---
			*/
			

			// Initilize the controls
			EditXYModePrep(hdlg);
			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);

			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
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

			// IDC_DEG	--sam	10/7/16
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t);
			
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
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.yres/EditYScale);

			// IDC_DMCOLS
			hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.cols);

			// IDC_DMROWS
			hctrl=GetDlgItem(hdlg,IDC_DMROWS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.rows);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xmul);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.ymul);

			// IDC_XQZ
			hctrl=GetDlgItem(hdlg,IDC_XQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xqz);

			// IDC_YQZ
			hctrl=GetDlgItem(hdlg,IDC_YQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.yqz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_DMFORMAT
			//char format[] = "5 : ASCII";                                  //-----------------Linghai-7-13-09-
			hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=DM_MIN_FORMAT;i<=DM_MAX_FORMAT;i++)            //---------------------Linghai-5-28-09----
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetFormatName)(i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)format);
			//SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);

			// IDC_DMECC
			//char mecc[] = "ECC 200";
			hctrl=GetDlgItem(hdlg,IDC_DMECC);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_ECCS;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetECCName)((DM_ECC_Value)i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)mecc);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.eccval,0);

			// IDC_DMSTYLE
			hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_STYLES;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetStyleName)((DM_Style)i));
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.style,0);

			// IDC_DMBORDER
			hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=1;i<=15;i++)
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.border-1,0);

			// IDC_NEG
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.uid.dminfo.neg);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.uid.dminfo.xflip);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.uid.dminfo.yflip);

			//IDC_BEGIN                    --------------------------------------------------------Linghai-7-10-09----
			//GetPrivateProfileString(SectionUName,"DUNS","",first,1024,IniFileName);
			SendDlgItemMessage(hdlg,IDC_BEGIN,WM_SETTEXT,0,(LONG)(LPSTR)begin);

			//IDC_UIDFORMAT
			GetPrivateProfileString(SectionUName,"FORMAT","",first,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_UIDFORMAT,WM_SETTEXT,0,(LONG)(LPSTR)first);

			//IDC_ENTERID
			GetPrivateProfileString(SectionUName,"CAGE","",first,1024,IMSIni);
			if(strcmp(first,"0"))
			{
				SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
				GetPrivateProfileString(SectionCName,"CAGECODE","",first,1024,IMSIni);
				SendDlgItemMessage(hdlg,IDC_ENTERIDv,WM_SETTEXT,0,(LONG)(LPSTR)first);
			}
			else
			{
				GetPrivateProfileString(SectionUName,"DUNS","",first,1024,IMSIni);
				if(strcmp(first,"0"))
				{
					SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					GetPrivateProfileString(SectionCName,"DUNS","",first,1024,IMSIni);
					SendDlgItemMessage(hdlg,IDC_ENTERIDv,WM_SETTEXT,0,(LONG)(LPSTR)first);
				}
				else
				{
					GetPrivateProfileString(SectionUName,"GS1","",first,1024,IMSIni);
					if(strcmp(first,"0"))
					{
						SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					}
					else
					{
						GetPrivateProfileString(SectionUName,"DODAAC","",first,1024,IMSIni);
						if(strcmp(first,"0"))
						{
							SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
						}
						else
						{
							GetPrivateProfileString(SectionUName,"OTHER","",first,1024,IMSIni);
							if(strcmp(first,"0"))
							{
								SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
							}
						}
					}
				}
			}

			//SendDlgItemMessage(hdlg,IDC_FIRST,WM_SETTEXT,0,(LONG)(LPSTR)first);
			//SetDlgItemText(hdlg,IDC_FIRST,first);--------------------------------------------------Linghai-7-10-09--

			SendDlgItemMessage(hdlg,IDC_END,WM_SETTEXT,0,(LONG)(LPSTR)end);                    //----Linghai-7-13-09--

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.uid.text);
			//SetDlgItemText(hdlg,IDC_WEXT,textArea);
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;

			if(hcontrol) // If from control
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDC_HINTDMM:                 //------------Linghai-11-17-08--idc_hintdmm begin
						MessageBox(hdlg,
							"This is UID Hint:\n"
							"It will be updated later.\n"
							".....\n","UID Hint",
							MB_OK|MB_ICONINFORMATION);
						return TRUE;                   //------------Linghai-11-17-08--idc_hintdmm end
					case IDC_PRE:                                  //--------------------------Linghai-7-10-09--
						
						//GetDlgItemText(hdlg,IDC_BEGIN,tem,sizeof(tem));
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,rsV);
							strcat(first,tem);
						}
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						SetDlgItemText(hdlg,IDC_WEXT,first);
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

						// IDC_DEG	--sam	10/7/16
						/*	hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
						*/
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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.xres))
							||(EditorMark.uid.xres < 10.0)
							||(EditorMark.uid.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.yres))
							||(EditorMark.uid.yres < 10.0)
							||(EditorMark.uid.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.yres*=EditYScale;

						// IDC_DMCOLS
						hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMCOLS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.cols=(WORD)dw;

						// IDC_DMROWS
						hctrl=GetDlgItem(hdlg,IDC_DMROWS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMROWS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.rows=(WORD)dw;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xmul=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.ymul=(WORD)dw;

						// IDC_XQZ
						hctrl=GetDlgItem(hdlg,IDC_XQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xqz=(WORD)dw;

						// IDC_YQZ
						hctrl=GetDlgItem(hdlg,IDC_YQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.yqz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.ms=(WORD)dw;

						// IDC_DMFORMAT
						hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
						EditorMark.uid.dminfo.format=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_DMECC
						hctrl=GetDlgItem(hdlg,IDC_DMECC);
						EditorMark.uid.dminfo.eccval=(DM_ECC_Value)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMSTYLE
						hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
						EditorMark.uid.dminfo.style=(DM_Style)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMBORDER
						hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
						EditorMark.uid.dminfo.border=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

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
						EditorMark.uid.dminfo.neg=IsDlgButtonChecked(
							hdlg,IDC_NEG);

						// IDC_XFLIP
						EditorMark.uid.dminfo.xflip=IsDlgButtonChecked(
							hdlg,IDC_XFLIP);

						// IDC_YFLIP
						EditorMark.uid.dminfo.yflip=IsDlgButtonChecked(
							hdlg,IDC_YFLIP);


						// IDC_WEXT
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,rsV);
							strcat(first,tem);
						}
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						SetDlgItemText(hdlg,IDC_WEXT,first);

						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.uid.text,IMPLCDM+1);
						if(!EditorMark.cdm.text[0])
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDC_WEXT:
						switch(ncode)
							{
							case XN_INS:
								DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
									&EditorIMP.udhas,
									&EditorIMP.vars,
									&EditorIMP.gvars);
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
								SetFocus(GetDlgItem(hdlg,IDC_NEG));
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


/******************************************************************************
	uid06con2EDITDP

	uid editor dialog process.
******************************************************************************/
void SQURFINDCOLANDROW(int length, int& col, int& row)
{
	if(length > 0 && length <=3)
	{
		col = row = 10;
	}
	else if(length > 3 && length <= 6)
	{
		col = row = 12;
	}
	else if(length > 6 && length <= 10)
	{
		col = row = 14;
	}
	else if(length > 10 && length <= 16)
	{
		col = row = 16;
	}
	else if(length > 16 && length <= 25)
	{
		col = row = 18;
	}
	else if(length > 25 && length <= 31)
	{
		col = row = 20;
	}
	else if(length > 31 && length <= 43)
	{
		col = row = 22;
	}
	else if(length > 43 && length <= 52)
	{
		col = row = 24;
	}
	else if(length > 52 && length <= 64)
	{
		col = row = 26;
	}
	else if(length > 64 && length <= 91)
	{
		col = row = 32;
	}
	else if(length > 91 && length <= 127)
	{
		col = row = 36;
	}
	else if(length > 127 && length <= 169)
	{
		col = row = 40;
	}
	else if(length > 169 && length <= 214)
	{
		col = row = 44;
	}
	else if(length > 214 && length <= 259)
	{
		col = row = 48;
	}
	else if(length > 259 && length <= 304)
	{
		col = row = 52;
	}
	else if(length > 304 && length <= 418)
	{
		col = row = 64;
	}
	else if(length > 418 && length <= 550)
	{
		col = row = 72;
	}
	else if(length > 550 && length <= 682)
	{
		col = row = 80;
	}
	else if(length > 682 && length <= 862)
	{
		col = row = 88;
	}
	else if(length > 862 && length <= 1042)
	{
		col = row = 96;
	}
	else if(length > 1042 && length <= 1222)
	{
		col = row = 104;
	}
	else if(length > 1222 && length <= 1573)
	{
		col = row = 120;
	}
	else if(length > 1573 && length <= 1954)
	{
		col = row = 132;
	}
	else if(length > 1954 && length <= 2335)
	{
		col = row = 144;
	}
	else
	{
	}
}
void RECTFINDCOLANDROW(int length, int& col, int& row)
{
	if(length > 0 && length <= 6)
	{
		col = 18;
		row = 8;
	}
	else if(length > 6 && length <= 13)
	{
		col = 32;
		row = 8;
	}
	else if(length > 13 && length <= 22)
	{
		col = 26;
		row = 12;
	}
	else if(length > 22 && length <= 31)
	{
		col = 36;
		row = 12;
	}
	else if(length > 31 && length <= 46)
	{
		col = 36;
		row = 16;
	}
	else if(length > 46 && length <= 72)
	{
		col = 48;
		row = 16;
	}
	else 
	{
	}
}
BOOL CALLBACK UID06CON2EDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
		char SectionCName[] = "COMPANY";
		char SectionUName[] = "UID";
		char begin[] = "UID Format:";
		char end[]   = "RsEOT";
		char groupSep[] = "Gs";
		char recordSet[] = "Rs";
		char rsV[] = "%30%";
		char gsV[] = "%29%";
		char endV[] = "%4%";
		char first[1024];
		int lengthOfFirst = 0;
		int colsINT = 0;
		int rowsINT = 0;
		char colsCHAR[256];
		char rowsCHAR[256];
		BOOL rect = FALSE;

		char construct[1024];
		char tem[265];
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
		
			int i;

			// Initilize the controls
			EditXYModePrep(hdlg);
			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);

			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
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

			// IDC_DEG	--sam	10/7/16
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t);
			
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
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.yres/EditYScale);

			// IDC_DMCOLS
			hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.cols);

			// IDC_DMROWS
			hctrl=GetDlgItem(hdlg,IDC_DMROWS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.rows);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xmul);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.ymul);

			// IDC_XQZ
			hctrl=GetDlgItem(hdlg,IDC_XQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xqz);

			// IDC_YQZ
			hctrl=GetDlgItem(hdlg,IDC_YQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.yqz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_DMFORMAT
			//char format[] = "5 : ASCII";                                  //-----------------Linghai-7-13-09-
			hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=DM_MIN_FORMAT;i<=DM_MAX_FORMAT;i++)            //---------------------Linghai-5-28-09----
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetFormatName)(i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)format);
			//SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);

			// IDC_DMECC
			//char mecc[] = "ECC 200";
			hctrl=GetDlgItem(hdlg,IDC_DMECC);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_ECCS;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetECCName)((DM_ECC_Value)i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)mecc);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.eccval,0);

			// IDC_DMSTYLE
			hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_STYLES;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetStyleName)((DM_Style)i));
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.style,0);

			// IDC_DMBORDER
			hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=1;i<=15;i++)
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.border-1,0);

			// IDC_NEG
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.uid.dminfo.neg);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.uid.dminfo.xflip);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.uid.dminfo.yflip);

			//IDC_BEGIN                    --------------------------------------------------------Linghai-7-10-09----
			//GetPrivateProfileString(SectionUName,"DUNS","",first,1024,IniFileName);
			SendDlgItemMessage(hdlg,IDC_BEGIN,WM_SETTEXT,0,(LONG)(LPSTR)begin);

			//IDC_UIDFORMAT
			GetPrivateProfileString(SectionUName,"FORMAT","",first,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_UIDFORMAT,WM_SETTEXT,0,(LONG)(LPSTR)first);

			GetPrivateProfileString(SectionUName,"CONSTRUCT","",construct,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_UIICON,WM_SETTEXT,0,(LONG)(LPSTR)construct);

			//IDC_ENTERID
hctrl=GetDlgItem(hdlg,IDC_ENTERIDv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);

			GetPrivateProfileString(SectionUName,"CAGE","",first,1024,IMSIni);
			if(strcmp(first,"0"))
			{
				SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
				GetPrivateProfileString(SectionCName,"CAGECODE","",first,1024,IMSIni);
				SendDlgItemMessage(hdlg,IDC_ENTERIDv,WM_SETTEXT,0,(LONG)(LPSTR)first);
			}
			else
			{
				GetPrivateProfileString(SectionUName,"DUNS","",first,1024,IMSIni);
				if(strcmp(first,"0"))
				{
					SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					GetPrivateProfileString(SectionCName,"DUNS","",first,1024,IMSIni);
					SendDlgItemMessage(hdlg,IDC_ENTERIDv,WM_SETTEXT,0,(LONG)(LPSTR)first);
				}
				else
				{
					GetPrivateProfileString(SectionUName,"GS1","",first,1024,IMSIni);
					if(strcmp(first,"0"))
					{
						SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					}
					else
					{
						GetPrivateProfileString(SectionUName,"DODAAC","",first,1024,IMSIni);
						if(strcmp(first,"0"))
						{
							SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
						}
						else
						{
							GetPrivateProfileString(SectionUName,"OTHER","",first,1024,IMSIni);
							if(strcmp(first,"0"))
							{
								SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
							}
							else
							{
								MessageBox(NULL,"You should choose one EID",NULL,MB_OK);
								return TRUE;
							}
						}
					}
				}
			}

			GetPrivateProfileString(SectionUName,"Part Num","",first,1024,IMSIni);
			if(strcmp(first,"0"))
			{
				SendDlgItemMessage(hdlg,IDC_PARTN,WM_SETTEXT,0,(LONG)(LPSTR)first);
			}
			else 
			{
				GetPrivateProfileString(SectionUName,"Lot/Batch","",first,1024,IMSIni);
				if(strcmp(first,"0"))
				{
					SendDlgItemMessage(hdlg,IDC_PARTN,WM_SETTEXT,0,(LONG)(LPSTR)first);
				}
			}
			
			//IDC_PARTNv
hctrl=GetDlgItem(hdlg,IDC_PARTNv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);

			//testing--sam
			GetPrivateProfileString(SectionUName,"SERNPIN","",first,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_PIN,WM_SETTEXT,0,(LONG)(LPSTR)first);
			//IDC_PINv
hctrl=GetDlgItem(hdlg,IDC_PINv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);

			//SendDlgItemMessage(hdlg,IDC_FIRST,WM_SETTEXT,0,(LONG)(LPSTR)first);
			//SetDlgItemText(hdlg,IDC_FIRST,first);--------------------------------------------------Linghai-7-10-09--

			//SendDlgItemMessage(hdlg,IDC_END,WM_SETTEXT,0,(LONG)(LPSTR)end);                    //----Linghai-7-13-09--

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.uid.text);
			//SetDlgItemText(hdlg,IDC_WEXT,textArea);
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;

			if(hcontrol) // If from control
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDC_HINTDMM:                 //------------Linghai-11-17-08--idc_hintdmm begin
						MessageBox(hdlg,
							"This is UID Hint:\n"
							"It will be updated later.\n"
							".....\n","UID Hint",
							MB_OK|MB_ICONINFORMATION);
						return TRUE;                   //------------Linghai-11-17-08--idc_hintdmm end
					case IDC_PRE:                                  //--------------------------Linghai-7-10-09--
						
						//GetDlgItemText(hdlg,IDC_BEGIN,tem,sizeof(tem));
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_UIDFORMAT,tem,sizeof(tem));
						strcat(first,tem);
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
							//strcat(first,EditorIMP.udhas.udha[1].val);
						}
						
						GetDlgItemText(hdlg,IDC_PARTN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}

						//
						GetDlgItemText(hdlg,IDC_PARTNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
							//strcat(first,EditorIMP.udhas.udha[1].name);
						}
						
						//testing--sam
						GetDlgItemText(hdlg,IDC_PIN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PINv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
							//strcat(first,EditorIMP.udhas.udha[1].name);
						}

						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						//FindValuesByVarName("abc");
						//strcat(first,tbuf);
						
						lengthOfFirst = strlen(first);
						SetDlgItemText(hdlg,IDC_WEXT,first);
						rect = IsDlgButtonChecked(hdlg,IDC_RECT) == BST_CHECKED;
						if(rect)
						{
							RECTFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);

						}
						else
						{
							SQURFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);
						}
						itoa(colsINT,colsCHAR,10);
						itoa(rowsINT,rowsCHAR,10);
						SetDlgItemText(hdlg,IDC_DMCOLS,colsCHAR);
						SetDlgItemText(hdlg,IDC_DMROWS,rowsCHAR);
						return TRUE;
					case IDOK:
						//SendMessage(hdlg,WM_COMMAND,IDC_PRE,1);
						// Validate each control and retrieve its value
					//GetDlgItemText(hdlg,IDC_BEGIN,tem,sizeof(tem));
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_UIDFORMAT,tem,sizeof(tem));
						strcat(first,tem);
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}

						GetDlgItemText(hdlg,IDC_PARTN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PARTNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
							
						}
						//testing--sam
						GetDlgItemText(hdlg,IDC_PIN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PINv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}
						
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						SetDlgItemText(hdlg,IDC_WEXT,first);
						/*
						lengthOfFirst = strlen(first);
						rect = IsDlgButtonChecked(hdlg,IDC_RECT) == BST_CHECKED;
						if(rect)
						{
							RECTFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);

						}
						else
						{
							SQURFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);
						}
						itoa(colsINT,colsCHAR,10);
						itoa(rowsINT,rowsCHAR,10);
SetDlgItemText(hdlg,IDC_DMCOLS,colsCHAR);
SetDlgItemText(hdlg,IDC_DMROWS,rowsCHAR);
*/
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

						// IDC_DEG	--sam	10/7/16
							/*hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
						*/
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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.xres))
							||(EditorMark.uid.xres < 10.0)
							||(EditorMark.uid.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.yres))
							||(EditorMark.uid.yres < 10.0)
							||(EditorMark.uid.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.yres*=EditYScale;

						// IDC_DMCOLS
						hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMCOLS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.cols=(WORD)dw;

						// IDC_DMROWS
						hctrl=GetDlgItem(hdlg,IDC_DMROWS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMROWS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.rows=(WORD)dw;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xmul=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.ymul=(WORD)dw;

						// IDC_XQZ
						hctrl=GetDlgItem(hdlg,IDC_XQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xqz=(WORD)dw;

						// IDC_YQZ
						hctrl=GetDlgItem(hdlg,IDC_YQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.yqz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.ms=(WORD)dw;

						// IDC_DMFORMAT
						hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
						EditorMark.uid.dminfo.format=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_DMECC
						hctrl=GetDlgItem(hdlg,IDC_DMECC);
						EditorMark.uid.dminfo.eccval=(DM_ECC_Value)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMSTYLE
						hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
						EditorMark.uid.dminfo.style=(DM_Style)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMBORDER
						hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
						EditorMark.uid.dminfo.border=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

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
						EditorMark.uid.dminfo.neg=IsDlgButtonChecked(
							hdlg,IDC_NEG);

						// IDC_XFLIP
						EditorMark.uid.dminfo.xflip=IsDlgButtonChecked(
							hdlg,IDC_XFLIP);

						// IDC_YFLIP
						EditorMark.uid.dminfo.yflip=IsDlgButtonChecked(
							hdlg,IDC_YFLIP);


						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.uid.text,IMPLCDM+1);
						if(!EditorMark.cdm.text[0])
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;
					    
					case IDC_PARTNv:
						{
						if(ncode == XN_INS)
						DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_PARTNv),
									&EditorIMP.vars,
									&EditorIMP.gvars);
						return TRUE;
						}
						
					//testing--sam	
						
					case IDC_PINv:
						{
						if(ncode == XN_INS)
						DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_PINv),
									
									&EditorIMP.vars,
									&EditorIMP.gvars);
						return TRUE;
						}

					case IDC_WEXT:
						switch(ncode)
							{
							case XN_INS:
								DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
									&EditorIMP.udhas,
									&EditorIMP.vars,
									&EditorIMP.gvars);
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
								SetFocus(GetDlgItem(hdlg,IDC_NEG));
								return TRUE;
							case XN_ENTER:
								SendMessage(hdlg,WM_COMMAND,IDOK,1);
								return TRUE;
							}
					}
				}
			else // From text insert menu
				{
				//hctrl=GetDlgItem(hdlg,IDC_WEXT);
					hctrl=GetCapture();
				if(ProcessInsertsPopup(wP,hctrl))
				{
					ReleaseCapture();
					return TRUE;
				}
				}
			break;
			}
		}
	return FALSE;
	}

/******************************************************************************
	uid06con1EDITDP

	uid editor dialog process.
******************************************************************************/
BOOL CALLBACK UID06CON1EDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
		char SectionCName[] = "COMPANY";
		char SectionUName[] = "UID";
		char begin[] = "UID Format";
		char end[]   = "RsEOT";
		char groupSep[] = "Gs";
		char recordSet[] = "Rs";
		char rsV[] = "%30%";
		char gsV[] = "%29%";
		char endV[] = "%4%";
		char first[1024];
		int lengthOfFirst = 0;
		int colsINT = 0;
		int rowsINT = 0;
		char colsCHAR[256];
		char rowsCHAR[256];
		BOOL rect = FALSE;

		char construct[1024];
		char tem[265];
	switch(message)
		{
	
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
		
			int i;
			
			
			// Initilize the controls
			EditXYModePrep(hdlg);
			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);

			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
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

			// IDC_DEG	--sam	10/7/16
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t);
			
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
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.yres/EditYScale);

			// IDC_DMCOLS
			hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.cols);

			// IDC_DMROWS
			hctrl=GetDlgItem(hdlg,IDC_DMROWS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.rows);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xmul);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.ymul);

			// IDC_XQZ
			hctrl=GetDlgItem(hdlg,IDC_XQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xqz);

			// IDC_YQZ
			hctrl=GetDlgItem(hdlg,IDC_YQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.yqz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_DMFORMAT
			//char format[] = "5 : ASCII";                                  //-----------------Linghai-7-13-09-
			hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=DM_MIN_FORMAT;i<=DM_MAX_FORMAT;i++)            //---------------------Linghai-5-28-09----
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetFormatName)(i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)format);
			//SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);

			// IDC_DMECC
			//char mecc[] = "ECC 200";
			hctrl=GetDlgItem(hdlg,IDC_DMECC);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_ECCS;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetECCName)((DM_ECC_Value)i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)mecc);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.eccval,0);

			// IDC_DMSTYLE
			hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_STYLES;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetStyleName)((DM_Style)i));
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.style,0);

			// IDC_DMBORDER
			hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=1;i<=15;i++)
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.border-1,0);

			// IDC_NEG
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.uid.dminfo.neg);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.uid.dminfo.xflip);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.uid.dminfo.yflip);

			//IDC_BEGIN                    --------------------------------------------------------Linghai-7-10-09----
			//GetPrivateProfileString(SectionUName,"DUNS","",first,1024,IniFileName);
			SendDlgItemMessage(hdlg,IDC_BEGIN,WM_SETTEXT,0,(LONG)(LPSTR)begin);

			//IDC_UIDFORMAT
			GetPrivateProfileString(SectionUName,"FORMAT","0",first,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_UIDFORMAT,WM_SETTEXT,0,(LONG)(LPSTR)first);

			char first06[] = "EID CAGE+S/N OR EID IAC+DUNS+S/N";
			char second06[] = "CURR P/N";
			char first12[] = "EID";
			char second12[] = "Serial Number within EID";

			if(!strcmp(first,"06"))
			{
				SendDlgItemMessage(hdlg,IDC_UID06CON1FIRST,WM_SETTEXT,0,(LONG)(LPSTR)first06);
				SendDlgItemMessage(hdlg,IDC_UID06CON1SECOND,WM_SETTEXT,0,(LONG)(LPSTR)second06);
			}
			if(!strcmp(first,"12"))
			{
				SendDlgItemMessage(hdlg,IDC_UID06CON1FIRST,WM_SETTEXT,0,(LONG)(LPSTR)first12);
				SendDlgItemMessage(hdlg,IDC_UID06CON1SECOND,WM_SETTEXT,0,(LONG)(LPSTR)second12);
			}

			GetPrivateProfileString(SectionUName,"CONSTRUCT","0",construct,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_UIICON,WM_SETTEXT,0,(LONG)(LPSTR)construct);

			//IDC_ENTERID
			GetPrivateProfileString(SectionUName,"Comp UII","0",first,1024,IMSIni);
			if(strcmp(first,"0"))
			{
				SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
			}
			else
			{
				GetPrivateProfileString(SectionUName,"UIIIAC","0",first,1024,IMSIni);
				if(strcmp(first,"0"))
				{
					SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
				}
				else
				{
					GetPrivateProfileString(SectionUName,"GS1","0",first,1024,IMSIni);
					if(strcmp(first,"0"))
					{
						SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					}
					
				}
			}

			GetPrivateProfileString(SectionUName,"CAGE","0",first,1024,IMSIni);
			if(strcmp(first,"0"))
			{
				SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
				GetPrivateProfileString(SectionCName,"CAGECODE","0",first,1024,IMSIni);
				SendDlgItemMessage(hdlg,IDC_ENTERIDv,WM_SETTEXT,0,(LONG)(LPSTR)first);
			}
			else
			{
				GetPrivateProfileString(SectionUName,"DUNS","0",first,1024,IMSIni);
				if(strcmp(first,"0"))
				{
					SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					GetPrivateProfileString(SectionCName,"DUNS","",first,1024,IMSIni);
					SendDlgItemMessage(hdlg,IDC_ENTERIDv,WM_SETTEXT,0,(LONG)(LPSTR)first);
				}
				else
				{
					GetPrivateProfileString(SectionUName,"GS1","0",first,1024,IMSIni);
					if(strcmp(first,"0"))
					{
						SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
					}
					
				}
			}

			//IDC_ENTERIDv
hctrl=GetDlgItem(hdlg,IDC_ENTERIDv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);

			GetPrivateProfileString(SectionUName,"SERNEID","0",first,1024,IMSIni);

			if(strcmp(first,"0"))
			{
				SendDlgItemMessage(hdlg,IDC_PARTN,WM_SETTEXT,0,(LONG)(LPSTR)first);
				
			}
			else
			{
				GetPrivateProfileString(SectionUName,"CPN","0",first,1024,IMSIni);
				if(strcmp(first,"0"))
				{
					SendDlgItemMessage(hdlg,IDC_PARTN,WM_SETTEXT,0,(LONG)(LPSTR)first);
				}
			}

			//IDC_PARTNv
hctrl=GetDlgItem(hdlg,IDC_PARTNv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.uid.text);
			//SetDlgItemText(hdlg,IDC_WEXT,textArea);
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;

			if(hcontrol) // If from control
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDC_HINTDMM:                 //------------Linghai-11-17-08--idc_hintdmm begin
						MessageBox(hdlg,
							"This is UID Hint:\n"
							"It will be updated later.\n"
							".....\n","UID Hint",
							MB_OK|MB_ICONINFORMATION);
						return TRUE;                   //------------Linghai-11-17-08--idc_hintdmm end
					case IDC_PRE:                                  //--------------------------Linghai-7-10-09--
						
						//GetDlgItemText(hdlg,IDC_BEGIN,tem,sizeof(tem));
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_UIDFORMAT,tem,sizeof(tem));
						strcat(first,tem);
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
						}
						
						GetDlgItemText(hdlg,IDC_PARTN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PARTNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
						}
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						lengthOfFirst = strlen(first);
						SetDlgItemText(hdlg,IDC_WEXT,first);
						rect = IsDlgButtonChecked(hdlg,IDC_RECT) == BST_CHECKED;
						if(rect)
						{
							RECTFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);

						}
						else
						{
							SQURFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);
						}
						itoa(colsINT,colsCHAR,10);
						itoa(rowsINT,rowsCHAR,10);
SetDlgItemText(hdlg,IDC_DMCOLS,colsCHAR);
SetDlgItemText(hdlg,IDC_DMROWS,rowsCHAR);
						return TRUE;
					case IDOK:
						// Validate each control and retrieve its value
						//IDC_WEXT,IDC_RECT AND IDC_DMROWS IDC_DMCOLS
						//SendMessage(hdlg,WM_COMMAND,IDC_PRE,1);
						
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_UIDFORMAT,tem,sizeof(tem));
						strcat(first,tem);
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}
						
						GetDlgItemText(hdlg,IDC_PARTN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PARTNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						SetDlgItemText(hdlg,IDC_WEXT,first);
					
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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.xres))
							||(EditorMark.uid.xres < 10.0)
							||(EditorMark.uid.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.yres))
							||(EditorMark.uid.yres < 10.0)
							||(EditorMark.uid.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.yres*=EditYScale;

						// IDC_DMCOLS
						hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMCOLS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.cols=(WORD)dw;

						// IDC_DMROWS
						hctrl=GetDlgItem(hdlg,IDC_DMROWS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMROWS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.rows=(WORD)dw;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xmul=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.ymul=(WORD)dw;

						// IDC_XQZ
						hctrl=GetDlgItem(hdlg,IDC_XQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xqz=(WORD)dw;

						// IDC_YQZ
						hctrl=GetDlgItem(hdlg,IDC_YQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.yqz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.ms=(WORD)dw;

						// IDC_DMFORMAT
						hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
						EditorMark.uid.dminfo.format=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_DMECC
						hctrl=GetDlgItem(hdlg,IDC_DMECC);
						EditorMark.uid.dminfo.eccval=(DM_ECC_Value)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMSTYLE
						hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
						EditorMark.uid.dminfo.style=(DM_Style)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMBORDER
						hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
						EditorMark.uid.dminfo.border=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

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
						EditorMark.uid.dminfo.neg=IsDlgButtonChecked(
							hdlg,IDC_NEG);

						// IDC_XFLIP
						EditorMark.uid.dminfo.xflip=IsDlgButtonChecked(
							hdlg,IDC_XFLIP);

						// IDC_YFLIP
						EditorMark.uid.dminfo.yflip=IsDlgButtonChecked(
							hdlg,IDC_YFLIP);


						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.uid.text,IMPLCDM+1);
						if(!EditorMark.cdm.text[0])
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---

						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDC_PARTNv:
						{
						if(ncode == XN_INS)
						DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_PARTNv),
									
									&EditorIMP.vars,
									&EditorIMP.gvars);
						return TRUE;
}

					case IDC_WEXT:
						switch(ncode)
							{
							case XN_INS:
								DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
									&EditorIMP.udhas,
									&EditorIMP.vars,
									&EditorIMP.gvars);
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
								SetFocus(GetDlgItem(hdlg,IDC_NEG));
								return TRUE;
							case XN_ENTER:
								SendMessage(hdlg,WM_COMMAND,IDOK,1);
								return TRUE;
							}
					}
				}
			else // From text insert menu
				{
				//hctrl=GetDlgItem(hdlg,IDC_WEXT);
					hctrl=GetCapture();
				if(ProcessInsertsPopup(wP,hctrl))
				{
					ReleaseCapture();
					return TRUE;
				}
				}
			break;
			}
		}
	return FALSE;
	}

/******************************************************************************
	uid05EDITDP

	uid editor dialog process.
******************************************************************************/
BOOL CALLBACK UID05EDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
		char SectionCName[] = "COMPANY";
		char SectionUName[] = "UID";
		char begin[] = "UID Format";
		char end[]   = "RsEOT";
		char groupSep[] = "Gs";
		char recordSet[] = "Rs";
		char rsV[] = "%30%";
		char gsV[] = "%29%";
		char endV[] = "%4%";
		char first[1024];

		int lengthOfFirst = 0;
		int colsINT = 0;
		int rowsINT = 0;
		char colsCHAR[256];
		char rowsCHAR[256];
		BOOL rect = FALSE;

		char construct[1024];
		char tem[265];
		 
	switch(message)
		{

		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
		
			int i;
			
			
			// Initilize the controls
			EditXYModePrep(hdlg);
			// IDC_X
			hctrl=GetDlgItem(hdlg,IDC_X);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.origin.x*EditXScale);

			// IDC_Y
			hctrl=GetDlgItem(hdlg,IDC_Y);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;	// format -99.999
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

			// IDC_DEG	--sam	10/7/16
			hctrl=GetDlgItem(hdlg,IDC_DEG);
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t);
			
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
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.uid.yres/EditYScale);

			// IDC_DMCOLS
			hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.cols);

			// IDC_DMROWS
			hctrl=GetDlgItem(hdlg,IDC_DMROWS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.rows);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xmul);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.ymul);

			// IDC_XQZ
			hctrl=GetDlgItem(hdlg,IDC_XQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.xqz);

			// IDC_YQZ
			hctrl=GetDlgItem(hdlg,IDC_YQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.dminfo.yqz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.uid.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_DMFORMAT
			//char format[] = "5 : ASCII";                                  //-----------------Linghai-7-13-09-
			hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=DM_MIN_FORMAT;i<=DM_MAX_FORMAT;i++)            //---------------------Linghai-5-28-09----
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetFormatName)(i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)format);
			//SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.format-1,0);

			// IDC_DMECC
			//char mecc[] = "ECC 200";
			hctrl=GetDlgItem(hdlg,IDC_DMECC);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_ECCS;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetECCName)((DM_ECC_Value)i));
			//SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)mecc);
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.eccval,0);

			// IDC_DMSTYLE
			hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=0;i<DM_NUM_STYLES;i++)
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(*DMGetStyleName)((DM_Style)i));
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.style,0);

			// IDC_DMBORDER
			hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
			SendMessage(hctrl,CB_RESETCONTENT,0,0);
			for(i=1;i<=15;i++)
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.uid.dminfo.border-1,0);

			// IDC_NEG
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.uid.dminfo.neg);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.uid.dminfo.xflip);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.uid.dminfo.yflip);

			//IDC_BEGIN                    --------------------------------------------------------Linghai-7-10-09----
			//GetPrivateProfileString(SectionUName,"DUNS","",first,1024,IniFileName);
			SendDlgItemMessage(hdlg,IDC_BEGIN,WM_SETTEXT,0,(LONG)(LPSTR)begin);

			//IDC_UIDFORMAT
			GetPrivateProfileString(SectionUName,"FORMAT","0",first,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_UIDFORMAT,WM_SETTEXT,0,(LONG)(LPSTR)first);

			//GetPrivateProfileString(SectionUName,"CONSTRUCT","0",construct,1024,IMSIni);
			//SendDlgItemMessage(hdlg,IDC_UIICON,WM_SETTEXT,0,(LONG)(LPSTR)construct);

			//IDC_ENTERID
			GetPrivateProfileString(SectionUName,"GIAI","0",first,1024,IMSIni);
			SendDlgItemMessage(hdlg,IDC_LOTBN,WM_SETTEXT,0,(LONG)(LPSTR)first);
			

			GetPrivateProfileString(SectionUName,"UIDF5EID","0",first,1024,IMSIni);
SendDlgItemMessage(hdlg,IDC_ENTERID,WM_SETTEXT,0,(LONG)(LPSTR)first);
//IDC_ENTERIDv
hctrl=GetDlgItem(hdlg,IDC_ENTERIDv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);

			GetPrivateProfileString(SectionUName,"UIDF5SN","0",first,1024,IMSIni);
SendDlgItemMessage(hdlg,IDC_PARTN,WM_SETTEXT,0,(LONG)(LPSTR)first);

//IDC_PARTNv
hctrl=GetDlgItem(hdlg,IDC_PARTNv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);

			//testing--sam

GetPrivateProfileString(SectionUName,"CPN","0",first,1024,IMSIni);
SendDlgItemMessage(hdlg,IDC_PIN,WM_SETTEXT,0,(LONG)(LPSTR)first);

			//IDC_PINv
hctrl=GetDlgItem(hdlg,IDC_PINv);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			
			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCDM;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.uid.text);
			//SetDlgItemText(hdlg,IDC_WEXT,textArea);
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;

			if(hcontrol) // If from control
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDC_HINTDMM:                 //------------Linghai-11-17-08--idc_hintdmm begin
						MessageBox(hdlg,
							"This is UID Hint:\n"
							"It will be updated later.\n"
							".....\n","UID Hint",
							MB_OK|MB_ICONINFORMATION);
						return TRUE;                   //------------Linghai-11-17-08--idc_hintdmm end
					case IDC_PRE:                                  //--------------------------Linghai-7-10-09--
						
						//GetDlgItemText(hdlg,IDC_BEGIN,tem,sizeof(tem));
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_UIDFORMAT,tem,sizeof(tem));
						strcat(first,tem);
						GetDlgItemText(hdlg,IDC_LOTBN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_LOTBNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
						}
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
						}

						GetDlgItemText(hdlg,IDC_PARTN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PARTNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
						}
						//testing---sam						
						GetDlgItemText(hdlg,IDC_PIN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PINv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							UIDPreText(tem);
							strcat(first,textBuf);
						}
						
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						SetDlgItemText(hdlg,IDC_WEXT,first);
						rect = IsDlgButtonChecked(hdlg,IDC_RECT) == BST_CHECKED;
						lengthOfFirst = strlen(first);
						if(rect)
						{
							RECTFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);

						}
						else
						{
							SQURFINDCOLANDROW(lengthOfFirst,colsINT,rowsINT);
						}
						itoa(colsINT,colsCHAR,10);
						itoa(rowsINT,rowsCHAR,10);
SetDlgItemText(hdlg,IDC_DMCOLS,colsCHAR);
SetDlgItemText(hdlg,IDC_DMROWS,rowsCHAR);
						return TRUE;
					case IDOK:
						// Validate each control and retrieve its value
						//SendMessage(hdlg,WM_COMMAND,IDC_PRE,1);
					
						// IDC_X
		//GetDlgItemText(hdlg,IDC_BEGIN,tem,sizeof(tem));
						
						strcpy(first,"[)>");
						strcat(first,rsV);
						GetDlgItemText(hdlg,IDC_UIDFORMAT,tem,sizeof(tem));
						strcat(first,tem);
						GetDlgItemText(hdlg,IDC_LOTBN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_LOTBNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERID,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_ENTERIDv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}

						GetDlgItemText(hdlg,IDC_PARTN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PARTNv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}

						//testing--sam-06/23
						GetDlgItemText(hdlg,IDC_PIN,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							strcat(first,gsV);
							strcat(first,tem);
						}
						GetDlgItemText(hdlg,IDC_PINv,tem,sizeof(tem));
						if(strcmp(tem,""))
						{
							//strcat(first,rsV);
							strcat(first,tem);
						}
						
						//strcat(first,tem);
						//GetDlgItemText(hdlg,IDC_END,tem,sizeof(tem));
						strcat(first,rsV);
						strcat(first,endV);
						SetDlgItemText(hdlg,IDC_WEXT,first);
						
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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.xres))
							||(EditorMark.uid.xres < 10.0)
							||(EditorMark.uid.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.uid.yres))
							||(EditorMark.uid.yres < 10.0)
							||(EditorMark.uid.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.yres*=EditYScale;

						// IDC_DMCOLS
						hctrl=GetDlgItem(hdlg,IDC_DMCOLS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMCOLS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.cols=(WORD)dw;

						// IDC_DMROWS
						hctrl=GetDlgItem(hdlg,IDC_DMROWS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADDMROWS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.rows=(WORD)dw;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xmul=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.ymul=(WORD)dw;

						// IDC_XQZ
						hctrl=GetDlgItem(hdlg,IDC_XQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.xqz=(WORD)dw;

						// IDC_YQZ
						hctrl=GetDlgItem(hdlg,IDC_YQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.dminfo.yqz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.uid.ms=(WORD)dw;

						// IDC_DMFORMAT
						hctrl=GetDlgItem(hdlg,IDC_DMFORMAT);
						EditorMark.uid.dminfo.format=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_DMECC
						hctrl=GetDlgItem(hdlg,IDC_DMECC);
						EditorMark.uid.dminfo.eccval=(DM_ECC_Value)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMSTYLE
						hctrl=GetDlgItem(hdlg,IDC_DMSTYLE);
						EditorMark.uid.dminfo.style=(DM_Style)SendMessage(
							hctrl,CB_GETCURSEL,0,0);

						// IDC_DMBORDER
						hctrl=GetDlgItem(hdlg,IDC_DMBORDER);
						EditorMark.uid.dminfo.border=1+(short)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

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
						EditorMark.uid.dminfo.neg=IsDlgButtonChecked(
							hdlg,IDC_NEG);

						// IDC_XFLIP
						EditorMark.uid.dminfo.xflip=IsDlgButtonChecked(
							hdlg,IDC_XFLIP);

						// IDC_YFLIP
						EditorMark.uid.dminfo.yflip=IsDlgButtonChecked(
							hdlg,IDC_YFLIP);


						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.uid.text,IMPLCDM+1);
						if(!EditorMark.cdm.text[0])
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}
						EndDialog(hdlg,TRUE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);
						//SendMessage(hdlg,WM_INITDIALOG,1,0);               //------------Linghai-11-24-08---
						return TRUE;
						
					case IDC_ENTERIDv:
						
							if(ncode == XN_INS)
								DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_ENTERIDv),
									
									&EditorIMP.vars,
									&EditorIMP.gvars);
								return TRUE;
										
					case IDC_PARTNv:
						
							if(ncode == XN_INS)
						//case XN_INS:
						DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_PARTNv),
									
									&EditorIMP.vars,
									&EditorIMP.gvars);
							return TRUE;
              //testing--sam
						
					case IDC_PINv:
						
						
						if(ncode == XN_INS)
DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_PINv),
									
									&EditorIMP.vars,
									&EditorIMP.gvars);
						
					
						return TRUE;


					case IDC_WEXT:
						switch(ncode)
							{
							case XN_INS:
								DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
									&EditorIMP.udhas,
									&EditorIMP.vars,
									&EditorIMP.gvars);
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
								SetFocus(GetDlgItem(hdlg,IDC_NEG));
								return TRUE;
							case XN_ENTER:
								SendMessage(hdlg,WM_COMMAND,IDOK,1);
								return TRUE;
							}
					}
				}
			else // From text insert menu
				{
				//hctrl=GetDlgItem(hdlg,IDC_WEXT);
					//SetCapture(GetDlgItem(hdlg,IDC_PINv));
				hctrl=GetCapture();
				if(ProcessInsertsPopup(wP,hctrl))
				{
					ReleaseCapture();
					return TRUE;
				}
				}
			break;
			}
		}
	return FALSE;
	}
