#ifndef _EDITOR_H
#define _EDITOR_H

/***************************************************************************
    NAME : EDITOR.H
    DESC : Definitions for IMP editor window for IMS
***************************************************************************/
#include "imp.h"

// "Inserts" popup menu ID masks
#define IDM_VMENU  0x1000
#define IDM_HAMENU 0x2000
#define IDM_SVMENU 0x4000
//#define IDM_UVMENU 0x5000

/***************************************************************************
    PUBLIC VARIABLES
***************************************************************************/
extern IMP  EditorIMP;    // IMP to edit
extern IMPVALS EditorPreIVals; // Instance preview values
extern IMPVALS EditorPreGVals; // Global preview values
extern IMPMARK EditorMark;  // Copy of mark being editied by dialog
extern IMPMARK ClipboardMark;  // Mark on fake clipboard
extern BOOL ClipboardMarkValid;
extern BOOL EditorIJFound; // True if editing with attached ij
extern HWND EditorIJDev;         // Device HWND of inkjet if EditorIJFound
extern IJINFO EditorIJInfo;      // IJINFO of inkjet if EditorIJFound
extern BOOL EditHasRadius; // True if the file being edited has a radius
extern IMXYMODE EditXYMode; // The xy mode of the file being edited
extern double EditXScale;  // Metric display scaling factor X or 1.0
extern double EditYScale;  // Metric display scaling factor Y or 1.0
extern double EditZScale;  // Metric display scaling factor Z or 1.0
//////////////REMOVED WHEN BFI ADDED
///extern POINT EditBmpExt;  // Bitmap extent for selected inkjet in edit or 0,0
////////////////////////////////////
extern BOOL EditBandEnable; // Band preview enable
extern COLORREF EditEvenBandColor; // even band background color
extern COLORREF EditOddBandColor;  // odd band background color
extern BOOL EditHasRot;   // No mark rotation flag
extern BOOL EditHasRotDeg;   // No mark rotation Degree flag
extern BOOL EditHasSlant; // No mark slant flag
extern IMCORR EditorIMCorr;   // Copy of im correction being editied by dialog

/***************************************************************************
    PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
HWND CreateEditorWnd(HWND parent,WORD id,int x,int y,int w,int h);
void DoInsertsPopup(HWND hdlg,HWND hctrl,
	IMPUDHAS *pudhas,IMPVARS *pvars,IMPVARS *pgvars);
void DoInsertsPopup(HWND hdlg,HWND hctrl,
	IMPVARS *pvars,IMPVARS *pgvars);
BOOL ProcessInsertsPopup(WORD wP,HWND hctrl);
void EditXYModePrep(HWND hdlg);
void DoMarkLocate(HWND hdlg,int idcx,int idcy);
void DoCureLocate(HWND hdlg,int idcx,int idcy);
int EditorIMPMapFontName(char *fontname);
BOOL DoSubMarkEdit(HWND parent,IMPMARK *mark);
void InitNewMark(IMPMARK *mark,WORD idm);
void FreePreVals();
BOOL BuildPreVals(PIMP pimp);
void SelectCurrentFont(HWND fontlist, LPSTR fontname);

#endif // _EDITOR_H
