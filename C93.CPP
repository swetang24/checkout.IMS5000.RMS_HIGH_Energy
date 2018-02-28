/******************************************************************************
	NAME : C93.C
	DESC : Code 93 barcode implementation for IMS.
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
#include "c93.h"
#include "lbmp.h"
#include "stdloc.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define XSGLYPH 9 // Number of modules per glyph

/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static BYTE GlyphMap[256]; // Indexes into Glyph[]
static int GlyphMapLen=0;
// Code93 bitmap glyph data in 9 LSB read from left to right)
static WORD Glyph[48]=
	{
	0x0114, // 0
	0x0148, // 1
	0x0144, // 2
	0x0142, // 3
	0x0128, // 4
	0x0124, // 5
	0x0122, // 6
	0x0150, // 7
	0x0112, // 8
	0x010A, // 9
	0x01A8, // A
	0x01A4, // B
	0x01A2, // C
	0x0194, // D
	0x0192, // E
	0x018A, // F
	0x0168, // G
	0x0164, // H
	0x0162, // I
	0x0134, // J
	0x011A, // K
	0x0158, // L
	0x014C, // M
	0x0146, // N
	0x012C, // O
	0x0116, // P
	0x01B4, // Q
	0x01B2, // R
	0x01AC, // S
	0x01A6, // T
	0x0196, // U
	0x019A, // V
	0x016C, // W
	0x0166, // X
	0x0136, // Y
	0x013A, // Z
	0x012E, // -
	0x01D4, // .
	0x01D2, // space
	0x01CA, // $
	0x016E, // /
	0x0176, // +
	0x01AE, // %
	0x0126, // ($)
	0x01DA, // (%)
	0x01D6, // (/)
	0x0132, // (+)
	0x015E  // start/stop
	};

/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s);
static BOOL GlyphMapAppendChar(int c);
static void GlyphMapAppendSumC(void);
static void GlyphMapAppendSumK(void);
static void GlyphMapAppend(int i);
static int BitsAppendModule(BYTE *bits,int dx,int x,BOOL dark);
static int BitsAppendGlyph(BYTE *bits,int dx,int x,int gi);

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	hbmp = C93RenderBitmap(m,t)

	Renders the supplied text to a bitmap image of its Code 93 barcode
	following the format described in the mark.

	HBITMAP hbmp = Handle of new bitmap or NULL on error.
	IMPMARK *m = Mark format information.
	LPSTR t = Text for mark.
******************************************************************************/
HBITMAP C93RenderBitmap(IMPMARK *m,LPSTR t)
	{
	int xsbits,ysbits; // Bitmap dimensions in bits
	int xsbytes;       // Bitmap scanline width in bytes (scans word aligned)
	BYTE *bits;        // Bitmap bits fixed on local heap
	int x;             // X pixel address during scanline computation
	int i;             // General purpose loops
	HBITMAP hbmp;      // Handle of newly created bitmap
	LPSTR dsc;
	WORD ldsc;

	// compute dsc ldsc - updated for Win32 change in int size
	int sz = sizeof(int);
	int UpToT = 5+4*sz;
	ldsc=UpToT+lstrlen(t)+1;
	dsc=new char[ldsc];
	lstrcpy(dsc,"[C93]");             // 5
	*(int *)(dsc+5)=m->c93.dx;   // 4 - was 2
	*(int *)(dsc+5+sz)=m->c93.dy;   // 4 - was 2
	*(int *)(dsc+5+2*sz)=m->c93.qz;   // 4 - was 2
	*(int *)(dsc+5+3*sz)=m->c93.neg; // 4 - was 2
	lstrcpy(dsc+UpToT,t);                // lstrlen(t)+1

	// Check for cache hit
	hbmp=LoadedBitmapsFind(dsc,ldsc);
	if(hbmp)
		{
		delete [] dsc;
		return hbmp;
		}

	if(!ComputeGlyphMap(t))
		{
		delete [] dsc;
		return 0;
		}

	// Compute bitmap dimensions
	xsbits=m->c93.dx*(2*m->c93.qz+(GlyphMapLen+2)*XSGLYPH+1);  // zzz May need to change
	ysbits=m->c93.dy;
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
	x=0;                                           // Start x at left edge
	for(i=0;i<m->c93.qz;i++)                       // Leading QZ
		x=BitsAppendModule(bits,m->c93.dx,x,FALSE);
	x=BitsAppendGlyph(bits,m->c93.dx,x,47);        // Start char
	for(i=0;i<GlyphMapLen;i++)                     // Data C K
		x=BitsAppendGlyph(bits,m->c93.dx,x,GlyphMap[i]);
	x=BitsAppendGlyph(bits,m->c93.dx,x,47);        // Stop char
	x=BitsAppendModule(bits,m->c93.dx,x,TRUE);     // Ending module
	for(i=0;i<m->c93.qz;i++)                       // Ending QZ
		x=BitsAppendModule(bits,m->c93.dx,x,FALSE);

	// Negate the scanline if neg
	if(m->c93.neg)
		for(i=0;i<xsbytes;i++)
			bits[i]^=255;

	// Replicate first scan to remaining scans
	for(i=1;i<ysbits;i++)
		memcpy(bits+i*xsbytes,bits,xsbytes);

	// Create the bitmap
	hbmp=CreateBitmap(xsbits,ysbits,1,1,bits);

	delete [] bits;

	LoadedBitmapsAdd(dsc,ldsc,hbmp);
	delete [] dsc;
	return hbmp;
	}

/******************************************************************************
	C93Init(m)

	Initializes a new IMPC93MARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void C93Init(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->type=mtC93;
	m->c93.xres=100.0;
	m->c93.yres=100.0;
	m->c93.dx=1;
	m->c93.dy=24;
	m->c93.qz=0;
	m->c93.ms=1;
	m->c93.neg=FALSE;
	m->c93.text[0]=0;
	}

/******************************************************************************
	C93MARKEDITDP

	mtC93 editor dialog process.
******************************************************************************/
BOOL CALLBACK C93MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
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
			FeditSetVal(hctrl,EditorMark.c93.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.c93.yres/EditYScale);

			// IDC_XDOTS
			hctrl=GetDlgItem(hdlg,IDC_XDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=8;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c93.dx);

			// IDC_YDOTS
			hctrl=GetDlgItem(hdlg,IDC_YDOTS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=128;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c93.dy);

			// IDC_QZONE
			hctrl=GetDlgItem(hdlg,IDC_QZONE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=30;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c93.qz);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.c93.ms);

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
			CheckDlgButton(hdlg,IDC_NEG,EditorMark.c93.neg);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLC93;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.c93.text);
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
					/*		hctrl=GetDlgItem(hdlg,IDC_DEG);
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
							||(!FeditGetVal(hctrl,&EditorMark.c93.xres))
							||(EditorMark.c93.xres < 10.0)
							||(EditorMark.c93.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c93.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.c93.yres))
							||(EditorMark.c93.yres < 10.0)
							||(EditorMark.c93.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c93.yres*=EditYScale;

						// IDC_XDOTS
						hctrl=GetDlgItem(hdlg,IDC_XDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADXDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c93.dx=(WORD)dw;

						// IDC_YDOTS
						hctrl=GetDlgItem(hdlg,IDC_YDOTS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADYDOTS);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c93.dy=(WORD)dw;

						// IDC_QZONE
						hctrl=GetDlgItem(hdlg,IDC_QZONE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADQZONE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c93.qz=(WORD)dw;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.c93.ms=(WORD)dw;

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
						EditorMark.c93.neg=IsDlgButtonChecked(hdlg,IDC_NEG);

	                    // IDC_WEXT
	                    hctrl=GetDlgItem(hdlg,IDC_WEXT);
	                    if((!KeditValidate(hctrl))||
	                        (!KeditGetVal(hctrl,(LPSTR)&(EditorMark.c93.text)))||
	                        (!EditorMark.c93.text[0]))
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
						if(ncode==XN_INS)
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
	ok=ComputeGlyphMap(s)

	Computes the GlphMap for the given string including the checksums C and K
	but not including start and stop.  GlyphMap is an array of indexes into
	Glyps,  wich are the bit patters for each CODE93 barcode character.

	BOOL ok = TRUE on success.
	LPSTR s = string to compute GlyphMap for.
******************************************************************************/
static BOOL ComputeGlyphMap(LPSTR s)
	{
	LPSTR p;

	GlyphMapLen=0;
	for(p=s;*p;p++)  // For each character in s
		if(!GlyphMapAppendChar(*p)) // Append its rep to GlyphMap
			return FALSE;
	if(!GlyphMapLen) // Error if no characters mapped
		return FALSE;
	GlyphMapAppendSumC();
	GlyphMapAppendSumK();
	return TRUE;
	}

/******************************************************************************
	ok = GlyphMapAppendChar(c)

	Appends the Glyph indexes for the character c to GlyphMap.
	Performs full ascii to code93 conversion.

	int c = character to append Glyph indexes for.
******************************************************************************/
static BOOL GlyphMapAppendChar(int c)
	{
	if(c<0)
		return FALSE;
	if(c==0) // 0
		{
		GlyphMapAppend(44); // (%)
		GlyphMapAppend(30); //  U
		return TRUE;
		}
	if(c<27) // 1..26
		{
		GlyphMapAppend(43);  // ($)
		GlyphMapAppend(c+9); //  A..Z
		return TRUE;
		}
	if(c<32) // 27..31
		{
		GlyphMapAppend(44);   // (%)
		GlyphMapAppend(c-17); //  A..E
		return TRUE;
		}
	if(c==32) // 32
		{
		GlyphMapAppend(38);   // space
		return TRUE;
		}
	if(c<36)  // 33..35
		{
		GlyphMapAppend(45);   // (/)
		GlyphMapAppend(c-23); // A..C
		return TRUE;
		}
	if(c==36) // 36
		{
		GlyphMapAppend(39);   // $
		return TRUE;
		}
	if(c==37) // 37
		{
		GlyphMapAppend(42);   // %
		return TRUE;
		}
	if(c<43) // 38..42
		{
		GlyphMapAppend(45);   // (/)
		GlyphMapAppend(c-23); // F..J
		return TRUE;
		}
	if(c==43) // 43
		{
		GlyphMapAppend(41);   // +
		return TRUE;
		}
	if(c==44) // 44
		{
		GlyphMapAppend(45);   // (/)
		GlyphMapAppend(21);   //  L
		return TRUE;
		}
	if(c<47) // 45..46
		{
		GlyphMapAppend(c-9);  // '-'..'.'
		return TRUE;
		}
	if(c==47) // 47
		{
		GlyphMapAppend(40);   // /
		return TRUE;
		}
	if(c<58) // 48..57
		{
		GlyphMapAppend(c-48);   // 0..9
		return TRUE;
		}
	if(c==58) // 58
		{
		GlyphMapAppend(45);   // (/)
		GlyphMapAppend(35);   //  Z
		return TRUE;
		}
	if(c<64) // 59..63
		{
		GlyphMapAppend(44);   // (%)
		GlyphMapAppend(c-44); //  F..J
		return TRUE;
		}
	if(c==64) // 64
		{
		GlyphMapAppend(44);   // (%)
		GlyphMapAppend(31);   //  V
		return TRUE;
		}
	if(c<91) // 65..90
		{
		GlyphMapAppend(c-55); //  A..Z
		return TRUE;
		}
	if(c<96) // 91..95
		{
		GlyphMapAppend(44);   // (%)
		GlyphMapAppend(c-71); //  K..O
		return TRUE;
		}
	if(c==96) // 96
		{
		GlyphMapAppend(44);   // (%)
		GlyphMapAppend(32);   //  W
		return TRUE;
		}
	if(c<123) // 97..122
		{
		GlyphMapAppend(46);   // (+)
		GlyphMapAppend(c-87); //  A..Z
		return TRUE;
		}
	if(c<128) // 123..127
		{
		GlyphMapAppend(44);      // (%)
		GlyphMapAppend(c-98);    //  P..T
		return TRUE;
		}
	return FALSE;
	}

/******************************************************************************
	GlyphMapAppendSumC()

	Computes and appends the type C checksum to the GlyphMap
	C is the mod 47 sum of the product
	of the data chars and the sequence 1..20..1..20 
	counting from right to left

	Example:
		 C   O   D   E   _   9   3 : DATA
		12  24  13  14  38  09  03 : V
		 7   6   5   4   3   2   1 : W
		84 144  65  56 114  18   3 : V*W

		C=SUM(V*W)%47=484%47=14
******************************************************************************/
static void GlyphMapAppendSumC(void)
	{
	int i,w,sum;

	sum=0;
	for(i=0;i<GlyphMapLen;i++)
		{
		w=1+(GlyphMapLen-(i+1))%20;
		sum+=w*GlyphMap[i];
		}
	sum%=47;
	GlyphMapAppend(sum);
	}

/******************************************************************************
	GlyphMapAppendSumK()

	Computes and appends the type K checksum to the GlyphMap
	K is the mod 47 sum of
	the product of the data chars and C and
	the sequence 1..15..1..15 counting from right to left

	Example:
		 C   O   D   E   _   9   3   E: DATA and C
		12  24  13  14  38  09  03  14: V
		 8   7   6   5   4   3   2   1: W
		96 168  78  70 152  27   6  14: V*W

		C=SUM(V*W)%47=611%47=0
******************************************************************************/
static void GlyphMapAppendSumK(void)
	{
	int i,w,sum;

	sum=0;
	for(i=0;i<GlyphMapLen;i++)
		{
		w=1+(GlyphMapLen-(i+1))%15;
		sum+=w*GlyphMap[i];
		}
	sum%=47;
	GlyphMapAppend(sum);
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
	i=9;
	while(i--)
		x=BitsAppendModule(bits,dx,x,(glyph>>i)&1);
	return x;
	}
