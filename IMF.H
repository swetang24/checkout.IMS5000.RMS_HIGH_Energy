#ifndef _IMF_H
#define _IMF_H

/***************************************************************************
	NAME : IMF.H
	DESC : Header for IMF windows and structures .
***************************************************************************/

/***************************************************************************
	PUBLIC DEFINES
***************************************************************************/
// IMF Run Window Messages
#define IMFM_SETNLOADED     WM_USER    // lP = nloaded
#define IMFM_QTY            WM_USER+1  // same as clicking Qty button
#define IMFM_ALL            WM_USER+2  // same as clicking All button
#define IMFM_INVALIDATENEST WM_USER+3  // wP=nesti : force nest redraw

// IMF Run Window Notifications
#define IMFN_SETFOCUS 1000
#define IMFN_TAB      1001
#define IMFN_BKTAB    1002
#define IMFN_CHANGED  1003

// Sizes
#define IMFMAXNESTS 1000  // Maximum number of nests in a fixture. (1000 is crazy big : need to add grid object for these)
#define IMFLDESC      63  // Length of header.Desc
#define IMFLRFDRV     32  // Max length of RF driver name

// IMFNEST bit flags
#define nbfOCCUPIED  0x01 // nest is occupied
#define nbfSTARTED   0x02 // part in nest marking started
#define nbfMARKED    0x04 // part in nest has been marked
#define nbfINSPECTED 0x08 // part in nest has been inspected
#define nbfPASSED    0x10 // part in nest has passed inspection
#define nbfCURED     0x20 // part in nest has been cured
#define nbfLOCPASS1	 0X0100 // part in nest has been located1
#define nbfLOCPASS2	 0X0200 // part in nest has been located2
#define nbfLOCFAIL1	 0X0400 // part in nest has been located1
#define nbfLOCFAIL2	 0X0800 // part in nest has been located2

// Combined NBFs that indicate progress not occupation
#define PROGRESSNBFS (nbfSTARTED|nbfMARKED|nbfINSPECTED|nbfPASSED|nbfCURED)

#define VERIFYNEEDED(f)	 (((f)&nbfOCCUPIED) && ( (((f)&nbfLOCFAIL1)||((f)&nbfLOCFAIL2)) || (!((f)&nbfLOCPASS1) && !((f)&nbfLOCPASS1)) ))
#define INSPECTNEEDED(f) (((f)&nbfOCCUPIED) && !((f)&nbfINSPECTED))
#define STARTNEEDED(f)   (((f)&nbfOCCUPIED) && !((f)&nbfSTARTED))
#define MARKNEEDED(f)    (((f)&nbfOCCUPIED) && !((f)&nbfMARKED))
#define OCCUPIED(f)      ((f)&nbfOCCUPIED)
#define CURENEEDED(f)    (((f)&nbfOCCUPIED) && !((f)&nbfCURED))

extern bool xyzttmpbuffer; //my test var

/***************************************************************************
	PUBLIC TYPES
***************************************************************************/
#pragma warning(disable : 4200) // disable warnings about empty array in struct

class IMFNEST
	{
public:
	friend class IMFNESTS;
	
	P2 origin;
	double diameter;
	double rotation; // Frame Rotation about it origin CW DEG
	double degree;	// Frame Rotation Degree	----sam	10/7/16
	P2 extent;  // 0 component maps to partSize
	P2 originrot;  //sam 1/18/2017
	P4 rotfixextent;  //sam 1/18/2017

	WORD flags; // nest bit flags
	IMFNEST();     // Fill in members with defaults

	BOOL readnative(HANDLE fh, WORD ver);
	BOOL write(HANDLE fh);
	BOOL translateVers345Thru357(HANDLE fh);
	BOOL translateVers330Thru338(HANDLE fh);
	BOOL translateOriginalVersion(HANDLE fh);
private:
	BOOL translate(HANDLE fh, WORD ver);

	};

class IMFNESTS
	{
	public:
	WORD count;      // number of nests
	WORD capac;      // current capacity
	IMFNEST * nest; // count IMFNEST structs
	IMFNESTS(const IMFNESTS & src);
	IMFNESTS& operator= (const IMFNESTS & src);
	IMFNESTS();
	IMFNESTS(int initCapac);
	~IMFNESTS();
	
	BOOL realloc(int ncount);
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver, WORD count);
private:
	BOOL readnative(HANDLE fh, WORD ver, WORD count);
	BOOL translate(HANDLE fh, WORD ver, WORD count);
	void free();
};

class IMFHEADER
	{
public:
	P2 origin;             // The fixture origin
	P3 extent;             // The fixtures outer dimensions.
	P4 extentrot;			// The rotary fxtures dimension.		----sam
	P2 originrot;  //sam 1/18/2017
	char desc[IMFLDESC+1]; // Description text
	IMXYMODE xymode;           // XY or RF mode = XY|XS|SY
	//IMLRMODE lrmode;   // ----------sam
	BOOL ccwplus;              // RF Direction control if XS|SY
	char rfdrv[IMFLRFDRV+1];   // RF Driver if XS|SY
	double rotation;        // Frame Rotation about it origin CW DEG
	double deg;			//rotary degree	----sam
	IMFHEADER();     // Fill in header members with defaults
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver);
private:
	BOOL readnative(HANDLE fh, WORD ver);
	BOOL translate(HANDLE fh, WORD ver);
	BOOL translateVers345Thru357(HANDLE fh, WORD ver);
	BOOL translateVer338(HANDLE fh);	
	BOOL translateOriginalVersion(HANDLE fh);
	};

class IMF : public IMFHEADER
	{
public:
	IMFNESTS nests;			// The nests
	WORD valssize;			// Size of each vals in bytes
	IMPVALS* pvalslist;		// each vals (p(nests)->count*valssize bytes)
	IMP imp;				// The imp being marked in each frame
	BOOL changed;			// Changed flag
	char fn[MAX_PATH];		// The file
//	OFSTRUCT ofs;			// zzz
	AESETUP *aesetup;		// Per IMF Alignment setup or NULL if none.
	AEINFO *aeinfo;			// Per IMF or IMP Alignment results info

	IMF();
	void init();
	};

typedef enum {vmINSPECTOR,vmREADER,vmLOCATOR} VISMODE;

/***************************************************************************
	PUBLIC VARIABLES
***************************************************************************/
extern IMF RunIMF;
extern IMF EditIMF;
extern IMFNEST EditNEST;
extern BOOL RefRightEdge;  // Causes IMFs to ref right edge instead of left
extern BOOL RefBottomEdge; // Causes IMFs to ref bottom edge instead of top
extern BOOL ShowRefEdge;   // Causes IMF painting to display ref edge
extern HPEN RefEdgePen;    // Pen used to draw ref edge if ShowRefEdge
extern double zlast;       // Last z position found during IMPCompile
extern double zsafe;        // TCS safe height for xy travel
extern double zmarker;      // WCS marker height
extern double zvis;         // WCS reader/inspecor height

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
BOOL IMFNew(PIMF pimf);
BOOL IMFAllocVals(PIMF pimf);
IMPVALS* IMFGetNestVals(PIMF pimf,WORD nest);
BOOL IMFInitVals(PIMF pimf);
void IMFNextVals(PIMF pimf);
void IMFPropogateVals(PIMF pimf,int ni,int vi);
BOOL IMFLoad(PIMF pimf);
BOOL IMFSave(PIMF pimf);
void IMFFree(PIMF pimf);
HWND IMFOpenRun(HWND parent,WORD id);
HWND IMFOpenEdit(HWND parent,WORD id,int x,int y,int w,int h);
PIMSTEP IMFCompile(PIMF pimf);
PIMSTEP IMFCompileConveyor();
PIMSTEP  IMFCompileReads(PIMF pimf);
PIMSTEP  IMFCompileLocates(PIMF pimf);
PIMSTEP  IMFCompileVPosMove(PIMF pimf,int nesti,VISMODE vismode);
int IMFLoadDesc(char *fname,char *fdesc);
BOOL IMFSigMatch(LPSTR sig);
void ConveyorInit(void);
void ConveyorQuit(void);
BOOL ConveyorEmpty(void);
void ConveyorStopReq(void);
void RefEdgeInit(void);
BOOL GetNestWCS(P3 *nestwcs,PIMF imf,int nesti);
BOOL GetFCS(M3 *fcs,PIMF pimf,BOOL aligned);
BOOL GetNCS(M3 *ncs,PIMF pimf,IMFNEST *pnest,int ni,BOOL aligned);
BOOL GetIMF(PIMF imf,LPSTR name);
BOOL GetPWCS(LPM3 pwcs,PIMF pimf,IMPHEADER *h,int nesti);
BOOL GetMWCSZ(LPM3 mwcs,LPDOUBLE z,PIMF imf,int ni,int mi);
BOOL GetIMFIMP(PIMF imf,LPSTR caldir,LPSTR calimp);
void DebugMatrix(LPM3 m);

#endif // _IMF_H
