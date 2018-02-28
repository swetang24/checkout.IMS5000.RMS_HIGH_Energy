#ifndef _CAL_H
#define _CAL_H

/******************************************************************************
	Name : CAL
	Desc : Calibrations for IMS5000
******************************************************************************/
#define ACSEQMAX 5

class CALC;

extern HWND CALWnd;
extern CALC *TheCal;
extern BOOL CalMarked; // Set on any start and cleared on IJ cal update

typedef enum {calOFFSET,calTHETA} CALTYPES;

class CALC
	{
	char locator[IMPLFNBASE+1]; // VIS being calibrated or being used for IJcal
	char prep[IMPLVISPREP+1];   // optional prep expression or ""
	CALTYPE caltype;            // Calibration type
	char calimp[IMPLFNBASE+5];  // "" | Basename of imp that must be marked
	char caldir[128];           // Programs dir for calimp if calimp[0]
	double caltheta;             // Theta angle for theta calibrations
	BOOL updatez;               // True if cal should update zbase
	int calvlen;                // Length of display vectors 2 or 3
	P3 calpos;                  // known wcs position of the ref feature
	P3 locpos;                  // located position of feature
	HWND hwnd;                  // cal dialog when open : 0 when closed
	HWND hparent;               // editor parent window
	HWND *pADWnd;          // Apps active dialog hwnd
	FARPROC dp;                 // ProcInstance for CALDP when open
	BOOL imok;                  // True if im is ready during editing
	BOOL visok;                  // True if vis is found and ready during dialog
	BOOL calok;                 // This cal itself is ok (no init problems)
	M3 irtm;                    // Inverse Robot Transform for XY coordinates from LOCATE
	double irtz;                 // Inverse Robot Transform for Z coordinates from LOCATE
	int cursel;                 // current callist selection index
	PIMSTEP prepsteps;         // pre live prep step
	PIMSTEP locsteps;          // locate step sequence
	PIMSTEP marksteps;
	BOOL SkipM1;               // True if locsteps skips 1sWMOVE
	char destsect[32];         // IM5000.INI dest section name
	char destkey[32];          // IM5000.INI dest key name
	BOOL isvis;                // TRUE if the cal is a VIS instead of an IJ
	WORD updatelevel;          // min user level required for update
	BOOL destroy;              // !0 defers DestroyWindow until IMN_CANCEL
	                           //   allows IM to cancel before cal frees steps
	int acseq[ACSEQMAX];       // auto calibrate sequence (IDC_CALLIST indices)
	int acseqlen;              // length of acseq
	BOOL autocal;              // True when in autocal sequence
	BOOL vislamp;              // True if visionlamp should be on during this cal
	char instructions[256];    // Instructive text from ini
public:
	CALC();
	~CALC();
	HWND open(HWND parent,HWND *padwnd);
	void close(void);
	void imnlocate(LPP3 ptcs);
private:
	void init(LPSTR calname);
	static BOOL CALLBACK CALDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	BOOL CALLBACK DP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	void updateenables(HWND hdlg);
	PIMSTEP CALCompileLocate(void);
	PIMSTEP compile(void);
	BOOL loadcallist(HWND cb);
	void savecalhist(void);
	void writeprofilevect(LPSTR sect,LPSTR key,LPDOUBLE v,int vlen,LPSTR ini);
	BOOL loadautocal(void);
	BOOL autocalnext(void);
	void mark(void);
	void update(HWND hdlg);
	void correcttheta(LPDOUBLE cv,LPP3 e);
	};

#endif // _CAL_H
