/******************************************************************************
	NAME : DEVBC.C
	DESC : Device BarCode implementation for IMS
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
#include "devbc.h"
#include "stdloc.h"

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	DEVBCInit(m)

	Initializes a new IMPDEVBCMARK.

	IMPMARK *m = mark to initialize.
******************************************************************************/
void DEVBCInit(IMPMARK *m)
	{
	m->origin.x=0.0;
	m->origin.y=0.0;
	m->justx=mjxLEFT;
	m->justy=mjyTOP;
	m->type=mtDEVBC;
	m->devbc.ijfmt.font=-1;
	m->devbc.ijfmt.ms=1;
	m->devbc.ijfmt.xflip=0;
	m->devbc.ijfmt.yflip=0;
	m->devbc.cw=0.2;
	m->devbc.ch=0.25;
	m->devbc.text[0]=0;
	}

/***************************************************************************
    DEVBCMARKEDITDP

    mtDEVBC editor dialog process.
***************************************************************************/
BOOL CALLBACK DEVBCMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
			{
			FEDITINFO finfo;
			SEDITINFO sinfo;
			KEDITINFO kinfo;
			HWND hctrl;
			int i;

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
			// IDC_FONTS
			hctrl=GetDlgItem(hdlg,IDC_FONTS);
			// Limit edit text to maximum font name
			SendMessage(hctrl,CB_LIMITTEXT,sizeof(IJFONTNAME)-1,0);
			// Fill the listbox with the used and available fonts
			if(EditorIJFound)
				{  // Add all fonts from imp not in ij
				for(i=0;i<EditorIMP.fonts.count;i++)
					if(SendMessage(IMWnd,IMM_FINDFONT,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i])==-1)
						SendMessage(hctrl,CB_ADDSTRING,
							0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
				// Add fonts from ij
				SendMessage(IMWnd,IMM_CBADDFONTS,(WPARAM)hctrl,MMCBAF_BARCODE);
				}
			else // inkjet not found : Add fonts from imp
				for(i=0;i<EditorIMP.fonts.count;i++)
					SendMessage(hctrl,CB_ADDSTRING,
						0,(LONG)(LPSTR)&EditorIMP.fonts.name[i]);
			// Select marks current font
			if(EditorMark.devbc.ijfmt.font==-1) // No current font
				SendMessage(hctrl,CB_SETCURSEL,-1,0); // No current sel
			else
				SelectCurrentFont(hctrl,(LPSTR)EditorIMP.fonts.name[EditorMark.devbc.ijfmt.font]);
				//SendMessage(hctrl,CB_SELECTSTRING,
				//	-1,(LONG)(LPSTR)EditorIMP.fonts.name[EditorMark.devbc.ijfmt.font]);

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

			// IDC_DEG	sam	10/7/16
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

			// IDC_WIDTH
			hctrl=GetDlgItem(hdlg,IDC_WIDTH);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=1;	// format 1.0000
			finfo.flen=4;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.devbc.cw*EditXScale);

			// IDC_HEIGHT
			hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
			FeditGetInfo(hctrl,&finfo);
			finfo.yextent=0; // auto extent y
			finfo.ilen=1;	// format 1.0000
			finfo.flen=4;
			FeditSetInfo(hctrl,&finfo);
			FeditSetVal(hctrl,EditorMark.devbc.ch*EditYScale);

			// IDC_MULTISTROKE
			hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=1;
			sinfo.maxval=IMPMAXMS;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.devbc.ijfmt.ms);

			// IDC_REPEATS
			hctrl=GetDlgItem(hdlg,IDC_REPEATS);
			SeditGetInfo(hctrl,&sinfo);
			sinfo.yextent=0; // auto extent y
			sinfo.minval=0;
			sinfo.maxval=IMPMAXREP;
			sinfo.step=1;
			SeditSetInfo(hctrl,&sinfo);
			SeditSetVal(hctrl,EditorMark.rep);

			// IDC_WEXT
			hctrl=GetDlgItem(hdlg,IDC_WEXT);
			KeditGetInfo(hctrl,&kinfo);
			kinfo.yextent=0; // auto extent y
			kinfo.length=IMPLTEXT;
			kinfo.digits=1;
			kinfo.lowers=1;
			kinfo.uppers=1;
			kinfo.spaces=1;
			kinfo.others=1;
			KeditSetInfo(hctrl,&kinfo);
			SetWindowText(hctrl,EditorMark.devbc.text);

			if(!StdLoc)
				ShowWindow(GetDlgItem(hdlg,IDC_LOCATE),SW_HIDE);

			return TRUE;
			}

		case WM_COMMAND:  // Win32 changed
			{
			HWND hctrl;
			DWORD dw;
			int i;

			if(lP) // If from control
				{
				switch(LOWORD(wP))
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
						// IDC_FONTS
						hctrl=GetDlgItem(hdlg,IDC_FONTS);
						// Get the fontname
						SendMessage(hctrl,WM_GETTEXT,
							sizeof(IJFONTNAME),(LONG)(LPSTR)tbuf);
						// Trim leading and trailing spaces from the font name
						strtrimts(tbuf);
						strtrimls(tbuf);
						i=EditorIMPMapFontName(tbuf);
						if(i<0)
							{
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devbc.ijfmt.font=i;

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

						// IDC_WIDTH
						hctrl=GetDlgItem(hdlg,IDC_WIDTH);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.devbc.cw))||
							(!(EditorMark.devbc.cw>0.0)))
							{
							ShowError(IDE_BADCW);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devbc.cw/=EditXScale;

						// IDC_HEIGHT
						hctrl=GetDlgItem(hdlg,IDC_HEIGHT);
						if((!FeditValidate(hctrl))||
							(!FeditGetVal(hctrl,&EditorMark.devbc.ch))||
							(!(EditorMark.devbc.ch>0.0)))
							{
							ShowError(IDE_BADCH);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devbc.ch/=EditYScale;

						// IDC_MULTISTROKE
						hctrl=GetDlgItem(hdlg,IDC_MULTISTROKE);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADMULTISTROKE);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.devbc.ijfmt.ms=(WORD)dw;

						// IDC_REPEATS
						hctrl=GetDlgItem(hdlg,IDC_REPEATS);
						if((!SeditValidate(hctrl))||(!SeditGetVal(hctrl,&dw)))
							{
							ShowError(IDE_BADREP);
							SetFocus(hctrl);
							return TRUE;
							}
						EditorMark.rep=(WORD)dw;

						// IDC_WEXT
						hctrl=GetDlgItem(hdlg,IDC_WEXT);
						if((!KeditValidate(hctrl))||
							(!KeditGetVal(hctrl,(LPSTR)&(EditorMark.devbc.text)))||
							(!EditorMark.devbc.text[0]))
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
						{
						if(HIWORD(wP)==XN_INS)
							{
							hctrl=GetDlgItem(hdlg,IDC_WEXT);
							DoInsertsPopup(hdlg,hctrl,
								&EditorIMP.udhas,
								&EditorIMP.vars,
								&EditorIMP.gvars);
							}
						return TRUE;
						}
					}
				}
			else // From text insert menu
				{
				hctrl=GetDlgItem(hdlg,IDC_WEXT);
				if(ProcessInsertsPopup(LOWORD(wP),hctrl))
					return TRUE;
				}
			break;
			}
		}
	return FALSE;
	}
