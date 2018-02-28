#ifndef _AE_H
#define _AE_H

/******************************************************************************
	Name : AE
	Desc : Alignment Extensions for IMS5000
******************************************************************************/
#define LSIGAE		(sizeof(sigAE)-1)
#define LSIGAEV357	(sizeof(sigAEV357)-1)


const char sigAE[]=		"JETAEV400\x1a"; // Alignment Extension
const char sigAEV357[]=	"JETAEV357\x1a"; 

extern HWND AEWnd;

struct AEINFO;
typedef struct AEFILESETUP *pAEFILESETUP;
struct AEFILESETUP
	{
	// sig in file
	int locateStatus[2];			// used for pass/fail etc.
	char locator[2][IMPLFNBASE+1];     // Locator or "" for no locate
	char processtype[2][IMPLFNBASE+1]; // Process or "" for no Process
	char locateprep[2][IMPLVISPREP+1]; // optional prep expression or ""					//-----------d.dixon-11-21-13-- locateprep array created
	P3 refpos[2];                   // 2 reference positions, filled in at edit
    //order or nacqpos,nrefpos is due to int to 2 byte extension in 3.65
	char nacqpos;                   // n locate acquire positions
	char nrefpos;                   // 1 or 2 point alignement (also 0==2)
	P3 acqpos[2];                   // 0..2 acquire positions for camera
	double maxrotation;				// max rotation delta between ref and vis
	double maxlendelta;				// max length difference between ref and vis
	BOOL maskxrot;					// if true x axis rotations in results are removed
	BOOL maskyrot;					// if true y axis rotations in results are removed
	BOOL adjusta;					// adjust a if true, adjust b normally
	//pAEFILESETUP prevAE;			// previous linked AE									//-----------d.dixon-11-14-13--
	//pAEFILESETUP nextAE;			// next linked AE										//-----------d.dixon-11-14-13--
	};
	//pAEFILESETUP startAE = NULL;	// start of linked AE's									//-----------d.dixon-11-14-13--

class AESETUP : public AEFILESETUP
	{
public:
	// Memory only members
	HWND hwnd; // editor dialog when open : 0 when closed
	HWND hparent; // editor parent window
	HWND *pADWnd; // Apps active dialog hwnd
	BOOL imok; // True if im is ready during editing
	BOOL aeok; // True if ae is found and ready during editing
	BOOL injog; // True if joging
	int jogindex; // indicates which location jog is to digitize
	M3 irtm;    // Inverse Robot Transform for XY coordinates from JOG/LOCATE
	double irtz; // Inverse Robot Transform for Z coordinates from JOG/LOCATE
	M3 frame;   // FCS or NCS frame while editing
	double rotation; // WCS theta extracted from frame in DEG
	PIMSTEP movesteps; // move camera step sequence
	double partz;       // part z height
	PIMF pimf;         // the imf during edit
	int nesti;         // nest index or -1 if FCS
	PIMSTEP learnsteps; // learn align steps
	AESETUP();
	~AESETUP();
	PIMSTEP compileprep(PIMP pimp,IMPVALS *pvals,LPSTR preptext);
	PIMSTEP compile(PIMF pimf,int ni=-1,IMPVALS *pvals=NULL);
	LPM3 getxm(LPM3 xmt,LPM3 xmr,AEINFO *info,int ni);
	LPM3 getxm1pt(LPM3 xm,AEINFO *info,int ni);
	LPM3 getxm2pt_1(LPM3 xmt,LPM3 xmr,AEINFO *info,int ni);
	LPM3 getxm2pt_2(LPM3 xmt,LPM3 xmr,AEINFO *info,int ni);
	BOOL save(HANDLE fh);
	BOOL load(HANDLE fh,WORD ver);
	BOOL readsig(HANDLE fh,WORD & ver);
	HWND openedit(HWND parent,HWND *padwnd,PIMF pimf,double pz,int ni=-1);
	void closeedit(void);
	static BOOL CALLBACK AESETUPDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	BOOL DP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	void updateenables(HWND hdlg);
	void getvisobjects(HWND hIDC, char location[]);
private:
	PIMSTEP prepsteps;
	};

#pragma warning(disable : 4200) // disable warnings about empty array in struct
struct AEINFO
	{
	int nnests; // nnests==0 for IMF 1|2pt align : >0 for NEST nnests 1|2 pt aligns
	P3 pos[2];   // found positions // added 2x array d.dixon 1/15/14
	};

#endif // _AE_H
