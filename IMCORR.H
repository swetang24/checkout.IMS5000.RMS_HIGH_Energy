#ifndef _IMCORR_H
#define _IMCORR_H
/******************************************************************************
	Name : IMCORR
	Desc : IM Corections for IMS5000
******************************************************************************/
#define LSIGIMC		(sizeof(sigIMC)-1)
#define LSIGIMCV368	(sizeof(sigIMCV368)-1)

const char sigIMC[]="JETIMCORRV400\x1a"; // IM Correction Extension
const char sigIMCV368[]="JETIMCORRV368\x1a"; 

extern HWND IMCWnd;

class IMCORR : public IMCORRECTION// IMP correction parameters
	{
	// Memory only members
	HWND hwnd; // editor dialog when open : 0 when closed
	HWND hparent; // editor parent window
	double EditXScale;  // Metric display scaling factor X or 1.0
	double EditYScale;  // Metric display scaling factor Y or 1.0
	HWND *pADWnd; // Apps active dialog hwnd
	void closeedit(void);
	BOOL DP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	void updateenables(HWND hdlg);
	BOOL changed;
	int newid;
public:
	IMCORR();
	~IMCORR();
	static BOOL writesig(HANDLE fh);
	BOOL readsig(HANDLE fh,WORD & ver);
	BOOL read(HANDLE fh,WORD ver);
	BOOL save(HANDLE fh);
	BOOL edit(HWND parent,int mid,double EditXScale,double EditYScale);
	static BOOL CALLBACK IMCORRDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP);
	int id;   // the mark id that this object belongs to
	void init(void);
	};

#endif // _IMCORR_H
