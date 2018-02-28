/******************************************************************************
	NAME : CQR.C
	DESC : QR Code for IMS5000 via Dixons QRCoder.exe
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
#include "cqr.h"
#include "lbmp.h"
#include "stdloc.h"

BOOL QRCoderIsAvailable=false; // Indicates QRCoder can be used

static char* sROT(int rot)
	{
	static char* keywords[]={"0deg","90deg","180deg","270deg"};
	if(rot>=0 && rot<4)
		return keywords[rot];
	return "BAD";
	}

static char* sDEG(int rot)
	{
	return "DEG";
	}

static char* sENC(IMPCQRENC enc)
	{
	static char* keywords[]={"Byte","Alpha-Numeric","Numeric"};
	if(enc>=0 && enc<IMPCQRNENC)
		return keywords[enc];
	return "BAD";
	}

static char* sCOR(IMPCQRCOR cor)
	{

	static char* keywords[]={"L 7%","M 15%","Q 25%","H 30%"};
	if(cor>=0 && cor<IMPCQRNCOR)
		return keywords[cor];
	return "BAD";
	}
static char* sENCcode(IMPCQRENC enc)
	{
	static char* keywords[]={"Byte","AlphaNumeric","Numeric"};
	if(enc>=0 && enc<IMPCQRNENC)
		return keywords[enc];
	return "BAD";
	}

static char* sCORcode(IMPCQRCOR cor)
	{

	static char* keywords[]={"L","M","Q","H"};
	if(cor>=0 && cor<IMPCQRNCOR)
		return keywords[cor];
	return "BAD";
	}

static char* sECODE(int ecode)
	{
	if(ecode==0)
		return "Ok";
	if(ecode==-1)
		return "INTERNAL ERROR : No Arguments were passed";
	if(ecode==1)
		return "Bad Text";
	if(ecode==2)
		return "Bad Encoding";
	if(ecode==3)
		return "Bad Correction";
	if(ecode==4)
		return "Bad Sybol Version";
	if(ecode==5)
		return "Bad Size";
	if(ecode==77)
		return "Error in accessing directory location";
	if(ecode==88)
		return "Error in saving the image file";
	if(ecode==99)
		return "Too many arguments received";
	return "UNKNOWN";
	}

static HBITMAP _RenderViaQRCoder(char* text,IMPCQRENC enc,IMPCQRCOR cor,int ver,BOOL showerr)
	{
	char bfn[128]; // bmp full name
	strcpy_s(bfn,sizeof(bfn),IMSDir);
	strcat_s(bfn,sizeof(bfn),"\\");
	strcat_s(bfn,sizeof(bfn),"_QR.BMP");

	if(ver>IMPCQRVERMAX || ver<IMPCQRVERMIN)
		{
		if(showerr)
				MessageBox(GetActiveWindow(),"Bad Ver","QRCoder Interface",MB_OK|MB_ICONEXCLAMATION);
		return 0;
		}

	// Delete any existing _QR.BMP
	BOOL deleted=DeleteFile(bfn);

///create and use escaped text etext
	char cmdLine[256];
	wsprintf(cmdLine,"QRCoder \"%s\" %s %s %d 1 BMP _QR \"%s\""
		,text
		,sENCcode(enc)
		,sCORcode(cor)
		,ver
		,IMSDir
		);

	STARTUPINFO si={0};
	si.cb=sizeof(STARTUPINFO);
	si.wShowWindow=SW_HIDE;
	PROCESS_INFORMATION pi={0};
	BOOL ok=CreateProcess(NULL,cmdLine,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,IMSDir,&si,&pi);
	if(ok)
		{
		DWORD wfii=WaitForInputIdle(pi.hProcess,2000);
		DWORD wfso=WaitForSingleObject(pi.hProcess,2000);
		DWORD ecode;
		BOOL gecpOk=GetExitCodeProcess(pi.hProcess,&ecode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		if(ecode)
			{
			if(showerr)
				MessageBox(GetActiveWindow(),sECODE(ecode),"QRCoder Interface",MB_OK|MB_ICONEXCLAMATION);
			return 0;
			}
		return LoadBMP(bfn,showerr);
		}
	return 0;
	}

/***************************************************************************
	available = QRCoderCheck()

	Attempts to create a qr code with QRCoder to test that it
	both exists and works.
***************************************************************************/
BOOL QRCoderCheck()
	{
	QRCoderIsAvailable=false;
	HBITMAP hbmp=_RenderViaQRCoder("0000",IMPCQRENCN,IMPCQRCORL,1,false);
	if(hbmp)
		{
		DeleteObject(hbmp);
		QRCoderIsAvailable=true;
		}
	return QRCoderIsAvailable;
	}


/******************************************************************************
	hbmp = CQRRenderBitmap(m,t,showerr)

	Renders the supplied text to a bitmap image of its QR Code
	following the format described in the mark.
???	If the input data matches the cache then the cache bitmap itself
???	is returned (NOT A COPY).  You should call CQRClearCache() if you
???	want to be sure to get a new bitmap.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
	BOOL showerr = TRUE to display erros, FALSE to suppress them
******************************************************************************/
HBITMAP CQRRenderBitmap(IMPMARK *m,LPSTR t,BOOL showerr)
	{
	HBITMAP hbmp=_RenderViaQRCoder(t
		,m->cqr.enc
		,m->cqr.cor
		,m->cqr.ver
		,showerr);
	if(hbmp)
		{
		///zzz post processing neg qz xmul ymul
		BITMAP bitmap;
		GetObject(hbmp,sizeof(bitmap),&bitmap);
		HDC hdc=CreateCompatibleDC(NULL);
		HBITMAP ohbmp=(HBITMAP)SelectObject(hdc,hbmp);

		int xs=bitmap.bmWidth;
		int ys=bitmap.bmHeight;


		int rot=m->cqr.rot;
		BOOL xflip=m->cqr.xflip;
		BOOL yflip=m->cqr.yflip;
		int xmul=m->cqr.xmul;
		int ymul=m->cqr.ymul;
		int xqz=m->cqr.xqz;
		int yqz=m->cqr.yqz;

		if(rot==1||rot==3) //90|270
			{
			//ROT90
			HDC hdcNew=CreateCompatibleDC(NULL);
			HBITMAP hbmpNew=CreateBitmap(ys,xs,1,1,NULL);
			HBITMAP ohbmpNew=(HBITMAP)SelectObject(hdcNew,hbmpNew);
			for(int iy=0;iy<ys;iy++)
				{
				for(int ix=0;ix<xs;ix++)
					{
					COLORREF pix=GetPixel(hdc,ix,iy);
					int bx=ys-1-iy;
					int by=ix;
					SetPixel(hdcNew,bx,by,pix);
					}
				}
			SelectObject(hdcNew,ohbmpNew);
			DeleteDC(hdcNew);
			SelectObject(hdc,hbmpNew);
			DeleteObject(hbmp);
			hbmp=hbmpNew;
			int t=xs;
			xs=ys;
			ys=t;
			}
		if(rot==2||rot==3) //180|270
			{ // flip flips
			xflip=!xflip;
			yflip=!yflip;
			}
		if(xflip)
			{
			//FLIP X
			HDC hdcNew=CreateCompatibleDC(NULL);
			HBITMAP hbmpNew=CreateBitmap(xs,ys,1,1,NULL);
			HBITMAP ohbmpNew=(HBITMAP)SelectObject(hdcNew,hbmpNew);
			StretchBlt(hdcNew,xs-1,0,-xs,ys,hdc,0,0,xs,ys,SRCCOPY);
			BitBlt(hdc,0,0,xs,ys,hdcNew,0,0,SRCCOPY);
			SelectObject(hdcNew,ohbmpNew);
			DeleteDC(hdcNew);
			DeleteObject(hbmpNew);
			}
		if(yflip)
			{
			//FLIP Y
			HDC hdcNew=CreateCompatibleDC(NULL);
			HBITMAP hbmpNew=CreateBitmap(xs,ys,1,1,NULL);
			HBITMAP ohbmpNew=(HBITMAP)SelectObject(hdcNew,hbmpNew);
			StretchBlt(hdcNew,0,ys-1,xs,-ys,hdc,0,0,xs,ys,SRCCOPY);
			BitBlt(hdc,0,0,xs,ys,hdcNew,0,0,SRCCOPY);
			SelectObject(hdcNew,ohbmpNew);
			DeleteDC(hdcNew);
			DeleteObject(hbmpNew);
			}
		if(xmul>1||ymul>1)
			{
			// xmul ymul scale up
			int mxs=xs*xmul;
			int mys=ys*ymul;
			HDC hdcNew=CreateCompatibleDC(NULL);
			HBITMAP hbmpNew=CreateBitmap(mxs,mys,1,1,NULL);
			HBITMAP ohbmpNew=(HBITMAP)SelectObject(hdcNew,hbmpNew);
			StretchBlt(hdcNew,0,0,mxs,mys,hdc,0,0,xs,ys,SRCCOPY);
			SelectObject(hdcNew,ohbmpNew);
			DeleteDC(hdcNew);
			SelectObject(hdc,hbmpNew);
			DeleteObject(hbmp);
			hbmp=hbmpNew;
			xs=mxs;
			ys=mys;
			}
		if(xqz||yqz)
			{
			// pad quiet zones
			int qxs=xs+2*xqz;
			int qys=ys+2*yqz;
			HDC hdcNew=CreateCompatibleDC(NULL);
			HBITMAP hbmpNew=CreateBitmap(qxs,qys,1,1,NULL);
			HBITMAP ohbmpNew=(HBITMAP)SelectObject(hdcNew,hbmpNew);
			RECT qrect={0};
			qrect.right=qxs;
			qrect.bottom=qys;
			FillRect(hdcNew,&qrect,(HBRUSH)GetStockObject(WHITE_BRUSH));
			BitBlt(hdcNew,xqz,yqz,xs,ys,hdc,0,0,SRCCOPY);
			SelectObject(hdcNew,ohbmpNew);
			DeleteDC(hdcNew);
			SelectObject(hdc,hbmpNew);
			DeleteObject(hbmp);
			hbmp=hbmpNew;
			xs=qxs;
			ys=qys;
			}

		if(m->cqr.neg)
			BitBlt(hdc,0,0,xs,ys,NULL,0,0,DSTINVERT);

		SelectObject(hdc,ohbmp);
		DeleteDC(hdc);
		}
	return hbmp;
	}

/******************************************************************************
	CQRInit(m)

	Initializes a new IMPCQRMARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void CQRInit(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->rep=0;
	m->type=mtCQR;
	m->cqr.xres=100.0;
	m->cqr.yres=100.0;
	m->cqr.ms=1;
	m->cqr.neg=FALSE;
	m->cqr.xflip=false;
	m->cqr.yflip=false;
	m->cqr.xmul=1;
	m->cqr.ymul=1;
	m->cqr.xqz=0;
	m->cqr.yqz=0;
	m->cqr.rot=0;
	m->cqr.enc=IMPCQRENCB;
	m->cqr.cor=IMPCQRCORL;
	m->cqr.ver=7;
	m->cqr.text[0]=0;                       // Blank source text
	}

/******************************************************************************
	CQRMARKEDITDP

	mtCQR editor dialog process.
******************************************************************************/
BOOL CALLBACK CQRMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
///			int i;

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
			FeditSetVal(hctrl,EditorMark.cqr.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cqr.yres/EditYScale);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cqr.ms);

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
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.cqr.neg);

			// IDC_QRROT
			for(int i=0;i<4;i++)
				SendDlgItemMessage(hdlg,IDC_QRROT,CB_ADDSTRING,0,(LPARAM)sROT(i));
			SendDlgItemMessage(hdlg,IDC_QRROT,CB_SETCURSEL,EditorMark.cqr.rot,0);

			// IDC_QRDEG	--sam	10/10/16
			for(int i=0;i<4;i++)
				SendDlgItemMessage(hdlg,IDC_QRDEG,CB_ADDSTRING,0,(LPARAM)sROT(i));
			SendDlgItemMessage(hdlg,IDC_QRDEG,CB_SETCURSEL,EditorMark.cqr.deg,0);

			// IDC_QRENC
			for(int i=0;i<IMPCQRNENC;i++)
				SendDlgItemMessage(hdlg,IDC_QRENC,CB_ADDSTRING,0,(LPARAM)sENC((IMPCQRENC)i));
			SendDlgItemMessage(hdlg,IDC_QRENC,CB_SETCURSEL,EditorMark.cqr.enc,0);

			// IDC_QRCOR
			for(int i=0;i<IMPCQRNCOR;i++)
				SendDlgItemMessage(hdlg,IDC_QRCOR,CB_ADDSTRING,0,(LPARAM)sCOR((IMPCQRCOR)i));
			SendDlgItemMessage(hdlg,IDC_QRCOR,CB_SETCURSEL,EditorMark.cqr.cor,0);

			// IDC_QRVER
			for(int i=IMPCQRVERMIN;i<=IMPCQRVERMAX;i++)
				{
				char nbuf[4];
				wsprintf(nbuf,"%d",i);
				SendDlgItemMessage(hdlg,IDC_QRVER,CB_ADDSTRING,0,(LPARAM)&nbuf);
				}
			SendDlgItemMessage(hdlg,IDC_QRVER,CB_SETCURSEL,EditorMark.cqr.ver-IMPCQRVERMIN,0);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.cqr.xflip);

			// IDC_YFLIP
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.cqr.yflip);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cqr.xmul);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=64;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cqr.ymul);

			// IDC_XQZ
			hctrl=GetDlgItem(hdlg,IDC_XQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cqr.xqz);

			// IDC_YQZ
			hctrl=GetDlgItem(hdlg,IDC_YQZ);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=32;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cqr.yqz);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLCQR;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.cqr.text);
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

						// IDC_DEG	--sam	10/7/2016
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
							||(!FeditGetVal(hctrl,&EditorMark.cqr.xres))
							||(EditorMark.cqr.xres < 10.0)
							||(EditorMark.cqr.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.cqr.yres))
							||(EditorMark.cqr.yres < 10.0)
							||(EditorMark.cqr.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.yres*=EditYScale;


						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.ms=(WORD)dw;

						// IDC_NEG
						EditorMark.cqr.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

						// IDC_QRROT
						hctrl=GetDlgItem(hdlg,IDC_QRROT);
						EditorMark.cqr.rot=(int)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_QRDEG	--sam	10/10/16
						hctrl=GetDlgItem(hdlg,IDC_QRDEG);
						EditorMark.cqr.deg=(int)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_QRENC
						hctrl=GetDlgItem(hdlg,IDC_QRENC);
						EditorMark.cqr.enc=(IMPCQRENC)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_QRCOR
						hctrl=GetDlgItem(hdlg,IDC_QRCOR);
						EditorMark.cqr.cor=(IMPCQRCOR)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_QRVER
						hctrl=GetDlgItem(hdlg,IDC_QRVER);
						EditorMark.cqr.ver=IMPCQRVERMIN+(int)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_XFLIP
						EditorMark.cqr.xflip=IsDlgButtonChecked(hdlg,IDC_XFLIP);

						// IDC_YFLIP
						EditorMark.cqr.yflip=IsDlgButtonChecked(hdlg,IDC_YFLIP);

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.xmul=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.ymul=(WORD)dw;

						// IDC_XQZ
						hctrl=GetDlgItem(hdlg,IDC_XQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.xqz=(WORD)dw;

						// IDC_YQZ
						hctrl=GetDlgItem(hdlg,IDC_YQZ);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYQZ);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cqr.yqz=(WORD)dw;

						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						GetWindowText(hctrl,EditorMark.cqr.text,IMPLCQR+1);
						if(!EditorMark.cqr.text[0])
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
