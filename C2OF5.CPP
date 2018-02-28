/******************************************************************************
	NAME : C2OF5.C
	DESC : Code 2of5 barcode implementation for IMS.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <memory.h>
#include <imslib32\imslib32.h>
#include <jvmath32\jvmath32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "resource.h"
#include "imp.h"
#include "editor.h"
#include "c2OF5.h"
#include "lbmp.h"
#include "stdloc.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define SIZEWIDE 3                // Number of strokes per wide element
#define XSIGLYPH (3+(2*SIZEWIDE)) // Number of strokes per glyph (interleved)
#define XSGLYPH  (XSIGLYPH+5)     // Number of strokes per glyph non interleved
#define XSSTART  (2+2)            // Number of strokes per start char
#define XSSTOP   (2+1+SIZEWIDE)   // Number of strokes per stop char

// Special Glyph indexes
#define giSTART  10
#define giSTOP   11

/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,BOOL addsum);
static BOOL GlyphMapAppendChar(int c);
static void GlyphMapAppend(int i);
static int BitsAppendModule(BYTE *bits,int dx,int x,BOOL dark);
static int BitsAppendIGlyphs(BYTE *bits,int dx,int x,int gi0,int gi1);
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi);
static int BitsAppendSpecialGlyph(BYTE *bits,int dx,int x,int gi);
static int ComputeCI2OF5Checksum(LPSTR s,int sl);

/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static int GlyphMap[256]; // Indexes into Glyph[]
static int GlyphMapLen=0;

// CodeI2of5 bitmap Narrow Wide data in 5 LSB read from left to right
// 0 indicates narrow element
// 1 indicates wide element
// Start and stop are treated differently

static WORD Glyph[12]=
	{
	0x0006, // 0
	0x0011, // 1
	0x0009, // 2
	0x0018, // 3
	0x0005, // 4
	0x0014, // 5
	0x000C, // 6
	0x0003, // 7
	0x0012, // 8
	0x000A, // 9
	0x0000, // START
	0x0002, // STOP
	};

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	hbmp = C2OF5RenderBitmap(m,t)

	Renders the supplied text to a bitmap image of its Code 2of5 barcode
	following the format described in the mark.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
******************************************************************************/
HBITMAP C2OF5RenderBitmap(IMPMARK *m,LPSTR t)
	{
	int xsbits,ysbits; // Bitmap dimensions in bits
	int xsbytes;       // Bitmap scanline width in bytes (scans word aligned)
	BYTE *bits;        // Bitmap bits fixed on local heap
	int x;             // X pixel address during scanline computation
	int i,j;           // General purpose loops
	HBITMAP hbmp;      // Handle of newly created bitmap
	LPSTR dsc;
	WORD ldsc;
	char hrpict[IMPLC2OF5HRPICTURE+1]; // Local hrpict used for autocreation of hrpict
	char hrbuf[IMPLC2OF5HRPICTURE+1];
	int hrlen;
	SIZE te;
	HFONT hfont;
	int ix;
	int hrsize;
	
	if(m->c2of5.overlayhr)
		{
		if(m->c2of5.hrpict[0]==0)
			{
			hrlen=lstrlen(t);
			int pi;
			for( pi=0;pi<hrlen;pi++)
				hrpict[pi]='X';
			hrpict[pi]=0;
			}
		else
			lstrcpy(hrpict,m->c2of5.hrpict);
		}

	// compute dsc ldsc
	int sz = sizeof(int);  // int size changed for Win32
	int UpToT = 7+8*sz;
	ldsc=UpToT+lstrlen(t)+1+IMPLC2OF5HRPICTURE;
	dsc=new char[ldsc];
	lstrcpy(dsc,"[C2OF5]");                    // 7
	*(int *)(dsc+7)=m->c2of5.dx;          // 4 - was 2
	*(int *)(dsc+7+sz)=m->c2of5.dy;          // 4 - was 2
	*(int *)(dsc+7+2*sz)=m->c2of5.qz;         // 4 - was 2
	*(int *)(dsc+7+3*sz)=m->c2of5.neg;        // 4 - ...
	*(int *)(dsc+7+4*sz)=m->c2of5.interleave; // 4 - ...
	*(int *)(dsc+7+5*sz)=m->c2of5.bbars;      // 4 - ...
	*(int *)(dsc+7+6*sz)=m->c2of5.addsum;     // 4 - ...
	*(int *)(dsc+7+7*sz)=m->c2of5.overlayhr;  // 4 - ...
	lstrcpy(dsc+UpToT,t);                       // lstrlen(t)+1
	if(m->c2of5.overlayhr)
		lstrcpy(dsc+UpToT+lstrlen(t)+1,hrpict); // IMPLC2OF5HRPICTURE

	// Check for cache hit
	hbmp=LoadedBitmapsFind(dsc,ldsc);
	if(hbmp)
		{
		delete [] dsc;
		return hbmp;
		}

	if(!ComputeGlyphMap(t,m->c2of5.addsum))
		{
		delete [] dsc;
		return 0;
		}

	if(m->c2of5.overlayhr)
		{
		LOGFONT lf;
		lf.lfHeight=-8;
		lf.lfWidth=0;
		lf.lfEscapement=0;
		lf.lfOrientation=0;
		lf.lfWeight=FW_DONTCARE;
		lf.lfItalic=0;
		lf.lfUnderline=0;
		lf.lfStrikeOut=0;
		lf.lfCharSet=ANSI_CHARSET;
		lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
		lf.lfQuality=DEFAULT_QUALITY;
		lf.lfPitchAndFamily=VARIABLE_PITCH|FF_SWISS;
		lstrcpy(lf.lfFaceName,"Arial");

		hrsize=lf.lfHeight;
		// compute hrbuf and hrlen text
		LPSTR tp=t;
		hrlen=0;
		int pi=0;
		char c;
		while(c=hrpict[pi])
			{
			switch(c)
				{
				case ' ':
				case '(':
				case ')':
					hrbuf[hrlen++]=c;
					break;
				case 'X':
					if(*tp)
						hrbuf[hrlen++]=*tp++;
					break;
				}
			pi++;
			}
		hrbuf[hrlen]=0;
		hfont=CreateFontIndirect(&lf);
		HDC hdc=GetDC(NULL);
		HFONT ohfont=(HFONT)SelectObject(hdc,hfont);
		GetTextExtentPoint(hdc,hrbuf,hrlen,&te);
		SelectObject(hdc,ohfont);
		ReleaseDC(NULL,hdc);
		}
	else
	{
		te.cx=0;
		te.cy=0;
	}

	// Compute bitmap dimensions
	if(m->c2of5.interleave)
		xsbits=m->c2of5.dx*(2*m->c2of5.qz+GlyphMapLen*XSIGLYPH+XSSTOP+XSSTART+1); // zzz - May need change
	else
		xsbits=m->c2of5.dx*(2*m->c2of5.qz+GlyphMapLen*XSGLYPH+XSSTOP+XSSTART+1); // zzz - May need change

	if(xsbits<(int)te.cx) // Adjust ix and bitmap x dimension if hr is larger
		{
		ix=(te.cx-xsbits)>>1;
		xsbits=te.cx;
		}
	else
		ix=0;
	ysbits=m->c2of5.dy;
	xsbytes=((xsbits+15)>>3)&0xFFFE;
	// Allocate memory for bitmap bits
	bits=new BYTE[xsbytes*ysbits];
	if(!bits)
		{
		delete [] dsc;
		return 0;
		}

	for(i=0;i<xsbytes;i++)  // Set first scan to all white
		bits[i]=0xff;

	// Compute first scanline
	x=ix;                                            // Start x at left edge
	for(i=0;i<m->c2of5.qz;i++)                            // Leading QZ
		x=BitsAppendModule(bits,m->c2of5.dx,x,FALSE);
	x=BitsAppendModule(bits,m->c2of5.dx,x,FALSE);         // leading space
	x=BitsAppendSpecialGlyph(bits,m->c2of5.dx,x,giSTART); // Start char
	if(m->c2of5.interleave)                               // DATA
		for(i=0;i<GlyphMapLen;i+=2) // Interleaved
			x=BitsAppendIGlyphs(bits,m->c2of5.dx,x,GlyphMap[i],GlyphMap[i+1]);
	else
		for(i=0;i<GlyphMapLen;i++) // Non interleaved
			x=BitsAppendGlyph(bits,m->c2of5.dx,x,GlyphMap[i]);
	x=BitsAppendSpecialGlyph(bits,m->c2of5.dx,x,giSTOP);  // Stop char
	for(i=0;i<m->c2of5.qz;i++)                            // Ending QZ
		x=BitsAppendModule(bits,m->c2of5.dx,x,FALSE);

	// Replicate first scan to remaining scans
	for(i=1;i<ysbits;i++)
		memcpy(bits+i*xsbytes,bits,xsbytes);

	// Add the bearer bars
	if(m->c2of5.bbars)
		{
		int bsize=hrsize;
		if(bsize<0)
			bsize=-bsize;
		for(i=0;i<SIZEWIDE-1;i++)
			{
			if(m->c2of5.overlayhr==HRTOP)
				for(j=0;j<xsbytes;j++)
					bits[(i+bsize-2)*xsbytes+j]=0;
			else
				for(j=0;j<xsbytes;j++)
					bits[i*xsbytes+j]=0;

			if(m->c2of5.overlayhr==HRBOTTOM)
				for(j=0;j<xsbytes;j++)
					bits[((ysbits-i-1-bsize)*xsbytes)+j]=0;
			else
				for(j=0;j<xsbytes;j++)
					bits[((ysbits-i-1)*xsbytes)+j]=0;
			}
		}

	// Create the bitmap
	hbmp=CreateBitmap(xsbits,ysbits,1,1,bits);

	delete [] bits;
	bits=NULL;

	// Create DC to bitmap for modifictions
	HDC sdc=GetDC(NULL);
	HDC bdc=CreateCompatibleDC(sdc);
	ReleaseDC(NULL,sdc);

	HBITMAP ob=(HBITMAP)SelectObject(bdc,hbmp);

	// Overlay human readable
	if(m->c2of5.overlayhr)
		{
		RECT r;
		HFONT of=(HFONT)SelectObject(bdc,hfont);
		if(hrsize<0)
			hrsize*=-1;
		if(m->c2of5.overlayhr==HRTOP)
			r.top=-2;
		else
			r.top=ysbits-hrsize;
		r.bottom=r.top+hrsize;
		r.left=0;r.right=xsbits;
		FillRect(bdc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
		SetBkMode(bdc,TRANSPARENT);
		TextOut(bdc,(xsbits-te.cx)>>1,r.top,hrbuf,hrlen);
		SelectObject(bdc,of);
		DeleteObject(hfont);
		}

	// Negate the entire bitmap if neg
	if(m->c2of5.neg)
		PatBlt(bdc,0,0,xsbits,ysbits,PATINVERT);

	SelectObject(bdc,ob);
	DeleteDC(bdc);

	LoadedBitmapsAdd(dsc,ldsc,hbmp);
	delete [] dsc;
	return hbmp;
	}

/******************************************************************************
	C2OF5Init(m)

	Initializes a new IMPC2OF5MARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void C2OF5Init(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->type=mtC2OF5;
	m->c2of5.xres=100.0;
	m->c2of5.yres=100.0;
	m->c2of5.dx=1;
	m->c2of5.dy=24;
	m->c2of5.qz=0;
	m->c2of5.ms=1;
	m->c2of5.neg=FALSE;
	m->c2of5.overlayhr=HRNONE;
	m->c2of5.text[0]=0;
	m->c2of5.hrpict[0]=0;
	m->c2of5.interleave=TRUE;
	m->c2of5.bbars=FALSE;
	m->c2of5.addsum=FALSE;
	}

/******************************************************************************
	C2OF5MARKEDITDP

	mtC2OF5 editor dialog process.
******************************************************************************/
BOOL CALLBACK C2OF5MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
//			int i;

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

			// IDC_DEG	--sam	10/7/2016
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
			FeditSetVal(hctrl,EditorMark.c2of5.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.c2of5.yres/EditYScale);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=8;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c2of5.dx);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=39;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c2of5.dy);

			// IDC_QZONE
			hctrl=GetDlgItem(hdlg,IDC_QZONE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=30;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c2of5.qz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c2of5.ms);

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
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.c2of5.neg);

			// IDC_INTERLEAVE
			CheckDlgButton(hdlg,IDC_INTERLEAVE,EditorMark.c2of5.interleave);

			// IDC_BBARS
			CheckDlgButton(hdlg,IDC_BBARS,EditorMark.c2of5.bbars);

			// IDC_ADDSUM
			CheckDlgButton(hdlg,IDC_ADDSUM,EditorMark.c2of5.addsum);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC2OF5;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c2of5.text);

			// IDC_HRPICT
			hctrl=GetDlgItem(hdlg,IDC_HRPICT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC2OF5HRPICTURE;
			kinfo.digits=0;
			kinfo.lowers=0;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c2of5.hrpict);

			// IDC_HROVERLAY
			switch(EditorMark.c2of5.overlayhr)
				{
				case HRBOTTOM:
					CheckDlgButton(hdlg,IDC_HROVERLAY2,TRUE);
					break;
				case HRTOP:
					CheckDlgButton(hdlg,IDC_HROVERLAY1,TRUE);
					break;
				case HRNONE:
				default:
					CheckDlgButton(hdlg,IDC_HROVERLAY,TRUE);
					break;
				}
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND://Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;
//			int i;

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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.c2of5.xres))
							||(EditorMark.c2of5.xres < 10.0)
							||(EditorMark.c2of5.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c2of5.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.c2of5.yres))
							||(EditorMark.c2of5.yres < 10.0)
							||(EditorMark.c2of5.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c2of5.yres*=EditYScale;

						// IDC_XDOTS
    	                hctrl=GetDlgItem(hdlg,IDC_XDOTS);
	                    if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
	                        {
	                        ShowError(IDE_BADXDOTS);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }
	                    EditorMark.c2of5.dx=(WORD)dw;

						// IDC_YDOTS
    	                hctrl=GetDlgItem(hdlg,IDC_YDOTS);
	                    if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
	                        {
	                        ShowError(IDE_BADYDOTS);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }
	                    EditorMark.c2of5.dy=(WORD)dw;

						// IDC_QZONE
    	                hctrl=GetDlgItem(hdlg,IDC_QZONE);
	                    if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
	                        {
	                        ShowError(IDE_BADQZONE);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }
	                    EditorMark.c2of5.qz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c2of5.ms=(WORD)dw;

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
						EditorMark.c2of5.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

						// IDC_INTERLEAVE
						EditorMark.c2of5.interleave=IsDlgButtonChecked(hdlg,IDC_INTERLEAVE);
						// IDC_BBARS
						EditorMark.c2of5.bbars=IsDlgButtonChecked(hdlg,IDC_BBARS);
						// IDC_ADDSUM
						EditorMark.c2of5.addsum=IsDlgButtonChecked(hdlg,IDC_ADDSUM);
                        // IDC_WEXT
                        hctrl=GetDlgItem(hdlg,IDC_WEXT);
                        if((!KeditValidate(hctrl))||
                            (!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c2of5.text)))||
                            (!EditorMark.c2of5.text[0]))
                            {
                            ShowError(IDE_BADTEXT);
                            SetFocus(hctrl);
                            return TRUE;
                            }

/*						{
							
							int texlen = lstrlen(EditorMark.c2of5.text);
							for (int i=0;i<texlen;i++){
								char ch = EditorMark.c2of5.text[i];
								if (ch<'0' || ch>'9'){
									ShowError(IDE_NUMERICCHAR);
									SetFocus(hctrl);
									return TRUE;
								}
							}
							
						}
*/
                        // IDC_HRPICT
                        hctrl=GetDlgItem(hdlg,IDC_HRPICT);
                        if((!KeditValidate(hctrl)) ||
                            (!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c2of5.hrpict))))
                            {
                            ShowError(IDE_BADHRPICT);
                            SetFocus(hctrl);
                            return TRUE;
                            }

	                    // IDC_HROVERLAYx
						EditorMark.c2of5.overlayhr=
							IsDlgButtonChecked(hdlg,IDC_HROVERLAY2) ? HRBOTTOM :
							IsDlgButtonChecked(hdlg,IDC_HROVERLAY1) ? HRTOP : HRNONE;

						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;

					case IDC_WEXT:
						{
						if(ncode==XN_INS)
							DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
								&EditorIMP.udhas,
								&EditorIMP.vars,
								&EditorIMP.gvars);
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
	STATIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	ok=ComputeGlyphMap(s,addsum)

	Computes the GlphMap for the given string including the checksum when
	specified but not including start and stop.  GlyphMap is an array of indexes into
	Glyps,  which are the bit patterns for each CODE2of5 barcode character.

	BOOL ok = TRUE on success.
	LPSTR s = string to compute GlyphMap for.
	BOOL addsum = Flag to add checksum
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,BOOL addsum)
	{
	LPSTR p;
	BOOL lead = FALSE;

	int slen=lstrlen(s);
	// Validate s
	if(addsum)
		{
		if((slen+1)&1)  // If string+checksum is odd, pad
			lead=TRUE;
		}
	else
		{
		if(slen&1)      // If string is odd, pad
			lead=TRUE;
		}

	p=s; // must be all digits
	while(*p)
		{
		if(*p<'0' || *p>'9')
			return 0;
		p++;
		}

	GlyphMapLen=0;
	if(lead)
		GlyphMapAppend(0); // Append a leading zero
	for(p=s;*p;p++)  // For each character in s
		if(!GlyphMapAppendChar(*p)) // Append its rep to GlyphMap
			return FALSE;
	if(addsum)
		GlyphMapAppend(ComputeCI2OF5Checksum(s,slen));    // Append its rep to GlyphMap

	if(!GlyphMapLen) // Error if no characters mapped
		return FALSE;

	if(GlyphMapLen&1) // Error if odd number of characters mapped
		return FALSE;

	return TRUE;
	}

/******************************************************************************
	ok = GlyphMapAppendChar(c)

	Appends the Glyph indexes for the character c to GlyphMap.
	Performs full ascii to code2of5 conversion.

	int c = character to append Glyph indexes for.
******************************************************************************/
static BOOL GlyphMapAppendChar(int c)
	{
	if(c<0x30||c>0x39)
		return FALSE;
	GlyphMapAppend(c-0x30);
	return TRUE;
	}

/******************************************************************************
	GlyphMapAppend(i)

	Appends a Glyph index to the GlyphMap.

	int i = Glyph index to append.
******************************************************************************/
static void GlyphMapAppend(int i)
	{
	GlyphMap[GlyphMapLen++]=(BYTE)(i&0xff);
	}

/******************************************************************************
	newx=BitsAppendModule(bits,dx,x,dark)

	Appends a dark or light module to a scanline buffer at x

	int newx = new x after appending the module.
	BYTE *bits = pointer to start of scanline
	int dx = module x size in pixels
	BOOL dark = module color TRUE for dark	
******************************************************************************/
static int BitsAppendModule(BYTE *bits,int dx,int x,BOOL dark)
	{
	BYTE m;

	while(dx--)
		{
		m=128>>(x&7); // mask
		if(dark)
			bits[x>>3]&=~m;
		else
			bits[x>>3]|=m;
		x++;
		}
	return x;
	}

/******************************************************************************
	newx = BitsAppendIGlyphs(bits,dx,x,gi0,gi1)

	Appends a pair of interleaved barcode glyph by looking them up
	and appending all of there modules.

	int newx = x after appending the glyph
	BYTE *bits = start of scanline to append glyph to.
	int dx = module x dimension in pixels
	int gi0 = First glyph index to interleave.
	int gi1 = Second glyph index to interleave.
******************************************************************************/
static int BitsAppendIGlyphs(BYTE *bits,int dx,int x,int gi0,int gi1)
	{
	int i;
	register WORD glyph0,glyph1;

	glyph0=Glyph[gi0];
	glyph1=Glyph[gi1];

	i=5;
	while(i--)
		{
		x=BitsAppendModule(bits,(glyph0>>i)&1 ? (dx*SIZEWIDE) : dx ,x,TRUE);
		x=BitsAppendModule(bits,(glyph1>>i)&1 ? (dx*SIZEWIDE) : dx ,x,FALSE);
		}
	return x;
	}

/******************************************************************************
	newx = BitsAppendGlyph(bits,dx,x,gi)

	Appends a non interleaved glyph to the bits.

	int newx = x after appending the glyph
	BYTE *bits = start of scanline to append glyph to.
	int dx = module x dimension in pixels
	int gi = glyph index to append.
******************************************************************************/
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi)
	{
	int i;
	register WORD glyph;

	glyph=Glyph[gi];

	i=5;
	while(i--)
		{
		x=BitsAppendModule(bits,(glyph>>i)&1 ? (dx*SIZEWIDE) : dx ,x,TRUE);
		x=BitsAppendModule(bits,dx,x,FALSE);
		}
	return x;
	}

/******************************************************************************
	newx = BitsAppendSpecialGlyph(bits,dx,x,gi)

	Appends a barcode glyph by looking it up and appending all of its
	modules.

	int newx = x after appending the glyph
	BYTE *bits = start of scanline to append glyph to.
	int dx = module x dimension in pixels
	int gi = Glyph index.
******************************************************************************/
static int BitsAppendSpecialGlyph(BYTE *bits,int dx,int x,int gi)
	{
	int i;
	register WORD glyph;

	glyph=Glyph[gi];

	i=2;
	while(i--)
		{
		x=BitsAppendModule(bits,(glyph>>i)&1 ? (dx*SIZEWIDE) : dx ,x,TRUE);
		x=BitsAppendModule(bits,dx,x,FALSE);
		}
	return x;
	}

/******************************************************************************
	sum = ComputeCI2OF5Checksum(s,sl)

	Computes the CI2OF5 mod 10 checksum C.

	LPSTR s = digit string to compute checksum for.
	int sl = length of the data string
******************************************************************************/
static int ComputeCI2OF5Checksum(LPSTR s,int sl)
	{
	int i,w,sum;

	sum=0;

	for(i=0;i<sl;i++)
		{
		w=1+(2*((i&1)^(sl%2)));  // Generates 3 1 ... weight sequence
		sum+=w*(s[i]-0x30);
		}

	sum=10-(sum%10);
	return sum%10;
	}

