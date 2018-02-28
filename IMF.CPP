/******************************************************************************
	NAME : IMF.C
	DESC : IMF Inkjet Marking Fixtures
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <commdlg.h>
#include <memory.h>
#include <math.h>
#include <imslib32\imslib32.h>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "xload.h"
#include "cdm.h"
#include "lbmp.h"
#include "clamp.h"
#include "stdloc.h"
#include "cure.h"
#include "resource.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
// Run IDCs
#define IDC_PLUS1    1000
#define IDC_MINUS1   1001
#define IDC_QTY      1002
#define IDC_ALL      1003
#define IDC_CLAMPREL 1004

// Edit IDCs
#define IDC_EXTENT   2001
#define IDC_NESTS    2002
#define IDC_ADDNEST  2003
#define IDC_DELNEST  2004
#define IDC_DELANEST 2011   //----------Linghai-11-4-08
#define IDC_ADDGRID  2005
#define IDC_ORIGIN   2006
#define IDC_DESCRIPT 2007
#define IDC_EXTRT  2012  //--Sam 09/15/2016 add dropdown menu for rotary fixture
#define IDC_XYMODE   2008
#define IDC_CCWPLUS  2009
#define IDC_RFDRV    2010

#define LINCH  1000.0
#define LMAT     50.0

#define LSIGIMF sizeof(sigIMF)-1
#define LSIGIMFV357 (sizeof(sigIMFV357)-1)
#define LSIGIMFV345 (sizeof(sigIMFV345)-1)
#define LSIGIMFV338 (sizeof(sigIMFV338)-1)
#define LSIGIMFV330 (sizeof(sigIMFV330)-1)
#define LSIGIMF4000 (sizeof(sigIMF4000)) // null included


/******************************************************************************
	CALLBACK FUNCTION PROTOTYPES
******************************************************************************/
LRESULT CALLBACK IMFRUNWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);
LRESULT CALLBACK IMFEDITWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK GRIDDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK NESTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static void setmapping(HDC hdc);
static void drawpart(HDC hdc,LPP2 origin,LPP2 extent,double rotation,int index);
static void drawrect(HDC hdc,LPP2 origin,LPP2 extent,double rotation);

static void setrotmapping(HDC hdc);// outer part line for rotary --sam 01/19/2017
static void drawpartcircle(HDC hdc,LPP2 originrot,LPP4 extentrot,int index); //Draw rotary part ---sam 1/17/2017
static void drawcircle(HDC hdc,LPP2 originrot,LPP4 rotfixextent); //Draw rotary fixture	--sam1/17

static long RunWMCreate(HWND hwnd);
static long RunWMCommandIDC(HWND hwnd,WORD idc,WORD ncode);
static BOOL EditIMFUpdate(HWND hwnd,BOOL herr);
static long EditWMCreate(HWND hwnd);
static long EditWMCommandIDM(HWND hwnd,WPARAM wP);
static long EditWMCommandIDC(HWND hwnd,WPARAM wP,LPARAM lP);
static long EditWMNotifyIDC(HWND hwnd,WPARAM wP,LPARAM lP);
static long EditAddNestBNCLicked(void);
static long EditDelNestBNCLicked(void);
static long EditDelANestBNCLicked(void);   //--------Linghai-11-4-08-------
static long EditAddGridBNCLicked(void);
static void DoQty(HWND hwnd);
static void DoAll(HWND hwnd);
static BOOL LoadNativeIMF(HANDLE fh,PIMF pimf,int ver);
static HBRUSH FlagBrush(WORD flags);
static void ResetProgressNBFs(HWND hwnd);
static PIMSTEP CompileInspectPrep(PIMP pimp,IMPVALS *pvals);
static PIMSTEP CompileReadPrep(PIMP pimp,IMPVALS *pvals);
static BOOL PtInRectRotated(LPRECT r,POINT p,double rotation);
static void ComputeNestBounds(LPRECT r,double rotation);
static void GetEditNEST(int i);
void InsertEditNEST(int i,BOOL bReplace);
static void InitEditNEST(void);
static BOOL DoNestEdit(HWND parent);
static void DoNestLocate(HWND hdlg);
static void AddNest(double orgx,double orgy,double extx,double exty,double rot);
static void InsertNest(double orgx,double orgy,double extx,double exty,double rot,int idx);
static void GetNestVect(LPSTR vect,int idx);

/******************************************************************************
	PUBLIC VARIABLES
******************************************************************************/
IMF RunIMF;
IMF EditIMF;
IMFNEST EditNEST;
BOOL RefRightEdge;  // Causes IMFs to ref right edge instead of left
BOOL RefBottomEdge; // Causes IMFs to ref bottom edge instead of top
BOOL ShowRefEdge;   // Causes IMF painting to display ref edge
HPEN RefEdgePen;    // Pen used to draw ref edge if ShowRefEdge
double zlast;        // Last WCS height used in IMPCompile
double zsafe;        // TCS safe height for xy travel
double zmarker;      // WCS marker height
double zvis;         // WCS reader/inspector height
double currpos;

/******************************************************************************
	STATIC VARIABLES
******************************************************************************/
static char IMFRunClass[]="IMFRUN"; // IMF Run window class name
static char IMFEditClass[]="IMFEDIT"; // IMF Edit window class name
static char sigIMF[]="JETIMF5000V400\x1a"; // Native File signiture
static char sigIMFV357[]="JETIMF4001V357\x1a"; // Native File signiture
static char sigIMFV345[]="JETIMF4001V345\x1a"; // Old 3.45
static char sigIMFV338[]="JETIMF4001V338\x1a"; // Old 3.38-3.34 signiture
static char sigIMFV330[]="JETIMF4001V330\x1a"; // Old 3.30 File signiture
static char sigIMF4000[]="JETIMF4000\x1a"; // Pre 3.30 File signiture
static POINT bsize; // run button size
static POINT wsize; // run window size
static char IMFIniSection[]="IMF";
static HBRUSH FixtureBrush;
static HBRUSH NoPartBrush;
static HBRUSH PartBrush;
static HBRUSH StartedPartBrush;
static HBRUSH MarkedPartBrush;
static HBRUSH PassedPartBrush;
static HBRUSH FailedPartBrush;
static HBRUSH CuredPartBrush; // for cure only programs (no marks or inspects)
static HBRUSH CuredMarkedPartBrush;
static HBRUSH CuredPassedPartBrush;
static HBRUSH LabelBrush;
static COLORREF LabelColor=RGB(0,0,128);
static WORD FocusID=0;
static HWND QtyBtn;      // Run : Qty button
static HWND AllBtn;      // Run : All n button
static HWND Plus1Btn;    // Run : +1 button
static HWND Minus1Btn;   // Run : -1 button
static HWND ClampRelBtn; // Run : ClampRelease button
static HWND ExtentWnd;   // Edit : Extent KEDIT
static HWND RotExtentWnd;   // Edit : ROTARY Extent KEDIT-----------SAM ----10/5/2016

static HWND NestsWnd;    // Edit : Nests XLISTVIEW
static HWND AddNestBtn;  // Edit : Add Location button
static HWND DelNestBtn;  // Edit : Del Location button
static HWND DelANestBtn; // Edit : Del All Location button       --------Linghai-11-4-08
static HWND AddGridBtn;  // Edit : Add Grid button
static HWND OriginWnd;   // Edit : Origin KEDIT
static HWND OriginrotWnd;  // Edit : rotary Origin KEDIT -------sam-1/19/2017
static HWND DescWnd;     // Edit : Description KEDIT
static HWND XYModeWnd;   // Edit : XCOMBOBOX
static HWND CCWPlusWnd;  // Edit : X button (checkbox)
static HWND RFDrvWnd;    // Edit : KEDIT
static HWND LblDesc;     // Desc label
static HWND LblOrg;      // Origin label
static HWND LblNests;    // Nests label
static HWND LblXYMode;   // XYMode label
static HWND LblRFDrv;    // RFDrv label
static HWND LblExtRt;    // Rotary Extent label
static HWND LblExt;		 // Extent label

static int UNVS;         // User Nest V Size :  5 | 4 if HideOptRotation
static unsigned convtimer=0; // Timer used by ThruConv or simplimatic reset

static char dbuf[512];

/******************************************************************************
	GRID Class
******************************************************************************/
class GRID
	{
public:
	POINT order; // x and y dimensions in nests
	P2 origin;   // Origin of 0,0 nest
	P2 size;     // Size of each nest or 0 component to take on PartSize
	P2 spacing;  // Nest to nest spacing
	double rotation;
	double degree;
	GRID();
	BOOL Update(HWND hdlg,int count); // updates the grid from the dialog
	void UpdateDisplay(HWND hdlg); // update the dialog from the grid
	void DoLocate(HWND hdlg);
	void DoSize(HWND hdlg);
	};

static GRID EditGrid; // The grid during grid edit

IMF::IMF(){
	origin.x=0.0;
	origin.y=0.0;
	originrot.x=0.0;//sam
	originrot.y=0.0;//sam

	extent.x=10.0;
	extent.y=10.0;
	extent.z=0.0;
	extentrot.d=10.0;	//--sam	10/6/16
	extentrot.z=0.5;	//--sam	10/6/16

	desc[0]=0;
	xymode=imXY;
	ccwplus=FALSE;
	rfdrv[0]=0;
	rotation=0.0f;
}

void IMF::init(){
	origin.x=0.0;
	origin.y=0.0;
	originrot.x=0.0;//sam
	originrot.y=0.0;//sam
	extent.x=10.0;
	extent.y=10.0;
	extent.z=0.0;
	extentrot.d=10.0;	//--sam	10/6/16
	extentrot.z=0.5;	//--sam	10/6/16
	desc[0]=0;
	xymode=imXY;
	ccwplus=FALSE;
	rfdrv[0]=0;
	rotation=0.0f;
}

void DebugMatrix(LPM3 m)
	{
	static char mbuf[128];
	sprintf(mbuf,"         {%0.4f,%0.4f,%0.4f}\n",m->m3[0][0],m->m3[0][1],m->m3[0][2]);
	OutputDebugString(mbuf);
	sprintf(mbuf,"         {%0.4f,%0.4f,%0.4f}\n",m->m3[1][0],m->m3[1][1],m->m3[1][2]);
	OutputDebugString(mbuf);
	sprintf(mbuf,"         {%0.4f,%0.4f,%0.4f}\n",m->m3[2][0],m->m3[2][1],m->m3[2][2]);
	OutputDebugString(mbuf);
	}

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	ok = IMFNew(pimf)

	Initializes and allocates memory for 0 nests

	BOOL ok = TRUE on success.
	PIMF pimf = Pointer to the IMF to load.
******************************************************************************/
BOOL IMFNew(PIMF pimf)
	{
	pimf->init();
	pimf->fn[0]=0;
	pimf->desc[0]=0;
	pimf->nests=IMFNESTS();
//	if(!pimf->nests.nest[0])
//		{
//		ShowError(IDE_NOMEM);
//		return FALSE;
//		}
	pimf->nests.count=0;
	pimf->valssize=0;
	pimf->pvalslist=NULL;
	pimf->changed=FALSE;
	pimf->aesetup=NULL;
	pimf->aeinfo=NULL;
	return TRUE;
	}

/******************************************************************************
	ok = IMFAllocVals(pimf)

	Allocates the vals for an IMF and the gvals of the imp.
	They must then be IMFInitVals()ed.

	BOOL ok = TRUE on success.
	PIMF pimf = Pointer to the IMF to allocate vals for.
******************************************************************************/
BOOL IMFAllocVals(PIMF pimf)
	{
	int i;

	// Allocate instance vals block for all nests
	pimf->valssize=sizeof(IMPVALS)+pimf->imp.vars.count*sizeof(IMPVAL);
	
	// zzz - New format
	pimf->pvalslist=new IMPVALS[pimf->nests.count];
	
	
	if(!pimf->pvalslist)
		{
		ShowError(IDE_NOMEM);
		return FALSE;
		}
	for(i=0;i<pimf->nests.count;i++)
		{
	//	IMPVALS *pvals=(IMPVALS *)(pimf->pvalslist+i*pimf->valssize);
		pimf->pvalslist[i] = IMPVALS(pimf->imp.vars.count);
	    // Allocate a vals list for each nest
		if(!pimf->pvalslist[i].val && pimf->imp.vars.count > 0)
		{
			ShowError(IDE_NOMEM);
			return FALSE;
		}
		pimf->pvalslist[i].count=pimf->imp.vars.count;
		}

	// Allocate all global vals
	pimf->imp.gvals= IMPVALS(pimf->imp.gvars.count);
	if(!pimf->imp.gvals.val && pimf->imp.gvars.count>0)
		{
		ShowError(IDE_NOMEM);
		return FALSE;
		}
	pimf->imp.gvals.count=pimf->imp.gvars.count;

	return TRUE;
	}

/******************************************************************************
	pval = IMFGetNestVals(pimf,nestID)

    Get a reference to the vals for the nest

	pval = Pointer to first val in nest on success.
	PIMF pimf = Pointer to the IMF to get data from
	WORD nestID = The id of the nest
******************************************************************************/
IMPVALS* IMFGetNestVals(PIMF pimf,WORD nestID){
	// Out of range
	if (nestID < 0 || nestID >= pimf->nests.count)
		return NULL;
	// Return the IMPVALS for the nest
	return &(pimf->pvalslist[nestID]);
}

/******************************************************************************
	ok = IMFInitVals(pimf)

	Initializes the vals of an IMF and the gvals of the IMF's IMP by first
	call IMPInitVals for each one and then for the instance vals, calling
	IMPNextVals once for the 2nd 2 times for the 3rd etc.. to get them into
	succesor order.

	BOOL ok = TRUE on success.
	PIMF pimf = Pointer to the IMF to initialize vals for.
******************************************************************************/
BOOL IMFInitVals(PIMF pimf)
	{
	// zzz - New format
	IMFNEST *pnest;
	IMPVALS *pvals;
	BOOL showerror;
	int i,j;
	BOOL ok;

	ok=TRUE;

	// Initialize global vals
	ok=IMPInitVals(&pimf->imp.gvals,&pimf->imp.gvars);
	if(ok)
		{
		// Initialize vals for all nests
		showerror=TRUE;
		for(i=0;i<pimf->nests.count;i++)
			{
			pnest=&(pimf->nests.nest[i]);
			pvals=&(pimf->pvalslist[i]);
			// Initialize them
			if(!IMPInitVals(pvals,&pimf->imp.vars))
				{
				ok=FALSE;
				break; // error
				}
			// Next them i times
			for(j=i;j--;)
				if(IMPNextVals(pvals,&pimf->imp.vars,showerror))
					showerror=FALSE; // only show error once
			}
		}
	return ok;
	}

/******************************************************************************
	ok = IMFNextVals(pimf)

	Calls IMPNextVals for each nest n times where n is the nest number
    of the highest occupied nest.  Also calls IMPNextVals for the globals.

	Example
		nloaded=
		1 2 3 4
	becomes
		5 6 7 8

	BOOL ok = TRUE on success.
	PIMF pimf = Pointer to the IMF to Next vals for.
******************************************************************************/
void IMFNextVals(PIMF pimf)
	{
	// zzz - New format
	IMPVALS* pvals;
	WORD ntimes;
	int i,j;
	BOOL showerror;

	// Do Globals 1 time

	showerror=TRUE;
	if(IMPNextVals(&pimf->imp.gvals,&pimf->imp.gvars,showerror))
		showerror=FALSE;

	// Do Instance vars n times

	// Deternine ntimes
	ntimes=0;
	for(i=0;i<pimf->nests.count;i++)
		if(pimf->nests.nest[i].flags & nbfOCCUPIED)
			ntimes++;

	// Next vals in all nests ntimes times
	for(i=0;i<pimf->nests.count;i++)
		{
		pvals=&(pimf->pvalslist[i]);
		// Next them n times
		for(j=ntimes;j--;)
			if(IMPNextVals(pvals,&pimf->imp.vars,showerror))
				showerror=FALSE;
		}
	}

/******************************************************************************
	IMFPropogateVals(pimf,ni)

	Copies the successors of the vals for the nest at ni to the vals of
	all higher nests.

	PIMF pimf = Pointer to the IMF to propogate vals for.
	int ni = nest index of vals to propogate
******************************************************************************/
void IMFPropogateVals(PIMF pimf,int ni,int vi)
	{
	IMPVAR *pvar;
	IMPVAL *pdval,*psval;
	IMPVALS *psrcvals,*pdstvals;
	BOOL svdone,bvdone; // counter done error flags
//	int i,j;

	if(pimf->imp.vars.count<1)
		return;
	svdone=FALSE;
	bvdone=FALSE;

	while(ni<pimf->nests.count-1)
		{
		psrcvals=&(pimf->pvalslist[ni]);
		pdstvals=&(pimf->pvalslist[ni+1]);
		pvar=&(pimf->imp.vars.var[vi]);
		pdval=&(pdstvals->val[vi]);
		psval=&(psrcvals->val[vi]);
		if(pvar->type==vtKEYED)
			lstrcpy(pdval->k,psval->k);
		else if(pvar->type==vtBASED)
			{
			lstrcpy(pdval->b,psval->b);
			if(!IMPNextBasedVal(pdval,pvar))
				bvdone=TRUE;
			}
		else // Serial
			{
			pdval->s=psval->s;
			pdval->c=psval->c;
			if(pimf->nests.nest[ni].flags&nbfOCCUPIED)
				{
				if(!IMPNextSerialVal(pdval,pvar))
					svdone=TRUE;
				}
			}
		ni++;
		}
	if(bvdone)
		ShowError(IDE_BVALDONE);
	if(svdone)
		ShowError(IDE_SVALDONE);
	}

/******************************************************************************
	ok = IMFLoad(pimf)

	Loads an IMF file into a memory IMF and allocates its nests.
	The ofs in the imf identifies the file.

	BOOL ok = TRUE on success.
	PIMF pimf = Pointer to the IMF to load.
******************************************************************************/
BOOL IMFLoad(PIMF pimf)
	{
	HANDLE fh;         // file handle
	BOOL ok;       // return from LoadNativeIMF  or XLoadIMFXXXXX

	// Initialize all members except ofs

	// Init all member pointers to NULL 
//	pimf->pnests=NULL;
	pimf->pvalslist=NULL;
	pimf->valssize=0;
	pimf->aesetup=NULL;
	pimf->aeinfo=NULL;
	pimf->changed=FALSE;

	WORD ver;

	// ReOpen the file
	fh=CreateFile(pimf->fn,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE) // On open error
		{
		IDError=IDE_OPENIMF;
		return FALSE;
		}

	// Read the signiture
	_lread32(fh,tbuf,MAXSIGNITURE);

	// Dispatch loader
	if(!memcmp(tbuf,sigIMF,LSIGIMF))
		{
		_llseek32(fh,LSIGIMF,0);
		ver=0;
	//	ok=LoadNativeIMF(fh,pimf,0);
		}
	else if(!memcmp(tbuf,sigIMFV357,LSIGIMFV357))
		{
		_llseek32(fh,LSIGIMFV357,0);
		ver=357;
		}
	else if(!memcmp(tbuf,sigIMFV345,LSIGIMFV345))
		{
		_llseek32(fh,LSIGIMFV345,0);
		ver=345;
	//	ok=LoadNativeIMF(fh,pimf,345);
		}
	else if(!memcmp(tbuf,sigIMFV338,LSIGIMFV338))
		{
		_llseek32(fh,LSIGIMFV338,0);
		ver=338;
//zzz		ok=XLoadIMFV338(fh,pimf);
		}
	else if(!memcmp(tbuf,sigIMFV330,LSIGIMFV330))
		{
		_llseek32(fh,LSIGIMFV330,0);
		ver=330;
//		ok=XLoadIMFV330(fh,pimf);
		}
	else if(!memcmp(tbuf,sigIMF4000,LSIGIMF4000))
		{
		_llseek32(fh,LSIGIMF4000,0);
		ver=300;
//		ok=XLoadIMF4000(fh,pimf);
		}
	else
		{ // Signiture not recognized
		IDError=IDE_FILETYPE;
		CloseHandle(fh);
		return FALSE;
		}

	//Read the header
	if(!((IMFHEADER*)pimf)->read(fh,ver)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	WORD count;
	// ----------- NESTS -------------- //
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the nestCount
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	if(count>IMFMAXNESTS)              // Check range
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	
	pimf->nests=IMFNESTS(count); // Allocate nests and read it
	if(!pimf->nests.nest && count>0)
	{
		IDError=IDE_NOMEM;
		ok=FALSE;
	}
	if(!pimf->nests.read(fh,ver,count)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	
	// Check for Alignment Extension
	AESETUP aetemp;
	WORD aever;
	if(aetemp.readsig(fh,aever))
		{
		// Allocate aesetup and load it
		pimf->aesetup=new AESETUP();
		if(!pimf->aesetup)
			{
			IDError=IDE_NOMEM;
			return FALSE;
			}
		if(!pimf->aesetup->load(fh,aever))
			return FALSE;
		}
	

	CloseHandle(fh);
	pimf->changed=FALSE;

	return TRUE; // ok

	// On error free the successfully allocated parts
	if(!ok)
		{
//		DELZ(pimf->pnests);
		DELZ(pimf->pvalslist);
		}
	return ok;
	}

/******************************************************************************
	ok = IMFSave(pimf)

	Saves an IMF to a file.

	BOOL ok = TRUE on success.
	PIMF pimf = Pointer to the IMF to save.
******************************************************************************/
BOOL IMFSave(PIMF pimf)
	{
	HANDLE fh;
	DWORD wlen;

	// Recreate the file
	fh=CreateFile(pimf->fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,NULL);
	DWORD huh = GetLastError();
	if(fh==INVALID_HANDLE_VALUE)
		{
		ShowError(IDE_CREATEFILE);
		return FALSE;
		}

	// Write the signiture
	wlen=LSIGIMF;
	wlen-=_lwrite32(fh,(LPSTR)&sigIMF,wlen);
	if(wlen)
		{
		CloseHandle(fh);
		ShowError(IDE_WRITEFILE);
		return FALSE;
		}

	// Write header
	if(!((IMFHEADER*)pimf)->write(fh)){
		ShowError(IDE_WRITEFILE);
		return FALSE;
	}
	//wlen=sizeof(IMFHEADER);
	//wlen-=_lwrite32(fh,(LPSTR)(&pimf->origin),wlen);
	//if(wlen)
	//	{
	//	CloseHandle(fh);
	//	ShowError(IDE_WRITEFILE);
	//	return FALSE;
	//	}

	// Write nests
//	wlen=sizeof(WORD)+(pimf->nests.count)*sizeof(IMFNEST);
//	wlen-=_lwrite32(fh,(LPSTR)&pimf->nests,wlen);
//	if(wlen)
//		{
//		CloseHandle(fh);
//		ShowError(IDE_WRITEFILE);
//		return FALSE;
//		}

	if(!pimf->nests.write(fh)){
		ShowError(IDE_WRITEFILE);
		return FALSE;
	}

	pimf->changed=FALSE;

	if(pimf->aesetup)
		if(!pimf->aesetup->save(fh))
			return FALSE;

	CloseHandle(fh);
	return TRUE;
	}

/******************************************************************************
	IMFFree(pimf)

	Frees the memory associated with an IMF.  That is it frees the
	valslist and the nests. Also frees the imp gvals.
	The rest of the imp must be freed seperatly.

	PIMF pimf = Pointer to IMF to free memory from.
******************************************************************************/
void IMFFree(PIMF pimf)
	{
	//	DELZ(pimf->pvalslist);
    delete [] pimf->pvalslist; pimf->pvalslist=NULL;
//	DELZ(pimf->pnests);
	//DELZ(pimf->imp.pgvals);
	DELZ(pimf->aesetup);
	delete [] (LPSTR)(pimf->aeinfo);
	pimf->aeinfo=NULL;
	}

/******************************************************************************
	hwnd = IMFOpenRun(parent,id)

	Opens the IMF Runtime window for RunIMF.

	HWND hwnd = Handle of new IMFRUN window or NULL on error.
	HWND parent = Handle of parent window.
	WORD id = Child id of new window.
******************************************************************************/
HWND IMFOpenRun(HWND parent,WORD id)
	{
	static BOOL registered=FALSE;
	WNDCLASS wc;
//	COLORREF cr;
//	long lcr[3]; // long color ref vector for GetPrivateProvileLongVect()

	if(!registered)
		{
		wc.style=CS_VREDRAW|CS_HREDRAW;
		wc.lpfnWndProc=IMFRUNWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)(COLOR_BTNSHADOW+1); 
		wc.lpszMenuName=NULL;
		wc.lpszClassName=IMFRunClass;
		if(!RegisterClass(&wc))
			return NULL;
		registered=TRUE;
		}

	// FixtureBrush from FixtureColor=
	FixtureBrush=CreateSolidBrush(GetPrivateProfileCOLORREF(
		IMFIniSection,"FixtureColor",RGB(192,192,192),IMSIni));

	// NoPartBrush from NoPartColor=
	NoPartBrush=CreateSolidBrush(GetPrivateProfileCOLORREF(
		IMFIniSection,"NoPartColor",RGB(128,128,128),IMSIni));

	// PartBrush from PartColor=
	COLORREF partcolor=GetPrivateProfileCOLORREF(
		IMFIniSection,"PartColor",RGB(128,128,255),IMSIni);
	PartBrush=CreateSolidBrush(partcolor);

	// StartedPartBrush from StartedPartColor=
	StartedPartBrush=CreateSolidBrush(GetPrivateProfileCOLORREF(
		IMFIniSection,"StartedPartColor",RGB(255,128,255),IMSIni));

	// MarkedPartBrush from MarkedPartColor=
	COLORREF markedcolor=GetPrivateProfileCOLORREF(
		IMFIniSection,"MarkedPartColor",RGB(255,255,128),IMSIni);
	MarkedPartBrush=CreateSolidBrush(markedcolor);

	// PassedPartBrush from PassedPartColor=
	COLORREF passedcolor=GetPrivateProfileCOLORREF(
		IMFIniSection,"PassedPartColor",RGB(128,255,128),IMSIni);
	PassedPartBrush=CreateSolidBrush(passedcolor);

	// FailedPartBrush from FailedPartColor=
	FailedPartBrush=CreateSolidBrush(GetPrivateProfileCOLORREF(
		IMFIniSection,"FailedPartColor",RGB(255,128,128),IMSIni));

	// Cured brushes are hatch versions of regular marked and passed
	CuredPartBrush=CreateHatchBrush(HS_DIAGCROSS,partcolor);
	CuredMarkedPartBrush=CreateHatchBrush(HS_DIAGCROSS,markedcolor);
	CuredPassedPartBrush=CreateHatchBrush(HS_DIAGCROSS,passedcolor);

	return CreateWindow(IMFRunClass,NULL,
		WS_CHILD|WS_VISIBLE|WS_BORDER,
		0,0,1,1,parent,(HMENU)id,IMSInst,NULL);
	}

/******************************************************************************
	hwnd = IMFOpenEdit(parent,id)

	Opens the IMF Edit window for EditIMF.

	HWND hwnd = Handle of new IMFEDIT window or NULL on error.
	HWND parent = Handle of parent window.
	WORD id = Child id of new window.
	int x,y,w,h = x,y coordinate and width (w) and heigth (h)
******************************************************************************/
HWND IMFOpenEdit(HWND parent,WORD id,int x,int y,int w,int h)
	{
	static BOOL registered=FALSE;
	WNDCLASS wc;
	LabelBrush=(HBRUSH)CreateSolidBrush(LabelColor);

	if(!registered)
		{
		wc.style=CS_VREDRAW|CS_HREDRAW;
		wc.lpfnWndProc=IMFEDITWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)(COLOR_BTNSHADOW+1); 
		wc.lpszMenuName=NULL;
		wc.lpszClassName=IMFEditClass;
		if(!RegisterClass(&wc))
			return NULL;
		registered=TRUE;
		}

	UNVS=HideOptRotation ? 4 : 5;
	return CreateWindow(IMFEditClass,NULL,
		WS_CHILD|WS_VISIBLE|WS_BORDER,
		x,y,w,h,parent,(HMENU)id,IMSInst,NULL);
	}

/******************************************************************************
	fsteps = IMFCompile(pimf)

	Compiles an IMF into an IMSTEP sequence.
	The resultant IMSTEP sequence includes is relative WCS Frame and
	includes all sub CS's
	The associated IMP is compiled and used as a subroutine in each NCS.

	PIMSTEPS fsteps = The compiled step seqence or NULL on error.
	PIMF pimf = The imf to compile.
******************************************************************************/
PIMSTEP IMFCompile(PIMF pimf)
	{
	PIMSTEP fsteps,psteps,pstep;
	M3 frame;
	int hij;
	int i;
	IMFNEST *pnest;
	IMPVALS *pvals;
	BOOL anymarksneeded;    // Some marking needed
	BOOL anyinspectsneeded; // Some inspection needed
	BOOL anycuringneeded; // Some curing needed
	BOOL domark;    // Per part mark flag
	BOOL doinspect; // Per part inspect w/visinterleave flag
	BOOL anymarks;   // does the program contain any marks
	int OptMT; // MT Optimization override if >=0
	int OptBD; // BD Optimization override if >=0
	int OptBDC; // BDC Optimization override if >=0
	int OptUDR; // UDR Optimization override if >=0
	int OptUD; // UD Optimization override if >=0
	int OptGR; // GR Optimization override if >=0
	double zmove;
	
	int OptMAC;  // ZStartOnly & ReturnToStart override if >=0

	// Determine if any nests need to be marked
	anymarks=IMPCountMarks(&(pimf->imp))>0;
	anymarksneeded=FALSE;
	if(anymarks)
		{
		i=pimf->nests.count;
		while(i--)
			if(MARKNEEDED(pimf->nests.nest[i].flags))
				{
				anymarksneeded=TRUE;
				break;
				}
		}

	// Determine if any nests need to be inspected
	anyinspectsneeded=FALSE;
	if(VisOn && anymarks)
		{
		i=pimf->nests.count;
		while(i--)
			if(INSPECTNEEDED(pimf->nests.nest[i].flags))
				{
				anyinspectsneeded=TRUE;
				break;
				}
		}

	// Determine what curing if any must be done
	anycuringneeded=FALSE;
	if(IMPCountCureSteps(&(pimf->imp))>0)
		{
		i=pimf->nests.count;
		while(i--)
			if(CURENEEDED(pimf->nests.nest[i].flags))
				{
				anycuringneeded=TRUE;
				break;
				}
		}

	// return now if nothing to do
	if(!(anymarksneeded||anyinspectsneeded||anycuringneeded))
		{
		fsteps=NULL;
		goto EXIT;
		}

	// Find HIJ for the inkjet if it exists.
	hij=(int)SendMessage(IMWnd,IMM_FINDIJ,0,
		(LONG)(LPSTR)&pimf->imp.pheader->inkjet);
	if(hij==HIJERROR) // If IJ not found
		{
		ShowError(IDE_FINDIJ);
		fsteps=NULL;
		goto EXIT;
		}
	if(HideOptRotaryIMF)
	{
	// Compute zsafe and zmarker and zvis
	zsafe=pimf->imp.pheader->partSize.z+pimf->extent.z;
	// set returntostart here
	ZStartOnly=(BOOL)GetPrivateProfileInt(IMSName,"ZStartOnly",0,IMSIni);
	ReturnToStart=(BOOL)GetPrivateProfileInt(IMSName,"ReturnToStart",0,IMSIni);
	if(ZStartOnly)
		ReturnToStart=FALSE;

	OptMAC=IMPGetUDHAInt(&(pimf->imp),"MoveAfterCycle",-1);
	switch(OptMAC)
		{
		case 0:  // Move To Start of 1st Mark
			ZStartOnly=FALSE;
			ReturnToStart=TRUE;
			break;
		case 1:  // Move to ini Defined StartPoint
			ZStartOnly=FALSE;
			ReturnToStart=FALSE;
			break;
		case 2:  // Move Z only to ini Defined StartPoint
			ZStartOnly=TRUE;
			ReturnToStart=FALSE;
			break;
		}

	if(ZStartOnly||!ReturnToStart)
		zsafe+=0.25;
	zsafe=max(zsafe,StartPoint.z);
	zmarker=pimf->extent.z+pimf->imp.pheader->partSize.z;
	zvis=pimf->extent.z+pimf->imp.pheader->partSize.z+VisPos.z;
	//zzzzz
	if(DebugMode)
		{
		sprintf(dbuf,"IMFCompile() zsafe=%0.4f zmarker=%0.4f zvis=%0.4f zlast=%0.4f\n    ZStartOnly=%s ReturnToStart=%s\n",zsafe,zmarker,zvis,zlast,
			(LPCSTR)(ZStartOnly?"Yes":"No"),(LPCSTR)(ReturnToStart?"Yes":"No"));
		OutputDebugString(dbuf);
		}
	}

	else
	{
		// Compute zsafe and zmarker and zvis
		zsafe=pimf->extentrot.z;
	// set returntostart here
	ZStartOnly=(BOOL)GetPrivateProfileInt(IMSName,"ZStartOnly",0,IMSIni);
	ReturnToStart=(BOOL)GetPrivateProfileInt(IMSName,"ReturnToStart",0,IMSIni);
	if(ZStartOnly)
		ReturnToStart=FALSE;

	OptMAC=IMPGetUDHAInt(&(pimf->imp),"MoveAfterCycle",-1);
	switch(OptMAC)
		{
		case 0:  // Move To Start of 1st Mark
			ZStartOnly=FALSE;
			ReturnToStart=TRUE;
			break;
		case 1:  // Move to ini Defined StartPoint
			ZStartOnly=FALSE;
			ReturnToStart=FALSE;
			break;
		case 2:  // Move Z only to ini Defined StartPoint
			ZStartOnly=TRUE;
			ReturnToStart=FALSE;
			break;
		}

	if(ZStartOnly||!ReturnToStart)
		zsafe+=0.25;
	zsafe=max(zsafe,rotstartpoint.hi);
	zmarker=pimf->extentrot.z;
	zvis=pimf->extentrot.z+pimf->imp.pheader->partDiameter.hi+VisPos.z;
	//zzzzz
	if(DebugMode)
		{
		sprintf(dbuf,"IMFCompile() zsafe=%0.4f zmarker=%0.4f zvis=%0.4f zlast=%0.4f\n    ZStartOnly=%s ReturnToStart=%s\n",zsafe,zmarker,zvis,zlast,
			(LPCSTR)(ZStartOnly?"Yes":"No"),(LPCSTR)(ReturnToStart?"Yes":"No"));
		OutputDebugString(dbuf);
		}

	}
	// COMPILE FSTEPS
	// Select inkjet
	fsteps=IMSELIJSTEPNew(hij);

	// Select inspector if inspecting
	if(anyinspectsneeded)
		IMSTEPAppend(fsteps,
			IMSELINSPECTORSTEPNew(pimf->imp.pheader->inspector));

	// Select the IM XY MODE
    IMSTEPAppend(fsteps,IMXYMODESTEPNew(pimf->xymode));
	//sam--testing
	//IMSTEPAppend(fsteps,IMWMOVESTEPNew(pimf->xymode));

	if(anymarksneeded || anyinspectsneeded || anycuringneeded)
		{
		IMSTEPAppend(fsteps,IMMSGSTEPNew(anymarksneeded ? 
			"MARKING" : "INSPECTING"));
		// Move Z to start position, z first
		zmove=max(zsafe,zlast);
		if(DebugMode)
			{
			sprintf(dbuf,"IMFCompile() first Z move to %0.4f\n",zmove);
			OutputDebugString(dbuf);
			}
		if(ZEXISTS)
			IMSTEPAppend(fsteps,IMZMOVESTEPNew(zmove,xymcsTCS));
		}
	// Push aligned FCS System
	if(!GetFCS(&frame,pimf,TRUE))
		{
		IMSTEPFree(fsteps,TRUE);
		fsteps=NULL;
		goto EXIT;
		}
	IMSTEPAppend(fsteps,IMPUSHSTEPNew(&frame));

	// Do each part
	for(i=0;i<pimf->nests.count;i++)
		{
		if(!anymarks)
			continue;
		pnest=&(pimf->nests.nest[i]);
		//check for any location varification failures
		if ((pnest->flags & nbfLOCFAIL1)||(pnest->flags & nbfLOCFAIL2)){
			pnest->flags &= (~nbfOCCUPIED);			// stop part from being used
			continue;
			}

		domark=MARKNEEDED(pnest->flags);
		doinspect=VisOn && VisInterleave && INSPECTNEEDED(pnest->flags);

		if(!(domark||doinspect))
			continue;

		if(domark)
			IMSTEPAppend(fsteps,IMSTARTSTEPNew(i)); // Mark start of part

		// Get and push aligned NCS
		if(!GetNCS(&frame,pimf,pnest,i,TRUE))
			{
			IMSTEPFree(fsteps,TRUE);
			fsteps=NULL;
			goto EXIT;
			}
		IMSTEPAppend(fsteps,IMPUSHSTEPNew(&frame));
		// zzz - New format
		pvals=&(pimf->pvalslist[i]);
		if(domark)
			{
			psteps=IMPCompileMarks(&pimf->imp,pvals,pimf->xymode);
			if(!psteps)
				{
				IMSTEPFree(fsteps,TRUE);
				fsteps=NULL;
				goto EXIT;
				}
			IMSTEPAppend(fsteps,IMSUBSTEPNew(psteps));
			psteps=NULL;
			}
		if(doinspect) // Inspect  (VisInterleaved)
			{
			IMSTEPAppend(fsteps,IMMSGSTEPNew("INSPECTING"));
			// PREPINSPECTOR
			pstep=CompileInspectPrep(&pimf->imp,pvals);
			if(!pstep)
				{
				ShowError(IDE_BADINSPECTPREP);
				IMSTEPFree(fsteps,TRUE);
				fsteps=NULL;
				goto EXIT;
				}
			IMSTEPAppend(fsteps,pstep);
			// MOVE to VisPos.xy,zvis
			IMSTEPAppend(fsteps,
				IMXYMOVESTEPNew((LPP2)&VisPos,zvis,xymcsINSPECTOR));
			// INSPECT
			IMSTEPAppend(fsteps,IMINSPECTSTEPNew(i));
			IMSTEPAppend(fsteps,IMMSGSTEPNew("MARKING"));
			}
		IMSTEPAppend(fsteps,IMPOPSTEPNew()); // NCS
		if(domark)
			IMSTEPAppend(fsteps,IMENDSTEPNew(i));
		}
	  

	// VIS Inspection (!VisInterleaved)
	if(anyinspectsneeded && !VisInterleave)
		{
		IMSTEPAppend(fsteps,IMMSGSTEPNew("INSPECTING"));
		// Do each part
		for(i=0;i<pimf->nests.count;i++)
			{
			pnest=&(pimf->nests.nest[i]);
			// zzz - New format
			pvals=&(pimf->pvalslist[i]);
			if(INSPECTNEEDED(pnest->flags))
				{
				// PREPINSPECTOR
				pstep=CompileInspectPrep(&pimf->imp,pvals);
				if(!pstep)
					{
					ShowError(IDE_BADINSPECTPREP);
					IMSTEPFree(fsteps,TRUE);
					fsteps=NULL;
					goto EXIT;
					}
				IMSTEPAppend(fsteps,pstep);
				// Get and push aligned NCS
				if(!GetNCS(&frame,pimf,pnest,i,TRUE))
					{
					IMSTEPFree(fsteps,TRUE);
					fsteps=NULL;
					goto EXIT;
					}
				IMSTEPAppend(fsteps,IMPUSHSTEPNew(&frame));
				// MOVE to VisPos.xy,zvis
				IMSTEPAppend(fsteps,
					IMXYMOVESTEPNew((LPP2)&VisPos,zvis,xymcsINSPECTOR));
				// INSPECT
				IMSTEPAppend(fsteps,IMINSPECTSTEPNew(i));

				IMSTEPAppend(fsteps,IMPOPSTEPNew()); // NCS
				}
			}
		}

	if(anycuringneeded)
		{
		// Do each part
		for(i=0;i<pimf->nests.count;i++)
			{
			pnest=&(pimf->nests.nest[i]);
			if(!CURENEEDED(pnest->flags))
				continue;
			// Get and push aligned NCS
			if(!GetNCS(&frame,pimf,pnest,i,TRUE))
				{
				IMSTEPFree(fsteps,TRUE);
				fsteps=NULL;
				goto EXIT;
				}
			IMSTEPAppend(fsteps,IMPUSHSTEPNew(&frame));
			psteps=IMPCompileCureSteps(&pimf->imp,pimf->xymode,i);
			if(!psteps)
				{
				IMSTEPFree(fsteps,TRUE);
				fsteps=NULL;
				goto EXIT;
				}
			IMSTEPAppend(fsteps,psteps);
			psteps=NULL;

			IMSTEPAppend(fsteps,IMPOPSTEPNew()); // NCS
			}
		IMSTEPAppend(fsteps,IMCUREOFFSTEPNew()); // Make sure cure is off
		}

	IMSTEPAppend(fsteps,IMPDONESTEPNew());

	if(IMPGetUDHAInt(&(pimf->imp),"HomeAfterCycle",0))
		{
		zmove=max(zsafe,zlast);
		zmove=max(zmove,RehomePoint.z);
		if(DebugMode)
			{
			sprintf(dbuf,"IMFCompile() rehome zmove=%0.4f zlast=%0.4f RehomePoint.z=%0.4f\n",zmove,zlast,RehomePoint.z);
			OutputDebugString(dbuf);
			}
		if(ZEXISTS) // Move Z to safe position
			IMSTEPAppend(fsteps,IMZMOVESTEPNew(zmove,xymcsTCS));
		if(HideOptRotaryIMF)
		{
		IMSTEPAppend(fsteps,
		IMXYMOVESTEPNew((LPP2)&RehomePoint,zmove,xymcsTCS));
		//	IMSTEPAppend(fsteps,IMWMOVESTEPNew((LPP1)&RehomePoint,xymcsTCS));
		
		}
		else
		{
		IMSTEPAppend(fsteps,
			IMXYMOVESTEPNew((LPP2)&RehomePoint,zmove,xymcsTCS));
		
		}
		IMSTEPAppend(fsteps,IMRTSSTEPNew(TRUE));

		}
	else
		{
		zmove=max(zsafe,zlast);
		if(DebugMode)
			{
			sprintf(dbuf,"IMFCompile() after marks zmove=%0.4f zlast=%0.4f\n",zmove,zlast);
			OutputDebugString(dbuf);
			}
		if(ZEXISTS) // Move Z to safe position
			IMSTEPAppend(fsteps,IMZMOVESTEPNew(zmove,xymcsTCS));
		// Move to start position while head is up

		if(!ZStartOnly)
			{
			if(ReturnToStart)
				IMSTEPAppend(fsteps,IMRTSSTEPNew(FALSE));
			else
				IMSTEPAppend(fsteps,
					IMXYMOVESTEPNew((LPP2)&StartPoint,zmove,xymcsTCS));
			}
		}

	// if RF , Move it back to 0
	if(pimf->xymode==imXS||pimf->xymode==imSY)
		IMSTEPAppend(fsteps,IMXYMOVESTEPNew(NULL,zsafe,xymcsRF));

	// Pop FCS System
	IMSTEPAppend(fsteps,IMPOPSTEPNew());

	// Restore the default IM XY MODE
	IMSTEPAppend(fsteps,IMXYMODESTEPNew(imXY));

EXIT:
	// Clear the loaded bitmaps cache
	LoadedBitmapsClear(); // Clear loaded bitmaps cache

	OptMT=IMPGetUDHAInt(&(pimf->imp),"OptMT",-1);
	OptBD=IMPGetUDHAInt(&(pimf->imp),"OptBD",-1);
	OptBDC=IMPGetUDHAInt(&(pimf->imp),"OptBDC",-1);
	if(ReturnToStart) // RTS overrides BDC always
		OptBDC=FALSE;
	OptUDR=IMPGetUDHAInt(&(pimf->imp),"OptUDR",-1);
	OptUD=IMPGetUDHAInt(&(pimf->imp),"OptUD",-1);
	OptGR=IMPGetUDHAInt(&(pimf->imp),"OptGR",-1);
	return IMOptimizeSteps(fsteps,OptMT,OptBD,OptBDC,OptUDR,OptUD,OptGR);
	}

/******************************************************************************
	steps = IMFCompileConveyor()

	Compiles the conveyor eject load sequence.

	PIMSTEPS steps = The compiled step seqence or NULL on error.
******************************************************************************/
PIMSTEP IMFCompileConveyor()
	{
	PIMSTEP steps;
	steps=NULL;
switch (ConvType)
		{
		case 3: {
				if(IMReadInBit(bitConvPIP2))	// Part allready in position 2
					{	// Move to position 3
					steps=IMMSGSTEPNew("EJECTING 3");// Send Status ....
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP3,0,0,0,0,NULL,NULL));	// Wait till PIP3 is clear
					IMSTEPAppend(steps,IMMSGSTEPNew("EJECTING 2"));// Send Status ....
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// Wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC2,TRUE));					// Set PC2 to allow the PC3 request(Event routine will set PC3)
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP2,0,0,0,0,NULL,NULL));	// Wait till PIP2 is clear
					IMSTEPAppend(steps,IMMSGSTEPNew("LOADING 2"));										// Send Status ....
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,TRUE));					// Just making sure....
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP1,1,0,0,0,NULL,NULL));	// Wait till PIP1 is set (Event routine will clear PC1...)
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// Wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC2,TRUE));					// Make the PC2 request....
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,TRUE));					// Just making sure....
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP2,1,0,0,0,NULL,NULL));	// Wait till PIP2 is set (Event routine will clear PC2...)
					}
				else{							// if not in Position 2
					steps=IMMSGSTEPNew("LOADING 2");										// Send Status ....
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,TRUE));					// Just making sure....
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP1,1,0,0,0,NULL,NULL));	// Wait till PIP1 is set (Event routine will clear PC1...)
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC2,TRUE));					// Make the PC2 request....
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,TRUE));					// Set PC1 to allow the PC2 request....
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP2,1,0,0,0,NULL,NULL));	// Wait till PIP2 is set (Event routine will clear PC2...)
					}																		// Note... PC2 will be set after making complet via PC2Clamp
				break;
				}
		case 2:	{
				steps=NULL;
				if(IMReadInBit(bitConvPIP1)) // Part allready in position 1
					{ // Eject it
					steps=IMMSGSTEPNew("EJECTING");
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC2,TRUE));					// Make the PC2 request....
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,FALSE));
					IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,TRUE));
					IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP1,0,0,0,0,NULL,NULL));
					}

				PIMSTEP swaiting=IMMSGSTEPNew("WAITING");
				if(steps)
					IMSTEPAppend(steps,swaiting);
				else
					steps=swaiting;

				IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,FALSE));
				IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
				IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC1,TRUE));
				IMSTEPAppend(steps,IMWAITSTEPNew(ConvDelay));									// wait...
				IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP1,1,1,0,0,NULL,NULL));
				}
				break;
		case 1:{ // old style conveyor
				steps=IMMSGSTEPNew("WAITING");
				if(bitConvPS)
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPS,!LinkFail));
				IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC,TRUE));
				IMSTEPAppend(steps,IMWAITBITSTEPNew(bitConvPIP,1,1,0,0,NULL,NULL));
				IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPC,FALSE));
				if(bitConvPS)
					IMSTEPAppend(steps,IMOUTBITSTEPNew(bitConvPS,FALSE));
				}
			break;
		default:
				steps=IMMSGSTEPNew("NO CONV");
			break;
		}
	return steps;
	}

/******************************************************************************
	steps = IMFCompileReads(pimf)

	Compiles the read sequence for the IMF.

	PIMSTEPS steps = The compiled step seqence or NULL on error.
	PIMF pimf = The imf to compile reads for.
******************************************************************************/
PIMSTEP IMFCompileReads(PIMF pimf)
	{
	PIMSTEP steps,pstep;
	M3 frame;
	int ni; // nest index
	int vi; // var index
	IMFNEST *pnest;
	IMPVALS *pvals;
	
	// lookup the readvar to get vi
	vi=pimf->imp.vars.count;
	while(vi--)
		{
		if(pimf->imp.vars.var[vi].type!=vtKEYED)
			continue;
		if(!lstrcmp(pimf->imp.vars.var[vi].name,pimf->imp.pheader->readvar))
			break;
		}

	// Return if no parts require processing
	ni=pimf->nests.count;
	while(ni--)
		if(OCCUPIED(pimf->nests.nest[ni].flags))
			break;
	if(ni<0 || vi<0)
		{
		steps=NULL;
		goto EXIT;
		}
	if(HideOptRotaryIMF)
	{
	// Compute zsafe and zvis
	zsafe=pimf->imp.pheader->partSize.z+pimf->extent.z+0.25;
	zsafe=max(zsafe,StartPoint.z);
	//zzzzz
	if(DebugMode)
		{
		sprintf(dbuf,"IMFCompileReads() zsafe=%0.4f\n",zsafe);
		OutputDebugString(dbuf);
		}
	zvis=pimf->extent.z+
		pimf->imp.pheader->partSize.z+
		pimf->imp.pheader->readpos.z;
	}
	else
	{
	// Compute zsafe and zvis
		zsafe=pimf->imp.pheader->partDiameter.hi+pimf->extentrot.z+0.25;
	zsafe=max(zsafe,rotstartpoint.hi);
	//zzzzz
	if(DebugMode)
		{
		sprintf(dbuf,"IMFCompileReads() zsafe=%0.4f\n",zsafe);
		OutputDebugString(dbuf);
		}
		zvis=pimf->extentrot.z+
			pimf->imp.pheader->partDiameter.hi+
			pimf->imp.pheader->rotreadpos.z;
	}

	// Select reader
	steps=IMSELREADERSTEPNew(pimf->imp.pheader->reader);
	IMSTEPAppend(steps,IMMSGSTEPNew("READING"));
	if(ZEXISTS) // Move Z to start position, z first
		IMSTEPAppend(steps,IMZMOVESTEPNew(zsafe,xymcsTCS));
	if(!GetFCS(&frame,pimf,TRUE))
		{
		IMSTEPFree(steps,TRUE);
		steps=NULL;
		goto EXIT;
		}
	IMSTEPAppend(steps,IMPUSHSTEPNew(&frame));
	// Do each part
	for(ni=0;ni<pimf->nests.count;ni++)
		{
		// zzz - New format
		pnest=&(pimf->nests.nest[ni]);
		pvals=&(pimf->pvalslist[ni]);
		if(!OCCUPIED(pnest->flags))
			continue;
		// PREPREADER
		pstep=CompileReadPrep(&pimf->imp,pvals);
		if(!pstep)
			{
			ShowError(IDE_BADREADPREP);
			IMSTEPFree(steps,TRUE);
			steps=NULL;
			goto EXIT;
			}
		IMSTEPAppend(steps,pstep);
		// Get and push aligned NCS
		if(!GetNCS(&frame,pimf,pnest,ni,TRUE))
			{
			IMSTEPFree(steps,TRUE);
			steps=NULL;
			goto EXIT;
			}
		IMSTEPAppend(steps,IMPUSHSTEPNew(&frame));
		// MOVE to readpos.xy,zvis
		IMSTEPAppend(steps,
			IMXYMOVESTEPNew((LPP2)&pimf->imp.pheader->readpos,zvis,xymcsREADER));
		IMSTEPAppend(steps,IMREADSTEPNew(ni,vi)); // READ
		IMSTEPAppend(steps,IMPOPSTEPNew()); // POP NCS
		}
	if(ZEXISTS) // Move Z to safe position
		IMSTEPAppend(steps,IMZMOVESTEPNew(zsafe,xymcsTCS));
	// Pop FCS System
	IMSTEPAppend(steps,IMPOPSTEPNew());
EXIT:
	return IMOptimizeSteps(steps,0,0,0,0,0,0);
	}

/******************************************************************************
	steps = IMFCompileLocates(pimf)

	Compiles the locate sequence for the IMF at the IMF or NEST level.

	PIMSTEPS steps = The compiled step seqence or NULL on error.
	PIMF pimf = The imf to compile locates for.
******************************************************************************/
PIMSTEP IMFCompileLocates(PIMF pimf)
	{
	PIMSTEP steps,pstep;
	M3 frame;
	int ni; // nest index
	IMFNEST *pnest;
	IMPVALS *pvals;

	BOOL perimf=(pimf->imp.aesetup==NULL); // perimf or !perimf (per nest)
	AESETUP *aes=perimf ? pimf->aesetup : pimf->imp.aesetup;
	if(!aes) // Error if no AE extension in IMF or IMP
		return NULL;

	if(!pimf->aeinfo) // Allocate AEINFO if needed
		{
		int nnests=perimf ? 0 : pimf->nests.count;
		int npos=perimf ? aes->nrefpos : nnests*aes->nrefpos;
		pimf->aeinfo=(AEINFO *)new BYTE[sizeof(AEINFO)+npos*sizeof(P3)];
		pimf->aeinfo->nnests=nnests;
		}

	// Initialize all found locations to ref locations

	int npos=perimf ? 1 : pimf->nests.count;
	for(int pi=0;pi<npos;pi++)
		for(int ri=0;ri<aes->nrefpos;ri++)
			pimf->aeinfo->pos[pi*aes->nrefpos+ri]=aes->refpos[ri];

	// Return if no parts require processing
	ni=pimf->nests.count;
	while(ni--)
		if(OCCUPIED(pimf->nests.nest[ni].flags))
			break;
	if(ni<0)
		{
		steps=NULL;
		goto EXIT;
		}
	// Compute zsafe
	zsafe=0.25f;
	if(pimf)
		{
		zsafe+=pimf->extent.z;
		if(pimf->imp.pheader)
			zsafe+=pimf->imp.pheader->partSize.z;
		}
	zsafe=max(zsafe,StartPoint.z);
	//zzzzz
	if(DebugMode)
		{
		sprintf(dbuf,"IMFCompileLocates() zsafe=%0.4f\n",zsafe);
		OutputDebugString(dbuf);
		}

	// Select locator
	steps=IMSELLOCATORSTEPNew(aes->locator[0]);
	IMSTEPAppend(steps,IMMSGSTEPNew("LOCATING"));
	if(ZEXISTS) // Move Z to start position, z first
		IMSTEPAppend(steps,IMZMOVESTEPNew(zsafe,xymcsTCS));
	// Push model (non aligned) FCS System
	if(!GetFCS(&frame,pimf,FALSE))
		{
		IMSTEPFree(steps,TRUE);
		steps=NULL;
		goto EXIT;
		}
	IMSTEPAppend(steps,IMPUSHSTEPNew(&frame));
	if(perimf) // If fixture level alignement
		{
		pstep=aes->compile(pimf);
		if(!pstep)
			{
			IMSTEPFree(steps,TRUE);
			steps=NULL;
			goto EXIT;
			}
		IMSTEPAppend(steps,pstep);
		}
	else // per nest alignement
		{
		for(ni=0;ni<pimf->nests.count;ni++)
			{
			pnest=&(pimf->nests.nest[ni]);
			// zzz - New format
			pvals=&(pimf->pvalslist[ni]);
			if(!OCCUPIED(pnest->flags))
				continue;
			if(!VERIFYNEEDED(pnest->flags))								// If pre-verified then skip process. d.dixon.....1/3/14
				continue;
			// Get and push unaligned NCS
			if(!GetNCS(&frame,pimf,pnest,ni,FALSE))
				{
				IMSTEPFree(steps,TRUE);
				steps=NULL;
				goto EXIT;
				}
			IMSTEPAppend(steps,IMPUSHSTEPNew(&frame));
			pstep=aes->compile(pimf,ni,pvals);
			if(!pstep)
				{
				IMSTEPFree(steps,TRUE);
				steps=NULL;
				goto EXIT;
				}
			IMSTEPAppend(steps,pstep);
			IMSTEPAppend(steps,IMPOPSTEPNew()); // POP NCS
			}
		}
	if(ZEXISTS) // Move Z to safe position
		IMSTEPAppend(steps,IMZMOVESTEPNew(zsafe,xymcsTCS));
	// Pop FCS System
	IMSTEPAppend(steps,IMPOPSTEPNew());
EXIT:
	return IMOptimizeSteps(steps,0,0,0,0,0,0);
	}

/******************************************************************************
	fsteps = IMFCompileVPosMove(pimf,nesti,vismode)

	Compiles an IMF into an IMSTEP to move to the VisPos to operate
	on the part at the given nest index or the fixture if nesti==-1.
	 or the stage if pimf==0
	Model (non aligned) coordinates are used.

	PIMSTEPS fsteps = The compiled step seqence or NULL on error.
	PIMF pimf = The imf to compile.
	WORD nesti= nest index | -1 for FCS
	VISMODE vismode = What vis type is being moved?
******************************************************************************/
PIMSTEP IMFCompileVPosMove(PIMF pimf,int nesti,VISMODE vismode)
	{
	PIMSTEP steps;
	M3 frame;
	IMFNEST *pnest;
	
	steps=IMMSGSTEPNew("MOVEING");

	if(pimf)
		{
		// Push model (non aligned) FCS System
		if(!GetFCS(&frame,pimf,FALSE))
			return NULL;
		IMSTEPAppend(steps,IMPUSHSTEPNew(&frame));

		if(nesti>=0)
			{
			// Get and push unaligned NCS
			pnest=&(pimf->nests.nest[nesti]);
			if(!GetNCS(&frame,pimf,pnest,nesti,FALSE))
				{
				IMSTEPFree(steps,TRUE);
				return NULL;
				}
			IMSTEPAppend(steps,IMPUSHSTEPNew(&frame));
			}
		}

	XYMCS xymcs;	
	switch(vismode)
		{
		case vmINSPECTOR: xymcs=xymcsINSPECTOR; break;
		case vmREADER:    xymcs=xymcsREADER;    break;
		case vmLOCATOR:   xymcs=xymcsLOCATOR;   break;
		default:
			IMSTEPFree(steps,TRUE);
			return NULL;
		}
	// MOVE to VisPos.xy,zvis
	IMSTEPAppend(steps,IMXYMOVESTEPNew((LPP2)&VisPos,zvis,xymcs));
	if(pimf)
		{
		if(nesti>=0)
			IMSTEPAppend(steps,IMPOPSTEPNew()); // Pop NCS frame
		IMSTEPAppend(steps,IMPOPSTEPNew()); // Pop FCS frame
		}
	return steps;
	}

/******************************************************************************
	ldesc = IMFLoadDesc(fname,fdesc)

	Extracts the file description from an IMF file.

	int ldesc = length of desc or < 0 on error.
	char *fname = name of file to get desc from.
	char *fdesc = place to store file description if found.
******************************************************************************/
int IMFLoadDesc(char *fname,char *fdesc)
	{
	HANDLE fh;
//	OFSTRUCT ofs;
//	char * p;

	*fdesc=0;
	fh=CreateFile(fname,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE)
		return -1;
	_lread32(fh,tbuf,MAXSIGNITURE);

    // New case needed

	if(!memcmp(tbuf,sigIMF,LSIGIMF)) // Native
	{
		char dbuf[IMFLDESC+1];
		dbuf[0]=0;
		_llseek32(fh,LSIGIMF+40,0);   
		if(_lread32(fh,(LPSTR)&dbuf,sizeof(dbuf))==sizeof(dbuf)){
			if (dbuf[0]!=0){
				lstrcpy(fdesc,dbuf);
			}
		}
	} 
	else if(!memcmp(tbuf,sigIMFV357,LSIGIMFV357)
		||!memcmp(tbuf,sigIMFV345,LSIGIMFV345)) // 357 or 345 - 16 bit versions
	{
		char dbuf[IMFLDESC+1];
		dbuf[0]=0;
		_llseek32(fh,LSIGIMF+20,0);   
		if(_lread32(fh,(LPSTR)&dbuf,sizeof(dbuf))==sizeof(dbuf)){
			if (dbuf[0]!=0){
				lstrcpy(fdesc,dbuf);
			}
		}
	}
	else if(!memcmp(tbuf,sigIMFV338,LSIGIMFV338)) // 338 (16 bit)
	{
		_llseek32(fh,LSIGIMFV338,0);
		XLoadDescIMFV338(fh,fdesc);
	}
	CloseHandle(fh);			
	return lstrlen(fdesc);
	}

/******************************************************************************
	match = IMFSigMatch(sig)

	Compares a signiture against the set of IMF signitures.

	BOOL match = TRUE if sig is an IMF signiture.
	LPSTR sig = signiture to match.
******************************************************************************/
BOOL IMFSigMatch(LPSTR sig)
	{
	return (!memcmp(sig,sigIMF,LSIGIMF))||
		   (!memcmp(sig,sigIMFV357,LSIGIMFV357))||
	       (!memcmp(sig,sigIMFV345,LSIGIMFV345))||
	       (!memcmp(sig,sigIMFV338,LSIGIMFV338))||
	       (!memcmp(sig,sigIMFV330,LSIGIMFV330))||
	       (!memcmp(sig,sigIMF4000,LSIGIMF4000));
	}

// Callbacks for Conveyor 
void CALLBACK ThruConvTIMEOUT(HWND hwnd,UINT msg,UINT wP,DWORD lP)
	{
	Sleep(ConvDelay);
	IMSetOutBit(bitThruConvPC,TRUE);
	if(bitConvOutGate)
		IMSetOutBit(bitConvOutGate,TRUE);
	KillTimer(NULL,convtimer); // Kill the timer
	convtimer=0;
	}

// Completes the reset cycle of the init of the simplimatic conveyor
void CALLBACK ConvResetTIMEOUT(HWND hwnd,UINT msg,UINT wP,DWORD lP)
	{
	Sleep(ConvDelay);
	IMSetOutBit(bitConvReset,FALSE);
	KillTimer(NULL,convtimer); // Kill the timer
	convtimer=0;
	}

void ThruConvPIP(WORD event) // Bit monitor callback
	{
	if(event)
		{
		Sleep(ConvDelay);
		IMSetOutBit(bitThruConvPC,FALSE);
		if(bitConvOutGate)
			IMSetOutBit(bitConvOutGate,FALSE);
		convtimer=SetTimer(NULL,0,1000,ThruConvTIMEOUT);
		}
	else
		{
		Sleep(ConvDelay);
		IMSetOutBit(bitThruConvPC,TRUE);
		if(bitConvOutGate)
			IMSetOutBit(bitConvOutGate,TRUE);
		}
	}
void ThruConvPIP1(WORD event) // Bit monitor callback
	{
	Sleep(ConvDelay);
	if(event)
		{
		IMSetOutBit(bitThruConvPC1,FALSE);
		if(bitConvOutGate)
			IMSetOutBit(bitConvOutGate,FALSE);
		convtimer=SetTimer(NULL,0,1000,ThruConvTIMEOUT);
		}
	else
		{
		IMSetOutBit(bitThruConvPC1,TRUE);
		if(bitConvOutGate)
			IMSetOutBit(bitConvOutGate,TRUE);
		}
	}

void OnConvPIP1(WORD event) // Bit monitor callback
	{
	if(event) // ConvPIP1 0->1
		{
		Sleep(ConvDelay);
		IMSetOutBit(bitConvPC1,TRUE);
		Sleep(ConvDelay);
		IMSetOutBit(bitConvPC1,FALSE);
		}
	else      // ConvPIP1 1->0
		{
		Sleep(ConvDelay);
		IMSetOutBit(bitConvPC1,FALSE);
		Sleep(ConvDelay);
		IMSetOutBit(bitConvPC1,TRUE);
		}
	}
// ConvSimplimatic PIP2 event handler
void OnConvPIP2(WORD event) // Bit monitor callback
	{
	switch (ConvType)
		{
		case 1:
		case 2:	if(event) // ConvPIP2 0->1
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,FALSE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,TRUE);
					}
				else      // ConvPIP2 1->0
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,TRUE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,FALSE);
					}
				break;
		case 3:	if(event) // ConvPIP2 0->1
					{
					IMSetOutBit(bitConvPC2,TRUE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,FALSE);
					}
				else    // ConvPIP2 1->0
					{
					IMSetOutBit(bitConvPC2,TRUE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,FALSE);	// Let the marking routine will request PC2
					}

		default: break;
		}
	}
void OnConvPIP3(WORD event) // Bit monitor callback
	{
	if(event) // ConvPIP2 0->1
		{
		IMSetOutBit(bitConvPC3,TRUE);
		Sleep(ConvDelay);
		IMSetOutBit(bitConvPC3,FALSE);
		}
	else      // ConvPIP2 1->0
		{
		IMSetOutBit(bitConvPC3,FALSE);
		Sleep(ConvDelay);
		IMSetOutBit(bitConvPC3,TRUE);
		}
	}
// ConvSimplimatic ConvReady event handler
void  OnConvReady(WORD event) // Bit monitor callback
	{
	switch (ConvType)
		{
		case 1:	if(event) // ConvReady 0->1
					{
					IMSetOutBit(bitConvPC,TRUE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC,FALSE);
					}
				else      // ConvReady 1->0
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC,FALSE);
					}
				break;
		case 2:	if(event) // ConvReady 0->1
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,TRUE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,FALSE);
					}
				else      // ConvReady 1->0
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC1,FALSE);
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC2,FALSE);
					}
				break;
		case 3:	if(event) // ConvReady 0->1
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC1,TRUE);
					//IMSetOutBit(bitConvPC2,TRUE);
					IMSetOutBit(bitConvPC3,TRUE);
					}
				else      // ConvReady 1->0
					{
					Sleep(ConvDelay);
					IMSetOutBit(bitConvPC1,FALSE);
					IMSetOutBit(bitConvPC2,FALSE);
					IMSetOutBit(bitConvPC3,FALSE);
					}
				break;
		default :	break;
		}
	}


/******************************************************************************
	ConveyorInit()

	Called during app init to initialize the optional conveyor support.
******************************************************************************/
void ConveyorInit(void)
	{
	ConvOpen=FALSE;
	ConvSimplimatic=FALSE;

	ConvType=IMMapType();	// Get IO Conv type 1,2,3,4,etc...
	ConvDelay=IMMapDelay();	// in Milliseconds

	switch (ConvType)
		{
		case 1:{ // Old style PIP PC conveyor
				// Map conveyor in bits
				if(!(bitConvPIP=IMMapInBit(sConvPIP)))
					return;
				// Map conveyor out bits
				if(!(bitConvPC=IMMapOutBit(sConvPC)))
					return;
				bitConvPS=IMMapOutBit(sConvPS); // ok if not defined!
				bitThruConvPIP=IMMapInBit(sThruConvPIP);
				bitThruConvPC=IMMapOutBit(sThruConvPC);
				bitConvOutGate=IMMapOutBit(sConvOutGate);
				if(bitThruConvPIP && bitThruConvPC)
					{
					IMMonitorInBit(bitThruConvPIP,IBM01,1,ThruConvPIP);
					IMMonitorInBit(bitThruConvPIP,IBM10,0,ThruConvPIP);
					IMSetOutBit(bitThruConvPC,TRUE);
					if(bitConvOutGate)
						IMSetOutBit(bitConvOutGate,TRUE);
					}
				}
			   break;
		case 2: //if(bitConvRun=IMMapOutBit(sConvRun)) // Unused - ddd 10/03/14
					{
					ConvSimplimatic=TRUE;
					// Map out bits
					if(!(bitConvReset=IMMapOutBit(sConvReset)))
						return;
					//if(!(bitConvPR1=IMMapOutBit(sConvPR1)))	// Unused - ddd 10/03/14
					//	return;
					if(!(bitConvPC1=IMMapOutBit(sConvPC1)))
						return;
					//if(!(bitConvPR2=IMMapOutBit(sConvPR2)))	// Unused - ddd 10/03/14
					//	return;
					if(!(bitConvPC2=IMMapOutBit(sConvPC2)))
						return;
					// Map in bits
					if(!(bitConvReady=IMMapInBit(sConvReady)))
						return;
					//if(!(bitConvError=IMMapInBit(sConvError)))	// Unused - ddd 10/03/14
					//	return;
					if(!(bitConvPIP1=IMMapInBit(sConvPIP1)))
						return;
					if(!(bitConvPIP2=IMMapInBit(sConvPIP2)))
						return;

					//IMSetOutBit(bitConvRun,TRUE); // Start conveyor	// Unused - ddd 10/03/14
					IMSetOutBit(bitConvReset,TRUE); // Reset the conveyor	
					IMSetOutBit(bitConvReset,FALSE); // Reset the conveyor	
					convtimer=SetTimer(NULL,0,1000,ConvResetTIMEOUT);
				//IMMonitorInBit(abc);
					IMMonitorInBit(bitConvPIP1,IBM01,1,OnConvPIP1);
					IMMonitorInBit(bitConvPIP1,IBM10,0,OnConvPIP1);
					IMMonitorInBit(bitConvPIP2,IBM01,1,OnConvPIP2);
					IMMonitorInBit(bitConvPIP2,IBM10,0,OnConvPIP2);
					IMMonitorInBit(bitConvReady,IBM01,1,OnConvReady);
					IMMonitorInBit(bitConvReady,IBM10,0,OnConvReady);

					if(IMReadInBit(bitConvReady)) // Start THRU section
						{
						IMSetOutBit(bitConvPC2,FALSE);
						IMSetOutBit(bitConvPC2,TRUE);
						IMSetOutBit(bitConvPC1,FALSE);
						IMSetOutBit(bitConvPC1,TRUE);
						}
					}
				break;
		case 3:	{	// Map out bits
					if(!(bitConvReset=IMMapOutBit(sConvReset)))
						return;
					if(!(bitConvPC1=IMMapOutBit(sConvPC1)))
						return;
					if(!(bitConvPC2=IMMapOutBit(sConvPC2)))
						return;
					if(!(bitConvPC3=IMMapOutBit(sConvPC3)))
						return;
					// Map in bits
					if(!(bitConvReady=IMMapInBit(sConvReady)))
						return;
					if(!(bitConvError=IMMapInBit(sConvError)))
						return;
					if(!(bitConvPIP1=IMMapInBit(sConvPIP1)))
						return;
					if(!(bitConvPIP2=IMMapInBit(sConvPIP2)))
						return;
					if(!(bitConvPIP3=IMMapInBit(sConvPIP3)))
						return;

					IMSetOutBit(bitConvReset,TRUE); // Reset the conveyor
					convtimer=SetTimer(NULL,0,1000,ConvResetTIMEOUT);
					IMSetOutBit(bitConvReset,FALSE); // Reset the conveyor
					convtimer=SetTimer(NULL,0,1000,ConvResetTIMEOUT);
					IMSetOutBit(bitConvPC1,TRUE);
					IMSetOutBit(bitConvPC2,FALSE);
					IMSetOutBit(bitConvPC3,TRUE);
					IMSetOutBit(bitConvRun,TRUE); // Start conveyor

					IMMonitorInBit(bitConvPIP1,IBM01,1,OnConvPIP1);
					IMMonitorInBit(bitConvPIP1,IBM10,0,OnConvPIP1);
					IMMonitorInBit(bitConvPIP2,IBM01,1,OnConvPIP2);
					IMMonitorInBit(bitConvPIP2,IBM10,0,OnConvPIP2);
					IMMonitorInBit(bitConvPIP3,IBM01,1,OnConvPIP3);
					IMMonitorInBit(bitConvPIP3,IBM10,0,OnConvPIP3);
					IMMonitorInBit(bitConvReady,IBM01,1,OnConvReady);
					IMMonitorInBit(bitConvReady,IBM10,0,OnConvReady);
				
					if(IMReadInBit(bitConvReady)) // Start THRU section
						IMSetOutBit(bitConvPC3,TRUE);
					}
				break;
			default:
					break;
		}
		ConvOpen=TRUE;
	}

/******************************************************************************
	ConveyorQuit()

	Called during app shutdown to cleanup the conveyor system
******************************************************************************/
void ConveyorQuit(void)
	{
	if(ConvOpen)
		switch (ConvType)
			{
			case 1:	if(bitThruConvPIP && bitThruConvPC)// Old style PIP PC conveyor
						{
						IMMonitorInBit(bitThruConvPIP,IBM01,1,NULL);
						IMMonitorInBit(bitThruConvPIP,IBM10,0,NULL);
						}
					break;
			case 2:	IMSetOutBit(bitConvRun,FALSE); // Start conveyor
					IMMonitorInBit(bitConvPIP2,IBM01,1,NULL);
					IMMonitorInBit(bitConvPIP2,IBM10,0,NULL);
					IMMonitorInBit(bitConvReady,IBM01,1,NULL);
					IMMonitorInBit(bitConvReady,IBM10,0,NULL);
					break;
			case 3: IMSetOutBit(bitConvRun,FALSE); // Start conveyor
					IMMonitorInBit(bitConvPIP1,IBM01,1,NULL);
					IMMonitorInBit(bitConvPIP1,IBM10,0,NULL);
					IMMonitorInBit(bitConvPIP2,IBM01,1,NULL);
					IMMonitorInBit(bitConvPIP2,IBM10,0,NULL);
					IMMonitorInBit(bitConvReady,IBM01,1,NULL);
					IMMonitorInBit(bitConvReady,IBM10,0,NULL);
				break;
			default :	break;
			}
	ConvOpen=FALSE;
	}

/******************************************************************************
	empty = ConveyorEmpty()

	Stops the conveyor request for parts if conveyor is open.
******************************************************************************/
BOOL ConveyorEmpty(void)
	{
	if(ConvOpen)
		switch (ConvType)
			{
			case 1:	return !IMReadInBit(bitConvPIP);      // Old style PIP n PC Conveyor
					break;
			case 2:	return !IMReadInBit(bitConvPIP1);		// ConvSimplimatic New style PIP PR PC
					break;
			case 3: return !IMReadInBit(bitConvPIP1);
					break;
			default :	break;
			}
	return TRUE;
	}	

/******************************************************************************
	ConveyorStopReq()

	Stops the conveyor request for parts if conveyor is open.
******************************************************************************/
void ConveyorStopReq(void)
	{
	if(ConvOpen)
		switch (ConvType)
			{
			case 1:	IMSetOutBit(bitConvPC,0);		// Old style PIP n PC Conveyor
					if(bitConvPS)
						IMSetOutBit(bitConvPS,0);
					break;
			case 2:	IMSetOutBit(bitConvPR1,0);		// ConvSimplimatic New style PIP PR PC
					break;
			case 3: if (IMReadInBit(bitConvPIP1))
						IMSetOutBit(bitConvPC1,0);				
					break;
			default:break;
			}
	}

/******************************************************************************
	RefEdgeInit();
	
	Initializes the ref edge system by reading the ini file and creating
	the brush.
******************************************************************************/
void RefEdgeInit(void)
	{
	RefRightEdge=(BOOL)GetPrivateProfileInt(IMFIniSection,
		"RefRightEdge",0,IMSIni);
	RefBottomEdge=(BOOL)GetPrivateProfileInt(IMFIniSection,
		"RefBottomEdge",0,IMSIni);
	ShowRefEdge=(BOOL)GetPrivateProfileInt(IMFIniSection,
		"ShowRefEdge",0,IMSIni);
	RefEdgePen=CreatePen(PS_SOLID,0,GetPrivateProfileCOLORREF(
		IMFIniSection,"RefEdgeColor",RGB(0,255,0),IMSIni));
	}

/******************************************************************************
	ok = GetNestWCS(P3 *nestwcs,PIMF imf,int nesti)

	Accesses the fixture and computes the WCS origin of nest nesti as defined in the model.
******************************************************************************/
BOOL GetNestWCS(P3 *nestwcs,PIMF imf,int nesti)
	{
	M3 fcs;
	if(!GetFCS(&fcs,imf,FALSE))
		return FALSE;

	P2 temp=imf->nests.nest[nesti].origin;
	P2 owcs=temp*fcs;
	nestwcs->x=owcs.x;
	nestwcs->y=owcs.y;
	nestwcs->z=imf->extent.z;

	return TRUE;
	}

/******************************************************************************
	ok=GetFCS(fcs,pimf,aligned)

	Computes the FCS matrix, that is the matrix that maps FCS points to
	WCS points.  Takes IMF origin,rotation into account.
	Takes RefRightEdge and RefBottomEdge into account.
	Takes alignement info into acount if aligned and available for
	this level.

	BOOL ok = TRUE on success
	M3 *fcs = place to store result.
	PIMF pimf = IMF to compute fcs for.
	BOOL aligned = Take alignement into account if available.
******************************************************************************/
BOOL GetFCS(M3 *fcs,PIMF pimf,BOOL aligned)
	{
	BOOL xmok=FALSE;
	M3 ma(TRANSLATE,pimf->origin.x,pimf->origin.y,0); // IMF Origin translation matrix A
	//M3Translate(&ma,pimf->origin.x,pimf->origin.y);

	M3 mb(ROTATE,0,0,pimf->rotation*DEG); // IMF Rotation matrix B
	//M3Rotate(&mb,pimf->rotation*DEG);

	V2 ve; // Edge mapping matrix C
	ve.v2[0]=RefRightEdge ? -pimf->extent.x : 0.0f;
	ve.v2[1]=RefBottomEdge ? -pimf->extent.y : 0.0f;
	M3 mc(TRANSLATE,ve.v2[0],ve.v2[1],0);
	//M3Translate(&mc,ve.v2[0],ve.v2[1]);

	M3 xmt(IDENTITY,0,0,0); // alignment translation correction matrix
	M3 xmr(IDENTITY,0,0,0); // alignment rotation correction matrix
	if(aligned && pimf->aesetup && pimf->aeinfo)
		{
		if(!pimf->aesetup->getxm(&xmt,&xmr,pimf->aeinfo,0))
			return FALSE;
		xmok=TRUE;
		}
	M3 mba;
	mba=mb*ma;// mba = origin * rotation
	M3 mcba;
	mcba=mc*mba; //mcba = mba * edge mapping translation
	M3 temp=xmt*mcba; // temp = alignment translation * mcba
	*fcs=xmr*temp;  // fcs = alignment rotation * temp
	if(DebugMode)
		{
		OutputDebugString("  FCS MATRIX=\n");
		DebugMatrix(fcs);
/*		OutputDebugString("    TRANSLATION MATRIX=\n");
		DebugMatrix(&ma);
		if(pimf->rotation!=0.0f)
			{
			OutputDebugString("    ROTATION MATRIX=\n");
			DebugMatrix(&mb);
			}
		if(RefRightEdge||RefBottomEdge)
			{
			OutputDebugString("    REF EDGE MATRIX=\n");
			DebugMatrix(&mc);
			}
		if(xmok)
			{
			OutputDebugString("    ALIGN TRANSLATE=\n");
			DebugMatrix(&xmt);
			OutputDebugString("    ALIGN ROTATION=\n");
			DebugMatrix(&xmr);
			}*/
		}
	return TRUE;
	}

/******************************************************************************
	ok=GetNCS(ncs,pimf,pnest,ni,aligned)

	Computes the NCS matrix, that is the matrix that maps NCS points to
	FCS points.  Takes IMP alignement into account if aligned is true.

	BOOL ok = TRUE on success
	M3 *ncs = place to store result.
	PIMF pimf = IMF to compute ncs for.
	IMFNEST *pnest = The nest to compute ncs for.
	int ni = nest index.
	BOOL aligned = Take alignment into account if available.
******************************************************************************/
BOOL GetNCS(M3 *ncs,PIMF pimf,IMFNEST *pnest,int ni,BOOL aligned)
	{
	BOOL xmok=FALSE;
	M3 ma(TRANSLATE,pnest->origin.x,pnest->origin.y,0),
		mb(ROTATE,0,0,pnest->rotation*DEG);

	M3 xmt(IDENTITY,0,0,0); // alignment translation correction matrix X
	M3 xmr(IDENTITY,0,0,0); // alignment rotation correction matrix X
	if(aligned && pimf->imp.aesetup && pimf->aeinfo)
		{
		if(!pimf->imp.aesetup->getxm(&xmt,&xmr,pimf->aeinfo,ni))
			return FALSE;
		xmok=TRUE;
		}

	// ncs=xm*mb*ma
	M3 mba;
	mba=mb*ma;
	M3 temp=xmt*mba;//M3Mul(ncs,&xm,&mba);
	*ncs=temp*xmr;

	if(DebugMode)
		{
		OutputDebugString("  NCS MATRIX=\n");
		DebugMatrix(ncs);
/*		if(pnest->origin.x||pnest->origin.y)
			{
			OutputDebugString("    TRANSLATION MATRIX=\n");
			DebugMatrix(&ma);
			}
		if(pnest->rotation!=0.0f)
			{
			OutputDebugString("    ROTATION MATRIX=\n");
			DebugMatrix(&mb);
			}
		if(xmok)
			{
			OutputDebugString("    ALIGN TRANSLATE=\n");
			DebugMatrix(&xmt);
			OutputDebugString("    ALIGN ROTATION=\n");
			DebugMatrix(&xmr);
			}*/
		}

	return TRUE;
	}

/******************************************************************************
	ok = GetIMF(PIMF imf,LPSTR name)

	Loads the fixture of the given name.
******************************************************************************/
BOOL GetIMF(PIMF imf,LPSTR name)
	{
	// build complete path in tbuf
	if(IMSFixtures[0])
		{
		lstrcpy(tbuf,IMSFixtures);
		lstrcat(tbuf,"\\");
		}
	else
		tbuf[0]=0;
	lstrcat(tbuf,name);
	strfnextend(tbuf,"IMF");
	lstrcpy(imf->fn,tbuf);

	// Initialize the ofs for the name
	HANDLE fh = CreateFile(tbuf,GENERIC_READ,0,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	
	if(fh == INVALID_HANDLE_VALUE)
		{
		IDError=IDE_OPENIMF;
		return FALSE;
		}

	CloseHandle(fh);
	// Load the fixture
	return IMFLoad(imf);
	}

// GetPWCS : Computes a PCS to WCS transform
BOOL GetPWCS(LPM3 pwcs,PIMF pimf,IMPHEADER *h,int nesti)
	{
	BOOL ok=FALSE;
	M3 fcs;
	if(GetFCS(&fcs,pimf,FALSE))
		{
		// Get unaligned NCS
		M3 ncs;
		IMFNEST *pnest=&(pimf->nests.nest[nesti]);
		if(GetNCS(&ncs,pimf,pnest,nesti,FALSE))
			{
			M3 pcst(TRANSLATE,h->origin.x,h->origin.y,0),
				pcsr(ROTATE,0,0,h->rotation*DEG);
			//M3Translate(&pcst,h->origin.x,h->origin.y);
			//M3Rotate(&pcsr,h->rotation*DEG);
			M3 rt,rtn;
			rt=pcsr*pcst;//M3Mul(&rt,&pcsr,&pcst);
			rtn=rt*ncs;//M3Mul(&rtn,&rt,&ncs);

			M3 temp=rtn*fcs;//M3Mul(pwcs,&rtn,&fcs);
			*pwcs=temp;
			ok=TRUE;
			}
		}
	return ok;
	}

/******************************************************************************
	ok=GetMWCSZ(mwcs,z,imf,ni,mi)

	Get the MarkCS to WorldCS transform and z.

	BOOL ok = TRUE if it worked.
	LPM3 mwcs = address of matrix to receive the transform
	LPDOUBLE = address of double to receive z height
	PIMF = address if IMF with valid IMP and nests
	int ni = nest index
	int mi = mark index
******************************************************************************/
BOOL GetMWCSZ(LPM3 mwcs,LPDOUBLE z,PIMF imf,int ni,int mi)
	{
	BOOL ok=FALSE;
	if(ni<imf->nests.count)
		{
		if(mi<imf->imp.marks.count)
			{
			M3 pw; // PCS to WCS
			if(GetPWCS(&pw,imf,imf->imp.pheader,ni))
				{
				IMPMARK *mark=&(imf->imp.marks.mark[mi]);
				M3 r(ROTATE,0,0,mark->rotation*DEG),
					t(TRANSLATE,mark->origin.x,mark->origin.x,0),rt;
				//M3Rotate(&r,mark->rotation*DEG);
				//M3Translate(&t,mark->origin.x,mark->origin.x);
				rt=r*t;//M3Mul(&rt,&r,&t);
				M3 temp=rt*pw;//M3Mul(mwcs,&rt,&pw);
				*mwcs=temp;
				*z=imf->extent.z+imf->imp.pheader->partSize.z;
				ok=TRUE;
				}
			}
		}
	return ok;
	}

/******************************************************************************
	ok = GetIMFIMP(imf,caldir,calimp)

	Loads the imp and its referenced imf.

	PIMF imf = imf with imp both to load
	LPSTR caldir = if if not null specifies dir.
	LPSTR calimp = imp filename
******************************************************************************/
BOOL GetIMFIMP(PIMF imf,LPSTR caldir,LPSTR calimp)
	{
	if(caldir)
		if(chddir(caldir))
			{
			IDError=IDE_CDPROGS;
			return FALSE;
			}

	HANDLE fh=CreateFile(calimp,GENERIC_READ,0,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);

	if(fh == INVALID_HANDLE_VALUE)
		{
		IDError=IDE_FNF;
		return FALSE;
		}

	CloseHandle(fh);

	lstrcpy(imf->imp.fn,calimp);

	if(!IMPLoad(&imf->imp))
		return FALSE;

	//////int mps; // mps rotary test udha
	//////mps=IMPGetUDHAInt(&imf->imp,"RotaryGalil",0); // mps test udha
	//////char xygDir[128]; // mps rotary test udha
	//////strcpy(xygDir,IMSDir); // mps rotary test udha
	//////strncat (xygDir, "\\XYGRoty.ini", 12); // mps rotary test udha
	//////if (mps) // mps rotary test udha
	//////    WritePrivateProfileString("XYGRoty","RotaryGalil","1",xygDir);
	//////else
	//////	WritePrivateProfileString("XYGRoty","RotaryGalil","0",xygDir);
	// ZStartOnly=(BOOL)GetPrivateProfileInt(IMSName,"ZStartOnly",0,IMSIni);

	// Exctract fixture name out of the header
	// and optionally build complete path to it.
	if(IMSFixtures[0])
		{
		lstrcpy(tbuf,IMSFixtures);
		lstrcat(tbuf,"\\");
		}
	else
		tbuf[0]=0;
	lstrcat(tbuf,imf->imp.pheader->fixture);
	strfnextend(tbuf,"IMF");
	lstrcpy(imf->fn,tbuf);
	// Initialize the ofs for the imf


	fh = CreateFile(imf->fn,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh == INVALID_HANDLE_VALUE)
		{
		IDError=IDE_NOIMF;
		IMPFree(&imf->imp);
		return FALSE;
		}

	CloseHandle(fh);

	// Load the fixture
	if(!IMFLoad(imf))
		{
		IMPFree(&imf->imp);
		return FALSE;
		}
	// Allocate the vals
	if(!IMFAllocVals(imf))
		return FALSE;
	return TRUE;
	}

/******************************************************************************
	CALLBACK FUNCTIONS----IMP-> Menu-> Test-> Running window and fixture gui simulator
******************************************************************************/
LRESULT CALLBACK IMFRUNWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_CREATE:
			FocusID=0;
			return RunWMCreate(hwnd);

		case WM_SIZE:
			{
			int yb,x;
			wsize.x=LOWORD(lP);
			wsize.y=HIWORD(lP);
			yb=wsize.y-bsize.y;
			x=0;
			if(ClampActive)
				{
				MoveWindow(ClampRelBtn,x,yb,bsize.x,bsize.y,TRUE);
				x+=bsize.x;
				}
			if(QtyBtn)
				{
				MoveWindow(QtyBtn,x,yb,bsize.x,bsize.y,TRUE);
				x+=bsize.x;
				}
			if(AllBtn)
				{
				MoveWindow(AllBtn,x,yb,bsize.x,bsize.y,TRUE);
				x+=bsize.x;
				}
			if(Plus1Btn)
				{
				MoveWindow(Plus1Btn,x,yb,bsize.x>>1,bsize.y,TRUE);
				x+=bsize.x>>1;
				}
			if(Minus1Btn)
				{
				MoveWindow(Minus1Btn,x,yb,bsize.x>>1,bsize.y,TRUE);
				x+=bsize.x>>1;
				}
			return 0;
			}

		case WM_SETFOCUS:
			Notify(hwnd,IMFN_SETFOCUS);
			if(FocusID)
				SetFocus(GetDlgItem(hwnd,FocusID));
			else
				Notify(hwnd,IMFN_TAB);
			return 0;

		case WM_ENABLE: // Enable/Disable buttons so they grey properly
			if(QtyBtn) EnableWindow(QtyBtn,wP);
			if(AllBtn) EnableWindow(AllBtn,wP);
			if(Plus1Btn) EnableWindow(Plus1Btn,wP);
			if(Minus1Btn) EnableWindow(Minus1Btn,wP);
			if(ClampRelBtn) EnableWindow(ClampRelBtn,wP);
			return 0;

		case WM_COMMAND:  // Win32 changed
			if(lP)
				return RunWMCommandIDC(hwnd,LOWORD(wP),HIWORD(wP));
			return 0;

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			POINT fe; // fixture extent LINCH space
			RECT ri;//rotary extent linch
			IMFNEST *pnest, *rnest; //newly added rnest for rotary	----sam	10/13/16
			HBRUSH oldbrush;
			HPEN oldpen;
			int i;
			P2 ne; // Nest extent
			P4 re;
			int x,y;

			BeginPaint(hwnd,&ps);

			// Set window device mapping


			// Draw frame for outer platform
			if(HideOptRotaryIMF)
			{
				setmapping(ps.hdc);
				oldbrush=(HBRUSH)SelectObject(ps.hdc,FixtureBrush);
				fe.x=(int)(RunIMF.extent.x*LINCH);
				fe.y=(int)(RunIMF.extent.y*LINCH);
				Rectangle(ps.hdc,0,0,fe.x,fe.y);
				
				for(i=0;i<RunIMF.nests.count;i++)
				{
				pnest=&(RunIMF.nests.nest[i]);

				
					ne.x=pnest->extent.x ?
					pnest->extent.x : RunIMF.imp.pheader->partSize.x;
					ne.y=pnest->extent.y ?
					pnest->extent.y : RunIMF.imp.pheader->partSize.y;
					SelectObject(ps.hdc,NoPartBrush);
					drawrect(ps.hdc,&(pnest->origin),&ne,pnest->rotation);
				
					//Draw parts
				if(pnest->flags & nbfOCCUPIED)
					{
					SelectObject(ps.hdc,FlagBrush(pnest->flags));
					drawpart(ps.hdc,&(pnest->origin),
						(LPP2)&(RunIMF.imp.pheader->partSize),
						pnest->rotation,i+1);
						if(pnest->flags & nbfMARKED)
						{	// clear varification pass indicators if marked...
							pnest->flags &= (~nbfLOCPASS1);
							pnest->flags &= (~nbfLOCPASS2);
						}
				    }
			    
				if( (pnest->flags & nbfLOCPASS1)||(pnest->flags & nbfLOCFAIL1) )
					{
					P2 EXTENT;	// change location of extent
					EXTENT.x = (RunIMF.imp.pheader->partSize.x)/2;
					EXTENT.y = (RunIMF.imp.pheader->partSize.y)/2;
					if (pnest->flags & nbfLOCPASS1){
						SelectObject(ps.hdc,PassedPartBrush);
						}
					else{	// if (pnest->flags & nbfLOCFAIL1)
						SelectObject(ps.hdc,FailedPartBrush);
						}
					drawpart(ps.hdc
							,&(pnest->origin)
							,&EXTENT
							,pnest->rotation
							,i+1);
					}
				if((pnest->flags & nbfLOCPASS2)||(pnest->flags & nbfLOCFAIL2) )
					{
					P2 ORIGIN; // change location of origin
						ORIGIN.x = (pnest->origin.x) + (RunIMF.imp.pheader->partSize.x)/2;
						ORIGIN.y = (pnest->origin.y);
					P2 EXTENT;	// change location of extent
						EXTENT.x = (RunIMF.imp.pheader->partSize.x)/2;
						EXTENT.y = (RunIMF.imp.pheader->partSize.y)/2;
					if (pnest->flags & nbfLOCPASS2){
						SelectObject(ps.hdc,PassedPartBrush);
						}
					else{	//if (pnest->flags & nbfLOCFAIL2)
						SelectObject(ps.hdc,FailedPartBrush);
						}
					drawpart(ps.hdc
							,&ORIGIN
							,&EXTENT
							,pnest->rotation
							,i+1);
					}
				
				}
				
				if(ShowRefEdge) // Draw Ref Edge
					{
					oldpen=(HPEN)SelectObject(ps.hdc,RefEdgePen);
					x=RefRightEdge ? fe.x : 0;
					y=RefBottomEdge ? fe.y : 0;
					MoveToEx(ps.hdc,0,y,NULL);
					LineTo(ps.hdc,fe.x,y);
					MoveToEx(ps.hdc,x,0,NULL);
					LineTo(ps.hdc,x,fe.y);
					SelectObject(ps.hdc,oldpen);
					}
					SelectObject(ps.hdc,oldbrush);

					EndPaint(hwnd,&ps);
					return 0;
					}
			// Draw the occuped and empty parts
			
				else
				{
				setmapping(ps.hdc);
				oldbrush=(HBRUSH)SelectObject(ps.hdc,FixtureBrush);
				ri.left=(int)(RunIMF.extentrot.d*LINCH);
				ri.top=(int)(RunIMF.extentrot.d*LINCH);
				ri.bottom=(int)(RunIMF.extentrot.d*LINCH);
				ri.right=(int)(RunIMF.extentrot.d*LINCH);
				Ellipse(ps.hdc,ri.top,ri.bottom,ri.left,ri.right);
				
				
				for(i=0;i<RunIMF.nests.count;i++)
				{	
					pnest=&(RunIMF.nests.nest[i]);
				
					//Add rotary GUI support	--sam	10/18/16
					//re.d=pnest->rotfixextent.d ?
					//pnest->rotfixextent.d :  RunIMF.imp.pheader->partDiameter.di;
					re.d=pnest->rotfixextent.d*LINCH;
					//re.z=0;
					pnest->originrot.x;
					pnest->originrot.y;
					SelectObject(ps.hdc,NoPartBrush);
					drawcircle(ps.hdc,&(pnest->originrot),&re);
								
				//Draw parts
				if(pnest->flags & nbfOCCUPIED)
					{
					SelectObject(ps.hdc,FlagBrush(pnest->flags));
					drawpartcircle(ps.hdc,&(pnest->originrot),(LPP4)&(RunIMF.imp.pheader->partDiameter.di),i+1);
					P2 ORIGIN; // change location of origin
						ORIGIN.x = (pnest->originrot.x) + (RunIMF.imp.pheader->partDiameter.di)/2;
						ORIGIN.y = (pnest->originrot.y);
					P4 EXTENT;	// change location of extent
						EXTENT.d = (RunIMF.imp.pheader->partDiameter.di)/2;
					if (pnest->flags & nbfLOCPASS2)
					    {
						SelectObject(ps.hdc,PassedPartBrush);
						}
					else{	//if (pnest->flags & nbfLOCFAIL2)
						SelectObject(ps.hdc,FailedPartBrush);
						}
						drawpartcircle(ps.hdc,&ORIGIN,&EXTENT,i+1);
					}
					
					if(ShowRefEdge) // Draw Ref Edge
					{
					oldpen=(HPEN)SelectObject(ps.hdc,RefEdgePen);
					x=RefRightEdge ? fe.x : 0;
					y=RefBottomEdge ? fe.y : 0;
					MoveToEx(ps.hdc,0,y,NULL);
					LineTo(ps.hdc,fe.x,y);
					MoveToEx(ps.hdc,x,0,NULL);
					LineTo(ps.hdc,x,fe.y);
					SelectObject(ps.hdc,oldpen);
					}
					SelectObject(ps.hdc,oldbrush);

					EndPaint(hwnd,&ps);
					return 0;
				}
			  }
			}
			
			
			// end of "Draw the occuped and empty parts" / edge black outer side of platform frame
			case WM_LBUTTONDOWN:
					{
					IMFNEST *pnest;
					HDC hdc;
					POINT p;
					RECT nr; // nest rectangle
					int i;
					P2 ne; // nest extent
					p.x=LOWORD(lP);
					p.y=HIWORD(lP);
					hdc=GetDC(hwnd);
					setmapping(hdc);
					DPtoLP(hdc,&p,1);

					for(i=0;i<RunIMF.nests.count;i++)
						{
						pnest=&(RunIMF.nests.nest[i]);
						nr.left=(int)(LINCH*pnest->origin.x);
						nr.top=(int)(LINCH*pnest->origin.y);
						ne.x=pnest->extent.x ?
							pnest->extent.x : RunIMF.imp.pheader->partSize.x;
						ne.y=pnest->extent.y ?
							pnest->extent.y : RunIMF.imp.pheader->partSize.y;
						nr.right=nr.left+(int)(LINCH*ne.x);
						nr.bottom=nr.top+(int)(LINCH*ne.y);
						if(PtInRectRotated(&nr,p,pnest->rotation))
							{
							ResetProgressNBFs(hwnd);
							pnest->flags^=nbfOCCUPIED;
							// Reset pass/fail flags
							pnest->flags &= (~nbfLOCPASS1);
							pnest->flags &= (~nbfLOCFAIL1);
							pnest->flags &= (~nbfLOCPASS2);
							pnest->flags &= (~nbfLOCFAIL2);
							SelectObject(hdc,FlagBrush(pnest->flags));
							SendMessage(hwnd,IMFM_INVALIDATENEST,i,0);
							Notify(hwnd,IMFN_CHANGED);
							break;
							}
						}
					ReleaseDC(hwnd,hdc);
					return 0;
					}

					case WM_DESTROY:
						DELOBJ(FixtureBrush);
						DELOBJ(NoPartBrush);
						DELOBJ(PartBrush);
						DELOBJ(StartedPartBrush);
						DELOBJ(MarkedPartBrush);
						DELOBJ(PassedPartBrush);
						DELOBJ(FailedPartBrush);
						DELOBJ(CuredPartBrush);
						DELOBJ(CuredMarkedPartBrush);
						DELOBJ(CuredPassedPartBrush);
						return 0;
			
		case IMFM_INVALIDATENEST:
			{
			P2 ne;   // Nest extent
			RECT nr; // nest rectangle in client space
			HDC hdc;
			IMFNEST *pnest;
			int t;

			if(wP<RunIMF.nests.count) // If nesti in range
				{
				pnest=&(RunIMF.nests.nest[wP]);
				hdc=GetDC(hwnd);
				setmapping(hdc);

				// Expand auto size nests to part size
				ne.x=pnest->extent.x ?
					pnest->extent.x : RunIMF.imp.pheader->partSize.x;
				ne.y=pnest->extent.y ?
					pnest->extent.y : RunIMF.imp.pheader->partSize.y;

				// Expand nest extent to enclose part for invalidation
				ne.x=max(ne.x,RunIMF.imp.pheader->partSize.x);
				ne.y=max(ne.y,RunIMF.imp.pheader->partSize.y);

				// Compute logical nest rect
				nr.left=(int)(LINCH*pnest->origin.x);
				nr.top=(int)(LINCH*pnest->origin.y);
				nr.right=nr.left+(int)(LINCH*ne.x);
				nr.bottom=nr.top+(int)(LINCH*ne.y);
				ComputeNestBounds(&nr,pnest->rotation);
				// Convert nest rect to device coordinates
				LPtoDP(hdc,(LPPOINT)&nr,2);
				// Fix Upspray negative rectangle
				if(nr.left>nr.right)
					{
					t=nr.left;
					nr.left=nr.right;
					nr.right=t;
					}
				ReleaseDC(hwnd,hdc);
				InvalidateRect(hwnd,&nr,TRUE);
				}
			UpdateVarDisplay();
			return 0;
			}


		case IMFM_SETNLOADED:
			{
			IMFNEST *pnest;
			int maxload,i;

			maxload=RunIMF.nests.count;
			if(lP<=maxload)
				{
				for(i=0;i<maxload;i++)
					{
					pnest=&(RunIMF.nests.nest[i]);
					if(i < lP)
						pnest->flags=nbfOCCUPIED;
					else
						pnest->flags=0;
					}						
				InvalidateRect(hwnd,NULL,TRUE);
				Notify(hwnd,IMFN_CHANGED);
				}
			return 0;
			}

		case IMFM_QTY:
			DoQty(hwnd);
			return 0;

		case IMFM_ALL:
			DoAll(hwnd);
			return 0;
		}
	return DefWindowProc(hwnd,message,wP,lP);
	}

/******************************************************************************
	IMFEDITWP

	IMF Editor Window Proc.
******************************************************************************/
LRESULT CALLBACK IMFEDITWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_CREATE:
			return EditWMCreate(hwnd);

		case WM_SETFOCUS:
			SetFocus(FocusID ? GetDlgItem(hwnd,FocusID) : IMWnd);
			return 0;

		case WM_COMMAND: // Win32 changed
			if(!lP)
				return EditWMCommandIDM(hwnd,wP);
			else
				return EditWMCommandIDC(hwnd,wP,lP);
		case WM_NOTIFY:
			return EditWMNotifyIDC(hwnd,wP,lP);

		case WM_INITMENUPOPUP:
			EnableMenuItem((HMENU)wP,IDM_DELETEAE,
				MF_BYCOMMAND|(EditIMF.aesetup?MF_ENABLED:MF_GRAYED));
			return 0;
		case WM_CTLCOLORSTATIC:
			{
				HWND ctl = (HWND) lP;
				HDC hdcStatic = (HDC) wP;
				if (ctl == LblDesc   || ctl == LblOrg   || ctl == LblNests || 
					ctl == LblXYMode || ctl == LblRFDrv || ctl == LblExt || ctl == LblExtRt)
				{
					SetBkColor(hdcStatic,LabelColor);
					SetTextColor(hdcStatic,RGB(255,255,255));
					return (LONG) LabelBrush;
				}
				return 0;
			}
		case WM_DESTROY:
			DeleteObject(LabelBrush);
			return 0;
		}
	return DefWindowProc(hwnd,message,wP,lP);
	}

/******************************************************************************
	GRIDDP

	Grid cration dialog proc
******************************************************************************/
BOOL CALLBACK GRIDDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:
			EditGrid.UpdateDisplay(hdlg);
			CheckRadioButton(hdlg,IDC_GLR,IDC_GBT,IDC_GLR);
			if(HideOptRotation)
				{
				ShowWindow(GetDlgItem(hdlg,IDC_ROT),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_ROTLBL),SW_HIDE);
				}
			if(!StdLoc)
				{
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_LOCSIZE),SW_HIDE);
				}
			SetFocus(GetDlgItem(hdlg,IDC_GORDER));
			return TRUE;

		case WM_COMMAND:  // Win32 changed
			if(HIWORD(wP)!=BN_CLICKED) // Ignore all non button click msgs
				break;
			switch(LOWORD(wP))
				{
				case IDC_GLR:
				case IDC_GRL:
				case IDC_GTB:
				case IDC_GBT:
					CheckRadioButton(hdlg,IDC_GLR,IDC_GBT,wP);
					return TRUE;
				case IDC_LOCATE:
					if(!EditGrid.Update(hdlg,(int)ListView_GetItemCount(NestsWnd)-1))
						return TRUE;
					EditGrid.DoLocate(hdlg);
					return TRUE;
				case IDC_LOCSIZE:
					if(!EditGrid.Update(hdlg,(int)ListView_GetItemCount(NestsWnd)-1))
						return TRUE;
					EditGrid.DoSize(hdlg);
					return TRUE;
				case IDOK:
					{
					int count;
					double v[5];
					int i;
					int xi,yi;
					BOOL ymajor;
					int x1,y1,x2,y2,xinc,yinc;

					count=(int)ListView_GetItemCount(NestsWnd);
					count--;
					i=(int)ListView_GetNextItem(NestsWnd,-1,LVNI_SELECTED);

					if(!EditGrid.Update(hdlg,count))
						return TRUE;

					if(IsDlgButtonChecked(hdlg,IDC_GLR))
						{
						ymajor=TRUE;
						x1=0;
						x2=EditGrid.order.x;
						y1=0;
						y2=EditGrid.order.y;
						xinc=1;
						yinc=1;
						}
					else if(IsDlgButtonChecked(hdlg,IDC_GRL))
						{
						ymajor=TRUE;
						x1=EditGrid.order.x-1;
						x2=-1;
						y1=0;
						y2=EditGrid.order.y;
						xinc=-1;
						yinc=1;
						}
					else if(IsDlgButtonChecked(hdlg,IDC_GTB))
						{
						ymajor=FALSE;
						x1=0;
						x2=EditGrid.order.x;
						y1=0;
						y2=EditGrid.order.y;
						xinc=1;
						yinc=1;
						}
					else
						{
						ymajor=FALSE;
						x1=0;
						x2=EditGrid.order.x;
						y1=EditGrid.order.y-1;
						y2=-1;
						xinc=1;
						yinc=-1;
						}
					if(ymajor)
						for(yi=y1;yi!=y2;yi+=yinc)
							for(xi=x1;xi!=x2;xi+=xinc)
								{
								v[0]=EditGrid.origin.x+xi*EditGrid.spacing.x;
								v[1]=EditGrid.origin.y+yi*EditGrid.spacing.y;
								v[2]=EditGrid.size.x;
								v[3]=EditGrid.size.y;
								v[4]=EditGrid.rotation;
								MScaleVect((LPDOUBLE)&v,4,FALSE);
							//	VectToString(tbuf,(LPDOUBLE)&v,UNVS,",");
								InsertNest(v[0],v[1],v[2],v[3],v[4],i);
								i++;
								count++;
								}
					else // XMAJOR
						for(xi=x1;xi!=x2;xi+=xinc)
							for(yi=y1;yi!=y2;yi+=yinc)
								{
								v[0]=EditGrid.origin.x+xi*EditGrid.spacing.x;
								v[1]=EditGrid.origin.y+yi*EditGrid.spacing.y;
								v[2]=EditGrid.size.x;
								v[3]=EditGrid.size.y;
								v[4]=EditGrid.rotation;
								MScaleVect((LPDOUBLE)&v,4,FALSE);
							//	VectToString(tbuf,(LPDOUBLE)&v,UNVS,",");
								InsertNest(v[0],v[1],v[2],v[3],v[4],i);
								i++;
								count++;
								}
					ListView_SetItemState(NestsWnd,i,LVIS_SELECTED,LVIS_SELECTED);
					if(count)
					{
						EnableWindow(DelNestBtn,TRUE);
						EnableWindow(DelANestBtn,TRUE);
					}
					if(count==IMFMAXNESTS)
						{
						EnableWindow(AddNestBtn,FALSE);
						EnableWindow(AddGridBtn,FALSE);
						SetFocus(NestsWnd);
						}
					EndDialog(hdlg,TRUE);
					return TRUE;
					}
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}

		}
	return FALSE;
	}

/***************************************************************************
	NESTDP

	NEST Dialog Process.
***************************************************************************/
//Add support for IMF nest	----sam	10/14/16
BOOL CALLBACK NESTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			//Switch between linear and rotary nest	----sam	10/14/16
			if(!HideOptRotaryIMF)
			{
				ShowWindow(GetDlgItem(hdlg,IDC_XSIZE),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_XSIZELBL),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_YSIZE),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_YSIZELBL),SW_HIDE);
				
				SetFloatField(hdlg,IDC_X,EditNEST.originrot.x);
				SetFloatField(hdlg,IDC_Y,EditNEST.originrot.y);
				SetFloatField(hdlg,IDC_D,EditNEST.rotfixextent.d);// add extent.x for diameter of fixture storage
			}
			else
			{
				ShowWindow(GetDlgItem(hdlg,IDC_D),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_DLBL),SW_HIDE);

				SetFloatField(hdlg,IDC_X,EditNEST.origin.x);
				SetFloatField(hdlg,IDC_Y,EditNEST.origin.y);
				SetFloatField(hdlg,IDC_XSIZE,EditNEST.extent.x);
				SetFloatField(hdlg,IDC_YSIZE,EditNEST.extent.y);
			}
			if(HideOptRotation)
				{
				ShowWindow(GetDlgItem(hdlg,IDC_ROT),SW_HIDE);
				ShowWindow(GetDlgItem(hdlg,IDC_ROTLBL),SW_HIDE);
				}
			else
				SetFloatField(hdlg,IDC_ROT,EditNEST.rotation);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			SetFocus(GetDlgItem(hdlg,IDC_X));
			return TRUE;

		case WM_ACTIVATE:
			IMSADWnd=wP ? hdlg : 0;
			return FALSE;

		case WM_COMMAND:     // Command Win32 changed
			switch(LOWORD(wP))
				{
				case IDC_LOCATE:
					DoNestLocate(hdlg);
					return TRUE;
				case IDOK:
					//Switch between linear and rotary nest	----sam	10/14/16
					if(HideOptRotaryIMF)
					{
						if(!GetFloatField(hdlg,IDC_X,&EditNEST.origin.x))
							return TRUE;
						if(!GetFloatField(hdlg,IDC_Y,&EditNEST.origin.y))
							return TRUE;
						if(!GetFloatField(hdlg,IDC_XSIZE,&EditNEST.extent.x))
							return TRUE;
						if(!GetFloatField(hdlg,IDC_YSIZE,&EditNEST.extent.y))
							return TRUE;
					}
					else
					{

						if(!GetFloatField(hdlg,IDC_X,&EditNEST.originrot.x))
							return TRUE;
						if(!GetFloatField(hdlg,IDC_Y,&EditNEST.originrot.y))
							return TRUE;
						if(!GetFloatField(hdlg,IDC_D,&EditNEST.rotfixextent.d)) //add value for diameter
							return TRUE;
					}
					if(!HideOptRotation)
						if(!GetFloatField(hdlg,IDC_ROT,&EditNEST.rotation))
							return TRUE;

					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				}
		}
	return FALSE;            // Message not handled
	}

/******************************************************************************
	STATIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	setmapping(hdc)

	Sets up the mapping mode and window/viewport extents/origins to
	draw the imf.

	HDC hdc = Display context to setup.
******************************************************************************/
static void setmapping(HDC hdc)
	{
	POINT p;
	SIZE temp;
		if(HideOptRotaryIMF)
		{
		SetMapMode(hdc,MM_ISOTROPIC);

		p.x=(int)(RunIMF.extent.x*(LINCH+2.0*LMAT));
		p.y=(int)(RunIMF.extent.y*(LINCH+2.0*LMAT));
		SetWindowExtEx(hdc,p.x,p.y,NULL);
			if(UpSpray)
			SetViewportExtEx(hdc,-wsize.x,wsize.y-bsize.y,NULL); // Upspray
		else
			SetViewportExtEx(hdc,wsize.x,wsize.y-bsize.y,NULL); // Downspray
		GetViewportExtEx(hdc,&temp);
		p.x=temp.cx;
		p.y=temp.cy;
		p.x=(wsize.x-p.x)>>1;
		p.y=((wsize.y-bsize.y)-p.y)>>1;
		SetViewportExtEx(hdc,p.x,p.y,NULL);
		GetWindowOrgEx(hdc,&p);
		p.x-=(int)(RunIMF.extent.x*LMAT);
		p.y-=(int)(RunIMF.extent.y*LMAT);
		SetWindowOrgEx(hdc,p.x,p.y,NULL);
		}

		else   //------sam---
		{
		SetMapMode(hdc,CC_CIRCLES);
		p.x=(int)(RunIMF.extentrot.d*(LINCH+2.0*LMAT));
		p.y=(int)(RunIMF.extentrot.z*(LINCH+2.0*LMAT));
		//p.y=(int)(RunIMF.extentrot.y*(LINCH+2.0*LMAT));
		SetWindowExtEx(hdc,p.x,p.y,NULL);
			if(UpSpray)
			SetViewportExtEx(hdc,-wsize.x,wsize.y-bsize.y,NULL); // Upspray
		else
			SetViewportExtEx(hdc,wsize.x,wsize.y-bsize.y,NULL); // Downspray
		GetViewportExtEx(hdc,&temp);
		p.x=temp.cx;
		p.y=temp.cy;
		p.x=(wsize.x-p.x)>>1;
		p.y=((wsize.y-bsize.y)-p.y)>>1;
		SetViewportExtEx(hdc,p.x,p.y,NULL);
		GetWindowOrgEx(hdc,&p);
		p.x-=(int)(RunIMF.extentrot.d*LMAT);
		p.y-=(int)(RunIMF.extentrot.z*LMAT);
		SetWindowOrgEx(hdc,p.x,p.y,NULL);		
		}
  }

/******************************************************************************
	drawpart(hdc,origin,extent,rotation,index)

	Draws the part at the given origin with the given extent

	It draw the part diagramme.(yellow default)

	HDC hdc = Display context
	LPP2 origin = Origin coordinate in inches
	LPP2 extent = Extent in inches
	double rotation = CW Angle indegrees rotated about origin
	int index = nest index number to draw on part
******************************************************************************/
static void drawpart(HDC hdc,LPP2 origin,LPP2 extent,double rotation,int index)
	{
	POINT p[3];
	double s,c;
//	int oldrop;
	double e;

	drawrect(hdc,origin,extent,rotation);

	s=sin(rotation*DEG);
	c=cos(rotation*DEG);
	e=min(extent->x,extent->y)/4.0f;
	p[0].x=(int)(LINCH*origin->x);
	p[0].y=(int)(LINCH*origin->y);
	p[1].x=(int)(LINCH*(origin->x+e*c));
	p[1].y=(int)(LINCH*(origin->y+e*s));
	p[2].x=(int)(LINCH*(origin->x-e*s));
	p[2].y=(int)(LINCH*(origin->y+e*c));
	Polygon(hdc,p,3);
	}


/******************************************************************************
	drawrect(hdc,origin,extent,rotation)

	Draws the rectangle at the given origin with the given extent

	HDC hdc = Display context
	LPP2 origin = Origin coordinate in inches
	LPP2 extent = Extent in inches
	double rotation = CW Angle indegrees rotated about origin
******************************************************************************/
static void drawrect(HDC hdc,LPP2 origin,LPP2 extent,double rotation)
	{
	POINT p[4];
	double s,c;

	s=sin(rotation*DEG);
	c=cos(rotation*DEG);

	p[0].x=(int)(LINCH*origin->x);
	p[0].y=(int)(LINCH*origin->y);

	p[1].x=(int)(LINCH*(origin->x+extent->x*c));
	p[1].y=(int)(LINCH*(origin->y+extent->x*s));

	p[2].x=(int)(LINCH*(origin->x+extent->x*c-extent->y*s));
	p[2].y=(int)(LINCH*(origin->y+extent->x*s+extent->y*c));

	p[3].x=(int)(LINCH*(origin->x-extent->y*s));
	p[3].y=(int)(LINCH*(origin->y+extent->y*c));
	

	Polygon(hdc,p,4);
	}

/******************************************************************************
	drawcircle(hdc,origin,extent,rotation)

	Draws the circle at the given origin with the given extent

	HDC hdc = Display context
	LPP2 origin = Origin coordinate in inches
	LPP2 extent = Extent in inches
	double rotation = CW Angle indegrees rotated about origin
******************************************************************************/
//--sam	10/13/16
static void drawcircle(HDC hdc,LPP2 originrot,LPP4 rotfixextent)
	{
	int x1,y1,x2,y2;
	

	x1=(int)(LINCH*(originrot->x+rotfixextent->d));
	y1=(int)(LINCH*(originrot->y+rotfixextent->d));

	x2=(int)(LINCH*(originrot->x+rotfixextent->d));
	y2=(int)(LINCH*(originrot->y+rotfixextent->d));

	Ellipse(hdc,x1,y1,x2,y2);
	}

/******************************************************************************
	drawpartcircle(hdc,originrot,rotfixextent)

	Draws the circle at the given origin with the given extent

	HDC hdc = Display context
	LPP2 origin = Origin coordinate in inches
	LPP2 extent = Extent in inches
	double rotation = CW Angle indegrees rotated about origin
******************************************************************************/
//--sam	10/13/16
static void drawpartcircle(HDC hdc,LPP2 originrot,LPP4 extentrot,int index)
	{
	int x1,y1,x2,y2;
	

	x1=(int)(LINCH*(originrot->x+extentrot->d));
	y1=(int)(LINCH*(originrot->y+extentrot->d));

	x2=(int)(LINCH*(originrot->x+extentrot->d));
	y2=(int)(LINCH*(originrot->y+extentrot->d));

	Ellipse(hdc,x1,y1,x2,y2);
	}

/******************************************************************************
	ret = RunWMCreate(hwnd)

	Handles the IMFRUN WM_CREATE message.

	long ret = 0 on success -1L on faulure
	HWND hwnd = window receiving WM_CREATE message
******************************************************************************/
static long RunWMCreate(HWND hwnd)
	{
	int maxload=RunIMF.nests.count;
	if(ClampActive)
		ClampRelBtn=CreateWindow("XBUTTON","Rel-[F7]",
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			0,0,0,0,hwnd,(HMENU)IDC_CLAMPREL,IMSInst,NULL);
	else
		ClampRelBtn=0;
	if(maxload>1)
		{
		QtyBtn=CreateWindow("XBUTTON","Qty... (^Q)",
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			0,0,0,0,hwnd,(HMENU)IDC_QTY,IMSInst,NULL);
		wsprintf(tbuf,"All %d (^A)",maxload);
		AllBtn=CreateWindow("XBUTTON",tbuf,
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			0,0,0,0,hwnd,(HMENU)IDC_ALL,IMSInst,NULL);
		Plus1Btn=CreateWindow("XBUTTON","+1",
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			0,0,0,0,hwnd,(HMENU)IDC_PLUS1,IMSInst,NULL);
		Minus1Btn=CreateWindow("XBUTTON","-1",
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			0,0,0,0,hwnd,(HMENU)IDC_MINUS1,IMSInst,NULL);
		}
	else
		{
		QtyBtn=0;
		AllBtn=0;
		Plus1Btn=0;
		Minus1Btn=0;
		}
	bsize.x=10*sftm.tmAveCharWidth;
	bsize.y=7*sftm.tmHeight/4;
	if(ClampRelBtn)
		FocusID=IDC_CLAMPREL;
	else if(QtyBtn)
		FocusID=IDC_QTY;
	else
		FocusID=0;
	return 0;
	}

/******************************************************************************
	ret = RunWMCommandIDC(hwnd,idc,ncode)

	Handles Control WM_COMMAND messages for the IMFRUN window.

	long ret = return value = 0
	HWND hwnd = Window receiving message.
	WORD idc = Control ID
	WORD ncode =  Control notification code
******************************************************************************/
static long RunWMCommandIDC(HWND hwnd,WORD idc,WORD ncode)
	{
	IMFNEST *pnest;
//	long l;
	WORD count;
//	HDC hdc;
//	HBRUSH oldbrush;
	int i,j,n;

	switch(idc)
		{
		case IDC_CLAMPREL:
			switch(ncode)
				{
				case XN_TAB:
					if(!QtyBtn)
						{
						FocusID=IDC_CLAMPREL;
						Notify(hwnd,IMFN_TAB);
						}
					else
						SetFocus(QtyBtn);
					return 0;
				case XN_BKTAB:
					Notify(hwnd,IMFN_BKTAB);
					return 0;
				case XN_SETFOCUS:
					FocusID=idc;
					return 0;
				case BN_CLICKED:
					ClampToggle();
					return 0;
				}
			return 0;
		case IDC_QTY:
			switch(ncode)
				{
				case XN_TAB:
					SetFocus(AllBtn);
					return 0;
				case XN_BKTAB:
					if(!ClampRelBtn)
						Notify(hwnd,IMFN_BKTAB);
					else
						SetFocus(ClampRelBtn);
					return 0;
				case XN_SETFOCUS:
					FocusID=idc;
					return 0;
				case BN_CLICKED:
					ResetProgressNBFs(hwnd);
					DoQty(hwnd);
					UpdateVarDisplay();
					return 0;
				}
			return 0;

		case IDC_ALL:
			switch(ncode)
				{
				case XN_TAB:
					SetFocus(Plus1Btn);
					return 0;
				case XN_BKTAB:
					SetFocus(QtyBtn);
					return 0;
				case XN_SETFOCUS:
					FocusID=idc;
					return 0;
				case BN_CLICKED:
					ResetProgressNBFs(hwnd);
					DoAll(hwnd);
					UpdateVarDisplay();
					return 0;
				}
			return 0;

		case IDC_PLUS1:
			switch(ncode)
				{
				case XN_TAB:
					SetFocus(Minus1Btn);
					return 0;
				case XN_BKTAB:
					SetFocus(AllBtn);
					return 0;
				case XN_SETFOCUS:
					FocusID=idc;
					return 0;
				case BN_CLICKED:
					ResetProgressNBFs(hwnd);
					count=RunIMF.nests.count;
					// i = index of next nest or count if none
					i=count;
					while(i--)
						if(RunIMF.nests.nest[i].flags & nbfOCCUPIED)
							break;
					i++;
					// Update flags and display for i if found
					if(i<count)
						{
						pnest=&RunIMF.nests.nest[i];
						pnest->flags|=nbfOCCUPIED;
						SendMessage(hwnd,IMFM_INVALIDATENEST,i,0);
						Notify(hwnd,IMFN_CHANGED);
						}
					UpdateVarDisplay();
					return 0;
				}
			return 0;

		case IDC_MINUS1:
			switch(ncode)
				{
				case XN_TAB:
					FocusID=ClampRelBtn ? IDC_CLAMPREL : IDC_QTY;
					Notify(hwnd,IMFN_TAB);
					return 0;
				case XN_BKTAB:
					SetFocus(Plus1Btn);
					return 0;
				case XN_SETFOCUS:
					FocusID=idc;
					return 0;
				case BN_CLICKED:
					ResetProgressNBFs(hwnd);
					count=RunIMF.nests.count;
					// j = number of occupied nests
					// i = index of highest occupied nest if j>0
					j=0;
					for(n=0;n<count;n++)
						if(RunIMF.nests.nest[n].flags & nbfOCCUPIED)
							{
							i=n;
							j++;
							}
					// Update flags and display for i if j>1
					if(j>0)
						{
						pnest=&RunIMF.nests.nest[i];
						pnest->flags&=~nbfOCCUPIED;
						SendMessage(hwnd,IMFM_INVALIDATENEST,i,0);
						Notify(hwnd,IMFN_CHANGED);
						}
					UpdateVarDisplay();
					return 0;
				}
		}
	return 0;
	}

/******************************************************************************
	ok = EditIMFUpdate(hwnd,herr)

	Validates the IMF editor control values.
	If valid , extracts them to EditIMF.
	Detects changes and if any sets EditIMF.changed
	Detects value errors and if herr warns and then set focus back
		to the offending control.

	BOOL ok = True if updade completed without error.
	HWND hwnd = handle to the Mode window (should be a IMF1EDIT dialog)
	BOOL herr = True if errors are to be handled.
******************************************************************************/
static BOOL EditIMFUpdate(HWND hwnd,BOOL herr)
	{
	P2 origin,originrot;
	P3 extent;
	P4 extentrot;
	int count;
	int i;
	double v[5];

	// Get origin and validate it
	if(HideOptRotaryIMF)
	{	
		GetWindowText(OriginWnd,tbuf,TSIZE);
		if(!ParseVect(tbuf,(LPDOUBLE)&origin,2))
			{
			if(herr)
				{
				ShowError(IDE_BADIMFORIGIN);
				SetFocus(OriginWnd);
				}
		
			return FALSE;
			}
		MScaleVect((LPDOUBLE)&origin,2,TRUE);
		if(memcmp(&origin,&EditIMF.origin,sizeof(P2)))
			EditIMF.changed=TRUE;
	}

	else
	{
		GetWindowText(OriginrotWnd,tbuf,TSIZE);
		if(!ParseVect(tbuf,(LPDOUBLE)&originrot,2))
			{
			if(herr)
				{
				ShowError(IDE_BADIMFORIGIN);
				SetFocus(OriginrotWnd);
				}
		
			return FALSE;
			}
		MScaleVect((LPDOUBLE)&originrot,2,TRUE);
		if(memcmp(&originrot,&EditIMF.originrot,sizeof(P2)))
			EditIMF.changed=TRUE;
	
	}
	//Extent for rotary and linear fixtures	----sam 10/6/2016
	
/*	if(HideOptRotaryIMF)
	{
		GetWindowText(ExtentWnd,tbuf,TSIZE);
		if(!ParseVect(tbuf,(LPDOUBLE)&extentrot,2))
		{		
			if(herr)
			{
				ShowError(IDE_BADIMFROTEXTENT);
				SetFocus(RotExtentWnd);
			}
			return FALSE;
		}
		MScaleVect((LPDOUBLE)&extentrot,2,TRUE);
		if(memcmp(&extentrot,&EditIMF.extentrot,sizeof(P4)))
			EditIMF.changed=TRUE;
	}
	else
	{
		*/	// Get Rotary extent and validate it - ---------10/05/2016
		GetWindowText(ExtentWnd,tbuf,TSIZE);
		if(!ParseVect(tbuf,(LPDOUBLE)&extentrot,2))
		{		
			if(herr)
			{
				ShowError(IDE_BADIMFROTEXTENT);
				SetFocus(ExtentWnd);
			}
			return FALSE;
		}
		MScaleVect((LPDOUBLE)&extentrot,2,TRUE);
		if(memcmp(&extentrot,&EditIMF.extentrot,sizeof(P4)))
			EditIMF.changed=TRUE;
	//}	
	

	// Get nests count
	count=(int)ListView_GetItemCount(NestsWnd);
	count--;
	if(count==0)
		{
		if(herr)
			{
			ShowError(IDE_NOIMFNESTS);
			SetFocus(AddNestBtn);
			}
		return FALSE;
		}

	// Allocate new nests
	IMFNESTS newnests = IMFNESTS(count); 
	if(!newnests.nest && count>0)
		{
		if(herr)
			ShowError(IDE_NOMEM);
		return FALSE;
		}

	// Fill in new nests    //sam jha --01/19/2017
		newnests.count=count;
		for(i=0;i<count;i++)
			{
			if(HideOptRotaryIMF)
				{
					GetNestVect(tbuf,i);
					ParseVect(tbuf,&v[0],UNVS);
					MScaleVect((LPDOUBLE)&v,4,TRUE);
					newnests.nest[i].origin.x=v[0];
					newnests.nest[i].origin.y=v[1];
					newnests.nest[i].extent.x=v[2];
					newnests.nest[i].extent.y=v[3];
					newnests.nest[i].rotation=HideOptRotation ? 0.0f : v[4];
					newnests.nest[i].flags=0;
				}
				else
				{
					GetNestVect(tbuf,i);
					ParseVect(tbuf,&v[0],UNVS);
					MScaleVect((LPDOUBLE)&v,4,TRUE);
					newnests.nest[i].originrot.x=v[0];
					newnests.nest[i].originrot.y=v[1];
					newnests.nest[i].rotfixextent.d=v[2];
					v[3]=0.0f;
					v[4]=0.0f;
					newnests.nest[i].flags=0;
				}
		
			}
	
		
	// Detect changes
	if(EditIMF.nests.count!=count)
		EditIMF.changed=TRUE;
	else
		for(i=0;i<count;i++)
			if(memcmp(&EditIMF.nests.nest[i],&newnests.nest[i],sizeof(IMFNEST)))
				{
				EditIMF.changed=TRUE;
				break;
				}

	// Update IMF
	//DELZ(EditIMF.pnests);

	if(!HideOptRotaryIMF)  //sam  1/19/2017
		{
	EditIMF.nests=newnests;
	EditIMF.originrot=originrot;
	EditIMF.extentrot=extentrot;
	GetWindowText(DescWnd,EditIMF.desc,IMFLDESC);

			i=(int)SendMessage(XYModeWnd,CB_GETCURSEL,0,0);
		EditIMF.xymode=
			i==2 ? imSY :
			i==1 ? imXS : imXY;

		EditIMF.ccwplus=IsDlgButtonChecked(hwnd,IDC_CCWPLUS);
		GetWindowText(RFDrvWnd,EditIMF.rfdrv,IMFLRFDRV);
		}
	else
	{
	EditIMF.nests=newnests;
	EditIMF.origin=origin;
		EditIMF.extentrot=extentrot;
	//EditIMF.extent=extent;
	GetWindowText(DescWnd,EditIMF.desc,IMFLDESC);
	}

	return TRUE;
	}

/******************************************************************************
	ret = EditWMCreate(hwnd)

	Handles the IMFEDIT WM_CREATE message.

	long ret = 0 on success -1L on faulure
	HWND hwnd = window receiving WM_CREATE message
******************************************************************************/
static long EditWMCreate(HWND hwnd)
	{
	KEDITINFO ki;
	int i,nnests,x,y,/*ys*/xs;
	int yscap;   // y size of captions
	int yskedit; // y size of kedits
	int yslb;    // y size of list box
	int ysbtn;   // y size of button
	int xss,yss; // x and y space size
	int xs3;     // x size / 3
	double v[5],r[2];
	

	xss=4;
	yss=4;
	xs=64*sftm.tmAveCharWidth;
	yscap=GetSystemMetrics(SM_CYCAPTION);
	yskedit=(sftm.tmHeight+sftm.tmExternalLeading)*3/2;
	yslb=sftm.tmHeight*10;
	ysbtn=sftm.tmHeight*7/4;
	x=xss;
	y=yss;
	xs3=xs/4-2;

	// Create Desc caption (static child)
	LblDesc=CreateWindow("static","Description",
		WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,
		x,y,xs,yscap,hwnd,(HMENU)-1,IMSInst,NULL);

	// Create DescWnd Control
	y+=yscap-1;
	DescWnd=CreateWindow("KEDIT",EditIMF.desc,WS_BORDER|WS_CHILD|WS_VISIBLE,
		x,y,xs,yskedit,hwnd,(HMENU)IDC_DESCRIPT,IMSInst,NULL);
	ki.xextent=xs;
	ki.yextent=yskedit;
	ki.length=24;
	ki.digits=1;
	ki.lowers=1;
	ki.uppers=1;
	ki.spaces=1;
	ki.others=1;
	KeditSetInfo(DescWnd,&ki);

	// Create Origin caption (static child)
	y+=yskedit+yss;
	LblOrg=CreateWindow("static","Origin",WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,
		x,y,xs,yscap,hwnd,(HMENU)-1,IMSInst,NULL);
	
	if(HideOptRotaryIMF)
	{
		// Create Origin Control
		y+=yscap-1;
		v[0]=EditIMF.origin.x;
		v[1]=EditIMF.origin.y;
		MScaleVect((LPDOUBLE)&v,2,FALSE);
		VectToString(tbuf,(LPDOUBLE)&v,2,",");
		OriginWnd=CreateWindow("KEDIT",tbuf,WS_BORDER|WS_CHILD|WS_VISIBLE,
			x,y,xs,yskedit,hwnd,(HMENU)IDC_ORIGIN,IMSInst,NULL);
		ki.xextent=xs;
		ki.yextent=yskedit;
		ki.length=24;
		ki.digits=1;
		ki.lowers=0;
		ki.uppers=0;
		ki.spaces=1;
		ki.others=1;
		KeditSetInfo(OriginWnd,&ki);
		// Create Extent caption (static child)
		y+=yskedit+yss;
		LblExt=CreateWindow("static","Extents For Rotary Fixture (D,Z)",WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,x,y,xs,yscap,hwnd,(HMENU)-1,IMSInst,NULL);
		y+=yscap-1;
		r[0]=EditIMF.extentrot.d;
		r[1]=EditIMF.extentrot.z;
		MScaleVect((LPDOUBLE)&r,2,FALSE);
		VectToString(tbuf,(LPDOUBLE)&r,2,",");
		
		EditIMF.extent.x=r[0]/2;
		EditIMF.extent.y=r[0]/2;
		EditIMF.extent.z=r[1];
		//MScaleVect((LPDOUBLE)&v,3,FALSE);
		//VectToString(tbuf,(LPDOUBLE)&v,3,",");
		ExtentWnd=CreateWindow("KEDIT",tbuf,WS_BORDER|WS_CHILD|WS_VISIBLE,
			x,y,xs,yskedit,hwnd,(HMENU)IDC_EXTRT,IMSInst,NULL);
		//ExtentWnd=CreateWindow("KEDIT",tbuf,WS_BORDER|WS_CHILD|WS_VISIBLE,
		//	x,y,xs,yskedit,hwnd,(HMENU)IDC_EXTENT,IMSInst,NULL);
		ki.xextent=xs;
		ki.yextent=yskedit;
		ki.length=24;
		ki.digits=1;
		ki.lowers=0;
		ki.uppers=0;
		ki.spaces=1;
		ki.others=1;
		KeditSetInfo(ExtentWnd,&ki);
	}
	else
	{
		// Create ROTARY Origin Control
		y+=yscap-1;
		v[0]=EditIMF.originrot.x;
		v[1]=EditIMF.originrot.y;
		MScaleVect((LPDOUBLE)&v,2,FALSE);
		VectToString(tbuf,(LPDOUBLE)&v,2,",");
		OriginrotWnd=CreateWindow("KEDIT",tbuf,WS_BORDER|WS_CHILD|WS_VISIBLE,
			x,y,xs,yskedit,hwnd,(HMENU)IDC_ORIGIN,IMSInst,NULL);
		ki.xextent=xs;
		ki.yextent=yskedit;
		ki.length=24;
		ki.digits=1;
		ki.lowers=0;
		ki.uppers=0;
		ki.spaces=1;
		ki.others=1;
		KeditSetInfo(OriginrotWnd,&ki);
				//Create Rotary Extent caption (static child)             -------------sam 09/18/2016
		y+=yskedit+yss;
		LblExtRt=CreateWindow("static","Extent For Rotary Fixture (D,Z)",WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,
			x,y,xs,yscap,hwnd,(HMENU)-1,IMSInst,NULL);
					// Create Rotary Extent Control                              -------------sam 09/18/2016
		y+=yscap-1;
		v[0]=EditIMF.extentrot.d;
		v[1]=EditIMF.extentrot.z;
		MScaleVect((LPDOUBLE)&v,2,FALSE);
		VectToString(tbuf,(LPDOUBLE)&v,2,",");
		RotExtentWnd=CreateWindow("KEDIT",tbuf,WS_BORDER|WS_CHILD|WS_VISIBLE,
			x,y,xs,yskedit,hwnd,(HMENU)IDC_EXTRT,IMSInst,NULL);
		ki.xextent=xs;
		ki.yextent=yskedit;
		ki.length=24;
		ki.digits=1;
		ki.lowers=0;
		ki.uppers=0;
		ki.spaces=1;
		ki.others=1;
		KeditSetInfo(ExtentWnd,&ki);
	}

	//XY mode disabled for rotary IMF	----sam 10/6/1016
	if(!HideOptRotaryIMF)
	{
		y+=yskedit+yss;
		// Create XYMode  caption (static child)
		LblXYMode=CreateWindow("static","XYMode",WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,
			x,y,xs/3,yscap,hwnd,(HMENU)-1,IMSInst,NULL);
		x+=2*xs/3;


		// Create RFDrv caption (static child)
		LblRFDrv=CreateWindow("static","RF Driver",WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,
			x,y,xs/3,yscap,hwnd,(HMENU)-1,IMSInst,NULL);

		x=xss;
		y+=yscap-1;

		// Create XYMode Control
		XYModeWnd=CreateWindow("XCOMBOBOX",tbuf,WS_BORDER|WS_CHILD|WS_VISIBLE|
			CBS_DROPDOWNLIST,x,y,xs/3,yskedit*4,hwnd,(HMENU)IDC_XYMODE,IMSInst,NULL);
		SendMessage(XYModeWnd,CB_ADDSTRING,0,(LPARAM)(LPSTR)"XY");
		SendMessage(XYModeWnd,CB_ADDSTRING,0,(LPARAM)(LPSTR)"XS");
		SendMessage(XYModeWnd,CB_ADDSTRING,0,(LPARAM)(LPSTR)"SY");
		SendMessage(XYModeWnd,CB_SETCURSEL,
			EditIMF.xymode==imSY ? 2 : 
			EditIMF.xymode==imXS ? 1 : 0,0);

		x+=xs/3;
		// Create CCWPlusWnd control
		CCWPlusWnd=CreateWindow("XBUTTON","Reverse",
			WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
			x,y-yss,xs/3,ysbtn,hwnd,(HMENU)IDC_CCWPLUS,IMSInst,NULL);
		SendMessage(CCWPlusWnd,BM_SETCHECK,EditIMF.ccwplus ? 1 : 0,0);
		EnableWindow(CCWPlusWnd,EditIMF.xymode!=0);

		x+=xs/3;
		RFDrvWnd=CreateWindow("KEDIT",EditIMF.rfdrv,WS_BORDER|WS_CHILD|WS_VISIBLE,
			x,y,xs/3,yskedit,hwnd,(HMENU)IDC_RFDRV,IMSInst,NULL);
		ki.xextent=xs/3;
		ki.yextent=yskedit;
		ki.length=IMFLRFDRV;
		ki.digits=1;
		ki.lowers=1;
		ki.uppers=1;
		ki.spaces=0;
		ki.others=1;
		KeditSetInfo(RFDrvWnd,&ki);
		x=xss;
		EnableWindow(RFDrvWnd,EditIMF.xymode!=0);
	}

	// Create Nests caption (static child)
	//y+=yscap-1;
	y+=yskedit+yss;

	LblNests=CreateWindow("static","Parts",
		WS_BORDER|SS_CENTER|WS_CHILD|WS_VISIBLE,
		x,y,xs,yscap,hwnd,(HMENU)-1,IMSInst,NULL);
	// Create Nests control
	y+=yscap-1;
	NestsWnd=CreateWindow("XLISTVIEW","",
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|LVS_REPORT|
		LVS_SHOWSELALWAYS|WS_BORDER,
		x,y,xs,yslb,hwnd,(HMENU)IDC_NESTS,IMSInst,NULL);
	
	LOGFONT lf;
	HFONT font = (HFONT) SendMessage(NestsWnd,WM_GETFONT,0,0);
	if (font){
		GetObject(font, sizeof(LOGFONT), &lf);
		
		lf.lfHeight-=2;
		lf.lfWeight = FW_BOLD;
		font = CreateFontIndirect(&lf);
		
		SendMessage(NestsWnd,WM_SETFONT,(WPARAM) font,0);
	}

	DeleteObject(font);

	
	CreateListViewImageLists(NestsWnd,IMSInst,IDI_NEST,2);
	
	AddListViewColumn(NestsWnd,"Location",0,150);
	AddListViewColumn(NestsWnd,"Size",1,150);
	AddListViewColumn(NestsWnd,"Rotation",2,125);
	ListView_SetExtendedListViewStyle(NestsWnd,LVS_EX_FULLROWSELECT);
	//Add rotaryIMF single nest support	----sam	10/14/16
	nnests=EditIMF.nests.count;
	if(HideOptRotaryIMF)
	{
		for(i=0;i<nnests;i++)
		{
			v[0]=EditIMF.nests.nest[i].origin.x;
			v[1]=EditIMF.nests.nest[i].origin.y;
			v[2]=EditIMF.nests.nest[i].extent.x;
			v[3]=EditIMF.nests.nest[i].extent.y;
			v[4]=0.0;
			MScaleVect((LPDOUBLE)&v,4,FALSE);
			AddNest(v[0],v[1],v[2],v[3],v[4]);
		}
	}
	else
	{
		for(i=0;i<nnests;i++)
		{
			v[0]=EditIMF.nests.nest[i].originrot.x;
			v[1]=EditIMF.nests.nest[i].originrot.y;
			v[2]=EditIMF.nests.nest[i].rotfixextent.d;
			v[3]=0.0;
			v[4]=0.0;
			MScaleVect((LPDOUBLE)&v,4,FALSE);
			AddNest(v[0],v[1],v[2],v[3],v[4]);
		}
	}

	// Insert blank last item for normal insert position
	AddListViewItemWithImage(NestsWnd,(LPSTR)"",1);

	// Create AddNestBtn control
	y+=yslb+yss;
	AddNestBtn=CreateWindow("XBUTTON","Insert",
		WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
		x,y,xs3,ysbtn,hwnd,(HMENU)IDC_ADDNEST,IMSInst,NULL);

	// Create DelNestBtn control
	x+=xs3+2;
	DelNestBtn=CreateWindow("XBUTTON","Delete Sel",
		WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_DISABLED,
		x,y,xs3,ysbtn,hwnd,(HMENU)IDC_DELNEST,IMSInst,NULL);
	
	//Create DelANestBtn contro                  ------------------Linghai-11-4-08------
	x+=xs3+2;
	DelANestBtn=CreateWindow("XBUTTON","Delete All",
		WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_DISABLED,
		x,y,xs3,ysbtn,hwnd,(HMENU)IDC_DELANEST,IMSInst,NULL); //--------Linghai-11-4-08----

	// Create AddGridBtn control
	x+=xs3+2;
	AddGridBtn=CreateWindow("XBUTTON","Grid",
		WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
		x,y,xs3,ysbtn,hwnd,(HMENU)IDC_ADDGRID,IMSInst,NULL);

	ListView_SetItemState(NestsWnd,nnests,LVIS_SELECTED,LVIS_SELECTED);
	

	//Rotary only have single nest	----sam	10/13/16

	int nest;
	nest=(int)ListView_GetItemCount(NestsWnd)-1;
	
	if(nnests)
	{
		EnableWindow(DelNestBtn,TRUE);
		EnableWindow(DelANestBtn,TRUE);   //-------------Linghai-11-4-08----------
	}

	if(nest>0)
	{
		EnableWindow(AddNestBtn,FALSE);
	}
	if(nest==0)
	{
		EnableWindow(AddNestBtn,TRUE);
	}
	EnableWindow(AddGridBtn,FALSE);
	SetFocus(NestsWnd);
	FocusID=IDC_DESCRIPT;
	return 0;
	}

/******************************************************************************
	ret = EditWMCommandIDM(hwnd,idm)

	Handles Menu WM_COMMAND messages for the IMFEDIT window.

	long ret = return value = 0
	HWND hwnd = Window receiving message.
	WORD idm = Menu ID
******************************************************************************/
static long EditWMCommandIDM(HWND hwnd,WPARAM wP)  // Win32 changed
	{
	char bfn[MAX_PATH];
	bfn[0]=0;
	char * pbfn = NULL;
	// OFSTRUCT ofs,tofs;
	char tofs[MAX_PATH];

	switch(LOWORD(wP))
		{
		case IDM_QUIT:
			EditIMFUpdate(hwnd,FALSE);
			if(EditIMF.changed)
				{
				//bfn=GetOFSBaseName(&EditIMF.ofs);
				if(EditIMF.fn[0]){
					GetFileTitle(EditIMF.fn,bfn,MAX_PATH); // zzz 2/26
					pbfn=&bfn[0];
				}
				wsprintf(tbuf,"Save changes to %s",
					pbfn ? pbfn : (LPSTR)"(untitled)");
				switch(MessageBox(hwnd,tbuf,IMSName,
					MB_YESNOCANCEL|MB_ICONEXCLAMATION))
					{
					case IDYES:
						SendMessage(hwnd,WM_COMMAND,IDM_SAVE,0);
						if(EditIMF.changed) // if save failed
							return 0;       // don't quit
						break; // quit
					case IDNO:
						break; // quit
					case IDCANCEL:
						return 0; // don't quit
					}
				}
			if(EditIMF.aesetup)
				EditIMF.aesetup->closeedit();
			IMFFree(&EditIMF);
			SetMode(mFILES);
			return 0;
		case IDM_SAVE:
			{
				if(!EditIMFUpdate(hwnd,TRUE))
					return 0;
				//bfn=GetOFSBaseName(&EditIMF.ofs); // zzz 2/26
				pbfn = NULL;
				if (EditIMF.fn[0]!=0){
					GetFileTitle(EditIMF.fn,bfn,MAX_PATH);
					pbfn=&bfn[0];
				}
				if(!pbfn) // If untitled do save as instead
				{
					SendMessage(hwnd,WM_COMMAND,IDM_SAVEAS,0);
					return 0;
				}
				IMFSave(&EditIMF);
				return 0;
			}
		case IDM_SAVEAS:
			{
			if(!EditIMFUpdate(hwnd,TRUE))
				return 0;
			//bfn=GetOFSBaseName(&EditIMF.ofs);
			pbfn = NULL;
			bfn[0]=0;
			if (EditIMF.fn[0]!=0){
				GetFileTitle(EditIMF.fn,bfn,MAX_PATH);
				pbfn=&bfn[0];
			}
			else
				chddir(IMSFixtures);
			if(!DoPrompt(hwnd,"Save As",100,pbfn ? pbfn : ""))
				return 0;
			strfnextend(tbuf,"IMF");
			// Check for pre-existing file of same name
			HANDLE fh=CreateFile(tbuf,GENERIC_READ,0,NULL,OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,NULL);

			if(fh != INVALID_HANDLE_VALUE){
				if(MessageBox(hwnd,"Overwrite existing file?",
					IMSName,MB_YESNO|MB_ICONQUESTION)==IDNO)
					return 0;
				CloseHandle(fh);
			}

			// Create the file
			fh=CreateFile(tbuf,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,NULL);

			if(fh==INVALID_HANDLE_VALUE)
				{
				ShowError(IDE_CREATEIMF);
				return 0;
				}

			CloseHandle(fh);
			// save olf ofs in case of save failure
			lstrcpy(tofs,EditIMF.fn);   // zzz 2/26
			// Update IMF ofs for save
			lstrcpy(EditIMF.fn,tbuf);
			// Save the file
			if(!IMFSave(&EditIMF))
				{ // On error
				lstrcpy(EditIMF.fn,tofs); // Restore the old ofs   // zzz 2/26
				return 0;
				}
			// Update the caption
			char capt[MAX_PATH];
			GetFileTitle(EditIMF.fn,capt,MAX_PATH);  // zzz 2/26
			wsprintf(tbuf,"%s-ROTARY :FIXTURE EDITOR - %s",
				(LPSTR)IMSName,capt);
			SetWindowText(IMSWnd,tbuf);
			return 0;
			}
		case IDM_EDITAE: // Add or Edit AE
			if(!EditIMF.aesetup)
				{
				EditIMF.aesetup=new AESETUP();
				EditIMF.changed=TRUE;
				}
			if(EditIMF.aesetup)
				EditIMF.aesetup->openedit(hwnd,&IMSADWnd,&EditIMF,0.0f);
			return 0;
		case IDM_DELETEAE:
			if(EditIMF.aesetup)
				{
				if(MessageBox(hwnd,"Delete Alignment Extension",
					IMSName,MB_OKCANCEL|MB_ICONQUESTION)==IDCANCEL)
					return 0;
				EditIMF.changed=TRUE;
				EditIMF.aesetup->closeedit();
				delete EditIMF.aesetup;
				EditIMF.aesetup=NULL;
				}
			return 0;
		}
	return 0;
	}

/******************************************************************************
	ret = EditWMNotifyIDC(hwnd,wP,lP)

	Handles Control WM_NOTIFY messages for the IMFEDIT window.

	long ret = return value = 0
	HWND hwnd = Window receiving message.
	WORD idc = Control ID
	WORD ncode = Control notification code
******************************************************************************/
//Add support to RotoryIMF single nest	----sam	10/14/16
static long EditWMNotifyIDC(HWND hwnd,WPARAM wP,LPARAM lP){  // Win32 changed
	LPNMHDR pnmh = (LPNMHDR) lP;
	
	switch (pnmh->idFrom){
	case IDC_NESTS:
		switch (pnmh->code){
		case LVN_ITEMACTIVATE:			
			{
				int i,count;
				//					double v[5];
			    count=(int)ListView_GetItemCount(NestsWnd);
				count--;
				i=(int)ListView_GetNextItem(NestsWnd,-1,LVNI_SELECTED);
				
				if(count>=1)
					{
						EnableWindow(AddNestBtn,FALSE);
						EnableWindow(DelNestBtn,TRUE);
						EnableWindow(DelANestBtn,TRUE);
						SetFocus(NestsWnd);

						if(count==1 && i==0)
						{
							GetEditNEST(i);
							if(DoNestEdit(NestsWnd))
							{
								InsertEditNEST(i,TRUE);
							}
							ListView_SetItemState(NestsWnd,i,LVIS_SELECTED,LVIS_SELECTED);
						}

						return 0;
					}
					else if(count==0)
					{
						EnableWindow(AddNestBtn,TRUE);
						SetFocus(NestsWnd);
						if(i==count) // Convert click on last to Add nest
						{
							EditAddNestBNCLicked();
							return 0;
						}
						if(i==-1)
						{
							return 0;
						}

						GetEditNEST(i);
						if(DoNestEdit(NestsWnd))
						{
							InsertEditNEST(i,TRUE);
						}
						ListView_SetItemState(NestsWnd,i,LVIS_SELECTED,LVIS_SELECTED);

						EnableWindow(AddNestBtn,FALSE);
						EnableWindow(DelNestBtn,TRUE);
						EnableWindow(DelANestBtn,TRUE);
						SetFocus(NestsWnd);
						return 0;
					}
					return 0;
				}
				return 0;
			//}
			//return 0;
		}
		return 0;
	}
	return 0;
}

/******************************************************************************
	ret = EditWMCommandIDC(hwnd,wP,lP)

	Handles Control WM_COMMAND messages for the IMFEDIT window.

	long ret = return value = 0
	HWND hwnd = Window receiving message.
	WORD idc = Control ID
	WORD ncode = Control notification code
******************************************************************************/
static long EditWMCommandIDC(HWND hwnd,WPARAM wP,LPARAM lP)  // Win32 changed
	{
	WORD ncode=HIWORD(wP);
	int i;
	switch(LOWORD(wP))
		{
		case IDC_DESCRIPT:
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_DESCRIPT;
					return 0;
				case XN_TAB:
					SetFocus(OriginWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(IMWnd);
					return 0;
				}
			return 0;
		case IDC_ORIGIN:
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_ORIGIN;
					return 0;
				case XN_TAB:
					SetFocus(ExtentWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(DescWnd);
					return 0;
				}
			return 0;
		case IDC_EXTENT:
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_EXTENT;
					return 0;
				case XN_TAB:
					SetFocus(HideOptRotaryIMF ? NestsWnd : RotExtentWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(OriginWnd);
					return 0;
				}
			return 0;
		case IDC_EXTRT:          //--------SAM-10/05/2016
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_EXTRT;
					return 0;
				case XN_TAB:
					SetFocus(HideOptRotaryIMF ? NestsWnd : XYModeWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(ExtentWnd);
					return 0;
				}
			return 0;

		case IDC_XYMODE:
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_XYMODE;
					return 0;
				case XN_TAB:
					SetFocus(IsWindowEnabled(CCWPlusWnd) ?
						CCWPlusWnd : NestsWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(ExtentWnd);
					return 0;
				case CBN_SELCHANGE:
					i=SendMessage(XYModeWnd,CB_GETCURSEL,0,0);
					EnableWindow(CCWPlusWnd,i!=0);
					EnableWindow(RFDrvWnd,i!=0);
					return 0;
				}
			return 0;
		case IDC_CCWPLUS:
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_CCWPLUS;
					return 0;
				case XN_TAB:
					SetFocus(RFDrvWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(XYModeWnd);
					return 0;
				}
			return 0;
		case IDC_RFDRV:
			switch(ncode)
				{
				case XN_SETFOCUS:
					FocusID=IDC_RFDRV;
					return 0;
				case XN_TAB:
					SetFocus(NestsWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(CCWPlusWnd);
					return 0;
				}
			return 0;

		case IDC_NESTS:
			switch(ncode)
				{
				
				case XN_SETFOCUS:
					FocusID=IDC_NESTS;
					return 0;
				case XN_TAB:
					SetFocus(AddNestBtn);
					return 0;
				case XN_BKTAB:
					if(HideOptRotaryIMF)
						SetFocus(ExtentWnd);
					else if(IsWindowEnabled(RFDrvWnd))
						SetFocus(RFDrvWnd);
					else
						SetFocus(XYModeWnd);
					return 0;
				}
			return 0;
		case IDC_ADDNEST:
			switch(ncode)
				{
				case BN_CLICKED:
					return EditAddNestBNCLicked();
				case XN_SETFOCUS:
					FocusID=IDC_ADDNEST;
					return 0;
				case XN_TAB:
					SetFocus(IsWindowEnabled(DelNestBtn) ? DelNestBtn :
						IsWindowEnabled(AddGridBtn) ? AddGridBtn : IMWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(NestsWnd);
					return 0;
				}
			return 0;
		case IDC_DELNEST:
			switch(ncode)
				{
				case BN_CLICKED:
					return EditDelNestBNCLicked();
				case XN_SETFOCUS:
					FocusID=IDC_DELNEST;
					return 0;
				case XN_TAB:
					SetFocus(IsWindowEnabled(AddGridBtn) ?
						AddGridBtn : IMWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(IsWindowEnabled(DelNestBtn) ?
						DelNestBtn : AddNestBtn);
					SetFocus(AddNestBtn);
					return 0;
				}
			return 0;

        //---------------Linghai-11-4-08------------
		case IDC_DELANEST:
			switch(ncode)
				{
				case BN_CLICKED:
					return EditDelANestBNCLicked();
				case XN_SETFOCUS:
					FocusID=IDC_DELANEST;
					return 0;
				case XN_TAB:
					SetFocus(IsWindowEnabled(AddGridBtn) ?
						AddGridBtn : IMWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(IsWindowEnabled(DelANestBtn) ?
						DelANestBtn : AddNestBtn);
					SetFocus(AddNestBtn);
					return 0;
				}
			return 0;
		//-----------Linghai-11-4-08---

		case IDC_ADDGRID:
			switch(ncode)
				{
				case BN_CLICKED:
					return EditAddGridBNCLicked();
				case XN_SETFOCUS:
					FocusID=IDC_ADDGRID;
					return 0;
				case XN_TAB:
					SetFocus(IMWnd);
					return 0;
				case XN_BKTAB:
					SetFocus(IsWindowEnabled(DelNestBtn) ?
						DelNestBtn : AddNestBtn);
					return 0;
				}
			return 0;

		case IDC_IM:
			switch(ncode)
				{
				case IMN_SETFOCUS:
					FocusID=0;
					return 0;
				case IMN_TAB:
					SetFocus(DescWnd);
					return 0;
				case IMN_BKTAB:
					SetFocus(AddGridBtn);
					return 0;
				case IMN_ERROR:
					SendMessage(IMWnd,IMM_RESET,0,0);
					return 0;
				}
			if(AEWnd) // Relay IM notifications to AEWnd if open
				SendMessage(AEWnd,WM_COMMAND,wP,lP);
			return 0;
		}
	return 0;
	}

/******************************************************************************
	ret = EditAddNestsBNClicked()

	Handles BN_CLICKED notifications for the IMF editor add nest button.

	long ret = return value = 0
******************************************************************************/
//Add support to RotoryIMF single nest	----sam	10/14/16
static long EditAddNestBNCLicked(void)
	{
	int count,i;
//	double v[5];

	count=(int)ListView_GetItemCount(NestsWnd);
	count--; // Don't count blank last line

	if(count<IMFMAXNESTS)
		{
		InitEditNEST();
		if(DoNestEdit(NestsWnd))
			{
			//StringFromEditNEST(i);
			i=(int)ListView_GetNextItem(NestsWnd,-1,LVNI_SELECTED);
			InsertEditNEST(i,FALSE);
			ListView_SetItemState(NestsWnd,i+1,LVIS_SELECTED,LVIS_SELECTED);
			count++;
			}
		}

		/*if(count==1)
		{
			EnableWindow(DelNestBtn,TRUE);
			EnableWindow(DelANestBtn,TRUE);         //----------------Linghai-11-4-08-------
		}
		if(count==IMFMAXNESTS)
		{
			EnableWindow(AddNestBtn,FALSE);
			EnableWindow(AddGridBtn,FALSE);
			SetFocus(NestsWnd);
		}

		if(!HideOptRotaryIMF)
		{*/
			if(count>=1)
			{
				EnableWindow(AddNestBtn,FALSE);
				EnableWindow(DelNestBtn,TRUE);
				EnableWindow(DelANestBtn,TRUE);
				SetFocus(NestsWnd);
			}
			else
			{
				EnableWindow(AddNestBtn,TRUE);
				EnableWindow(DelNestBtn,FALSE);
				EnableWindow(DelANestBtn,FALSE);
				SetFocus(NestsWnd);
			}
		//}
	
	return 0;
	}

/******************************************************************************
	ret = EditDelNestsBNClicked()

	Handles BN_CLICKED notifications for the IMF editor del nest button.

	long ret = return value = 0
******************************************************************************/
//Add support to RotoryIMF single nest	----sam	10/14/16
static long EditDelNestBNCLicked(void)
	{
	int count;
	int nest1;
	int sel,firstsel;
	int selcount;
	WORD sellist[IMFMAXNESTS+1];
	WORD idx=0;

	//Get nests count and number of selected items
	count=(int)ListView_GetItemCount(NestsWnd)-1;
	selcount=(int)ListView_GetSelectedCount(NestsWnd);
	
	// No selection
	if(selcount==0)
		{
		EnableWindow(DelNestBtn,FALSE);
		EnableWindow(DelANestBtn,FALSE);     //-----------Linghai-11-4-08-------
		SetFocus(NestsWnd);
		return 0;
		}
	
	//Get first selected item 
	firstsel=(int)ListView_GetNextItem(NestsWnd,-1,LVNI_SELECTED);
	sel = firstsel;

	//Prepare list of items to delete
	for (int i=0;i<selcount;i++)
	{
		if (sel!=count)
		{
			sellist[idx++]=sel;
		}
		sel=(int)ListView_GetNextItem(NestsWnd,sel,LVNI_SELECTED);
	}

	//Delete
	for (int i=0;i<idx;i++)
	{
		int delitem = sellist[i]-i;
		ListView_SetItemState(NestsWnd,delitem,0,LVIS_SELECTED);
		ListView_DeleteItem(NestsWnd,delitem);
		count--;
	}

	if (count>=firstsel)
	{
		ListView_SetItemState(NestsWnd,firstsel,LVIS_SELECTED,LVIS_SELECTED);		
	}
	else
	{
		ListView_SetItemState(NestsWnd,0,LVIS_SELECTED,LVIS_SELECTED);
	}

	if(!count)
		{
		EnableWindow(DelNestBtn,FALSE);
		EnableWindow(DelANestBtn,FALSE);      //--------------Linghai-11-4-08-----
		SetFocus(NestsWnd);
		//return 0;
		}
	EnableWindow(AddNestBtn,TRUE);
	//EnableWindow(AddGridBtn,TRUE);


	/*if(HideOptRotaryIMF)
	{
		EnableWindow(AddNestBtn,TRUE);
		EnableWindow(AddGridBtn,TRUE);
	}
	else
	{*/
		count=(int)ListView_GetItemCount(NestsWnd)-1;
		if(count>=1)
		{
			EnableWindow(AddNestBtn,FALSE);
		}
		if(count==0)
		{
			EnableWindow(AddNestBtn,TRUE);
		}
		EnableWindow(AddGridBtn,FALSE);
	//}

	return 0;
	}

/******************************************************************************-----------Linghai-11-4-08---
	ret = EditDelANestsBNClicked()

	Handles BN_CLICKED notifications for the IMF editor del all nest button.

	long ret = return value = 0
******************************************************************************/
//Add support to RotoryIMF single nest	----sam	10/14/16
static long EditDelANestBNCLicked(void)
	{
	int count;
	int sel,firstsel;
	int selcount;
	WORD sellist[IMFMAXNESTS+1];
	WORD idx=0;

	//Get nests count and number of selected items
	count=(int)ListView_GetItemCount(NestsWnd)-1;

	if(count==0)
	{
		EnableWindow(AddNestBtn,TRUE);
	}

	//selcount=(int)ListView_GetSelectedCount(NestsWnd);
	selcount = count;        //----------------------Linghai-11-4-08---------

	// No selection
	if(selcount==0)
		{
		EnableWindow(DelNestBtn,FALSE);
		EnableWindow(DelANestBtn,FALSE);     //-----------Linghai-11-4-08-------
		SetFocus(NestsWnd);
		return 0;
		}

	//Get first selected item 
	//firstsel=(int)ListView_GetNextItem(NestsWnd,-1,LVNI_SELECTED);
	firstsel=(int)ListView_GetNextItem(NestsWnd,-1,LVNI_ALL);   //----------Linghai-11-4-08-----
	sel = firstsel;

	//Prepare list of items to delete
	for (int i=0;i<selcount;i++)
	{
		if (sel!=count)
		{
			sellist[idx++]=sel;
		}
		sel=(int)ListView_GetNextItem(NestsWnd,sel,LVNI_ALL);   //--------Linghai-11-4-08----
	}

	//Delete
	for (int i=0;i<idx;i++)
	{
		int delitem = sellist[i]-i;
		ListView_SetItemState(NestsWnd,delitem,0,LVIS_FOCUSED);//--------------Linghai-11-4-08---
		ListView_DeleteItem(NestsWnd,delitem);
		count--;
	}

	if (count>=firstsel)
	{
		//ListView_SetItemState(NestsWnd,firstsel,LVIS_SELECTED,LVIS_SELECTED);	
		ListView_SetItemState(NestsWnd,firstsel,LVIS_FOCUSED,LVIS_FOCUSED);  //--------Linghai-11-4-08---
	}
	else
	{
		//ListView_SetItemState(NestsWnd,0,LVIS_SELECTED,LVIS_SELECTED);
		ListView_SetItemState(NestsWnd,0,LVIS_FOCUSED,LVIS_FOCUSED);    //--------Linghai-11-4-08---
	}

	if(!count)
		{
		EnableWindow(DelNestBtn,FALSE);
		EnableWindow(DelANestBtn,FALSE);      //--------------Linghai-11-4-08-----
		SetFocus(NestsWnd);
		}
	EnableWindow(AddNestBtn,TRUE);
	//EnableWindow(AddGridBtn,TRUE);


	/*if(HideOptRotaryIMF)
	{
		EnableWindow(AddGridBtn,TRUE);
	}
	else
	{*/
		EnableWindow(AddGridBtn,FALSE);
	//}


	return 0;
	}

//----------------Linghai-11-4-08-----------------------------------------------
/******************************************************************************
	ret = EditAddGridBNClicked()

	Handles BN_CLICKED notifications for the IMF editor GRID button.

	long ret = return value = 0
******************************************************************************/
static long EditAddGridBNCLicked(void)
	{
	if(ListView_GetItemCount(NestsWnd) < IMFMAXNESTS)
		{
		DialogBox(IMSInst,"GRID",GetActiveWindow(),(DLGPROC)GRIDDP);
		}
	return 0;
	}

static void DoQty(HWND hwnd)
	{
	if(IsWindowEnabled(hwnd))
		{
		WORD count=RunIMF.nests.count;
		wsprintf(tbuf,"%d",count);
		if(DoPrompt(QtyBtn,"Enter the number of parts.",4,tbuf))
			{
			LONG l;
			ParseLong(tbuf,&l);
			if(l<=count)
				SendMessage(hwnd,IMFM_SETNLOADED,0,(LPARAM)l);
			}
		}
	}

static void DoAll(HWND hwnd)
	{
	if(IsWindowEnabled(hwnd))
		SendMessage(hwnd,IMFM_SETNLOADED,0,(LPARAM)RunIMF.nests.count);
	}

/*********************************************************************************
	ok = LoadNativeIMF(fh,pimf,ver)

	Loads the native IMF file into the IMF structure.

	HANDLE fh = File handle of open file.
	PIMP pimf = IMF structure to load.
	BOOL error = TRUE on error.
	int ver = 0 or an older version number (345)
*********************************************************************************/
static BOOL LoadNativeIMF(HANDLE fh,PIMF pimf,int ver)
	{
	int i;
	WORD nnests;
	LPSTR pbuf=(LPSTR)&pimf->origin;
	// Read the header
	if(ver==345) // 345 is missing rotation at end
		{
		_lread32(fh,pbuf,sizeof(IMFHEADER)-sizeof(double));
		((IMFHEADER *)pbuf)->rotation=0.0f;
		}
	else
		_lread32(fh,pbuf,sizeof(IMFHEADER));

	// Read nnests
	int rlen=_lread32(fh,(LPSTR)&nnests,sizeof(WORD));
	if(nnests>IMFMAXNESTS)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	// Allocate nests
	pimf->nests=IMFNESTS (nnests);
	if(!pimf->nests.nest && nnests>0)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}
	// Read the nest
	pimf->nests.count=nnests;
	_lread32(fh,(LPSTR)pimf->nests.nest,nnests*sizeof(IMFNEST));
	for(i=0;i<nnests;i++)
		pimf->nests.nest[i].flags=0;
	pimf->valssize=0; // vals allocated seperately
	pimf->pvalslist=NULL;
	pimf->changed=FALSE;
	
	// Check for Alignment Extension
	AESETUP aetemp;
	WORD aever;
	if(aetemp.readsig(fh,aever))
		{
		// Allocate aesetup and load it
		pimf->aesetup=new AESETUP();
		if(!pimf->aesetup)
			{
			IDError=IDE_NOMEM;
			return FALSE;
			}
		if(!pimf->aesetup->load(fh,aever))
			return FALSE;
		}
	return TRUE; // ok
	}

/*********************************************************************************
	hbrush = FlagBrush(flags)

	Returns the handle of the appropriate brush to paint a part
	given its flags.

	HBRUSH hbrush = Handle of brush matching flags
	WORD flags = flags to get brush for.
*********************************************************************************/
static HBRUSH FlagBrush(WORD flags)
	{
	return 
		flags & nbfOCCUPIED
			? flags & nbfSTARTED
				? flags & nbfMARKED
					? flags & nbfINSPECTED
						? flags & nbfPASSED
							? (flags & nbfCURED ? CuredPassedPartBrush : PassedPartBrush)
							: FailedPartBrush
						: (flags & nbfCURED ? CuredMarkedPartBrush : MarkedPartBrush)
					: StartedPartBrush
				: (flags & nbfCURED ? CuredPartBrush : PartBrush)
			: NoPartBrush;
	}

/*********************************************************************************
	ResetProgressNBFs(hwnd)

	Resets all nbf flags except nbfOCCUPIED for all parts, invalidating
	the nests for any that have the bits set on entry.

	HWND hwnd=IMFRUN window to get invalidation message.
*********************************************************************************/
static void ResetProgressNBFs(HWND hwnd)
	{
	IMFNEST *pnest;
	int i;
	for(i=0;i<RunIMF.nests.count;i++)
		{
		pnest=&(RunIMF.nests.nest[i]);
		if(pnest->flags & PROGRESSNBFS)
			{
			pnest->flags&=~PROGRESSNBFS;
			SendMessage(hwnd,IMFM_INVALIDATENEST,i,0);
			}
		}
	}

/******************************************************************************
	fsteps = CompileInspectPrep(pimp,pvals)

	Compiles a INSPECTPREP step for the current vals.

	PIMSTEPS fsteps = The compiled step NULL on error.
	PIMF pimp = The imp to compile.
	IMPVALS *pvals = The instance vals
******************************************************************************/
static PIMSTEP CompileInspectPrep(PIMP pimp,IMPVALS *pvals)
	{
	// Expand inspectprep to tbuf
	if(!IMPExpandTextToTbuf(pimp->pheader->inspectprep,&pimp->vars,pvals,
		&pimp->gvars,&pimp->gvals,pimp->pheader,&pimp->udhas))
		return 0;

	return IMPREPINSPECTORSTEPNew(tbuf);
	}

/******************************************************************************
	fsteps = CompileReadPrep(pimp,pvals)

	PIMSTEPS fsteps = The compiled step NULL on error.
	PIMF pimp = The imp to compile.
	IMPVALS *pvals = The instance vals
******************************************************************************/
static PIMSTEP CompileReadPrep(PIMP pimp,IMPVALS *pvals)
	{
	// Expand readprep to tbuf
	if(!IMPExpandTextToTbuf(pimp->pheader->readprep,&pimp->vars,pvals,
		&pimp->gvars,&pimp->gvals,pimp->pheader,&pimp->udhas))
		return 0;
	return IMPREPREADERSTEPNew(tbuf);
	}

/******************************************************************************
	PtInRectRotated
	Just like PtInRect except tests agains rectangle rotated about
	left top corner rotation degreees.  Actually rotats the point
	vector by -rotation and uses normal PtInRect for intersect test.
******************************************************************************/
BOOL PtInRectRotated(LPRECT r,POINT p,double rotation)
	{
	double s,c;
	s=sin(-rotation*DEG);
	c=cos(-rotation*DEG);
	p.x-=r->left;
	p.y-=r->top;
	int x=(int)(p.x*c-p.y*s);
	int y=(int)(p.x*s+p.y*c);
	p.x=x+r->left;
	p.y=y+r->top;
	return PtInRect(r,p);
	}

/******************************************************************************
	ComputeNestBounds
	Computes the bounding rectangle for a given rectangle rotated rotation
	degrees about its top left corner.
******************************************************************************/
static void ComputeNestBounds(LPRECT r,double rotation)
	{
	double s,c;
	POINT p[4];
	int i;

	s=sin(rotation*DEG);
	c=cos(rotation*DEG);

	// Compute Rotated Rectangle (Polygon)
	p[0].x=r->left;
	p[0].y=r->top;

	p[1].x=(int)(r->left+(r->right-r->left)*c);
	p[1].y=(int)(r->top +(r->right-r->left)*s);

	p[2].x=(int)(r->left+(r->right-r->left)*c-(r->bottom-r->top)*s);
	p[2].y=(int)(r->top +(r->right-r->left)*s+(r->bottom-r->top)*c);

	p[3].x=(int)(r->left-(r->bottom-r->top)*s);
	p[3].y=(int)(r->top +(r->bottom-r->top)*c);


	// Find bounding rect
	r->left=p[0].x;
	r->right=p[0].x;
	r->top=p[0].y;
	r->bottom=p[0].y;
	for(i=1;i<4;i++)
		{
		if(p[i].x<r->left)
			r->left=p[i].x;
		if(p[i].x>r->right)
			r->right=p[i].x;
		if(p[i].y<r->top)
			r->top=p[i].y;
		if(p[i].y>r->bottom)
			r->bottom=p[i].y;
		}
	}

// Gets the nest in NestWnd index i and parses into EditNEST
static void GetEditNEST(int i)
	{
	GetNestVect(tbuf,i);
	double v[5];
	ParseVect(tbuf,&v[0],UNVS);
	MScaleVect((LPDOUBLE)&v,4,TRUE);
	EditNEST.origin.x=v[0];
	EditNEST.origin.y=v[1];
	EditNEST.extent.x=v[2];
	EditNEST.extent.y=v[3];
	EditNEST.rotation=0.0f;
	//EditNEST.rotation=HideOptRotation ? 0.0f : v[4];

	//Add rotaryIMF support	----sam	10/14/16
	if(!HideOptRotaryIMF)
	{
	
	EditNEST.originrot.x=v[0];
	EditNEST.originrot.y=v[1];
	EditNEST.rotfixextent.d=v[2];
	v[3]=0.0f;
	v[4]=0.0f;
	}

	EditNEST.flags=0;
	}

// Insert the values in EditNEST as a string formatted for the locations list
void InsertEditNEST(int idx,BOOL bReplace)
	{
	double v[5];
	if(HideOptRotaryIMF)
	{
		v[0]=EditNEST.origin.x;
		v[1]=EditNEST.origin.y;
		v[2]=EditNEST.extent.x;
		v[3]=EditNEST.extent.y;
		v[4]=0.0f;
		if (bReplace)
			ListView_DeleteItem(NestsWnd,idx);
		InsertNest(v[0],v[1],v[2],v[3],v[4],idx);
	}
	else
	{
		v[0]=EditNEST.originrot.x;
		v[1]=EditNEST.originrot.y;
		v[2]=EditNEST.rotfixextent.d;
		v[3]=0.0f;
		v[4]=0.0f;
		if (bReplace)
			ListView_DeleteItem(NestsWnd,idx);
		InsertNest(v[0],v[1],v[2],v[3],v[4],idx);
	}
	}

static void InitEditNEST(void)
	{
//	EditNEST.init();
	}

// ok = DoNestEdit(parent) : edits EditNEST
static BOOL DoNestEdit(HWND parent)
	{
	BOOL ok;
	ok=DialogBox(IMSInst,"NEST",parent,(DLGPROC)NESTDP);
	return ok;
	}


static void DoNestLocate(HWND hdlg)
	{
	if(!StdLoc) return;
	// parse X Y fields to EditNEST
	if(!GetFloatField(hdlg,IDC_X,&EditNEST.origin.x)) return;
	if(!GetFloatField(hdlg,IDC_Y,&EditNEST.origin.y)) return;

	//setup frame
	IMF imf;
	GetWindowText(OriginWnd,tbuf,TSIZE);
	if(!ParseVect(tbuf,(LPDOUBLE)&(imf.origin),2)) return;
	
	GetWindowText(ExtentWnd,tbuf,TSIZE);
	if(!ParseVect(tbuf,(LPDOUBLE)&(imf.extent),3)) return;
	
	imf.rotation=0.0f; /// add when editable
	
	M3 fcs;
	if(!GetFCS(&fcs,&imf,FALSE)) return;
	// Find locations in FCS
	memcpy(&(StdLoc->locframe),&fcs,sizeof(M3));
	StdLoc->locpos.x=EditNEST.origin.x;
	StdLoc->locpos.y=EditNEST.origin.y;
	StdLoc->locpos.z=imf.extent.z;
	StdLocFocusX=GetDlgItem(hdlg,IDC_X);
	StdLocFocusY=GetDlgItem(hdlg,IDC_Y);
	StdLocFocusZ=0;
	StdLoc->locprompt="Part Location:";
	StdLoc->open(hdlg,&IMSADWnd); // do the locate
	// StdLoc auto updates IDC_X and IDC_Y on OK
	}

static void AddNest(double orgx,double orgy,double extx,double exty,double rot){
	char lbuf[100],sbuf[100],rbuf[50];

	//Add support for rotaryIMF	----sam	10/14/16
	if(HideOptRotaryIMF)
	{
		sprintf(lbuf,"%.4f,%.4f",orgx,orgy);
		sprintf(sbuf,"%.4f,%.4f",extx,exty);
		sprintf(rbuf,"%.4f",rot);
	}
	else
	{
	    sprintf(lbuf,"%.4f,%.4f",orgx,orgy);
		sprintf(sbuf,"%.4f",extx);
			sprintf(rbuf,"%.4f",rot);
	}
	int iItem=AddListViewItemWithImage(NestsWnd,(LPSTR)lbuf,0);
	ListView_SetItemText(NestsWnd,iItem,1,sbuf);
	ListView_SetItemText(NestsWnd,iItem,2,rbuf);
}

static void InsertNest(double orgx,double orgy,double extx,double exty,double rot,int idx){
	char lbuf[100],sbuf[100],rbuf[50];

	//Add support for rotaryIMF	----sam	10/14/16
	if(HideOptRotaryIMF)
	{
		sprintf(lbuf,"%.4f,%.4f",orgx,orgy);
		sprintf(sbuf,"%.4f,%.4f",extx,exty);
		sprintf(rbuf,"%.4f",rot);
	}
	else
	{
        sprintf(lbuf,"%.4f,%.4f",orgx,orgy);
		sprintf(sbuf,"%.4f",extx);
			sprintf(rbuf,"%.4f",rot);
	}
	int iItem=InsertListViewItemWithImage(NestsWnd,(LPSTR)lbuf,idx,0);
	ListView_SetItemText(NestsWnd,iItem,1,sbuf);
	ListView_SetItemText(NestsWnd,iItem,2,rbuf);
}

static void GetNestVect(LPSTR vect,int idx){
	char lbuf[100],sbuf[100],rbuf[50];
	ListView_GetItemText(NestsWnd,idx,0,lbuf,sizeof(lbuf));
	ListView_GetItemText(NestsWnd,idx,1,sbuf,sizeof(sbuf));
	ListView_GetItemText(NestsWnd,idx,2,rbuf,sizeof(rbuf));
	//Add support for rotaryIMF	----sam	10/14/16
	if(HideOptRotaryIMF)
	{
		lstrcpy(vect,lbuf);
		lstrcat(vect,",");
		lstrcat(vect,sbuf);
		lstrcat(vect,",");
		lstrcat(vect,rbuf);
	}
	else
	{
	    lstrcpy(vect,lbuf);
		lstrcat(vect,",");
		lstrcat(vect,sbuf);
		lstrcat(vect,",");
		lstrcat(vect,rbuf);
	}
}


// Initializes the default grid
GRID::GRID()
	{
	order.x=3;
	order.y=3;
	origin.x=0.0f;
	origin.y=0.0f;
	size.x=0.0f;
	size.y=0.0f;
	spacing.x=1.0f;
	spacing.y=1.0f;
	rotation=0.0f;
	}

// ok = GRID.Update(HWND hdlg,int count)
// Updates this grid from the GRID dialog controls
BOOL GRID::Update(HWND hdlg,int count)
	{
	// IDC_GORDER
	P2 go;
	tlen=GetDlgItemText(hdlg,IDC_GORDER,tbuf,TSIZE);
	if(!ParseVect(tbuf,(LPDOUBLE)&go,2))
		{
		ShowError(IDE_BADLOCATION);
		SetFocus(GetDlgItem(hdlg,IDC_GORDER));
		return FALSE;
		}
	order.x=(int)go.x;
	order.y=(int)go.y;
	if(order.x*order.y+count>IMFMAXNESTS)
		{
		ShowError(IDE_GRIDTOOBIG);
		SetFocus(GetDlgItem(hdlg,IDC_GORDER));
		return FALSE;
		}

	// IDC_GORG
	tlen=GetDlgItemText(hdlg,IDC_GORG,tbuf,TSIZE);
	if(!ParseVect(tbuf,&origin.x,2))
		{
		ShowError(IDE_BADLOCATION);
		SetFocus(GetDlgItem(hdlg,IDC_GORG));
		return FALSE;
		}
	MScaleVect(&origin.x,2,TRUE);

	// IDC_GNSIZE
	tlen=GetDlgItemText(hdlg,IDC_GNSIZE,tbuf,TSIZE);
	if(!ParseVect(tbuf,&size.x,2))
		{
		ShowError(IDE_BADLOCATION);
		SetFocus(GetDlgItem(hdlg,IDC_GORG));
		return FALSE;
		}
	MScaleVect(&size.x,2,TRUE);

	// IDC_GNSPACING
	tlen=GetDlgItemText(hdlg,IDC_GNSPACING,tbuf,TSIZE);
	if(!ParseVect(tbuf,&spacing.x,2))
		{
		ShowError(IDE_BADLOCATION);
		SetFocus(GetDlgItem(hdlg,IDC_GNSPACING));
		return FALSE;
		}
	MScaleVect(&spacing.x,2,TRUE);

	// IDC_ROT
	if(HideOptRotation)
		rotation=0.0f;
	else
		{
		tlen=GetDlgItemText(hdlg,IDC_ROT,tbuf,TSIZE);
		if(!ParseVect(tbuf,&rotation,1))
			{
			ShowError(IDE_BADLOCATION);
			SetFocus(GetDlgItem(hdlg,IDC_ROT));
			return FALSE;
			}
		}

	/*/ IDC_DEG	----sam	10/7/16
	if(HideOptRotaryDEG)
		degree=0.0f;
	else
		{
		tlen=GetDlgItemText(hdlg,IDC_DEG,tbuf,TSIZE);
		if(!ParseVect(tbuf,&degree,1))
			{
			ShowError(IDE_BADLOCATION);
			SetFocus(GetDlgItem(hdlg,IDC_DEG));
			return FALSE;
			}
		}*/

	return TRUE;
	}

// Updates the GRID Dialog controls from the grid
void GRID::UpdateDisplay(HWND hdlg)
	{
	P2 p;

	wsprintf(tbuf,"%d x %d",order.x,order.y);
	SetDlgItemText(hdlg,IDC_GORDER,tbuf);

	p=origin;
	MScaleVect(&p.x,2,FALSE);
	VectToString(tbuf,&p.x,2,",");
	SetDlgItemText(hdlg,IDC_GORG,tbuf);

	p=size;
	MScaleVect(&p.x,2,FALSE);
	VectToString(tbuf,&p.x,2,",");
	SetDlgItemText(hdlg,IDC_GNSIZE,tbuf);

	p=spacing;
	MScaleVect(&p.x,2,FALSE);
	VectToString(tbuf,&p.x,2,",");
	SetDlgItemText(hdlg,IDC_GNSPACING,tbuf);

	if(!HideOptRotation)
		{
		VectToString(tbuf,&rotation,1,",");
		SetDlgItemText(hdlg,IDC_ROT,tbuf);
		}
	/*/rotary degree	----sam 10/7/16
	if(!HideOptRotaryDEG)
		{
		VectToString(tbuf,&degree,1,",");
		SetDlgItemText(hdlg,IDC_DEG,tbuf);
		}*/

	}

void GRID::DoLocate(HWND hdlg)
	{
	if(!StdLoc) return;

	//setup frame
	IMF imf;
	GetWindowText(OriginWnd,tbuf,TSIZE);
	if(!ParseVect(tbuf,(LPDOUBLE)&(imf.origin),2)) return;
	GetWindowText(ExtentWnd,tbuf,TSIZE);
	if(!ParseVect(tbuf,(LPDOUBLE)&(imf.extentrot),2)) return;
	imf.rotation=0.0f; /// add when editable
	M3 fcs;
	if(!GetFCS(&fcs,&imf,FALSE)) return;
	// Find locations in FCS
	memcpy(&(StdLoc->locframe),&fcs,sizeof(M3));
	StdLoc->locpos.x=origin.x;
	StdLoc->locpos.y=origin.y;
	StdLoc->locpos.z=imf.extent.z;
	StdLocFocusX=GetDlgItem(hdlg,IDC_GORG);
	StdLocFocusY=GetDlgItem(hdlg,IDC_GORG);
	StdLocFocusZ=0;

	StdLoc->locprompt="Grid Origin:";
	StdLoc->open(hdlg,&IMSADWnd); // do the locate
	// StdLoc auto updates IDC_GORG
	}

void GRID::DoSize(HWND hdlg)
	{
	if(!StdLoc) return;

	//setup frame
	IMF imf;
	GetWindowText(OriginWnd,tbuf,TSIZE);
	if(!ParseVect(tbuf,(LPDOUBLE)&(imf.origin),2)) return;
	GetWindowText(ExtentWnd,tbuf,TSIZE);

	//--sam 10/6/16
	if(HideOptRotaryIMF)
		if(!ParseVect(tbuf,(LPDOUBLE)&(imf.extentrot),2)) return;
	if(!HideOptRotaryIMF)
	if(!ParseVect(tbuf,(LPDOUBLE)&(imf.extent),2)) return;

	imf.rotation=0.0f; /// add when editable
	M3 fcs;
	if(!GetFCS(&fcs,&imf,FALSE)) return;
	// Find locations in FCS
	memcpy(&(StdLoc->locframe),&fcs,sizeof(M3));
	StdLoc->locpos.x=origin.x+(order.x-1)*spacing.x;
	StdLoc->locpos.y=origin.y+(order.y-1)*spacing.y;
	StdLoc->locpos.z=imf.extent.z;
	StdLocFocusX=GetDlgItem(hdlg,IDC_GNSPACING);
	StdLocFocusY=GetDlgItem(hdlg,IDC_GNSPACING);
	StdLocFocusZ=0;
	StdLoc->locprompt="Total Grid Spacing:";
	StdLoc->open(hdlg,&IMSADWnd); // do the locate
	// StdLoc auto updates IDC_GNSPACING with incorrect value
	P3 p=StdLoc->locpos;
	if(order.x-1)
		p.x=(p.x-origin.x)/(order.x-1);
	else
		p.x=0.0f;
	if(order.y-1)
		p.y=(p.y-origin.y)/(order.y-1);
	else
		p.y=0.0f;
	spacing.x=p.x;
	spacing.y=p.y;
	UpdateDisplay(hdlg);
	}
