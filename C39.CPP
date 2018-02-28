/******************************************************************************
	NAME : C39.C
	DESC : Code 39 barcode implementation for IMS.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <memory.h>
#include <imslib32\imslib32.h>
#include <jvmath32\jvmath32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "editor.h"
#include "c39.h"
#include "lbmp.h"
#include "stdloc.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define NB   1     /* Narrow bar width in strokes */
#define WB   3     /* Wide bar width in strokes */
#define TS   1     /* Transition space width in strokes */

#define XSGLYPH ((3*WB)+(6*NB)) // Number of strokes per glyph

// Special Glyph indexes
#define giSTARTN    0 // Start of number values
#define giSTARTC   10 // Start of alpha characters
#define giSUB      36
#define giPOINT    37
#define giSPACE    38
#define giDOLLAR   39
#define giDIV      40
#define giADD      41
#define giPERCENT  42
#define giMUL      43

/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,BOOL fa,BOOL addsum);
static BOOL GlyphMapAppendChar(int c);
static void GlyphMapAppend(int i);
static int GetGlyphIndex(int i);
static LPCSTR FullAsciiLookUp(int i);
static int BitsAppendModule(BYTE *bits,int dx,int x,BOOL dark);
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi);
static char ComputeC39Checksum(LPSTR s,int slen,BOOL fa);

/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static int GlyphMap[256]; // Indexes into Glyph[]
static int GlyphMapLen=0;

// Code 39 bitmap glyph data in the XSGLYPH LSBs read from left to right)
// The checksum value of the character is the same as the sum its index.
static WORD Glyph[44]=
	{
	0x0034,  // "0 0011 0100",  /* 0 */
	0x0121,  // "1 0010 0001",  /* 1 */
	0x0061,  // "0 0110 0001",  /* 2 */
	0x0160,  // "1 0110 0000",  /* 3 */
	0x0031,  // "0 0011 0001",  /* 4 */
	0x0130,  // "1 0011 0000",  /* 5 */
	0x0070,  // "0 0111 0000",  /* 6 */
	0x0025,  // "0 0010 0101",  /* 7 */
	0x0124,  // "1 0010 0100",  /* 8 */
	0x0064,  // "0 0110 0100"   /* 9 */
	0x0109,  // "1 0000 1001",  /* A */
	0x0049,  // "0 0100 1001",  /* B */
	0x0148,  // "1 0100 1000",  /* C */
	0x0019,  // "0 0001 1001",  /* D */
	0x0118,  // "1 0001 1000",  /* E */
	0x0058,  // "0 0101 1000",  /* F */
	0x000d,  // "0 0000 1101",  /* G */
	0x010c,  // "1 0000 1100",  /* H */
	0x004c,  // "0 0100 1100",  /* I */
	0x001c,  // "0 0001 1100",  /* J */
	0x0103,  // "1 0000 0011",  /* K */
	0x0043,  // "0 0100 0011",  /* L */
	0x0142,  // "1 0100 0010",  /* M */
	0x0013,  // "0 0001 0011",  /* N */
	0x0112,  // "1 0001 0010",  /* O */
	0x0052,  // "0 0101 0010",  /* P */
	0x0007,  // "0 0000 0111",  /* Q */
	0x0106,  // "1 0000 0110",  /* R */
	0x0046,  // "0 0100 0110",  /* S */
	0x0016,  // "0 0001 0110",  /* T */
	0x0181,  // "1 1000 0001",  /* U */
	0x00c1,  // "0 1100 0001",  /* V */
	0x01c0,  // "1 1100 0000",  /* W */
	0x0091,  // "0 1001 0001",  /* X */
	0x0190,  // "1 1001 0000",  /* Y */
	0x00d0,  // "0 1101 0000"   /* Z */
	0x0085,  // "0 1000 0101";  /* - */    36
	0x0184,  // "1 1000 0100";  /* . */
	0x00c4,  // "0 1100 0100";  /* SPACE */
	0x00a8,  // "0 1010 1000";  /* $ */
	0x00a2,  // "0 1010 0010";  /* / */
	0x008a,  // "0 1000 1010";  /* + */
	0x002a,  // "0 0010 1010";  /* % */
	0x0094,  // "0 1001 0100";  /* * */
	};

static char CheckSumChar[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

// The full ascii lookup table
static char *FullAsciiTable[128]=
	{
	"%U",  //  0
	"$A",  //  1
	"$B",  //  2
	"$C",  //  3
	"$D",  //  4
	"$E",  //  5
	"$F",  //  6
	"$G",  //  7
	"$H",  //  8
	"$I",  //  9
	"$J",  // 10
	"$K",  // 11
	"$L",  // 12
	"$M",  // 13
	"$N",  // 14
	"$O",  // 15
	"$P",  // 16
	"$Q",  // 17
	"$R",  // 18
	"$S",  // 19
	"$T",  // 20
	"$U",  // 21
	"$V",  // 22
	"$W",  // 23
	"$X",  // 24
	"$Y",  // 25
	"$Z",  // 26
	"%A",  // 27
	"%B",  // 28
	"%C",  // 29
	"%D",  // 30
	"%E",  // 31
	" ",  // 32
	"/A",  // 33
	"/B",  // 34
	"/C",  // 35
	"/D",  // 36
	"/E",  // 37
	"/F",  // 38
	"/G",  // 39
	"/H",  // 40
	"/I",  // 41
	"/J",  // 42
	"/K",  // 43
	"/L",  // 44
	"-",  // 45
	".",  // 46
	"/O",  // 47
	"0",  // 48
	"1",  // 49
	"2",  // 50
	"3",  // 51
	"4",  // 52
	"5",  // 53
	"6",  // 54
	"7",  // 55
	"8",  // 56
	"9",  // 57
	"/Z",  // 58
	"%F",  // 59
	"%G",  // 60
	"%H",  // 61
	"%I",  // 62
	"%J",  // 63
	"%V",  // 64
	"A",  // 65
	"B",  // 66
	"C",  // 67
	"D",  // 68
	"E",  // 69
	"F",  // 70
	"G",  // 71
	"H",  // 72
	"I",  // 73
	"J",  // 74
	"K",  // 75
	"L",  // 76
	"M",  // 77
	"N",  // 78
	"O",  // 79
	"P",  // 80
	"Q",  // 81
	"R",  // 82
	"S",  // 83
	"T",  // 84
	"U",  // 85
	"V",  // 86
	"W",  // 87
	"X",  // 88
	"Y",  // 89
	"Z",  // 90
	"%K",  // 91
	"%L",  // 92
	"%M",  // 93
	"%N",  // 94
	"%O",  // 95
	"%W",  // 96
	"+A",  // 97
	"+B",  // 98
	"+C",  // 99
	"+D",  // 100
	"+E",  // 101
	"+F",  // 102
	"+G",  // 103
	"+H",  // 104
	"+I",  // 105
	"+J",  // 106
	"+K",  // 107
	"+L",  // 108
	"+M",  // 109
	"+N",  // 110
	"+O",  // 111
	"+P",  // 112
	"+Q",  // 113
	"+R",  // 114
	"+S",  // 115
	"+T",  // 116
	"+U",  // 117
	"+V",  // 118
	"+W",  // 119
	"+X",  // 120
	"+Y",  // 121
	"+Z",  // 122
	"%P",  // 123
	"%Q",  // 124
	"%R",  // 125
	"%S",  // 126
	"%T"   // 127
	};

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	hbmp = C39RenderBitmap(m,t)

	Renders the supplied text to a bitmap image of its Code 39 barcode
	following the format described in the mark.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
******************************************************************************/
HBITMAP C39RenderBitmap(IMPMARK *m,LPSTR t)
	{
	int xsbits,ysbits; // Bitmap dimensions in bits
	int xsbytes;       // Bitmap scanline width in bytes (scans word aligned)
	BYTE *bits;        // Bitmap bits fixed on local heap
	int x;             // X pixel address during scanline computation
	int i;             // General purpose loops
	HBITMAP hbmp;      // Handle of newly created bitmap
	LPSTR dsc;
	WORD ldsc;
	char hrpict[IMPLC39HRPICTURE+1]; // Local hrpict used for autocreation of hrpict
	char hrbuf[IMPLC39HRPICTURE+1];
	int hrlen;
	SIZE te;
	HFONT hfont;
	int ix;
	int hrsize;
	
	if(m->c39.overlayhr)
		{
		if(m->c39.hrpict[0]==0)
			{
			hrlen=lstrlen(t);
			int pi;
			for(pi=0;pi<hrlen;pi++)
				hrpict[pi]='X';
			hrpict[pi]=0;
			}
		else
			lstrcpy(hrpict,m->c39.hrpict);
		}

	// compute dsc ldsc - updated for Win32 change in int size
	int sz = sizeof(int);  // 4
	int UpToT = 5+7*sz;    // 33
	ldsc=UpToT+lstrlen(t)+1+IMPLC39HRPICTURE;
	dsc=new char[ldsc];
	lstrcpy(dsc,"[C39]");                    // 5
	*(int *)(dsc+5)=m->c39.dx;          // 4 - was 2
	*(int *)(dsc+5+sz)=m->c39.dy;          // 4 - was 2
	*(int *)(dsc+5+2*sz)=m->c39.qz;          // 4 ...
	*(int *)(dsc+5+3*sz)=m->c39.neg;        // 4 ...
	*(int *)(dsc+5+4*sz)=m->c39.fa;         // 4 ...
	*(int *)(dsc+5+5*sz)=m->c39.addsum;     // 4 ... 
	*(int *)(dsc+5+6*sz)=m->c39.overlayhr;  // 4 ...
	lstrcpy(dsc+UpToT,t);                       // lstrlen(t)+1
	if(m->c39.overlayhr)
		lstrcpy(dsc+UpToT+lstrlen(t)+1,hrpict); // IMPLC39HRPICTURE

	// Check for cache hit
	hbmp=LoadedBitmapsFind(dsc,ldsc);
	if(hbmp)
		{
		delete [] dsc;
		return hbmp;
		}

	if(!ComputeGlyphMap(t,m->c39.fa,m->c39.addsum))
		{
		delete [] dsc;
		return 0;
		}

	if(m->c39.overlayhr)
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
	xsbits=m->c39.dx*(2*m->c39.qz+(2+GlyphMapLen)*(XSGLYPH+TS)-TS);  // zzz - May need change
	if(xsbits<(int)te.cx) // Adjust ix and bitmap x dimension if hr is larger
		{
		ix=(te.cx-xsbits)>>1;
		xsbits=te.cx;
		}
	else
		ix=0;
	ysbits=m->c39.dy;
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
	for(i=0;i<m->c39.qz;i++)                           // Leading QZ
		x=BitsAppendModule(bits,m->c39.dx,x,FALSE);

	x=BitsAppendGlyph(bits,m->c39.dx,x,giMUL); // Start char

	for(i=0;i<GlyphMapLen;i++)                     // Data C K
		{
		x=BitsAppendModule(bits,m->c39.dx*TS,x,FALSE);  // Append transition space
		x=BitsAppendGlyph(bits,m->c39.dx,x,GlyphMap[i]);
		}

	x=BitsAppendModule(bits,m->c39.dx*TS,x,FALSE);  // Append transition space
	x=BitsAppendGlyph(bits,m->c39.dx,x,giMUL); // Stop char

	for(i=0;i<m->c39.qz;i++)                       // Ending QZ
		x=BitsAppendModule(bits,m->c39.dx,x,FALSE);

	// Replicate first scan to remaining scans
	for(i=1;i<ysbits;i++)
		memcpy(bits+i*xsbytes,bits,xsbytes);

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
	if(m->c39.overlayhr)
		{
		
		RECT r;
		HFONT of=(HFONT)SelectObject(bdc,hfont);
		if(hrsize<0)
			hrsize*=-1;
		if(m->c39.overlayhr==HRTOP)
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
	if(m->c39.neg)
		PatBlt(bdc,0,0,xsbits,ysbits,PATINVERT);

	SelectObject(bdc,ob);
	DeleteDC(bdc);

	LoadedBitmapsAdd(dsc,ldsc,hbmp);
	delete [] dsc;
	return hbmp;
	}

/******************************************************************************
	C39Init(m)

	Initializes a new IMPC39MARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void C39Init(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->type=mtC39;
	m->c39.xres=100.0;
	m->c39.yres=100.0;
	m->c39.dx=1;
	m->c39.dy=24;
	m->c39.qz=0;
	m->c39.ms=1;
	m->c39.neg=FALSE;
	m->c39.overlayhr=HRNONE;
	m->c39.text[0]=0;
	m->c39.hrpict[0]=0;
	m->c39.fa=FALSE;
	m->c39.addsum=FALSE;
	}

/******************************************************************************
	C39MARKEDITDP

	mtC39 editor dialog process.
******************************************************************************/
BOOL CALLBACK C39MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
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

			// IDC_DEG
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
			FeditSetVal(hctrl,EditorMark.c39.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.c39.yres/EditYScale);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=8;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c39.dx);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=39;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c39.dy);

			// IDC_QZONE
			hctrl=GetDlgItem(hdlg,IDC_QZONE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=30;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c39.qz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c39.ms);

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
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.c39.neg);

			// IDC_FA
			CheckDlgButton(hdlg,IDC_FA,EditorMark.c39.fa);

			// IDC_ADDSUM
			CheckDlgButton(hdlg,IDC_ADDSUM,EditorMark.c39.addsum);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC39;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c39.text);

			// IDC_HRPICT
			hctrl=GetDlgItem(hdlg,IDC_HRPICT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC39HRPICTURE;
			kinfo.digits=0;
			kinfo.lowers=0;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c39.hrpict);

			// IDC_HROVERLAY
			switch(EditorMark.c39.overlayhr)
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

		case WM_COMMAND: //Win32 changed
			{
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			HWND hctrl;
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
							||(!FeditGetVal(hctrl,&EditorMark.c39.xres))
							||(EditorMark.c39.xres < 10.0)
							||(EditorMark.c39.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c39.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.c39.yres))
							||(EditorMark.c39.yres < 10.0)
							||(EditorMark.c39.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c39.yres*=EditYScale;

						// IDC_XDOTS
    	                hctrl=GetDlgItem(hdlg,IDC_XDOTS);
	                    if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
	                        {
	                        ShowError(IDE_BADXDOTS);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }
	                    EditorMark.c39.dx=(WORD)dw;

						// IDC_YDOTS
    	                hctrl=GetDlgItem(hdlg,IDC_YDOTS);
	                    if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
	                        {
	                        ShowError(IDE_BADYDOTS);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }
	                    EditorMark.c39.dy=(WORD)dw;

						// IDC_QZONE
    	                hctrl=GetDlgItem(hdlg,IDC_QZONE);
	                    if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
	                        {
	                        ShowError(IDE_BADQZONE);
	                        SetFocus(hctrl);
	                        return TRUE;
	                        }
	                    EditorMark.c39.qz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c39.ms=(WORD)dw;

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
						EditorMark.c39.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

						// IDC_FA
						EditorMark.c39.fa=IsDlgButtonChecked(hdlg,IDC_FA);
						// IDC_ADDSUM
						EditorMark.c39.addsum=IsDlgButtonChecked(hdlg,IDC_ADDSUM);
                        // IDC_WEXT
                        hctrl=GetDlgItem(hdlg,IDC_WEXT);
                        if((!KeditValidate(hctrl))||
                            (!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c39.text)))||
                            (!EditorMark.c39.text[0]))
                            {
                            ShowError(IDE_BADTEXT);
                            SetFocus(hctrl);
                            return TRUE;
                            }

                        // IDC_HRPICT
                        hctrl=GetDlgItem(hdlg,IDC_HRPICT);
                        if((!KeditValidate(hctrl)) ||
                            (!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c39.hrpict))))
                            {
                            ShowError(IDE_BADHRPICT);
                            SetFocus(hctrl);
                            return TRUE;
                            }

	                    // IDC_HROVERLAYx
						EditorMark.c39.overlayhr=
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
	ok=ComputeGlyphMap(s,fa,addsum)

	Computes the GlyphMap for the given string including the checksum when
	specified but not including start and stop.  GlyphMap is an array of indexes into
	Glyps,  which are the bit patterns for each CODE 39 barcode character.

	BOOL ok = TRUE on success.
	LPSTR s = string to compute GlyphMap for.
	int slen = the number of characters contained in s.
	BOOL fa = Expand to full ASCII
	BOOL addsum = Add checksum
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,BOOL fa,BOOL addsum)
	{
	LPCSTR fas;
	int si;

	GlyphMapLen=0;
	int slen=lstrlen(s);
	for(si=0;si<slen;si++)  // For each character in s
		{
		if(fa)
			{
			fas=FullAsciiLookUp(s[si]);
			if(!fas)
				return FALSE;
			if(!GlyphMapAppendChar(fas[0])) // Append its rep to GlyphMap
				return FALSE;
			if(fas[1])
				if(!GlyphMapAppendChar(fas[1])) // Append its rep to GlyphMap
					return FALSE;
			}
		else
			{
			if(!GlyphMapAppendChar(s[si])) // Append its rep to GlyphMap
				return FALSE;
			}
		}

	if(addsum)
		GlyphMapAppendChar(ComputeC39Checksum(s,slen,fa));    // Append its rep to GlyphMap

	if(!GlyphMapLen) // Error if no characters mapped
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
	int i;
	i=GetGlyphIndex(c);
	if(i<0)
		return FALSE;
	GlyphMapAppend(i);
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
	index=GetGlyphIndex(i)

	Retrieves the glyph index for the value of i.

	int index = The glyph index. -1 if invalid.
	int i = Character value.
******************************************************************************/
static int GetGlyphIndex(int i)
	{
	if(i>=48 && i<=57)
		return giSTARTN+(i-48);
	else if(i>=65 && i<=90)
		return giSTARTC+(i-65);
	else if(i==32)
		return giSPACE;
	else if(i==45)
		return giSUB;
	else if(i==46)
		return giPOINT;
	else if(i==36)
		return giDOLLAR;
	else if(i==47)
		return giDIV;
	else if(i==43)
		return giADD;
	else if(i==37)
		return giPERCENT;

	return -1;
	}

/******************************************************************************
	sp=FullAsciiLookUp(i);

	Retrieves the full ascii string value of i.

	LPCSTR sp = Pointer to the full ascii string.
	int i = The full ascii value.
******************************************************************************/
static LPCSTR FullAsciiLookUp(int i)
	{
	if(i<0||i>127)
		return NULL;
	return (LPCSTR)FullAsciiTable[i];
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
	newx = BitsAppendGlyph(bits,dx,x,gi)

	Appends a barcode glyph by looking it up and appending all of its
	modules.

	int newx = x after appending the glyph
	BYTE *bits = start of scanline to append glyph to.
	int dx = module x dimension in pixels
	int x = x before appending the glyph
	int gi = Glyph index to append.
******************************************************************************/
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi)
	{
	int i;
	register WORD glyph;

	glyph=Glyph[gi];

	i=9;
	while(i--)
		x=BitsAppendModule(bits,(glyph>>i)&1 ? (dx*WB) : dx ,x,(i&1) ? FALSE : TRUE);
	return x;
	}

/******************************************************************************
	sum = ComputeC39Checksum(s,slen,fa)

	Computes the Code 39 checksum C.

	LPSTR s = Barcode data string to compute checksum for.
	int slen = The length of the barcode data string.
	BOOL fa = use full ascii set.
******************************************************************************/
static char ComputeC39Checksum(LPSTR s,int slen,BOOL fa)
	{
	int cs=0;
	int si;
	int cindex;
	LPCSTR fas;
	int ci;

	for(si=0;si<slen;si++)
		{
		if(fa)
			{
			fas=FullAsciiLookUp(s[si]);
			if(!fas)
				return '?';
			ci=0;
			while(fas[ci])
				{
				if((cindex=GetGlyphIndex(fas[ci++]))<0)
					return '?';
				cs+=cindex;
				}
			}
		else
			{
			if((cindex=GetGlyphIndex(s[si]))<0)
				return '?';
			cs+=cindex;
			}
		}
	cs%=43;
	return CheckSumChar[cs];
	}

