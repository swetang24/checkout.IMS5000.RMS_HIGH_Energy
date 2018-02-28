/******************************************************************************
	NAME : CUPCEAN.C
	DESC : Code UPC and EAN barcode implementation for IMS.
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
#include "cupcean.h"
#include "lbmp.h"
#include "stdloc.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define XSGLYPH 7  // Number of modules per glyph
#define XSLRG   3  // Number of modules per left or right guard
#define XSCG    5  // Number of modules per center guard

// Special Glyph indexes
#define giLRG 10  // left or right guard
#define giCG  11  // center guard

/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static int GlyphMap[256]; // Indexes into Glyph[]
static int GlyphMapLen=0;
static int CheckSum;

// UPCEAN bitmap glyph data in the XSGLYPH LSBs read from left to right)
// Except for LRG and CG which are XSLRG and XSCG respectively.
// The checksum value of the character is the same as its index.
// This map is the odd parity version, for even parity take the
// 1s complement.
static WORD Glyph[12]=
	{
	0x000D, // 0
	0x0019, // 1
	0x0013, // 2
	0x003D, // 3
	0x0023, // 4
	0x0031, // 5
	0x002F, // 6
	0x003B, // 7
	0x0037, // 8
	0x000B, // 9
	0x0005, // LRG=101
	0x000A, // CG=01010
	};

// Symbol map for each UPCEAN subtype
// O is for Odd parity digit
// E is for Even parity digit
// C is for CG
// L or R is for LRG
static char *SymbolMap[IMPNUPCEANSUBTYPES]=
	{
	"LOOOOOOCEEEEEER", // UPC A
	"LOOOOCEEEER"      // EAN 8
	};

// Symbol code length maps the subtype to the number of data+checksum chars
// This is the same as the number of Os and Es in the SumbolMap entry
static int SymbolCLMap[IMPNUPCEANSUBTYPES]={12,8};

/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,int subtype);
static int ComputeChecksum(LPSTR s,int sl);
static BOOL GlyphMapAppendChar(int c);
static void GlyphMapAppend(int i);
static int GlyphLen(int index);
static int BitsAppendModule(BYTE *bits,int dx,int x,BOOL dark);
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi,BOOL even);

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	hbmp = CUPCEANRenderBitmap(m,t)

	Renders the supplied text to a bitmap image of its UPCEAN barcode
	following the format described in the mark.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
******************************************************************************/
HBITMAP CUPCEANRenderBitmap(IMPMARK *m,LPSTR t)
	{
	int xsbits,ysbits; // Bitmap dimensions in bits
	int xsbytes;       // Bitmap scanline width in bytes (scans word aligned)
	BYTE *bits;        // Bitmap bits fixed on local heap
	int x;             // X pixel address during scanline computation
	int i;             // General purpose loops
	int ix;            // Starting x
	HBITMAP hbmp;      // Handle of newly created bitmap
	char hrbuf[IMPLCUPCEANHRPICTURE+1];
	int hrlen,pi;
	LOGFONT lf;
	HFONT hfont,ohfont;
	HDC hdcscreen,hdc;
	HBITMAP ohbmp;
	SIZE te;
	RECT r;
	LPSTR tp;
	char c;
	BOOL ig,pig;  // In guard, previosly in guard
	int xg;       // x size of glyph in decender rect calculation
	LPSTR dsc;
	WORD ldsc;

	// compute dsc ldsc - updated for Win32 change in int size
	int sz = sizeof(int);
	int UpToPict = 9+6*sz;

	ldsc=UpToPict+IMPLCUPCEANHRPICTURE+lstrlen(t)+1;
	dsc=new char[ldsc];
	lstrcpy(dsc,"[CUPCEAN]");                    // 9
	*(int *)(dsc+9)=m->cupcean.dx;          // 4 - was 2
	*(int *)(dsc+9+sz)=m->cupcean.dy;         // 4 - was 2
	*(int *)(dsc+9+2*sz)=m->cupcean.qz;         // 4 - was 2
	*(int *)(dsc+9+3*sz)=m->cupcean.neg;        // 4 - was 2
	*(int *)(dsc+9+4*sz)=m->cupcean.subtype;    // 4 - was 2
	*(int *)(dsc+9+5*sz)=m->cupcean.overlayhr;  // 4 - was 2
	if(m->cupcean.overlayhr)
		lstrcpy(dsc+UpToPict,m->cupcean.hrpict);       // IMPLCUPCEANHRPICTURE
	lstrcpy(dsc+UpToPict+IMPLCUPCEANHRPICTURE,t);      // lstrlen(t)+1

	// Check for cache hit
	hbmp=LoadedBitmapsFind(dsc,ldsc);
	if(hbmp)
		{
		delete [] dsc;
		return hbmp;
		}

	if(!ComputeGlyphMap(t,m->cupcean.subtype))
		{
		delete [] dsc;
		return 0;
		}

	// Compute human readable text and text extent if overlay requred
	if(m->cupcean.overlayhr)
		{
		// compute hrbuf and hrlen text
		tp=t;
		hrlen=0;
		pi=0;
		while(c=m->cupcean.hrpict[pi])
			{
			switch(c)
				{
				case ' ':
				case '(':
				case ')':
				case '-':
					hrbuf[hrlen++]=c;
					break;
				case 'X':
					if(*tp)
						hrbuf[hrlen++]=*tp++;
					break;
				case 'C':
					hrbuf[hrlen++]='0'+ComputeChecksum(t,
						SymbolCLMap[m->cupcean.subtype]-1);
					break;
				}
			pi++;
			}
		hrbuf[hrlen]=0;

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
		hfont=CreateFontIndirect(&lf);
		hdcscreen=GetDC(NULL);
		ohfont=(HFONT)SelectObject(hdcscreen,hfont);
		GetTextExtentPoint(hdcscreen,hrbuf,hrlen,&te);
		SelectObject(hdcscreen,ohfont);
		ReleaseDC(NULL,hdcscreen);
		}
	else
	{
		te.cx=0;
		te.cy=0;
	}
	// Compute bitmap dimensions
	xsbits=2*m->cupcean.qz;
	for(i=0;i<GlyphMapLen;i++)
		xsbits+=GlyphLen(GlyphMap[i]);
	xsbits*=m->cupcean.dx;
	if(xsbits<te.cx) // Adjust ix and bitmap x dimension if hr is larger
		{
		ix=(te.cx-xsbits)>>1;
		xsbits=te.cx;
		}
	else
		ix=0;
	ysbits=m->cupcean.dy;
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

	// Compute first scanline (no neg)
	x=ix;                                           // Start at ix
	for(i=0;i<m->cupcean.qz;i++)                      // Leading QZ
		x=BitsAppendModule(bits,m->cupcean.dx,x,FALSE);
	for(i=0;i<GlyphMapLen;i++)
		x=BitsAppendGlyph(bits,m->cupcean.dx,x,GlyphMap[i],
			SymbolMap[m->cupcean.subtype][i]=='E');
	for(i=0;i<m->cupcean.qz;i++)                      // Ending QZ
		x=BitsAppendModule(bits,m->cupcean.dx,x,FALSE);

	// Replicate first scan to remaining scans
	for(i=1;i<ysbits;i++)
		memcpy(bits+i*xsbytes,bits,xsbytes);

	// Create the bitmap
	hbmp=CreateBitmap(xsbits,ysbits,1,1,bits);

	delete [] bits;

	// Setup DC to modify bitmap
	hdcscreen=GetDC(NULL);
	hdc=CreateCompatibleDC(hdcscreen);
	ohbmp=(HBITMAP)SelectObject(hdc,hbmp);
	ReleaseDC(NULL,hdcscreen);

	// Clip between decenders
	r.top=ysbits-8;
	r.top=max(0,r.top);
	r.bottom=ysbits;
	pig=TRUE; // start in guard
	x=ix+(m->cupcean.qz)*(m->cupcean.dx);
	for(i=0;i<GlyphMapLen;i++)
		{
		switch(SymbolMap[m->cupcean.subtype][i])
			{
			case 'L':
			case 'R':
				xg=XSLRG*m->cupcean.dx;
				ig=TRUE;
				break;
			case 'C':
				xg=XSCG*m->cupcean.dx;
				ig=TRUE;
				break;
			case 'O':
			case 'E':
				xg=XSGLYPH*m->cupcean.dx;
				ig=FALSE;
				break;
			}
		if(pig && (!ig)) // if start of an open area
			r.left=x;
		else if((!pig) && ig) // if end of open area : draw it
			{
			r.right=x;
			FillRect(hdc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
			}		
		x+=xg;
		pig=ig;
		}

	// Overlay human readable if needed
	if(m->cupcean.overlayhr)
		{
		ohfont=(HFONT)SelectObject(hdc,hfont);
		SetBkMode(hdc,TRANSPARENT);
		TextOut(hdc,(xsbits-te.cx)>>1,ysbits-8,hrbuf,hrlen);
		SelectObject(hdc,ohfont);
		DeleteObject(hfont);
		}

	// Negate the entire bitmap if neg
	if(m->cupcean.neg)
		PatBlt(hdc,0,0,xsbits,ysbits,PATINVERT);

	// Delete DC
	SelectObject(hdc,ohbmp);
	DeleteDC(hdc);

	LoadedBitmapsAdd(dsc,ldsc,hbmp);
	delete [] dsc;
	return hbmp;
	}

/******************************************************************************
	CUPCEANInit(m)

	Initializes a new IMPCUPCEANMARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void CUPCEANInit(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->type=mtCUPCEAN;
	m->cupcean.xres=100;
	m->cupcean.yres=100;
	m->cupcean.dx=1;
	m->cupcean.dy=24;
	m->cupcean.qz=0;
	m->cupcean.ms=1;
	m->cupcean.neg=FALSE;
	m->cupcean.subtype=UPCA;
	m->cupcean.overlayhr=FALSE;
	m->cupcean.hrpict[0]=0;
	m->cupcean.text[0]=0;
	}

/******************************************************************************
	CUPCEANMARKEDITDP

	mtCUPCEAN editor dialog process.
******************************************************************************/
BOOL CALLBACK CUPCEANMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
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

			// IDC_SUBTYPE
			hctrl=GetDlgItem(hdlg,IDC_SUBTYPE);
			// Init it
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"UPC A");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"EAN 8");
			// Select current subtype
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.cupcean.subtype,0);

			// IDC_XRES
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cupcean.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cupcean.yres/EditYScale);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=8;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cupcean.dx);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cupcean.dy);

			// IDC_QZONE
			hctrl=GetDlgItem(hdlg,IDC_QZONE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=30;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cupcean.qz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cupcean.ms);

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
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.cupcean.neg);

			// IDC_HRPICT
			hctrl=GetDlgItem(hdlg,IDC_HRPICT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLCUPCEANHRPICTURE;
			kinfo.digits=0;
			kinfo.lowers=0;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.cupcean.hrpict);

			// IDC_HROVERLAY
			CheckDlgButton(hdlg,IDC_HROVERLAY,EditorMark.cupcean.overlayhr);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLCUPCEAN;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.cupcean.text);
			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND:
			{
			HWND hctrl;
			DWORD dw;
//			int i;

			if(lP) // If from control
				{
				switch(LOWORD(wP))
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDOK:
						{
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

						// IDC_SUBTYPE
						hctrl=GetDlgItem(hdlg,IDC_SUBTYPE);
						EditorMark.cupcean.subtype=(int)SendMessage(hctrl,
							CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.cupcean.xres))
							||(EditorMark.cupcean.xres < 10.0)
							||(EditorMark.cupcean.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cupcean.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.cupcean.yres))
							||(EditorMark.cupcean.yres < 10.0)
							||(EditorMark.cupcean.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cupcean.yres*=EditYScale;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cupcean.dx=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cupcean.dy=(WORD)dw;

						// IDC_QZONE
						hctrl=GetDlgItem(hdlg,IDC_QZONE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADQZONE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cupcean.qz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cupcean.ms=(WORD)dw;

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
						EditorMark.cupcean.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

	                    // IDC_HRPICT
	                    hctrl=GetDlgItem(hdlg,IDC_HRPICT);
	                    if((!KeditValidate(hctrl))||
	                        (!KeditGetVal(hctrl,
							(LPSTR)&(EditorMark.cupcean.hrpict)))||
	                        (!EditorMark.cupcean.hrpict[0]))
	                        {
	                        ShowError(IDE_BADHRPICT);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }

						// IDC_HROVERLAY
						EditorMark.cupcean.overlayhr=IsDlgButtonChecked(
							hdlg,IDC_HROVERLAY);

	                    // IDC_WEXT
	                    hctrl=GetDlgItem(hdlg,IDC_WEXT);
	                    if((!KeditValidate(hctrl))||
	                        (!KeditGetVal(hctrl,(LPSTR)&(EditorMark.cupcean.text)))||
	                        (!EditorMark.cupcean.text[0]))
							{
	                        ShowError(IDE_BADTEXT);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }

//						if(lstrlen(EditorMark.cupcean.text)!=11){
//							ShowError(IDE_UPCLENGTH);
//	                        SetFocus(hctrl);
//	                        return TRUE;
//						}
	                        
						EndDialog(hdlg,TRUE);
						return TRUE;
						}

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;

					case IDC_WEXT:
						if(HIWORD(wP)==XN_INS)
							DoInsertsPopup(hdlg,GetDlgItem(hdlg,IDC_WEXT),
								&EditorIMP.udhas,
								&EditorIMP.vars,
								&EditorIMP.gvars);
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

/******************************************************************************
	ok=ComputeGlyphMap(s,subtype)

	Computes the GlphMap for the given string including data
	checksum guards seperators and checksum.  GlyphMap is an
	array of indexes into Glyps,  wich are the bit patters for
	each CUPCEAN barcode character.

	BOOL ok = TRUE on success.
	LPSTR s = string to compute GlyphMap for.
	int subtype = UPCEAN subtype
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,int subtype)
	{
	int sl; // string length
	int cl; // code length required for given subtype
	int gl; // symbol length in glyphs
	int si; // string index
	int gi; // symbol glyph index

	if(subtype<0 || subtype > IMPNUPCEANSUBTYPES)
		return FALSE;
	sl=lstrlen(s);
	cl=SymbolCLMap[subtype];
	gl=lstrlen(SymbolMap[subtype]);
	if(sl<cl-1 || sl>cl) // If length out of range
		return FALSE;
	CheckSum=ComputeChecksum(s,cl-1);
	if(sl==cl) // Test checksum if provided
		if(CheckSum!=(s[cl-1]-'0'))
			return FALSE;

	GlyphMapLen=0;
	si=0;
	for(gi=0;gi<gl;gi++)
		switch(SymbolMap[subtype][gi])
			{
			case 'L':
			case 'R':
				GlyphMapAppend(giLRG);
				break;
			case 'C':
				GlyphMapAppend(giCG);
				break;
			case 'O':
			case 'E':
				if(si < cl-1)
					{
					if(!GlyphMapAppendChar(s[si++]))
						return FALSE;
					}
				else
					GlyphMapAppend(CheckSum);
				break;
			}
	if(!GlyphMapLen) // Error if no characters mapped
		return FALSE;

	return TRUE;
	}

/******************************************************************************
	sum = ComputeChecksum(s,sl)

	Computes the 3 1 weighted mod 10 checksum C.
	C is the 10 minus the mod 10 sum of the producs
	of the data chars and the sequence 3 1 3 1 ...

	LPSTR s = digit string to compute checksum for.
	int sl = number of characters to compute checksum for.
******************************************************************************/
static int ComputeChecksum(LPSTR s,int sl)
	{
	int i,w,sum;

	sum=0;
	for(i=0;i<sl;i++)
		{
		w=3-((i&1)<<1);
		sum+=w*(s[i]-'0');
		}

	sum%=10;
	sum=10-sum;
	sum%=10;

	return sum;
	}

/******************************************************************************
	ok = GlyphMapAppendChar(c)

	Appends the Glyph index for the character c to GlyphMap.
	

	int c = character to append Glyph index for.
******************************************************************************/
static BOOL GlyphMapAppendChar(int c)
	{
	if(c<'0' || c>'9')
		return FALSE;
	GlyphMapAppend(c-'0');
	return TRUE;
	}

/******************************************************************************
	GlyphMapAppend(i)

	Appends a Glyph index to the GlyphMap.

	int i = Glyph index to append.
******************************************************************************/
static void GlyphMapAppend(int i)
	{
	GlyphMap[GlyphMapLen++]=i;
	}

/******************************************************************************
	len=GlyphLen(index)

	Returns the length of a given glyph or guard glyph in x modules.

	int index = index of glyph to lookup (can be for a guard glyph too)
	int len = number of x modules required to represent this glyph.
******************************************************************************/
static int GlyphLen(int index)
	{
	if(index>=0 && index <= 9)
		return XSGLYPH;
	if(index==giLRG)
		return XSLRG;
	if(index==giCG)
		return XSCG;
	return 0;
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
	newx = BitsAppendGlyph(bits,dx,x,gi,even)

	Appends a barcode glyph by looking it up and appeniding all of its
	modules.  Maps the glyph from Odd to Even parity if needed.

	int newx = x after appending the glyph
	BYTE *bits = start of scanline to append glyph to.
	int dx = module x dimension in pixels
	int gi = Glyph index.
	BOOL even = TRUE for even parity
******************************************************************************/
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi,BOOL even)
	{
	int i;
	register WORD glyph;

	glyph=Glyph[gi];
	if(even)
		glyph^=255;
	i=GlyphLen(gi);
	while(i--)
		x=BitsAppendModule(bits,dx,x,(glyph>>i)&1);
	return x;
	}
