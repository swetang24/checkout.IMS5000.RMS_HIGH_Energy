/******************************************************************************
	Name : AE
	Desc : Alignment Extensions for IMS5000
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <memory.h>
#include <math.h>
#include <io.h>
#include <imslib32/imslib32.h>
#include <im5000/im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "run.h"
#include "clamp.h"
#include "ae.h"
#include "imsio.h"

HWND IMCWnd=0; // Set by IMCORR on openedit to its hwnd
              // Cleared by editors when they close
              // Allows app to track single window open state
              // and to relay im notifications.

IMCORR::IMCORR()
	{
	memset(this,0,sizeof(IMCORR));
	id=-1;
	}

IMCORR::~IMCORR()
	{
//	if(editdp)
//		FreeProcInstance(editdp);
	}

void IMCORR::init(void)
	{
	id=-1;
	fs=0;
	fo.v2[0]=0;
	fo.v2[1]=0;
	rs=0;
	ro.v2[0]=0;
	ro.v2[1]=0;
	}

// ok=save(fh) : saves this on fh (does not close unless error)
BOOL IMCORR::writesig(HANDLE fh)
	{
	if(_lwrite32(fh,sigIMC,LSIGIMC)!=LSIGIMC) // Write the signiture
		{
		CloseHandle(fh);
		ShowError(IDE_WRITEFILE);
		return FALSE;
		}
	return TRUE;
	}

// ok=save(fh) : saves this on fh (does not close unless error)
BOOL IMCORR::save(HANDLE fh)
	{
	// Write the correction
	// id
	if(!writeInt(fh,&id))
		return FALSE;
	// fs,rs
	if(!writeDouble(fh,&fs))
		return FALSE;
	if(!writeDouble(fh,&rs))
		return FALSE;
	// fo,ro
	if(!writeDouble(fh,&fo.v2[0]))
		return FALSE;
	if(!writeDouble(fh,&fo.v2[1]))
		return FALSE;
	if(!writeDouble(fh,&ro.v2[0]))
		return FALSE;
	if(!writeDouble(fh,&ro.v2[1]))
		return FALSE;
	return TRUE;	
	}
	

// ok=save(fh) : saves this on fh (does not close unless error)
BOOL IMCORR::read(HANDLE fh,WORD ver)
	{
//	int rlen;
//	rlen=sizeof(id);               // Read it
//	if(_lread32(fh,(LPSTR)&(id),rlen)!=rlen)
//		return FALSE;
//	rlen=sizeof(IMCORRECTION);               
//	if(_lread32(fh,(LPSTR)&(fs),rlen)!=rlen)
//		return FALSE;

	// Read in the correction data
	if (ver==0){  // Native
		// id
		if(!readInt(fh,&id))
			return FALSE;
		// fs,rs
		if(!readDouble(fh,&fs))
			return FALSE;
		if(!readDouble(fh,&rs))
			return FALSE;
		// fo,ro
		if(!readDouble(fh,&fo.v2[0]))
			return FALSE;
		if(!readDouble(fh,&fo.v2[1]))
			return FALSE;
		if(!readDouble(fh,&ro.v2[0]))
			return FALSE;
		if(!readDouble(fh,&ro.v2[1]))
			return FALSE;
	} else {  // V 3.68 - Win16
		// id
		if(!readInt(fh,&id,TRUE))
			return FALSE;
		// fs,rs
		if(!readDouble(fh,&fs,TRUE))
			return FALSE;
		if(!readDouble(fh,&rs,TRUE))
			return FALSE;
		// fo,ro
		if(!readDouble(fh,&fo.v2[0],TRUE))
			return FALSE;
		if(!readDouble(fh,&fo.v2[1],TRUE))
			return FALSE;
		if(!readDouble(fh,&ro.v2[0],TRUE))
			return FALSE;
		if(!readDouble(fh,&ro.v2[1],TRUE))
			return FALSE;
	}
	return TRUE;
	}

// ok = readsig(fh) : reads the sig and returns ok : or not
BOOL IMCORR::readsig(HANDLE fh,WORD & ver)
	{
	UINT cr;
	char sig[LSIGIMC]="";
	cr=_lread32(fh,sig,LSIGIMC);
	if(cr==HFILE_ERROR)
		return FALSE;
	if(memcmp(sig,sigIMC,LSIGIMC)==0){
		ver=0;
		return TRUE;
	} else if (memcmp(sig,sigIMCV368,LSIGIMCV368)==0){
		ver=368;
		return TRUE;
	}
	long sl=cr;
	sl*=-1;
	_llseek32(fh,sl,FILE_CURRENT);
	return FALSE;
	}

BOOL IMCORR::edit(HWND parent,int mid,double EditXScale,double EditYScale)
	{
	this->EditXScale=EditXScale;
	this->EditYScale=EditYScale;
	
	changed=FALSE;
    newid=mid;
	DialogBoxParam(IMSInst,"IMCORRECTION",parent,(DLGPROC)IMCORRDP,(LPARAM)this);
	return changed;
	}
	
// Closes the editor dialog for this IMCORR if its open
void IMCORR::closeedit(void)
	{
	if(hwnd)
		DestroyWindow(hwnd);
	hwnd=0;
	AEWnd=0;
	}

// IMCORR::IMCORRDP(...) Win level dialog proc for IMCORR
// STATIC
BOOL CALLBACK IMCORR::IMCORRDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	if(msg==WM_INITDIALOG) // Save this pointer in properties on init
		{
		SetProp(hdlg,"THISH",(HANDLE)HIWORD(lP));
		SetProp(hdlg,"THISL",(HANDLE)LOWORD(lP));
		}
	HANDLE th=GetProp(hdlg,"THISH"); // Get this ponter from properties
	HANDLE tl=GetProp(hdlg,"THISL");
	IMCORR *t=(IMCORR *)MAKELONG(tl,th);
	if(!t)
		return FALSE;
	return t->DP(hdlg,msg,wP,lP); // Displatch to instance
	}

// DP(...) Instance level dialog proc for IMCORR
BOOL IMCORR::DP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	HWND hctrl;
	switch(msg)
		{
		case WM_INITDIALOG:
			{
			FEDITINFO finfo;
			AEWnd=hdlg;
			hwnd=hdlg;

			// IDC_FXCOR
			hctrl=GetDlgItem(hdlg,IDC_FXCOR);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,fo.v2[0]*EditXScale);

			// IDC_FYCOR
			hctrl=GetDlgItem(hdlg,IDC_FYCOR);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,fo.v2[1]*EditYScale);

			// IDC_FSCOR
			hctrl=GetDlgItem(hdlg,IDC_FSCOR);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;
			finfo.flen=1;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,fs);

			// IDC_RXCOR
			hctrl=GetDlgItem(hdlg,IDC_RXCOR);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,ro.v2[0]*EditXScale);

			// IDC_RYCOR
			hctrl=GetDlgItem(hdlg,IDC_RYCOR);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=CFPP;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,ro.v2[1]*EditYScale);

			// IDC_RSCOR
			hctrl=GetDlgItem(hdlg,IDC_RSCOR);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=3;
			finfo.flen=1;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,rs);

//			updateenables(hdlg);

			return TRUE;
			}

		case WM_ACTIVATE:
			if(pADWnd) // Allow app to to IsDialogMessage
				*pADWnd=wP ? hdlg : 0;
			return FALSE;

		case WM_CLOSE:
			EndDialog(hdlg,FALSE);  // End the dialog
			closeedit();
			return FALSE;

		case WM_COMMAND:
			{
			HWND hctrl;
			WORD cid=LOWORD(wP);
			WORD ncode=HIWORD(wP);
			switch(cid)
				{
				case IDOK:
					// IDC_X
//					hctrl=GetDlgItem(hdlg,IDC_X);
//					if((!FeditValidate(hctrl))||
//						(!FeditGetVal(hctrl,&EditorMark.origin.x)))
//						{
//						ShowError(IDE_BADX);
//						SetFocus(hctrl);
//						return TRUE;
//						}
//					EditorMark.origin.x/=EditXScale;

					// IDC_FXCOR
					hctrl=GetDlgItem(hdlg,IDC_FXCOR);
					if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&fo.v2[0])))
						{
						ShowError(IDE_BADX);
						SetFocus(hctrl);
						return TRUE;
						}
					fo.v2[0]/=EditXScale;

					// IDC_FYCOR
					hctrl=GetDlgItem(hdlg,IDC_FYCOR);
					if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&fo.v2[1])))
						{
						ShowError(IDE_BADY);
						SetFocus(hctrl);
						return TRUE;
						}
					fo.v2[1]/=EditYScale;

					// IDC_FSCOR
					hctrl=GetDlgItem(hdlg,IDC_FSCOR);
					if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&fs)))
						{
						ShowError(IDE_BADSLANT);
						SetFocus(hctrl);
						return TRUE;
						}

					// IDC_RXCOR
					hctrl=GetDlgItem(hdlg,IDC_RXCOR);
					if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&ro.v2[0])))
						{
						ShowError(IDE_BADX);
						SetFocus(hctrl);
						return TRUE;
						}
					ro.v2[0]/=EditXScale;

					// IDC_RYCOR
					hctrl=GetDlgItem(hdlg,IDC_RYCOR);
					if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&ro.v2[1])))
						{
						ShowError(IDE_BADY);
						SetFocus(hctrl);
						return TRUE;
						}
					ro.v2[1]/=EditYScale;

					// IDC_RSCOR
					hctrl=GetDlgItem(hdlg,IDC_RSCOR);
					if((!FeditValidate(hctrl))||
						(!FeditGetVal(hctrl,&rs)))
						{
						ShowError(IDE_BADSLANT);
						SetFocus(hctrl);
						return TRUE;
						}

					id=newid;
					changed=TRUE;
					EndDialog(hdlg,TRUE);  // End the dialog
					closeedit();
					return FALSE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);  // End the dialog
					closeedit();
					return FALSE;
				}
			return FALSE;
			}
		case WM_DESTROY:
			return 0;
		}
	return FALSE;
	}

// updateenables : updates the enabled state of controls on the editor
// based on nacqpos, nrefpos, aeok and imok
void IMCORR::updateenables(HWND hdlg)
	{
	if(!hdlg)
		return;
	}
