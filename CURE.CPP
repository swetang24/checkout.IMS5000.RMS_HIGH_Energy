/******************************************************************************
	Name : CURE
	Desc : Flying UV Cure Extensions for IMS5000
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <stddef.h>
#include <imslib32/imslib32.h>
#include <im5000/im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "editor.h"
#include "stdloc.h"
#include "cure.h"

/******************************************************************************
	CURESTEPInit(m)

	Initializes a new IMPCURESTEP.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void CURESTEPInit(IMPMARK *m)
	{
	m->type=mtCURE;
	m->cure.xsize=1.0f;
	m->cure.vel=0.0f; // use default
	m->cure.keepon=FALSE;
	}

/******************************************************************************
	CURESTEPEDITDP

	mtCURE editor dialog process.
******************************************************************************/
BOOL CALLBACK CURESTEPEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			HWND hctrl;

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
			/*hctrl=GetDlgItem(hdlg,IDC_DEG);
			if(EditHasRotDeg)
				{
				FeditGetInfo(hctrl,&finfo);
				finfo.yextent=0;
				finfo.ilen=3;
				finfo.flen=1;
				FeditSetInfo(hctrl,&finfo);
				FeditSetVal(hctrl,EditorMark.deg.t);
				}*/

			// IDC_XSIZE
			hctrl=GetDlgItem(hdlg,IDC_XSIZE);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=3;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cure.xsize*EditXScale);

			// IDC_VEL
			hctrl=GetDlgItem(hdlg,IDC_VEL);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=2;    // format -99.999
			finfo.flen=2;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.cure.vel*EditXScale);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_KEEPON
			CheckDlgButton(hdlg,IDC_KEEPON,EditorMark.cure.keepon);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);
			return TRUE;
			}

		case WM_COMMAND: // Win32 changed
			{
			HWND hctrl;
			HWND fromCtrl = (HWND) lP;
			WORD id = LOWORD(wP); 
			WORD ncode = HIWORD(wP);

			DWORD dw;
//			IMPUDHAS *pudhas;
//			IMPVARS *pvars;
//			IMPVARS *pgvars;

			if(fromCtrl) // If from control
				{
				switch(id)
					{
					case IDC_LOCATE:
						DoCureLocate(hdlg,IDC_X,IDC_Y);
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
						/*if(EditHasRotDeg)
							{
							hctrl=GetDlgItem(hdlg,IDC_DEG);
							if((!FeditValidate(hctrl))||
								(!FeditGetVal(hctrl,&EditorMark.deg.t)))
								{
								ShowError(IDE_BADDEG);
								SetFocus(hctrl);
								return TRUE;
								}
							}*/

						// IDC_XSIZE
						hctrl=GetDlgItem(hdlg,IDC_XSIZE);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.cure.xsize)||
							(EditorMark.cure.xsize<=0.0f)))
							{
							ShowError(IDE_BADXSIZE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cure.xsize/=EditXScale;

						// IDC_VEL
						hctrl=GetDlgItem(hdlg,IDC_VEL);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.cure.vel)||
							(EditorMark.cure.vel<0.0f)))
							{
							ShowError(IDE_BADVEL);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.cure.vel/=EditXScale;

						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_KEEPON
						EditorMark.cure.keepon=IsDlgButtonChecked(hdlg,IDC_KEEPON);

						EndDialog(hdlg,TRUE);
						return TRUE;

					case IDCANCEL:
						EndDialog(hdlg,FALSE);
						return TRUE;
					}
				}
			break;
			}
		}
	return FALSE;
	}
