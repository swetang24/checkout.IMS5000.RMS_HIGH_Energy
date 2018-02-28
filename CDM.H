/***************************************************************************
    NAME : CDM.H
    DESC : Header for CDM.C
***************************************************************************/
#ifndef _CDM_H
#define _CDM_H

/***************************************************************************
	PUBLIC VARIABLES
***************************************************************************/
extern BOOL DM001IsLinked;


/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
BOOL DM001Link(void);
void DM001UnLink(void);
void SQURFINDCOLANDROW(int length, int& col, int& row);
void RECTFINDCOLANDROW(int length, int& col, int& row);
HBITMAP CDMRenderBitmap(IMPMARK *m,LPSTR t,BOOL showerr);
void CDMInit(IMPMARK *m);
void UIDInit(IMPMARK *m);
void FindValuesByVarName(char name[], char t);
void UIDPreText(char text[]);
BOOL CALLBACK CDMMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK UIDMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);   //-------Linghai-11-21-08--
BOOL CALLBACK UID06CON1EDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);   //-------Linghai-11-21-08--
BOOL CALLBACK UID06CON2EDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);   //-------Linghai-11-21-08--
BOOL CALLBACK UID05EDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);   //-------Linghai-11-21-08--

#endif // _CDM_H
