/******************************************************************************
	NAME : C128.C
	DESC : Code 128 and UCC-128 barcode implementation for IMS.
		   Currently Control Characters and Set A are unused.
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
#include "c128.h"
#include "lbmp.h"
#include "stdloc.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define XSGLYPH 11 // Number of modules per glyph
#define XSTERM  2  // Number of addional modules printed in terminator bar

// Special Glyph indexes
#define giB0      16 // '0' in set B
#define giF3      96
#define giF2      97
#define giTOC     99
#define giTOB    100
#define giTOA    101
#define giF1     102
#define giSTARTA 103
#define giSTARTB 104
#define giSTARTC 105
#define giSTOP   106


/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static int GlyphMap[256]; // Indexes into Glyph[]
static int GlyphMapLen=0;

static int CBuf[64]; // Compression buffer
static int CLen;     // Length of CBuf

// Code128 bitmap glyph data in the XSGLYPH LSBs read from left to right)
// The checksum value of the character is the same as its index.
static WORD Glyph[107]=
	{       // A   B   C
	0x06CC, // sp  sp  00
	0x066C, // !   !   01
	0x0666, // "   "   02
	0x0498, // #   #   03
	0x048C, // $   $   04
	0x044C, // %   %   05
	0x04C8, // &   &   06
	0x04C4, // '   '   07
	0x0464, // (   (   08
	0x0648, // )   )   09
	0x0644, // *   *   10
	0x0624, // +   +   11
	0x059C, // ,   ,   12
	0x04DC, // -   -   13
	0x04CE, // .   .   14
	0x05CC, // /   /   15
	0x04EC, // 0   0   16
	0x04E6, // 1   1   17
	0x0672, // 2   2   18
	0x065C, // 3   3   19
	0x064E, // 4   4   20
	0x06E4, // 5   5   21
	0x0674, // 6   6   22
	0x076E, // 7   7   23
	0x074C, // 8   8   24
	0x072C, // 9   9   25
	0x0726, // :   :   26
	0x0764, // ;   ;   27
	0x0734, // <   <   28
	0x0732, // =   =   29
	0x06D8, // >   >   30
	0x06C6, // ?   ?   31
	0x0636, // @   @   32
	0x0518, // A   A   33
	0x0458, // B   B   34
	0x0446, // C   C   35
	0x0588, // D   D   36
	0x0468, // E   E   37
	0x0462, // F   F   38
	0x0688, // G   G   39
	0x0628, // H   H   40
	0x0622, // I   I   41
	0x05B8, // J   J   42
	0x058E, // K   K   43
	0x046E, // L   L   44
	0x05D8, // M   M   45
	0x05C6, // N   N   46
	0x0476, // O   O   47
	0x0776, // P   P   48
	0x068E, // Q   Q   49
	0x062E, // R   R   50
	0x06E8, // S   S   51
	0x06E2, // T   T   52
	0x06EE, // U   U   53
	0x0758, // V   V   54
	0x0746, // W   W   55
	0x0716, // X   X   56
	0x0768, // Y   Y   57
	0x0762, // Z   Z   58
	0x071A, // [   [   59
	0x077A, // \   \   60
	0x0642, // ]   ]   61
	0x078A, // ^   ^   62
	0x0530, // _   _   63
	0x050C, // \0  `   64
	0x04B0, // ^A  a   65
	0x0486, // ^B  b   66
	0x042C, // ^C  c   67
	0x0426, // ^D  d   68
	0x0590, // ^E  e   69
	0x0584, // ^F  f   70
	0x04D0, // ^G  g   71
	0x04C2, // ^H  h   72
	0x0434, // ^I  i   73
	0x0432, // ^J  j   74
	0x0612, // ^K  k   75
	0x0650, // ^L  l   76
	0x07BA, // ^M  m   77
	0x0614, // ^N  n   78
	0x047A, // ^O  o   79
	0x053C, // ^P  p   80
	0x04BC, // ^Q  q   81
	0x049E, // ^R  r   82
	0x05E4, // ^S  s   83
	0x04F4, // ^T  t   84
	0x04F2, // ^U  u   85
	0x07A4, // ^V  v   86
	0x0794, // ^W  w   87
	0x0792, // ^X  x   88
	0x06DE, // ^Y  y   89
	0x06F6, // ^Z  z   90
	0x07B6, // esc {   91
	0x0578, // FS  |   92
	0x051E, // GS  }   93
	0x045E, // RS  ~   94
	0x05E8, // US  DEL 95
	0x05E2, // F3  F3  96
	0x07A8, // F2  F2  97
	0x07A2, // SFT SFT 98
	0x05DE, // <C> <C> 99
	0x05EE, // <B> F4  <B> V=100
	0x075E, // F4  <A> <A>   101
	0x07AE, // F1  F1  F1    102
	0x0684, // <A START>     103
	0x0690, // <B START>     104
	0x069C, // <C START>     105
	0x063A  // <STOP>        106
	};

/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,C128CSET cset);
static BOOL GlyphMapAppendAChar(int c);
static BOOL GlyphMapAppendBChar(int c);
static BOOL GlyphMapAppendCChars(int c1,int c2);
static void GlyphMapOptimize();
static void GlyphMapAppendSum(void);
static void GlyphMapAppend(int i);
static int BitsAppendModule(BYTE *bits,int dx,int x,BOOL dark);
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi);
static BOOL isGlyphDigit(int i);
static void CBufAppend(int i);
static void CBufAppendCompressed(int dsi,int dsl);
static int CBufReplace(int dsi,int dsl);

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	hbmp = C128RenderBitmap(m,t,hrt)

	Renders the supplied text to a bitmap image of its Code 128 barcode
	following the format described in the mark.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
	LPSTR hrt = Text for human readable overlay.
******************************************************************************/
HBITMAP C128RenderBitmap(IMPMARK *m,LPSTR t,LPSTR hrt)
	{
	int xsbits,ysbits; // Bitmap dimensions in bits
	int xsbytes;       // Bitmap scanline width in bytes (scans word aligned)
	BYTE *bits;        // Bitmap bits fixed on local heap
	int x;             // X pixel address during scanline computation
	int i;             // General purpose loops
	HBITMAP hbmp;      // Handle of newly created bitmap
	LPSTR dsc;
	WORD ldsc;

	LPSTR ti;
	char hrbuf[IMPLC128HRPICTURE+1];
	int hrlen; //pi;
	LOGFONT lf;
	HFONT hfont,ohfont;
	HDC hdcscreen,hdc;
	HBITMAP ohbmp;
	SIZE te;
	RECT r;
//zzz	LPSTR tp;
//	char c;
	int ix, hrsize, yoffset;
	
	if(m->c128.overlayhr)
		{
		if(hrt[0])
			lstrcpyn(hrbuf,hrt,IMPLC128HRPICTURE);
		else
			lstrcpyn(hrbuf,t,IMPLC128HRPICTURE);
		}

	// compute dsc ldsc  - Changed for Win32 , new int size
	int sz = sizeof(int); // 4
	int uptot = 6+5*sz;  // 26 - was 16
	ldsc=uptot+lstrlen(t)+1+IMPLC128HRPICTURE+1;
	dsc=new char[ldsc];
	lstrcpy(dsc,"[C128]");                   // 6
	*(int *)(dsc+6)=m->c128.dx;         // 4 - was 2
	*(int *)(dsc+6+sz)=m->c128.dy;         // 4 - was 2
	*(int *)(dsc+6+2*sz)=m->c128.qz;        // 4 - was 2
	*(int *)(dsc+6+3*sz)=m->c128.neg;       // 4 - was 2
	*(int *)(dsc+6+4*sz)=m->c128.overlayhr; // 4 - was 2
	lstrcpy(dsc+uptot,t);                       // lstrlen(t)+1
	if(m->c128.overlayhr)
		lstrcpy(dsc+uptot+lstrlen(t)+1,hrbuf); // IMPLC128HRPICTURE
	*(BYTE *)(dsc+uptot+lstrlen(t)+1+IMPLC128HRPICTURE)=m->c128.cset; // 1	

	// Check for cache hit
	hbmp=LoadedBitmapsFind(dsc,ldsc);
	if(hbmp)
		{
		delete [] dsc;
		return hbmp;
		}

	if(!ComputeGlyphMap(t,(C128CSET)m->c128.cset))
		{
		delete [] dsc;
		return 0;
		}

	if(m->c128.overlayhr)
		{
		yoffset=GetPrivateProfileInt(IMSName,"BCFONTYOFFSET",0,IMSIni);

		GetIniLOGFONT(IMSName,"BCLOGFONT","-8,0,0,0,400,0,0,0,0,3,2,1,34,Arial",&lf,IMSIni);

		hrsize=lf.lfHeight;
		// compute hrbuf and hrlen text

		hrlen=lstrlen(hrbuf);
		if(hrt[0])
			{
			ti=t;
			for(i=0;i<hrlen;i++)
				{
				if(hrbuf[i]=='X'&&*ti)
					{
					hrbuf[i]=*ti;
					ti++;
					}
				}
			}

		hfont=CreateFontIndirect(&lf);
		hdc=GetDC(NULL);
		ohfont=(HFONT)SelectObject(hdc,hfont);
		GetTextExtentPoint32(hdc,hrbuf,hrlen,&te);
		SelectObject(hdc,ohfont);
		ReleaseDC(NULL,hdc);
		}
	else
	{
		te.cx=0;
		te.cy=0;
	}
	// Compute bitmap dimensions
	xsbits=m->c128.dx*(2*m->c128.qz+GlyphMapLen*XSGLYPH+XSTERM); // zzz - May need to change

	if(xsbits<(int)te.cx) // Adjust ix and bitmap x dimension if hr is larger
		{
		ix=(te.cx-xsbits)>>1;
		xsbits=te.cx;
		}
	else
		ix=0;

	ysbits=m->c128.dy;
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
	x=ix;                                           // Start x at left edge
	for(i=0;i<m->c128.qz;i++)                      // Leading QZ
		x=BitsAppendModule(bits,m->c128.dx,x,FALSE);
	for(i=0;i<GlyphMapLen;i++)                     // START DATA SUM STOP
		x=BitsAppendGlyph(bits,m->c128.dx,x,GlyphMap[i]);
	x=BitsAppendModule(bits,m->c128.dx*XSTERM,x,TRUE); // Terminator Bar
	for(i=0;i<m->c128.qz;i++)                      // Ending QZ
		x=BitsAppendModule(bits,m->c128.dx,x,FALSE);

	// Replicate first scan to remaining scans
	for(i=1;i<ysbits;i++)
		memcpy(bits+i*xsbytes,bits,xsbytes);

	// Create the bitmap
	hbmp=CreateBitmap(xsbits,ysbits,1,1,bits);

	delete [] bits;

	// Create DC to bitmap for modifictions
	hdcscreen=GetDC(NULL);
	hdc=CreateCompatibleDC(hdcscreen);
	ReleaseDC(NULL,hdcscreen);
	ohbmp=(HBITMAP)SelectObject(hdc,hbmp);

	// Overlay human readable
	if(m->c128.overlayhr)
		{
		ohfont=(HFONT)SelectObject(hdc,hfont);
		if(hrsize<0)
			hrsize*=-1;

		if(m->c128.overlayhr==HRTOP)
			{
			r.top=yoffset;
			r.bottom=hrsize;
			}
		else 
			{
			r.top=ysbits-hrsize;
			r.bottom=r.top+hrsize;
			}
		r.left=0;r.right=xsbits;
		FillRect(hdc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
		SetBkMode(hdc,TRANSPARENT);
		TextOut(hdc,(xsbits-te.cx)>>1,r.top,hrbuf,hrlen);
		SelectObject(hdc,ohfont);
		DeleteObject(hfont);
		}

	// Negate the entire bitmap if neg
	if(m->c128.neg)
		PatBlt(hdc,0,0,xsbits,ysbits,PATINVERT);

	SelectObject(hdc,ohbmp);
	DeleteDC(hdc);

	LoadedBitmapsAdd(dsc,ldsc,hbmp);
	delete [] dsc;
	return hbmp;
	}

/******************************************************************************
	C128Init(m)

	Initializes a new IMPC128MARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void C128Init(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->type=mtC128;
	m->c128.xres=100.0;
	m->c128.yres=100.0;
	m->c128.dx=1;
	m->c128.dy=24;
	m->c128.qz=0;
	m->c128.ms=1;
	m->c128.neg=FALSE;
	m->c128.overlayhr=HRNONE;
	m->c128.text[0]=0;
	m->c128.hrpict[0]=0;
	m->c128.cset=C128OPT;
	m->c128.reserved=0;
	}

/******************************************************************************
	C128MARKEDITDP

	mtC128 editor dialog process.
******************************************************************************/
BOOL CALLBACK C128MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
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

			// IDC_XRES
			hctrl=GetDlgItem(hdlg,IDC_XRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.c128.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.c128.yres/EditYScale);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=8;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c128.dx);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c128.dy);

			// IDC_QZONE
			hctrl=GetDlgItem(hdlg,IDC_QZONE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=30;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c128.qz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c128.ms);

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
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.c128.neg);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC128;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c128.text);

			// IDC_C128CSET
			hctrl=GetDlgItem(hdlg,IDC_C128CSET);
			// Init it
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"Optimize");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"A");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"B");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"C");
			SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)"UCC128");
			// Select current cset
			SendMessage(hctrl,CB_SETCURSEL,EditorMark.c128.cset,0);

			// IDC_HRPICT
			hctrl=GetDlgItem(hdlg,IDC_HRPICT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC128HRPICTURE;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c128.hrpict);

			// IDC_HROVERLAY
			switch(EditorMark.c128.overlayhr)
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

		case WM_COMMAND://Changed Win32
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
						EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_XRES
						hctrl=GetDlgItem(hdlg,IDC_XRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.c128.xres))
							||(EditorMark.c128.xres < 10.0)
							||(EditorMark.c128.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c128.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.c128.yres))
							||(EditorMark.c128.yres < 10.0)
							||(EditorMark.c128.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c128.yres*=EditYScale;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c128.dx=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c128.dy=(WORD)dw;

						// IDC_QZONE
						hctrl=GetDlgItem(hdlg,IDC_QZONE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADQZONE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c128.qz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c128.ms=(WORD)dw;

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
						EditorMark.c128.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						if((!KeditValidate(hctrl))||
							(!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c128.text)))||
							(!EditorMark.c128.text[0]))
							{
							ShowError(IDE_BADTEXT);
							SetFocus(hctrl);
							return TRUE;
							}

						// IDC_C128CSET
						hctrl=GetDlgItem(hdlg,IDC_C128CSET);
						EditorMark.c128.cset=(C128CSET)SendMessage(hctrl,CB_GETCURSEL,0,0);

						// IDC_HRPICT
						hctrl=GetDlgItem(hdlg,IDC_HRPICT);
						if((!KeditValidate(hctrl)) ||
							(!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c128.hrpict))))
							{
							ShowError(IDE_BADHRPICT);
							SetFocus(hctrl);
							return TRUE;
							}

						// IDC_HROVERLAY
						// IDC_HROVERLAY1
						// IDC_HROVERLAY2
						EditorMark.c128.overlayhr=HRNONE;
						if(IsDlgButtonChecked(hdlg,IDC_HROVERLAY2))
							EditorMark.c128.overlayhr=HRBOTTOM;
						else if(IsDlgButtonChecked(hdlg,IDC_HROVERLAY1))
							EditorMark.c128.overlayhr=HRTOP;
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
	ok=ComputeGlyphMap(s,cset)

	Computes the GlphMap for the given string including start data checksum
	and stop characters.  Then GlphMapOptimize() is called to optimize
	digit spans with set C.    GlyphMap is an array of indexes into
	Glyps,  wich are the bit patters for each CODE128 barcode character.

	Set A is currently not supported.

	BOOL ok = TRUE on success.
	LPSTR s = string to compute GlyphMap for.
	BYTE cset = (C128CSET) to control character set mapping
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s,C128CSET cset)
	{
	LPSTR p;
	int l;

	// If cset is C then make sure all digits and even length
	if(cset==C128SETC||cset==C128UCC)
		{
		l=0;
		for(p=s;*p;p++)
			{
			if(!('0'<=*p && *p<='9'))
				return FALSE;
			l++;
			}
		if(l & 1)
			return FALSE; // must be even
		}

	// Compute glyph map
	GlyphMapLen=0;
	if(cset==C128SETA)  // with in set A if cset=C128SETA
		{
		GlyphMapAppend(giSTARTA);
		for(p=s;*p;p++)  // For each character in s
			if(!GlyphMapAppendAChar(*p)) // Append its A set rep to GlyphMap
				return FALSE;
		}
	else if(cset==C128UCC)  // with in set A if cset=C128SETA
		{
		GlyphMapAppend(giSTARTC);
		GlyphMapAppend(giF1);
		for(p=s;*p && *(p+1);p+=2) // for each char pair
			if(!GlyphMapAppendCChars(*p,*(p+1))) // Append set C rep to GlyphMap
				return FALSE;
		}
	else // Compute initial glyph map with set B if C B or OPT
		{
		GlyphMapAppend(giSTARTB);
		for(p=s;*p;p++)  // For each character in s
			if(!GlyphMapAppendBChar(*p)) // Append its B set rep to GlyphMap
				return FALSE;
		}
	if(!GlyphMapLen) // Error if no characters mapped
		return FALSE;

	if(cset==C128SETC||cset==C128OPT)
		GlyphMapOptimize(); // Optimize digit spans using set C

	GlyphMapAppendSum();
	GlyphMapAppend(giSTOP);
	return TRUE;
	}

/******************************************************************************
	ok = GlyphMapAppendAChar(c)

	Appends the Glyph indexes for the character c in set B to GlyphMap.

	int c = character to append Glyph indexes for.
******************************************************************************/
static BOOL GlyphMapAppendAChar(int c)
	{
	if(c<1 || c>95) // Set A range except 0
		return FALSE;
	GlyphMapAppend(c<32 ? (c+64) : (c-32));
	return TRUE;
	}


/******************************************************************************
	ok = GlyphMapAppendBChar(c)

	Appends the Glyph indexes for the character c in set B to GlyphMap.

	int c = character to append Glyph indexes for.
******************************************************************************/
static BOOL GlyphMapAppendBChar(int c)
	{
	if(c<32 || c>127)
		return FALSE;
	GlyphMapAppend(c-32);
	return TRUE;
	}

/******************************************************************************
	ok = GlyphMapAppendCChars(c1,c2)

	Appends the Glyph indexe for the character pair c1,c2 in set C to GlyphMap.

	int c1,c2 = character pair to append Glyph indexe for.
******************************************************************************/
static BOOL GlyphMapAppendCChars(int c1,int c2)
	{
	if(c1<'0' || c1>'9' || c2<'0' || c2>'9')
		return FALSE;
	GlyphMapAppend((c1-'0')*10+(c2-'0'));
	return TRUE;
	}

/******************************************************************************
	GlyphMapOptimize()

	Opdtimizes digit spans an GlyphMap using set C
******************************************************************************/
static void GlyphMapOptimize()
	{
	int i;   // GlypMap scanning index
	int dsi; // Digit span index
	int dsl; // Digit span length
	BOOL leading,trailing,even;

	i=0;
	while(i<GlyphMapLen)
		{
		// Scan until digit or end
		if(!isGlyphDigit(GlyphMap[i]))
			{
			i++;
			continue;
			}
		dsi=i;
		dsl=0;
		while(isGlyphDigit(GlyphMap[i]) && i<GlyphMapLen)
			{
			dsl++;
			i++;
			}
		// Now i points to remainder
		//     dsi points to start of span
		//     dsl contains span length

		// Setup flags
		leading=dsi==1;
		trailing=i==GlyphMapLen;
		even=!(dsl&1);
		if(leading)
			{
			if(trailing && dsl==2)
				{
				GlyphMap[0]=giSTARTC;
				CLen=0;
				CBufAppendCompressed(dsi,dsl);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			if(trailing && dsl>3 && even)
				{
				GlyphMap[0]=giSTARTC;
				CLen=0;
				CBufAppendCompressed(dsi,dsl);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			if(dsl>3 && !even)
				{
				GlyphMap[0]=giSTARTC;
				CLen=0;
				CBufAppendCompressed(dsi,dsl-1);
				CBufAppend(giTOB);
				CBufAppend(GlyphMap[i-1]);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			if(!trailing && dsl>3 && even)
				{
				GlyphMap[0]=giSTARTC;
				CLen=0;
				CBufAppendCompressed(dsi,dsl);
				CBufAppend(giTOB);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			}
		else if(trailing) // and not leading
			{
			if(dsl>3 && even)
				{
				CLen=0;
				CBufAppend(giTOC);
				CBufAppendCompressed(dsi,dsl);
				CBufAppend(giTOB);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			if(dsl >3 && !even) // SPECIAL CASE
				{
				CLen=0;
				CBufAppend(GlyphMap[dsi]);
				CBufAppend(giTOC);
				CBufAppendCompressed(dsi+1,dsl-1);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			}
		else // not leading or trailing
			{
			if(dsl>5 && even)
				{
				CLen=0;
				CBufAppend(giTOC);
				CBufAppendCompressed(dsi,dsl);
				CBufAppend(giTOB);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			if(dsl>5 && !even)
				{
				CLen=0;
				CBufAppend(giTOC);
				CBufAppendCompressed(dsi,dsl-1);
				CBufAppend(giTOB);
				CBufAppend(GlyphMap[i-1]);
				i-=CBufReplace(dsi,dsl);
				continue;
				}
			}
		}
	}

/******************************************************************************
	GlyphMapAppendSum()

	Computes and appends the checksum C to the GlyphMap
	C is the mod 103 sum of the start char and the products 
	of the data chars and the sequence 1 1..2..3..
******************************************************************************/
static void GlyphMapAppendSum(void)
	{
	int i,w,sum;

	sum=0;
	for(i=0;i<GlyphMapLen;i++)
		{
		w=i ? i : 1;
		sum+=w*GlyphMap[i];
		}
	sum%=103;
	GlyphMapAppend(sum);
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

	Appends a barcode glyph by looking it up and appeniding all of its
	modules.

	int newx = x after appending the glyph
	BYTE *bits = start of scanline to append glyph to.
	int dx = module x dimension in pixels
	int gi = Glyph index.
******************************************************************************/
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi)
	{
	int i;
	register WORD glyph;

	glyph=Glyph[gi];
	i=XSGLYPH;
	while(i--)
		x=BitsAppendModule(bits,dx,x,(glyph>>i)&1);
	return x;
	}

/******************************************************************************
	isGlyphDigit(i)

	Returns true if the Glyph at index i in set A or B is a digit.

	int i = Glyph index to check.
******************************************************************************/
static BOOL isGlyphDigit(int i)
	{
	register int d;
	d=i-giB0;
	return ((d>=0)&&(d<=9));
	}

/******************************************************************************
	CBufAppend(i)

	Appends a Glyph index to the compression buffer.

	int i = index to append.
******************************************************************************/
static void CBufAppend(int i)
	{
	CBuf[CLen++]=i;
	}


/******************************************************************************
	CBufAppendCompressed(dsi,dsl)

	Appends the compressed set C digit for the digit span specified to
	the compression buffer.

	int dsi = GlyphMap index if start of digit span
	int dsl = length of digit span
******************************************************************************/
static void CBufAppendCompressed(int dsi,int dsl)
	{
	int i;
	int c;

	for(i=0;i<dsl;i+=2)
		{
		c=10*(GlyphMap[dsi+i]-giB0)+(GlyphMap[dsi+i+1]-giB0);
		CBufAppend(c);
		}
	}

/******************************************************************************
	sf = CBufReplace(dsi,dsl)

	Replaces in GlyphMap a span with the rep in CBuf and then removes the
	memory hole.

	int sf = Shrink Facotor (the amount the replacement shrunk GlyphMapLen and
			 the digit span.
	int dsi = digit span index.
	int dsl = original digit span length.
******************************************************************************/
static int CBufReplace(int dsi,int dsl)
	{
	int sf; // Shrink factor
	int rl; // Remainder length
	int i;

	sf=dsl-CLen;
	for(i=0;i<CLen;i++)
		GlyphMap[dsi+i]=CBuf[i];
	rl=GlyphMapLen-(dsi+dsl);
	for(i=0;i<rl;i++)
		GlyphMap[dsi+CLen+i]=GlyphMap[dsi+dsl+i];
	GlyphMapLen-=sf;
	return sf;
	}
