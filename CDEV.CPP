/******************************************************************************
	NAME : CDEV.C
	DESC : Compound Device Marks.  That is marks which can include
	inline font changes and sub bitmaps that can be printed by the
	device in a single pass.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <imslib32\imslib32.h>
#include <jvmath32\jvmath32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "editor.h"
#include "cdev.h"
#include "lbmp.h"
#include "stdloc.h"
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
#include "resource.h"

#define MENAB (MF_ENABLED|MF_BYCOMMAND)
#define MGRAY (MF_GRAYED|MF_BYCOMMAND)

// IDMList identifies and classifies each IDM on the insert menu
struct {int idm;BOOL isbmp;} IDMList[]=
	{{IDM_NEWTEXTMARK,0},
	{IDM_NEWDEVBCMARK,0},
	{IDM_NEWC2OF5MARK,1},
	{IDM_NEWC39MARK,1},
	{IDM_NEWC93MARK,1},
	{IDM_NEWC128MARK,1},
	{IDM_NEWCUPCEANMARK,1},
	{IDM_NEWCDMMARK,1},
	{IDM_NEWCQRMARK,1},
	{IDM_NEWUIDMARK,1},               //--------------Linghai-4-9-09--
	//{IDM_NEWFORMAT12,1},             //-------------Linghai-11-21-08----
	//{IDM_NEWFORMAT6,1},             //-------------Linghai-11-21-08----
	//{IDM_NEWFORMAT5,1},             //-------------Linghai-11-21-08----
	{IDM_NEWBITMAPMARK,1},
	{IDM_NEWWINFONTMARK,1},
	{IDM_NEWPATCHMARK,1},
	{IDM_NEWWMTRXMARK,1},
	{0,0}};

class CDEVBLOCK;

static BOOL CDevSysInitialized=FALSE;
static CDEVBLOCK *EditBlocks=NULL;
static CDEVBLOCK *SelBlock=NULL; // Currently selected block or NULL if none
static IJBMPFMT EditBf; // Current bitmap format when one is selected
static BOOL EditBfValid=FALSE; // True when EditBf is valid
// Preview font and logfont
static HFONT font=0;
static LOGFONT logfont={0,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New"};
static int zoom=1; // preview zoom factor
static HWND dwnd=0; // dialog window
static HWND vwnd=0; // view window
static int scrollmin;
static int scrollmax;
static int scrollpos=0;
static int scrollpage;
static int scrollextra=4; // extra pixels on each end of scroll range
static int woy=-4;  // view window y window origin
static POINT bext; // extent of edit blocks
static int OriginSize; // Origin cross size in pixels
static COLORREF SelColor; // Color of the selection rectangle
static COLORREF OriginColor; // Color of origin indicator
static COLORREF InkColor; // Color of the ink applied to the object
static COLORREF ObjColor; // Color of the object being marked
static HBRUSH ObjBrush=0; // ObjColor Brush
static HPEN SelPen=0;     // SelColor pen
static HPEN OriginPen;    // OriginColor pen
static HBITMAP *BSMPreviews; // Array of up to EditBf.nbuf preview bitmaps
static WORD nBSMPreviews=0;
static char sCDEV[]="CDEV"; // Ini section name
static enum {msUP,msDOWN,msDRAG,msSELECT} mstate=msUP; // Mouse state
static POINTS DownPoint; // Point where mouse button was pressed
static POINTS DragPoint; // Point where mouse is now or when released
static RECT DragRect;   // BBox of item being dragged
static int badxextent;  // x extent of ? is system font for bad blocks
static IJFONTNAME newFontName;

static void EditBlocksBuild();
static void EditBlocksFree();
static void EditBlocksAppend(CDEVBLOCK *newblock);
static void EditBlocksInsert(CDEVBLOCK *newblock);
static void EditBlocksDelete(CDEVBLOCK *block);
static BOOL EditBlocksValid();
static BOOL EditBlocksCompile(char *cbuf);
static void EditBlocksPaint(HDC hdc);
static CDEVBLOCK *EditBlocksPrev(CDEVBLOCK *block);
static void EditBlocksExtent(POINT *extent);
static void CDEVMARKEDITUpdateEnables(HWND hdlg);
static void EnableIDMList(HWND hwnd,BOOL txt,BOOL bmp);
static void TXTCDEVEDITUpdateEnables(HWND hdlg);
static void GetEditBfInfo();
static int linecopyn(char *dest,char *text,int ln);
static int linelenn(char *text,int ln);
static void prepfont(int csx,int csy);
static void freefont();
static void InvalidateView();
static void DoDelBlock();
static void DoEditBlock();
static void AllocBSMPreviews();
static void FreeBSMPreviews();
static void ClearBSMPreviews();
static void UpdateBSMPreviews(IMPMARK *m);
static void DoMouseDown(HWND hwnd,WPARAM wP,LPARAM lP);
static void DoMouseUp(HWND hwnd,WPARAM wP,LPARAM lP);
static void DoMouseMove(HWND hwnd,WPARAM wP,LPARAM lP);
static void DoMouseDoubleClick(HWND hwnd,WPARAM wP,LPARAM lP);
static CDEVBLOCK *HitTestSelect(CDEVBLOCK *first,POINTS p);
static BOOL DlgEnableWindow(HWND hdlg,int idc,BOOL enable);
static void PaintOrigin(HDC hdc);
static void PaintFocus(HDC hdc);
static void ComputeScrollRange(BOOL init);
static void ShowBufStat();
static void DrawSelRect(HDC hdc,RECT *r);

BOOL CALLBACK TXTCDEVEDITDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
BOOL CALLBACK UPDATEFONTDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
LRESULT CALLBACK CDEVVIEWWP(HWND hwnd,UINT msg,WPARAM wP,LPARAM lP);

/******************************************************************************
	CDEVBLOCK BMPCDEVBLOCK and TXTCDEVBLOCK classes
******************************************************************************/
class CDEVBLOCK
	{
protected:
	BOOL valid;
	RECT bbox;
public:
	POINT extent;
	CDEVBLOCK *next;
	CDEVBLOCK(); // Construct invalid block
	virtual ~CDEVBLOCK();
	void calcbbox(int x);
	void DrawSel(HDC hdc);
	void getbbox(RECT *r);
	BOOL hittest(POINTS p);
	virtual int compile(char *cbuf)=0;
	virtual BOOL edit(HWND parent)=0;
	virtual void calcextent()=0;
	virtual void paint(HDC hdc,int *x)=0;
	virtual void clearprev()=0;
	virtual CDEVBLOCK *findbi(int _bi);
	virtual void adjustbi(int delbi);
	virtual int getbi();
	friend void EditBlocksFree();
	friend void EditBlocksAppend(CDEVBLOCK *newblock);
	friend void EditBlocksInsert(CDEVBLOCK *newblock);
	friend void EditBlocksDelete(CDEVBLOCK *block);
	friend BOOL EditBlocksValid();
	friend BOOL EditBlocksCompile(char *cbuf);
	friend void EditBlocksPaint(HDC hdc);
	friend CDEVBLOCK *EditBlocksPrev(CDEVBLOCK *block);
	};

class BMPCDEVBLOCK : public CDEVBLOCK
	{
	int bi; // bitmap index into cdev.bsm
	        // the bitmap generating marks are in cdev.bsm[]
            // to allow multiple BMPCDEVBLOCKs to ref each one
public:
	BMPCDEVBLOCK(int _bi);
	virtual ~BMPCDEVBLOCK();
	virtual int compile(char *cbuf);
	virtual BOOL edit(HWND parent);
	virtual void calcextent();
	virtual void paint(HDC hdc,int *x);
	virtual CDEVBLOCK *findbi(int _bi);
	virtual void adjustbi(int delbi);
	virtual int getbi();
	virtual void clearprev();
	};


class TXTCDEVBLOCK : public CDEVBLOCK
	{
	int lfi;                 // logical font index
	int dfi;                 // device font index
	int nl;                  // n lines in font from IJFONTINFO.nlines
	WORD cpcc[IJMLMAXLINES]; // chars per each char cell line from IJFONTINFO
	WORD spc;                // strokes per char cell from IJFMTINFO
	BOOL bc;                 // Indicates device barcode font
	WORD ichars;             // Implied extra chars (for bc like ESI code 39)
	char *text;              // text allocated on heap
	char *ctext;             // Compiled text or NULL
public:
	TXTCDEVBLOCK(int fi,char *_text,int ltext,BOOL _bc);
	void init(int fi,char *_text,int ltext);
	virtual ~TXTCDEVBLOCK();
	void BuildCText();
	virtual int compile(char *cbuf);
	virtual BOOL edit(HWND parent);
	virtual void calcextent();
	virtual void paint(HDC hdc,int *x);
	virtual void clearprev();
	BOOL editdp(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	friend BOOL CALLBACK TXTCDEVEDITDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	};

/******************************************************************************
******************************************************************************/
CDEVBLOCK::CDEVBLOCK()
	{
	valid=FALSE;
	extent.x=0;
	extent.y=0;
	next=NULL;
	}

/******************************************************************************
******************************************************************************/
CDEVBLOCK::~CDEVBLOCK()
	{
	}

/******************************************************************************
	CDEVBLOCK::calcbbox(x)

	Called during paint to calculate the bounding box used for mouse
	hit testing given the extent has been calculated and the supplied
	x pixel coordinate.  This bbox is always EdidBF.extent.y tall so
	it not appropriate for BitBlt or StretchBlt coordinate computation.

	int x = left x pixel coordinate of block.
******************************************************************************/
void CDEVBLOCK::calcbbox(int x)
	{
	bbox.left=x;
	bbox.top=0;
	bbox.right=x+extent.x;
	bbox.bottom=EditBf.extent.y*zoom;
	}

/******************************************************************************
	CDEVBLOCK::DrawSel(hdc)

	Draws the selection rectangle
******************************************************************************/
void CDEVBLOCK::DrawSel(HDC hdc)
	{
	DrawSelRect(hdc,&bbox);
	}

/******************************************************************************
	CDEVBLOCK::getbbox(r)

	Fills in the supplied rectangle with the current bbox of the block.
	Assumes bbox is valid since last painting.

	RECT *r = rectangle to fill in.
******************************************************************************/
void CDEVBLOCK::getbbox(RECT *r)
	{
	*r=bbox;
	}

/******************************************************************************
	hit = CDEVBLOCK::hittest(p)

	Returns true if the point p is inside of the blocks bbox.

	BOOL hit = True if the point hits the block.
	POINT p = The point to test.
******************************************************************************/
BOOL CDEVBLOCK::hittest(POINTS p)
	{
	POINT pp;
	pp.x=p.x;
	pp.y=p.y;
	return PtInRect(&bbox,pp);
	}

/******************************************************************************
	blk = CDEVBLOCK::findbi(int _bi)

	Searches from the current block forward until either the end of the list
	or a BMPCDEVBLOCK with bi==_bi is found.  If found, the pointer to this
	block is returned.  If not found, NULL is returned.  Used in ~BMPCDEVBLOCK
	to search for other blocks that refer to the same bsm so that it can
	free the bsm itself when no more refs exist.  This uses a recursive
	traversal of the list but should not use up too much stack space if
	the list is not too long.

	CDEVBLOCK *blk = found block or NULL if not found.
	int _bi = bsm index to search for.
******************************************************************************/
CDEVBLOCK *CDEVBLOCK::findbi(int _bi)
	{
	if(next)
		return next->findbi(_bi);
	return NULL;
	}

/******************************************************************************
	CDEVBLOCK::adjustbi(delbi)

	Adjusts the bi index of a block to close the hole created by a deletion.
	The bi is decremented if the blocks bi is > delbi.

	int delbi = bsm index that was deleted.
******************************************************************************/
void CDEVBLOCK::adjustbi(int delbi)
	{
	}

/******************************************************************************
	bi = getbi()

	Returns the bi of a BMPCDEVBLOCK or -1 if not a BMPCDEVBLOCK
******************************************************************************/
int CDEVBLOCK::getbi()
	{
	return -1; // this is not a bmp cdev block
	}

/******************************************************************************
******************************************************************************/
TXTCDEVBLOCK::TXTCDEVBLOCK(int fi,char *_text,int ltext,BOOL _bc)
	{
	text=NULL;
	ctext=NULL;
	bc=_bc;
	ichars=0;
	init(fi,_text,ltext);
	}

/******************************************************************************
******************************************************************************/
void TXTCDEVBLOCK::init(int fi,char *_text,int ltext)
	{
	lfi=fi;
	if(ctext)
		{
		delete [] ctext;
		ctext=NULL;
		}
	if(text)
		{
		delete [] text;
		text=NULL;
		}
	if(_text)
		{
		text=new char [ltext+1];
		lstrcpyn(text,_text,ltext+1);
		}
	else
		text=NULL;

	if(EditorIJFound && lfi>=0)
		{
		BOOL bUpdateFont = FALSE;
		IJBMPFMT bf;
		IJFONTINFO fonti;
		IJFMTINFO fmti;
		SendMessage(EditorIJDev,IJM_GETBMPFMT,EditorMark.cdev.bfi,(LPARAM)(LPSTR)&bf);
		dfi=SendMessage(EditorIJDev,IJM_FINDFONT,0,
			(LPARAM)(LPSTR)EditorIMP.fonts.name[lfi]);
	
		while (dfi==-1){
			ShowWarning(IDW_NOFONTS,EditorIMP.fonts.name[lfi]);
			edit(IMSWnd);
		}
		
		//if (bUpdateFont)
		//	EditorIMPMapFontName(newFontName);

		SendMessage(EditorIJDev,IJM_GETFONTINFO,dfi,(LPARAM)(LPSTR)&fonti);
		bc=fonti.ft==ftBARCODE;
		ichars=fonti.ichars;
		fmti.fmt=EditorMark.cdev.bijfmt;
		fmti.fmt.font=dfi;
		SendMessage(EditorIJDev,IJM_GETFMTINFO,0,(LPARAM)(LPSTR)&fmti);
		nl=fonti.nlines;

		for(int i=0;i<nl;i++)
			cpcc[i]=fonti.cpcc[i];
		spc=fmti.spc;
		}
	valid=TRUE;
	}

/******************************************************************************
******************************************************************************/
TXTCDEVBLOCK::~TXTCDEVBLOCK()
	{
	delete [] text;
	}

/******************************************************************************
	TXTCDEVBLOCK::BuildCText()

	Builds the compiled text ctext for the given mark using the PreVals
******************************************************************************/
void TXTCDEVBLOCK::BuildCText()
	{
	delete [] ctext;
	ctext=NULL;

	char buf[128];

	int len=IMPExpandTextToBuf(buf,sizeof(buf),text,
		&EditorIMP.vars,&EditorPreIVals,&EditorIMP.gvars,&EditorPreGVals,
		EditorIMP.pheader,&EditorIMP.udhas,FALSE);
 
	if(len)
		{
		ctext = new char[len+1];
		lstrcpy(ctext,buf);
		}
	}

/******************************************************************************
	l=TXTCDEVBLOCK::compile(cbuf)
	int l = length of compiled text
	char *cbuf = buffer for compiled text
******************************************************************************/
int TXTCDEVBLOCK::compile(char *cbuf)
	{
	return wsprintf((LPSTR)cbuf,"\033F%c%s",lfi+1,(LPSTR)text);
	}

/******************************************************************************
******************************************************************************/
BOOL TXTCDEVBLOCK::edit(HWND parent)
	{
	BOOL ok=DialogBoxParam(IMSInst,"TXTCDEVEDIT",parent,
		(DLGPROC)TXTCDEVEDITDP,(LPARAM)this);
	return ok;
	}

/******************************************************************************
******************************************************************************/
void TXTCDEVBLOCK::calcextent()
	{
	extent.x=0;
	extent.y=0;
	if(!ctext)
		BuildCText();
	if(!EditBfValid)
		return;
	extent.y=EditBf.extent.y*zoom;
	for(int li=0;li<nl;li++)
		{
		int len=linelenn(ctext,li)+ichars;
		int lex=(len+cpcc[li]-1)/cpcc[li]*spc*zoom;
		if(lex>extent.x)
			extent.x=lex;
		}
	}

/******************************************************************************
	TXTCDEVBLOCK::paint(hdc,x)

	Paint txt block on hdc at *x then update *x

	HDC hdc = display context to paint on.
	int *x = pointer to current x coordinate.
******************************************************************************/
void TXTCDEVBLOCK::paint(HDC hdc,int *x)
	{
	if(!ctext)
		BuildCText();
	if(extent.x==0&&extent.y==0)
		calcextent();
	calcbbox(*x);

	// Draw text preview
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,InkColor);
	int csy=extent.y/nl;
	for(int li=0;li<nl;li++)
		{
		tlen=0;
		if(bc && ichars) // Prefix text with half of ichars as *s
			{
			int n=ichars-(ichars>>1);
			for(int i=0;i<n;i++)
				tbuf[i]='*';
			tlen+=n;
			}
			
		tlen+=linecopyn(tbuf+tlen,ctext,li); // Always copy the text

		if(bc && ichars) // Suffix text with remaining half of ichars as *s
			{
			int n=ichars>>1;
			for(int i=0;i<n;i++)
				tbuf[tlen+i]='*';
			tlen+=n;
			}

		int csx=zoom*spc/cpcc[li];
		prepfont(csx,csy);
		SelectObject(hdc,font);
		TextOut(hdc,(*x),li*csy,tbuf,tlen);
		}
	if(bc)
		{
		for(int bx=0;bx<extent.x;bx++)
			if(!(bx&3))
				{
				MoveToEx(hdc,(*x)+bx,0,NULL);
				LineTo(hdc,(*x)+bx,extent.y);
				}
		}


	// Draw selection if selected
	if(this==SelBlock)
		DrawSel(hdc);

	(*x)+=extent.x; // Advance x
	}

/******************************************************************************
	blk.clearprev()
	Invalidates the preview information for the block.
	That is the ctext or preview bitmap referenced
******************************************************************************/
void TXTCDEVBLOCK::clearprev()
	{
	delete [] ctext;
	ctext=NULL;
	}

/******************************************************************************
	ret = TXTCDEVBLOCK::editdp(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)

	TXTCDEVBLOCK edit dialog proc (after binding).
******************************************************************************/
BOOL TXTCDEVBLOCK::editdp(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	switch(msg)
		{
		case WM_INITDIALOG:
			if (bc){
				SetWindowText(hdlg,"Device Text ffg");
			}
			SetWindowText(hdlg,bc?"Device Barcode":"Device Text");

			//IDC_FONTS
			{
			int sel=-1;
			for(int i=0;i<EditBf.nfonts;i++)
				{
				int _dfi=EditBf.fontlist[i];
				IJFONTINFO ijfi; // font info
				SendMessage(EditorIJDev,IJM_GETFONTINFO,_dfi,(LPARAM)(LPSTR)&ijfi);
				if(ijfi.ft==ftBARCODE && !bc)
					continue;
				if(ijfi.ft!=ftBARCODE && bc)
					continue;
				int _lfi=EditorIMPMapFontName(ijfi.name);
				// Add font to list
				int li=SendDlgItemMessage(hdlg,IDC_FONTS,
					CB_ADDSTRING,0,(LONG)(LPSTR)ijfi.name);
				// put _lfi in low data and _dfi in high data
				SendDlgItemMessage(hdlg,IDC_FONTS,
					CB_SETITEMDATA,li,MAKELONG(_lfi,_dfi));
				if(_lfi==lfi)
					sel=li;
				}
			// set initial lb selection based on blks lfi
			SendDlgItemMessage(hdlg,IDC_FONTS,CB_SETCURSEL,sel,0);
			}
			// IDC_WEXT
			{
			KEDITINFO kinfo;
			HWND hctrl;
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.length=IMPLMLTEXT;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			if(text)
				SetWindowText(hctrl,text);
			}
			TXTCDEVEDITUpdateEnables(hdlg);
			return TRUE;

		case WM_ACTIVATE:
			IMSADWnd=wP ? hdlg : 0;
			return TRUE;

		case WM_CLOSE:
			EndDialog(hdlg,FALSE);  // End the dialog
			return TRUE;

		case WM_COMMAND: //Win32 changed
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			if(hcontrol) // If from control
				{
				switch(id)
					{
					case IDC_FONTS:
						if(ncode==CBN_SELCHANGE)
							{
							TXTCDEVEDITUpdateEnables(hdlg);
							return TRUE;
							}
						break;
					case IDC_WEXT:
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						switch(ncode)
							{
							case EN_CHANGE:
								TXTCDEVEDITUpdateEnables(hdlg);
								return TRUE;
							case XN_INS:
								DoInsertsPopup(hdlg,hctrl,&EditorIMP.udhas,
									&EditorIMP.vars,&EditorIMP.gvars);
								return TRUE;
							case XN_ENTER:
								SendMessage(hdlg,WM_COMMAND,IDOK,1);
								return TRUE;
							case XN_TAB:
								{
								HWND hc=GetNextDlgTabItem(hdlg,hctrl,FALSE);
								if(!hc)
									hc=GetDlgItem(hdlg,IDC_FONTS);
								SetFocus(hc);
								}
								return TRUE;
							case XN_BKTAB:
								{
								HWND hc=GetNextDlgTabItem(hdlg,hctrl,TRUE);
								if(!hc)
									hc=GetDlgItem(hdlg,IDCANCEL);
								SetFocus(hc);
								}
								return TRUE;
							}
						break;
					case IDOK:
						{
						int li=SendDlgItemMessage(hdlg,IDC_FONTS,CB_GETCURSEL,0,0);
						if(li<0)
							return TRUE;
						int ltext=SendDlgItemMessage(hdlg,IDC_WEXT,
							WM_GETTEXTLENGTH,0,0);
						if(ltext==0)
							return TRUE;
						// Get _lfi from selection itemdata low word
						DWORD dw=SendDlgItemMessage(hdlg,IDC_FONTS,
							CB_GETITEMDATA,li,0);
						int _lfi=LOWORD(dw);
						// Get the text
						GetDlgItemText(hdlg,IDC_WEXT,tbuf,ltext+1);
						init(_lfi,tbuf,ltext); // updte this block
						EndDialog(hdlg,TRUE);  // End the dialog
						return TRUE;
						}
					case IDCANCEL:
						EndDialog(hdlg,FALSE);  // End the dialog
						return TRUE;
					}
				}
			else // From text insert menu
				{
				hctrl=GetDlgItem(hdlg,IDC_WEXT);
				if(ProcessInsertsPopup(wP,hctrl))
					return TRUE;
				}
			}
			break;

		case WM_DESTROY:
			return TRUE;
		}
	return FALSE;
	}

BMPCDEVBLOCK::BMPCDEVBLOCK(int _bi)
	{
	bi=_bi;
	valid=TRUE;
	}

BMPCDEVBLOCK::~BMPCDEVBLOCK()
	{
	}

// l=BMPCDEVBLOCK::compile(cbuf)
// int l = length of compiled text
// char *cbuf = buffer for compiled text
int BMPCDEVBLOCK::compile(char *cbuf)
	{
	cbuf[0]='\033';
	cbuf[1]='B';
	cbuf[2]=bi+1;
	return 3;
	}

BOOL BMPCDEVBLOCK::edit(HWND parent)
	{
	return DoSubMarkEdit(parent,EditorMark.cdev.bsm[bi]);
	}

void BMPCDEVBLOCK::calcextent()
	{
	extent.x=0;
	extent.y=0;
	if(!EditBfValid)
		return;
	
	if(!BSMPreviews[bi])
		UpdateBSMPreviews(&EditorMark);

	if(BSMPreviews[bi])
		{
		BITMAP bm;
		GetObject(BSMPreviews[bi],sizeof(BITMAP),&bm);
		extent.x=min(bm.bmWidth,EditBf.extent.x)*zoom;
		extent.y=min(bm.bmHeight,EditBf.extent.y)*zoom;
		}
	else
		{
		extent.x=badxextent;
		extent.y=EditBf.extent.y;
		}
	}

// Paint bmp block on hdc at *x then update *x
void BMPCDEVBLOCK::paint(HDC hdc,int *x)
	{
	if(!BSMPreviews[bi])
		UpdateBSMPreviews(&EditorMark);

	if(extent.x==0&&extent.y==0)
		calcextent();
	calcbbox(*x);

	if(BSMPreviews[bi])
		{
		SetTextColor(hdc,InkColor);
		SetBkColor(hdc,ObjColor);
		HDC hmc=CreateCompatibleDC(hdc);
		HBITMAP oldbmp=(HBITMAP)SelectObject(hmc,BSMPreviews[bi]);
		StretchBlt(hdc,
			*x,0,extent.x,extent.y,
			hmc,0,0,extent.x/zoom,extent.y/zoom,SRCCOPY);
		SelectObject(hmc,oldbmp);
		DeleteDC(hmc);
		}
	else
		{
		SelectObject(hdc,GetStockObject(SYSTEM_FONT));
		SetBkMode(hdc,OPAQUE);
		SetTextColor(hdc,ObjColor);
		SetBkColor(hdc,InkColor);
		TextOut(hdc,*x,0,"?",1);
		}

	// Draw selection if selected
	if(this==SelBlock)
		DrawSel(hdc);

	(*x)+=extent.x; // Advance x
	}

/******************************************************************************
	blk.clearprev()

	Invalidates the preview information for the block.
	That is the ctext or preview bitmap referenced
******************************************************************************/
void BMPCDEVBLOCK::clearprev()
	{
	if(EditBfValid)
		if(bi<EditBf.nbuf)
			if(BSMPreviews[bi])
				DELOBJ(BSMPreviews[bi]);
	}

/******************************************************************************
	blk = CDEVBLOCK::findbi(int _bi)

	Searches from the current block forward until either the end of the list
	or a BMPCDEVBLOCK with bi==_bi is found.  If found, the pointer to this
	block is returned.  If not found, NULL is returned.  Used in ~BMPCDEVBLOCK
	to search for other blocks that refer to the same bsm so that it can
	free the bsm itself when no more refs exist.  This uses a recursive
	traversal of the list but should not use up too much stack space if
	the list is not too long.

	CDEVBLOCK *blk = found block or NULL if not found.
	int _bi = bsm index to search for.
******************************************************************************/
CDEVBLOCK *BMPCDEVBLOCK::findbi(int _bi)
	{
	if(_bi==bi)
		return this;
	if(next)
		return next->findbi(_bi);
	return NULL;
	}

/******************************************************************************
	BMPCDEVBLOCK::adjustbi(delbi)

	Adjusts the bi index of a block to close the hole created by a deletion.
	The bi is decremented if the blocks bi is > delbi.

	int delbi = bsm index that was deleted.
******************************************************************************/
void BMPCDEVBLOCK::adjustbi(int delbi)
	{
	if(bi>delbi)
		bi--;
	}

/******************************************************************************
	bi = getbi()

	Returns the bi of a BMPCDEVBLOCK or -1 if not a BMPCDEVBLOCK
******************************************************************************/
int BMPCDEVBLOCK::getbi()
	{
	return bi;
	}

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	CDevSysInit()
	Initializes the CDev system
	registers the CDEVVIEW control window class
******************************************************************************/
void CDevSysInit()
	{
	if(!CDevSysInitialized)
		{
		zoom=GetPrivateProfileInt(sCDEV,"Zoom",1,IMSIni);
		if(zoom<1)
			zoom=1;

		OriginSize=GetPrivateProfileInt(sCDEV,"OriginSize",8,IMSIni);

		SelColor=GetPrivateProfileCOLORREF(sCDEV,"SelColor",
			RGB(0,0,255),IMSIni);
		OriginColor=GetPrivateProfileCOLORREF(sCDEV,"OriginColor",
			RGB(255,0,0),IMSIni);
		InkColor=GetPrivateProfileCOLORREF(sCDEV,"InkColor",
			RGB(0,0,0),IMSIni);
		ObjColor=GetPrivateProfileCOLORREF(sCDEV,"ObjColor",
			RGB(255,255,255),IMSIni);

		SelPen=CreatePen(PS_DOT,0,SelColor);
		OriginPen=CreatePen(PS_SOLID,0,OriginColor);
		ObjBrush=CreateSolidBrush(ObjColor);

		// Register the CDEVVIEW control class
		WNDCLASS wc;
		wc.style=CS_DBLCLKS;
		wc.lpfnWndProc=CDEVVIEWWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=ObjBrush;
		wc.lpszMenuName=NULL;
		wc.lpszClassName="CDEVVIEW";
		RegisterClass(&wc);


		CDevSysInitialized=TRUE;
		}
	}

/******************************************************************************
	CDevReloadSysSettings()

	Reloads the CDev ini settings

******************************************************************************/
void CDevReloadSysSettings(){

	DELOBJ(SelPen);
	DELOBJ(OriginPen);
	DELOBJ(ObjBrush);

	zoom=GetPrivateProfileInt(sCDEV,"Zoom",1,IMSIni);
	if(zoom<1)
		zoom=1;
	
	OriginSize=GetPrivateProfileInt(sCDEV,"OriginSize",8,IMSIni);
	
	SelColor=GetPrivateProfileCOLORREF(sCDEV,"SelColor",
		RGB(0,0,255),IMSIni);
	OriginColor=GetPrivateProfileCOLORREF(sCDEV,"OriginColor",
		RGB(255,0,0),IMSIni);
	InkColor=GetPrivateProfileCOLORREF(sCDEV,"InkColor",
		RGB(0,0,0),IMSIni);
	ObjColor=GetPrivateProfileCOLORREF(sCDEV,"ObjColor",
		RGB(255,255,255),IMSIni);
	
	SelPen=CreatePen(PS_DOT,0,SelColor);
	OriginPen=CreatePen(PS_SOLID,0,OriginColor);
	ObjBrush=CreateSolidBrush(ObjColor);

	WNDCLASS oldwc;
	if(GetClassInfo(IMSInst,"CDEVVIEW",&oldwc)){
		UnregisterClass("CDEVVIEW",IMSInst);
		// Re-register the CDEVVIEW control class for new background color
		WNDCLASS wc;
		wc.style=CS_DBLCLKS;
		wc.lpfnWndProc=CDEVVIEWWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=ObjBrush;
		wc.lpszMenuName=NULL;
		wc.lpszClassName="CDEVVIEW";
		RegisterClass(&wc);
	}
 
}

/******************************************************************************
	CDevSysQuit()
	Frees any resources owned by the cdev system before app shutdown.
******************************************************************************/
void CDevSysQuit()
	{
	DELOBJ(SelPen);
	DELOBJ(OriginPen);
	DELOBJ(ObjBrush);
	}

/******************************************************************************
	CDEVInit(m)

	Initializes a new IMPCDEVMARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void CDEVInit(IMPMARK *m)
	{
	m->type=mtCDEV;
	m->cdev.bijfmt.font=-1; // No initial font
	m->cdev.bijfmt.ms=1;
	m->cdev.bijfmt.xflip=0;
	m->cdev.bijfmt.yflip=0;
	m->cdev.bfi=-1;        // No initial bitmap format index
	m->cdev.xres=100.0;
	m->cdev.yres=100.0;
	m->cdev.text[0]=0;
	m->cdev.nbsm=0;        // No initial bitmap sub marks
	for(int i=0;i<IMPMAXCDEVBSM;i++)
		m->cdev.bsm[0]=NULL;
	}

BOOL CDevMarkUsesFont(IMPMARK *m,int idf)
	{
	static char fsel[4]="\033Fn";
	fsel[2]=idf+1; // construct font selection match string
	return (m->cdev.bijfmt.font==idf)       // True if base idf is base font
		||(strstr(m->cdev.text,fsel));   // or text contains idf
	}

void CDevMarkDecGreaterFonts(IMPMARK *m,int idf)
	{
	if(m->cdev.bijfmt.font>idf)
		m->cdev.bijfmt.font--;
	char *p=m->cdev.text;
	while(p=strstr(p,"\033F"))
		{
		p+=2;
		if(*p)
			{
			if(*p>(idf+1))
				(*p)--;
			p++;
			}
		}
	}

//ok=CDevMapFonts(text,fontmap,nfonts)
//replaces the lfi in the \033Fn font refs with dfi from fontmap
//BOOL ok = true on success
//char *text = text to map font refs in
//WORD *fontmap = map of dfi indexed by lfi
//int nfonts = number of fonts in fontmap
BOOL CDevMapFonts(char *text,WORD *fontmap,int nfonts)
	{
	char *p=text;
	while(p=strstr(p,"\033F")) // for each font ref \033Fn
		{
		p+=2; // advance to n
		if(*p)
			{
			if(((*p)-1)>=nfonts) // if font out of range
				{
				ShowError(IDE_NOTIJFONT);
				return FALSE;
				}
			*p=1+fontmap[(*p)-1]; // map logical font to device font
			p++; // advance to next char
			}
		}
	return TRUE;
	}

/******************************************************************************
	CDEVMARKEDITDP

	mtCDEV editor dialog process.
******************************************************************************/
BOOL CALLBACK CDEVMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			GetEditBfInfo();
			HDC hdc=GetDC(hdlg);
			SIZE te;
			GetTextExtentPoint(hdc,"?",1,&te);
			//DWORD te=GetTextExtent(hdc,"?",1);
			badxextent=te.cx;
			ReleaseDC(hdlg,hdc);

			dwnd=hdlg;
			FEDITINFO finfo;
			SEDITINFO sinfo;
//			KEDITINFO kinfo;
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

			// IDC_BUFSTAT
			ShowBufStat();
			// zzz 04/22/2004
			// Just warn if more bitmaps than buffers for now
			if (EditorMark.cdev.nbsm > EditBf.nbuf)
				ShowWarning(IDW_TOOMANYBMPS,NULL);


			
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
			FeditSetVal(hctrl,EditorMark.cdev.xres/EditXScale);

			// IDC_YRES
			hctrl=GetDlgItem(hdlg,IDC_YRES);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;    // format 999.99
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cdev.yres/EditYScale);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.cdev.bijfmt.ms);

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
			CheckDlgButton(hdlg,IDC_YFLIP,EditorMark.cdev.bijfmt.yflip ? 1: 0);

			// IDC_XFLIP
			CheckDlgButton(hdlg,IDC_XFLIP,EditorMark.cdev.bijfmt.xflip ? 1: 0);

			// IDC_BFI
			hctrl=GetDlgItem(hdlg,IDC_BFI);
			// Fill the listbox with the EditorIJDev's bitmap formats
			if(EditorIJFound)
				{
				for(i=0;i<EditorIJInfo.nbmpfmts;i++)
					{
					IJBMPFMT bf;
					IJFONTINFO ijfi;
					SendMessage(EditorIJDev,IJM_GETBMPFMT,
						i,(LPARAM)(LPSTR)&bf);

					// map all fonts in each bmpfmt and get first font info
					for(int j=bf.nfonts;j--;)
						{
						SendMessage(EditorIJDev,IJM_GETFONTINFO,
							bf.fontlist[j],(LPARAM)(LPSTR)&ijfi);
						// Map each font to assure that the imps font
						// table contains all possible logical font numbers
						EditorIMPMapFontName(ijfi.name);
						}

					// put descriptor string in the listbox
					wsprintf(tbuf,"%d*(%d by %d) %s",
						bf.nbuf,bf.extent.x,bf.extent.y,(LPSTR)ijfi.name);
					SendMessage(hctrl,CB_ADDSTRING,0,(LONG)(LPSTR)tbuf);
					}
				EditBfValid=FALSE;
				if(EditorMark.cdev.bfi>=EditorIJInfo.nbmpfmts)
					{
					EditorMark.cdev.bfi=-1;
					ShowWarning(IDW_BADBMPFMT,NULL);
						DlgEnableWindow(hdlg,IDC_BFI,FALSE);
					DlgEnableWindow(hdlg,IDOK,FALSE);
					}
				else if(EditorMark.cdev.bfi >= 0) // Good and predefined
					{
					SendMessage(hctrl,CB_SETCURSEL,EditorMark.cdev.bfi,0);
					DlgEnableWindow(hdlg,IDC_BFI,FALSE);
					GetEditBfInfo();
					EnableIDMList(hdlg,1,EditorMark.cdev.nbsm<EditBf.nbuf);
					}
				else if(EditorMark.cdev.bfi<0)
					{
					DlgEnableWindow(hdlg,IDOK,FALSE);
					EnableIDMList(hdlg,0,0);
					}
				}
			else
				{
				EditorMark.cdev.bfi=-1;
				ShowWarning(IDW_NOIJ,NULL);
				DlgEnableWindow(hdlg,IDC_BFI,FALSE);
				DlgEnableWindow(hdlg,IDOK,FALSE);
				EnableIDMList(hdlg,0,0);
				}

			EditBlocksBuild();
			if(!EditBlocksValid())
				{
				DlgEnableWindow(hdlg,IDOK,FALSE);
				EnableIDMList(hdlg,0,0);
				}

			//IDC_CDEVVIEW
			vwnd=GetDlgItem(hdlg,IDC_CDEVVIEW);
			AllocBSMPreviews();
			SendMessage(vwnd,WM_INITDIALOG,0,0);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);

			return TRUE;
			}

		case WM_ACTIVATE:
			IMSADWnd=wP ? hdlg : 0;
			return TRUE;

		case WM_COMMAND: // Changed for Win32
			{
			HWND hctrl;
			HWND hcontrol=(HWND)lP;
			WORD id=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			DWORD dw;
//			int i;

			if(hcontrol) // If from control was LOWORD(lp)
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoMarkLocate(hdlg,IDC_X,IDC_Y);
						return TRUE;
					case IDC_BFI:
						if(ncode==CBN_SELCHANGE)
							{
							//hctrl=LOWORD(lP);
							hctrl=(HWND)lP;
				/* --> */	int bfi=SendMessage(hctrl,CB_GETCURSEL,0,0);
				/* --> */	EditorMark.cdev.bfi=bfi;
				/* --> */	GetEditBfInfo();
							EnableIDMList(hdlg,1,EditorMark.cdev.nbsm<EditBf.nbuf);
							IJFONTINFO ijfi; // font info
				/* --> */	SendMessage(EditorIJDev,IJM_GETFONTINFO,
								EditBf.fontlist[0],(LPARAM)(LPSTR)&ijfi);
				/* --> */	EditorMark.cdev.bijfmt.font=
								EditorIMPMapFontName(ijfi.name);
							AllocBSMPreviews(); // Previews must change
							ShowBufStat();
							}
						return TRUE;
					case IDC_JUST:
						if(ncode==CBN_SELCHANGE)
							{
							hctrl=GetDlgItem(hdlg,IDC_JUST);
							EditorMark.justx=(IMPMARKJUSTX)SendMessage(hctrl,
								CB_GETCURSEL,0,0);
							InvalidateView();
							return TRUE;
							}
						break;						
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
							||(!FeditGetVal(hctrl,&EditorMark.cdev.xres))
							||(EditorMark.cdev.xres < 10.0)
							||(EditorMark.cdev.xres > 500.0))
							{
							ShowError(IDE_BADXRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdev.xres*=EditXScale;

						// IDC_YRES
						hctrl=GetDlgItem(hdlg,IDC_YRES);
						if((!FeditValidate(hctrl))
							||(!FeditGetVal(hctrl,&EditorMark.cdev.yres))
							||(EditorMark.cdev.yres < 10.0)
							||(EditorMark.cdev.yres > 500.0))
							{
							ShowError(IDE_BADYRES);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdev.yres*=EditYScale;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cdev.bijfmt.ms=(WORD)dw;

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
						EditorMark.cdev.bijfmt.yflip=IsDlgButtonChecked(hdlg,IDC_YFLIP);
						// IDC_XFLIP
						EditorMark.cdev.bijfmt.xflip=IsDlgButtonChecked(hdlg,IDC_XFLIP);

						// IDC_BFI -> bfi and bijfmt.font
						hctrl=GetDlgItem(hdlg,IDC_BFI);
						EditorMark.cdev.bfi=SendMessage(hctrl,CB_GETCURSEL,0,0);
						if(EditorMark.cdev.bfi<0)
							{
							ShowError(IDE_BADBFI);
							SetFocus(hctrl);
							EditorMark.cdev.bijfmt.font=-1;
							return TRUE;
							}
						else // bfi and EditorIJDev are valid
							{
							IJFONTINFO ijfi; // font info
							SendMessage(EditorIJDev,IJM_GETFONTINFO,
								EditBf.fontlist[0],(LPARAM)(LPSTR)&ijfi);
							int fi=EditorIMPMapFontName(ijfi.name);
							if(fi<0)
								{
								SetFocus(hctrl);
								EditorMark.cdev.bijfmt.font=-1;
								return TRUE;
								}
							EditorMark.cdev.bijfmt.font=fi;
							}

						// Compile EditBlocks to cdev.text
						if(!EditBlocksCompile(EditorMark.cdev.text))
							{
							IERROR;
							ShowError(0);
							return TRUE;
							}
						freefont();
						EditBlocksFree();
						FreeBSMPreviews();
						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						freefont();
						EditBlocksFree();
						FreeBSMPreviews();
						EndDialog(hdlg,FALSE);
						return TRUE;
					}
				}
			else // From menu
				{
				switch(id)
					{
					case IDM_NEWTEXTMARK:
					case IDM_NEWDEVBCMARK:
						{
						BOOL bc=id==IDM_NEWDEVBCMARK;
						TXTCDEVBLOCK *newblock=new TXTCDEVBLOCK(-1,NULL,0,bc);
						if(newblock->edit(hdlg))
							{
							EditBlocksInsert(newblock);
							ComputeScrollRange(FALSE);
							}
						else
							delete newblock;
						}
						InvalidateView();
						CDEVMARKEDITUpdateEnables(hdlg);
						return TRUE;
					case IDM_NEWC2OF5MARK: // All Raster Marks
					case IDM_NEWC39MARK:
					case IDM_NEWC93MARK:
					case IDM_NEWC128MARK:
					case IDM_NEWCUPCEANMARK:
					case IDM_NEWCDMMARK:
					case IDM_NEWUIDMARK:
					//case IDM_NEWFORMAT12:                   //----------------Linghai-11-21-08---
					//case IDM_NEWFORMAT6:                    //---------------Linghai-11-21-08---
					//case IDM_NEWFORMAT5:                    //----------------Linghai-11-21-08---
					case IDM_NEWBITMAPMARK:
					case IDM_NEWWINFONTMARK:
					case IDM_NEWPATCHMARK:
					case IDM_NEWWMTRXMARK:
					case IDM_NEWCQRMARK:
						if(EditorMark.cdev.nbsm<EditBf.nbuf)
							{
							IMPMARK *m=new IMPMARK;
							InitNewMark(m,wP);
							int bi=EditorMark.cdev.nbsm;
							if(!DoSubMarkEdit(hdlg,m))
								{
								delete m;
								return TRUE;
								}
							EditorMark.cdev.bsm[bi]=m;
							EditorMark.cdev.nbsm++;
							EditBlocksInsert(new BMPCDEVBLOCK(bi));
							ShowBufStat();
							ComputeScrollRange(FALSE);
							InvalidateView();
							CDEVMARKEDITUpdateEnables(hdlg);
							}
						return TRUE;
					case IDM_EDITCDEVBLOCK:
						DoEditBlock();
						return TRUE;
					case IDM_DELCDEVBLOCK:
						DoDelBlock();
						break;
					}
				}
			break;
			}
		}
	return FALSE;
	}

BOOL CALLBACK TXTCDEVEDITDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	if(msg==WM_INITDIALOG) // Save pointer in properties on init
		{
		SetProp(hdlg,"HSELF",(HANDLE)(lP));
//		SetProp(hdlg,"LSELF",(HANDLE)LOWORD(lP));
		}
	DWORD self;
	self=(unsigned long)GetProp(hdlg,"HSELF");
//	self<<=16;
//	self=(unsigned short)GetProp(hdlg,"LSELF");
	TXTCDEVBLOCK *_this=(TXTCDEVBLOCK *)self;

	return _this->editdp(hdlg,msg,wP,lP);
	}

/******************************************************************************
	CDEVVIEWWP

	CDEVVIEW controls display the preview in the CDEVMARKEDIT dialog
******************************************************************************/
LRESULT CALLBACK CDEVVIEWWP(HWND hwnd,UINT msg,WPARAM wP,LPARAM lP)
	{
	switch(msg)
		{
		case WM_INITDIALOG: // Sent from parent during it's INITDIALOG
			ComputeScrollRange(TRUE);
			return 0;

		case WM_HSCROLL:
			{
			int osp=scrollpos;
			switch(wP)
				{
				case SB_PAGEDOWN:
					scrollpos+=scrollpage;
					break;
				case SB_PAGEUP:
					scrollpos-=scrollpage;
					break;
				case SB_LINEDOWN:
					scrollpos++;
					break;
				case SB_LINEUP:
					scrollpos--;
					break;
				case SB_THUMBPOSITION:
				case SB_THUMBTRACK:
					scrollpos=LOWORD(lP);
				}
			scrollpos=max(scrollpos,-scrollextra);
			scrollpos=min(scrollpos,scrollmax);
			if(scrollpos!=osp)
				{
				SetScrollPos(hwnd,SB_HORZ,scrollpos,TRUE);
				InvalidateView();
				}
			}
			return 0;

		case WM_SETFOCUS: // both SET and KILL Draw/Undraw Focus Rect
		case WM_KILLFOCUS:
			{
			HDC hdc=GetDC(hwnd);
			PaintFocus(hdc);
			ReleaseDC(hwnd,hdc);
			}
			return 0;

		case WM_GETDLGCODE:
			return DLGC_WANTARROWS;

		case WM_KEYDOWN:
			switch(wP)
				{
				case VK_LEFT:
					if(SelBlock!=EditBlocks) // NOT (at front or both null)
						{
						SelBlock=EditBlocksPrev(SelBlock);
						InvalidateView();
						}
					break;
				case VK_RIGHT:
					if(SelBlock)
						{
						SelBlock=SelBlock->next;
						InvalidateView();
						}
					break;
				case VK_HOME:
					if(SelBlock!=EditBlocks)
						{
						SelBlock=EditBlocks;
						InvalidateView();
						}
					break;
				case VK_END:
					if(SelBlock)
						{
						SelBlock=NULL;
						InvalidateView();
						}
					break;
				case VK_DELETE:
					DoDelBlock();
					break;
				}
			return 0;
		case WM_PAINT:
			{
			//return 0;
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			if(hwnd==GetFocus())
				PaintFocus(ps.hdc);
			SetWindowOrgEx(ps.hdc,scrollpos,woy,NULL);
			EditBlocksPaint(ps.hdc);
			PaintOrigin(ps.hdc);
			EndPaint(hwnd,&ps);
			}
			return 0;
		case WM_LBUTTONDOWN:
			DoMouseDown(hwnd,wP,lP);
			return 0;
		case WM_LBUTTONUP:
			DoMouseUp(hwnd,wP,lP);
			return 0;
		case WM_MOUSEMOVE:
			DoMouseMove(hwnd,wP,lP);
			return 0;
		case WM_LBUTTONDBLCLK:
			DoMouseDoubleClick(hwnd,wP,lP);
			return 0;
		}
	return DefWindowProc(hwnd,msg,wP,lP);
	}

// Parses the text to create the linked list of CDEVBLOCKS EditBlocks
static void EditBlocksBuild()
	{
	EditBlocksFree();

	IMPCDEVMARK &cdev=EditorMark.cdev;
	if(cdev.bfi<0) // construct no blocks if no format
		return;

	// Block construction parser
	int fi=cdev.bijfmt.font; // running logical font index. init from bijfmt
	char *s=cdev.text; // start of block pointer
	char *p=s;         // block parsing pointer
	char *t=s;         // text start pointer ==s when block starts with text
	while(*s)
		{
		p=s;
		t=s;
		int bi=-1;
		while(*p)
			{
			if(*p=='\033') // ESC ?
				{
				if(p!=s) // If not at start of block then end of block
					break;
				p++;
				if(*p==0)
					break; // ESC unexpected end of block
				if(*p=='F')    // ESC F
					{
					p++;
					if(*p==0)
						break; // unexpected end of block
					// ESC F n
					fi=(*p)-1;
					p++;
					t=p;
					continue; // text may follow
					}
				if(*p=='B')
					{
					p++;
					if(*p==0)
						break; // unexpected end of block
					bi=(*p)-1;
					p++;
					break;
					}
				// ESC <ANYTHING ELSE>
				p++;
				continue;				
				} // end if ESC
			p++; // text
			} // end while *p
		// Create a block?
		int l=p-t;
		if((l==0)||(*t=='\033' && bi<0)) // Invalid block?
			{
			/// WARN INVALID BLOCK
			}
		else if(bi>=0)              // Bitmap block?
			{
			if(bi<cdev.nbsm)
				{
				// Construct BMP BLOCK bi
				EditBlocksAppend(new BMPCDEVBLOCK(bi));
					
				}
			else
				{
				//Invalid if bi>=nbsm
				///WARN INVALID BMP REF
				}
			}
		else                        // Text block?
			{
			// Construct TEXT BLOCK fi t l
			EditBlocksAppend(new TXTCDEVBLOCK(fi,t,l,FALSE));
				
			}
		s=p; // more blocks follow?
		} // end while *s
	}

static void EditBlocksFree()
	{
	if(EditBlocks)
		{
		CDEVBLOCK *b=EditBlocks;
		while(b)
			{
			CDEVBLOCK *n=b->next;
			delete b;
			b=n;
			}
		EditBlocks=NULL;
		SelBlock=NULL;
		}
	}

// Appends newblock to EditBlocks
static void EditBlocksAppend(CDEVBLOCK *newblock)
	{
	if(EditBlocks)
		{
		CDEVBLOCK *b=EditBlocks;
		while(b->next)
			b=b->next;
		b->next=newblock;
		}
	else
		EditBlocks=newblock;
	}

/******************************************************************************
	EditBlocksInsert(newblock)

	Inserts the new block into EditBlocks at SelBlock or at the
	end if SelBlock==NULL.  If EditBlocks is null, EditBlocks becomes
	newblock.
******************************************************************************/
static void EditBlocksInsert(CDEVBLOCK *newblock)
	{
	if(!SelBlock)
		EditBlocksAppend(newblock);
	else
		{
		CDEVBLOCK *pb=EditBlocksPrev(SelBlock);
		if(pb)
			{
			newblock->next=pb->next;
			pb->next=newblock;
			}
		else
			{
			newblock->next=EditBlocks;				
			EditBlocks=newblock;
			}
		}
	}

/******************************************************************************
	EditBlocksDelete(block)

	Deletes the block if it is in EditBlocks.
	If block==SelBlock then Selblock is advanced.
******************************************************************************/
static void EditBlocksDelete(CDEVBLOCK *block)
	{
	if(block==SelBlock) // If deleting selection move sel to next
		SelBlock=SelBlock->next;
	CDEVBLOCK *pb=EditBlocksPrev(block);
	if(pb)
		pb->next=block->next;
	else
		EditBlocks=block->next;

	int bi=block->getbi();
	delete block;
	// Delete the bsm if no other blocks ref it
	if(bi>=0 && (!EditBlocks || !EditBlocks->findbi(bi)))
		{ // Delete the bsm
		delete EditorMark.cdev.bsm[bi];
		EditorMark.cdev.bsm[bi]=0;
		EditorMark.cdev.nbsm--;
		// If not last bsm, close the hole and adjust refs>bi
		if(bi<EditorMark.cdev.nbsm)
			{
			for(int i=bi;i<EditorMark.cdev.nbsm;i++) // Close hole
				EditorMark.cdev.bsm[i]=EditorMark.cdev.bsm[i+1];
			for(CDEVBLOCK *b=EditBlocks;b;b=b->next) // Adjust refs>bi --
				b->adjustbi(bi);
			}
		ClearBSMPreviews();
		}
	}

static BOOL EditBlocksValid()
	{
	if(EditBlocks)
		{
		CDEVBLOCK *b=EditBlocks;
		while(b)
			{
			if(!b->valid)
				return FALSE;
			b=b->next;
			}
		return TRUE;
		}
	return FALSE;
	}

static BOOL EditBlocksCompile(char *cbuf)
	{
	if(!EditBlocksValid())
		return FALSE;

	CDEVBLOCK *p=EditBlocks;
	int l=0;
	while(p)
		{
		l+=p->compile(cbuf+l);
		p=p->next;
		}
	cbuf[l]=0;
	return TRUE;
	}

static void EditBlocksPaint(HDC hdc)
	{
	if(!EditBfValid)
		return;
	SaveDC(hdc);
	int x=0;
	if(EditBlocks)
		{
		CDEVBLOCK *p=EditBlocks;
		while(p)
			{
			p->paint(hdc,&x);
			p=p->next;
			}
		}
	if(!SelBlock)
		{
		RECT r;
		r.left=x;
		r.top=0;
		r.right=x+1;
		r.bottom=EditBf.extent.y*zoom;
		DrawSelRect(hdc,&r);
		}
	RestoreDC(hdc,-1);
	}

/******************************************************************************
	pblk = EditBlocksPrev(block)

	Finds and returns the block previous to the supplied block in EditBlocks
	If block is not in edit blocks, NULL is returned.  If block
	is NULL, the last block in EditBlocks is returned.
******************************************************************************/
static CDEVBLOCK *EditBlocksPrev(CDEVBLOCK *block)
	{
	CDEVBLOCK *b=EditBlocks;
	while(b && b->next!=block)
		b=b->next;
	return b;
	}

/******************************************************************************
	EditBlocksExtent(extent)

	Computs the combined extent of the edit blocks.

	POINT *extent = filled in with the extent or 0,0 on error or no extent
******************************************************************************/
static void EditBlocksExtent(POINT *extent)
	{
	extent->x=0;
	extent->y=0;
	CDEVBLOCK *b=EditBlocks;
	while(b)
		{
		if(b->extent.x==0 && b->extent.y==0)
			b->calcextent();
		extent->x+=b->extent.x;
		extent->y=max(extent->y,b->extent.y);
		b=b->next;
		}
	}

static void CDEVMARKEDITUpdateEnables(HWND hdlg)
	{
	if(EditBlocksValid())
		{
		DlgEnableWindow(hdlg,IDOK,TRUE);
		DlgEnableWindow(hdlg,IDC_BFI,FALSE);
		}
	else
		{
		DlgEnableWindow(hdlg,IDOK,FALSE);
		if(!EditBlocks)
			DlgEnableWindow(hdlg,IDC_BFI,TRUE);
		}
	}

// EnableIDMList enables and disables menu items on the insert menu
// based on the txt and bmp enable flags and the idms classification
// in IDMList
static void EnableIDMList(HWND hwnd,BOOL txt,BOOL bmp)
	{
	HMENU hmenu=GetMenu(hwnd);
	for(int i=0;IDMList[i].idm;i++)
		{
		if(IDMList[i].isbmp)
			EnableMenuItem(hmenu,IDMList[i].idm,bmp?MENAB:MGRAY);
		else
			EnableMenuItem(hmenu,IDMList[i].idm,txt?MENAB:MGRAY);
		}
	}



// Updtes the enabled state of the OK button as follows:
// enabled if there is a font and there is text
static void TXTCDEVEDITUpdateEnables(HWND hdlg)
	{
	int fi=SendDlgItemMessage(hdlg,IDC_FONTS,CB_GETCURSEL,0,0);
	int ltext=SendDlgItemMessage(hdlg,IDC_WEXT,WM_GETTEXTLENGTH,0,0);


	DlgEnableWindow(hdlg,IDOK,(fi>=0 && ltext>0));
	}

/******************************************************************************
	GetEditBfInfo()
	Gets EditBf and all font info for it
******************************************************************************/
static void GetEditBfInfo()
	{
	IMPCDEVMARK &cdev=EditorMark.cdev;
	SendMessage(EditorIJDev,IJM_GETBMPFMT,cdev.bfi,(LPARAM)(LPSTR)&EditBf);
	EditBfValid=TRUE;
	}

/******************************************************************************
	len = LineCopyN(dest,text,ln)

	Copies line ln of text to dest.  Lines are seperated with \n.

	int len = length of line ln or 0 if not found or 0 length
	char *dest = copy destination
	LPSTR text = multi line text to copy from
	int ln = line number
******************************************************************************/
static int linecopyn(char *dest,char *text,int ln)
	{
	int l;

	// Scan to start of line ln or to 0
	l=0;
	while(l<ln)
		{
		if(!*text)
			break;
		if(*text=='\n')
			{
			l++;
			if(l==ln)
				{
				text++;
				break;
				}
			}
		text++;
		}

	// Copy chars in line up to \n \r or \0
	l=0;
	while(*text && *text!='\n' && *text!='\r')
		{
		*dest=*text;
		dest++;
		text++;
		l++;
		}
	*dest=0;
	return l;
	}

/******************************************************************************
	len = LineLenN(text,ln)

	Computes the length of line ln of the text.  Lines are seperated with \n.

	int len = length of line ln or 0 if not found or 0 length
	LPSTR text = multi line text
	int ln = line number
******************************************************************************/
static int linelenn(char *text,int ln)
	{
	int l;

	// Scan to start of line ln or to 0
	l=0;
	while(l<ln)
		{
		if(!*text)
			break;
		if(*text=='\n')
			{
			l++;
			if(l==ln)
				{
				text++;
				break;
				}
			}
		text++;
		}

	// Count chars in line up to \n \r or \0
	l=0;
	while(*text && *text!='\n' && *text!='\r')
		{
		text++;
		l++;
		}
	return l;
	}

/******************************************************************************
	prepfont(csx,csy)

	Updates the preview font for the given text size (csx,csy)

	int csx = char size x
	int csy = char size y
******************************************************************************/
static void prepfont(int csx,int csy)
	{
	if(!font || logfont.lfHeight!=-csy || logfont.lfWidth!=csx)
		{
		if(font)
			DeleteObject(font);
		logfont.lfWidth=csx;
		logfont.lfHeight=-csy;
		font=CreateFontIndirect(&logfont);
		}
	}

/******************************************************************************
	freefont()
	Frees (deletes) the preview font
******************************************************************************/
static void freefont()
	{
	if(font)
		DeleteObject(font);
	font=0;
	}

/******************************************************************************
	InvalidateView
	Invalidates the CDEVVIEW window forcing it to repaint
******************************************************************************/
static void InvalidateView()
	{
	InvalidateRect(vwnd,NULL,TRUE);
	}

/******************************************************************************
	DoDelBlock()

	Deletes the selected block and updates the view.
******************************************************************************/
static void DoDelBlock()
	{
	if(SelBlock)
		{
		EditBlocksDelete(SelBlock);
		ShowBufStat();
		InvalidateView();
		if(!EditBlocks) // If last block deleted
			CDEVMARKEDITUpdateEnables(dwnd);
		}
	}

/******************************************************************************
	DoEditBlock()

	Pops up the editor for the selected block (if any)
******************************************************************************/
static void DoEditBlock()
	{
	if(SelBlock)
		{
		if(SelBlock->edit(dwnd))
			{
			// Forget context info
			SelBlock->extent.x=0;
			SelBlock->extent.y=0;
			SelBlock->clearprev();
			ComputeScrollRange(FALSE);
			}
		InvalidateView();
		CDEVMARKEDITUpdateEnables(dwnd);
		}
	}

/******************************************************************************
	AllocBSMPreviews()

	Called at start of CDEV edit to allocate the space for EditBf.nbuf
	preview bitmaps.
******************************************************************************/
static void AllocBSMPreviews()
	{
	if(!EditBfValid)
		return;
	FreeBSMPreviews();
	nBSMPreviews=EditBf.nbuf;
	BSMPreviews=new HBITMAP [nBSMPreviews];
	for(int i=0;i<nBSMPreviews;i++)
		BSMPreviews[i]=0;
	}


/******************************************************************************
	FreeBSMPreviews()

	Frees memory used by BSMPrevies
******************************************************************************/
static void FreeBSMPreviews()
	{
	if(BSMPreviews)
		{
		for(int i=0;i<nBSMPreviews;i++)
			if(BSMPreviews[i])
				DELOBJ(BSMPreviews[i]);
		delete [] BSMPreviews;
		BSMPreviews=NULL;
		nBSMPreviews=0;
		}
	}

/******************************************************************************
	ClearBSMPreviews()

	Clears all BSM previews so that they will  be regenerated.
******************************************************************************/
static void ClearBSMPreviews()
	{
	if(BSMPreviews)
		{
		for(int i=0;i<nBSMPreviews;i++)
			if(BSMPreviews[i])
				DELOBJ(BSMPreviews[i]);
		}
	}

/******************************************************************************
	UpdateBSMPreviews(m)

	Updates any invalid BSM previews by compiling the mark.
******************************************************************************/
static void UpdateBSMPreviews(IMPMARK *m)
	{
	if(!EditBfValid)
		return;

	if(nBSMPreviews!=EditBf.nbuf)
		return;

	int bfi;
	double xres,yres;
	WORD ms;
	BOOL neg;
	for(int i=0;i<m->cdev.nbsm;i++)
		if(!BSMPreviews[i])
			BSMPreviews[i]=IMPCompileRaster(m->cdev.bsm[i],
				&EditorIMP.vars,&EditorPreIVals,
				&EditorIMP.gvars,&EditorPreGVals,
				EditorIMP.pheader,
				&EditorIMP.udhas,
				&bfi,&xres,&yres,&ms,&neg,FALSE);
	LoadedBitmapsClear();
	}

/******************************************************************************
	DoMouseDown(hwnd,wP,lP)

	Handles WM_LBUTTONDOWN messages.
******************************************************************************/
static void DoMouseDown(HWND hwnd,WPARAM wP,LPARAM lP)
	{
	SetCapture(hwnd);
	SetFocus(hwnd);
	DownPoint=MAKEPOINTS(lP);
	DownPoint.x+=scrollpos;
	DownPoint.y+=woy;
	mstate=msDOWN;
	}

/******************************************************************************
	DoMouseUp(hwnd,wP,lP)

	Handles WM_LBUTTONUP messages.
******************************************************************************/
static void DoMouseUp(HWND hwnd,WPARAM wP,LPARAM lP)
	{
	ReleaseCapture();
	switch(mstate)
		{
		case msDOWN: // If click without drag : attempt selection
			{
			CDEVBLOCK *NewSelBlock=HitTestSelect(EditBlocks,DownPoint);
			if(NewSelBlock!=SelBlock)
				{
				SelBlock=NewSelBlock;
				InvalidateRect(hwnd,NULL,TRUE);
				}
			break;
			}
		case msDRAG:
			{
			RECT r;
			POINT ofs;
			HDC hdc=GetDC(hwnd);
			SetWindowOrgEx(hdc,scrollpos,woy,NULL); // Set origin based on scrollpos
			SetROP2(hdc,R2_XORPEN);
			SelectObject(hdc,GetStockObject(NULL_BRUSH));
			SelectObject(hdc,SelPen);
			r=DragRect;
			ofs.x=DragPoint.x-DownPoint.x;
			ofs.y=DragPoint.y-DownPoint.y;
			OffsetRect(&r,ofs.x,ofs.y);
			Rectangle(hdc,r.left,r.top,r.right,r.bottom);
			DragPoint=MAKEPOINTS(lP);
			DragPoint.x+=scrollpos;
			DragPoint.y+=woy;
			ofs.x=DragPoint.x-DownPoint.x;
			ofs.y=DragPoint.y-DownPoint.y;
///MOVE THE BLOCK IN THE LIST SEQUENCE TO IST CLOSEST
///POSITION IN THE LIST
///			SelBlock->moverel(ofs);
///
			InvalidateView();
			ReleaseDC(hwnd,hdc);
			break;
			}
		case msSELECT:
			{
			RECT r;
			HDC hdc=GetDC(hwnd);
			SetWindowOrgEx(hdc,scrollpos,woy,NULL); // Set origin based on scrollpos
			SetROP2(hdc,R2_NOT);
			SelectObject(hdc,GetStockObject(NULL_BRUSH));
			r.left=DownPoint.x;
			r.top=DownPoint.y;
			r.right=DragPoint.x;
			r.bottom=DragPoint.y;
			Rectangle(hdc,r.left,r.top,r.right,r.bottom);
			SetROP2(hdc,R2_COPYPEN);	
			ReleaseDC(hwnd,hdc);
			InvalidateView();
			break;
			}
		}
	mstate=msUP;
	}

/******************************************************************************
	DoMouseMove(hwnd,wP,lP)

	Handles WM_MOUSEMOVE messages.
******************************************************************************/
static void DoMouseMove(HWND hwnd,WPARAM wP,LPARAM lP)
	{
	if(mstate!=msUP)
		{
		RECT r;
		POINT ofs;
		CDEVBLOCK *NewSelBlock;
		HDC hdc=GetDC(hwnd);
		SetWindowOrgEx(hdc,scrollpos,woy,NULL); // Set origin based on scrollpos
		switch(mstate)
			{
			case msDOWN:
				NewSelBlock=HitTestSelect(EditBlocks,DownPoint);
				if(NewSelBlock!=SelBlock)
					{
					SelBlock=NewSelBlock;
					InvalidateRect(hwnd,NULL,TRUE);
					}
				if(SelBlock) // Block being dragged
					{
					SelBlock->getbbox(&DragRect);
					SetROP2(hdc,R2_XORPEN);
					SelectObject(hdc,GetStockObject(NULL_BRUSH));
					SelectObject(hdc,SelPen);
					DragPoint=MAKEPOINTS(lP);
					DragPoint.x+=scrollpos;
					DragPoint.y+=woy;
					r=DragRect;
					ofs.x=DragPoint.x-DownPoint.x;
					ofs.y=DragPoint.y-DownPoint.y;
					OffsetRect(&r,ofs.x,ofs.y);
					Rectangle(hdc,r.left,r.top,r.right,r.bottom);
					SetROP2(hdc,R2_COPYPEN);	
					mstate=msDRAG;
					}
				else // No object selected, do SELECT
					{
					SetROP2(hdc,R2_NOT);
					SelectObject(hdc,GetStockObject(NULL_BRUSH));
					DragPoint=MAKEPOINTS(lP);
					DragPoint.x+=scrollpos;
					DragPoint.y+=woy;
					r.left=DownPoint.x;
					r.top=DownPoint.y;
					r.right=DragPoint.x;
					r.bottom=DragPoint.y;
					Rectangle(hdc,r.left,r.top,r.right,r.bottom);
					SetROP2(hdc,R2_COPYPEN);	
					mstate=msSELECT;
					}
				break;
			case msDRAG:
				SetROP2(hdc,R2_XORPEN);
				SelectObject(hdc,GetStockObject(NULL_BRUSH));
				SelectObject(hdc,SelPen);
				r=DragRect;
				ofs.x=DragPoint.x-DownPoint.x;
				ofs.y=DragPoint.y-DownPoint.y;
				OffsetRect(&r,ofs.x,ofs.y);
				Rectangle(hdc,r.left,r.top,r.right,r.bottom);
				DragPoint=MAKEPOINTS(lP);
				DragPoint.x+=scrollpos;
				DragPoint.y+=woy;
				r=DragRect;
				ofs.x=DragPoint.x-DownPoint.x;
				ofs.y=DragPoint.y-DownPoint.y;
				OffsetRect(&r,ofs.x,ofs.y);
				Rectangle(hdc,r.left,r.top,r.right,r.bottom);
				SetROP2(hdc,R2_COPYPEN);	
				break;
			case msSELECT:
				SetROP2(hdc,R2_NOT);
				SelectObject(hdc,GetStockObject(NULL_BRUSH));
				r.left=DownPoint.x;
				r.top=DownPoint.y;
				r.right=DragPoint.x;
				r.bottom=DragPoint.y;
				Rectangle(hdc,r.left,r.top,r.right,r.bottom);
				DragPoint=MAKEPOINTS(lP);
				DragPoint.x+=scrollpos;
				DragPoint.y+=woy;

				r.right=DragPoint.x;
				r.bottom=DragPoint.y;
				Rectangle(hdc,r.left,r.top,r.right,r.bottom);
				SetROP2(hdc,R2_COPYPEN);	
				break;
			}
		ReleaseDC(hwnd,hdc);
		}
	}

/******************************************************************************
	DoMouseDoubleClick(hwnd,wP,lP)

	Handles WM_LBUTTONDBLCLICK messages.
******************************************************************************/
static void DoMouseDoubleClick(HWND hwnd,WPARAM wP,LPARAM lP)
	{
	if(mstate!=msUP)
		ReleaseCapture();
	mstate=msUP;
	DoEditBlock();
	}

/******************************************************************************
	block = HitTestSelect(first,p)

	Searches the block list starting with first for the first block that
	contains the point p.  If found that blocks pointer is returned.  If
	not found, NULL is returned.

	CDEVBLOCK *block = Pointer to found block or NULL if not found.
	CDEVBLOCK *first = First block in list to search.
	POINT p = Point to hit test with.
******************************************************************************/
static CDEVBLOCK *HitTestSelect(CDEVBLOCK *first,POINTS p)
	{
	CDEVBLOCK *b=first;
	while(b)
		{
		if(b->hittest(p))
			break;
		b=b->next;
		}
	return b;
	}

/******************************************************************************
	BOOL DlgEnableWindow(hdlg,idc,enable)

	Simmilar to EnableWindow except that it uses a hdlg,idc pair instead
	of a hwnd and also it is sensitive to disabeling the focus in the dialog.
	When it detects that the focus window it to be disabled, it sends a
	WM_NEXTDLGCTL message to the dlg to get the focus to move.
******************************************************************************/
static BOOL DlgEnableWindow(HWND hdlg,int idc,BOOL enable)
	{
	HWND hwnd=GetDlgItem(hdlg,idc);
	HWND focus=GetFocus();
	if(hwnd==focus && !enable)
		PostMessage(hdlg,WM_NEXTDLGCTL,0,0);
	return EnableWindow(hwnd,enable);
	}

/******************************************************************************
	PaintOrigin(hdc)

	Paints the origin idicator based on the current justification.
	Assumes the WindowOrigin has been set with respect to the scroll bar.
******************************************************************************/
static void PaintOrigin(HDC hdc)
	{
	int x=0;
	switch(EditorMark.justx)
		{
		case mjxLEFT:   x=0;        break;
		case mjxCENTER: x=bext.x/2; break;
		case mjxRIGHT:  x=bext.x;   break;
		}
	int y=0;
	switch(EditorMark.justy)
		{
		case mjyTOP:    y=0;        break;
		case mjyMIDDLE: y=bext.y/2; break;
		case mjyBOTTOM: y=bext.y;   break;
		}
	HPEN oldpen=(HPEN)SelectObject(hdc,OriginPen);
	MoveToEx(hdc,x-(OriginSize>>1),y,NULL);
	LineTo(hdc,x+(OriginSize>>1)+1,y);
	MoveToEx(hdc,x,y-(OriginSize>>1),NULL);
	LineTo(hdc,x,y+(OriginSize>>1)+1);
	SelectObject(hdc,oldpen);
	}

/*****************************************************************************
	PaintFocus(hdc)
	Paints or Unpaints the focus rect
******************************************************************************/
static void PaintFocus(HDC hdc)
	{
	RECT crect;
	GetClientRect(vwnd,&crect);
	DrawFocusRect(hdc,&crect);
	}

/******************************************************************************
	ComputeScrollRange(init)
	Computes the scroll bar range, Hides/Shows the scrollbar and optionally
	inits the scroll position to the end.
******************************************************************************/
static void ComputeScrollRange(BOOL init)
	{ // Set scroll range or hide scroll bar if no range required
	EditBlocksExtent(&bext);
	RECT crect;
	GetClientRect(vwnd,&crect);
	scrollpage=crect.right*9/10;
	int range=(bext.x+2*scrollextra)-crect.right;
	range=max(range,0);
	scrollmin=-scrollextra;
	scrollmax=scrollmin+range;
	SetScrollRange(vwnd,SB_HORZ,scrollmin,scrollmax,TRUE);
	if(init)
		scrollpos=scrollmax;
	else
		scrollpos=min(scrollpos,scrollmax);
	SetScrollPos(vwnd,SB_HORZ,scrollpos,FALSE);
	}

/******************************************************************************
	ShowBufStat()

	Update the IDC_BUFSTAT text control on the CDEVEDIT dialog to reflect
	the current number of used bitmap buffers over the total buffers for
	the current bitmap format.  Sets the text to blank if there is no
	current valid bitmap format.
******************************************************************************/
static void ShowBufStat()
	{
	char txt[32];
	if(EditBfValid)
		wsprintf(txt,"%d/%d buffers used",EditorMark.cdev.nbsm,EditBf.nbuf);
	else
		txt[0]=0;
	SetDlgItemText(dwnd,IDC_BUFSTAT,txt);
	}

/******************************************************************************
	DrawSelRect(hdc,r)

	Draws a rectangle in the selection style
******************************************************************************/
static void DrawSelRect(HDC hdc,RECT *r)
	{
	HBRUSH oldbrush=(HBRUSH)SelectObject(hdc,GetStockObject(NULL_BRUSH));
	HPEN oldpen=(HPEN)SelectObject(hdc,SelPen);
	int oldbkm=SetBkMode(hdc,TRANSPARENT);
	Rectangle(hdc,r->left,r->top,r->right,r->bottom);
	SetBkMode(hdc,oldbkm);
	SelectObject(hdc,oldpen);
	SelectObject(hdc,oldbrush);
	}

BOOL CALLBACK UPDATEFONTDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	switch(msg){
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		{
			switch(LOWORD(wP)){
			case IDOK:
				GetDlgItemText(hdlg,IDC_EDIT1,newFontName,sizeof(newFontName));
				EndDialog(hdlg,TRUE);
				return TRUE;
			case IDCANCEL:
				EndDialog(hdlg,FALSE);
				break;
			}
		}
		break;
	}
	return FALSE;
}