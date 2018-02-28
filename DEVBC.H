/***************************************************************************
    NAME : DEVBC.H
    DESC : Header for DEVBC.C
***************************************************************************/
#ifndef _DEVBC_H
#define _DEVBC_H

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
void DEVBCInit(IMPMARK *m);
BOOL CALLBACK DEVBCMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _DEVBC_H
