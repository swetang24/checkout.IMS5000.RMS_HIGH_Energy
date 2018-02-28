/***************************************************************************
    NAME : C39.H
    DESC : Header for C39.C
***************************************************************************/
#ifndef _C39_H
#define _C39_H

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
HBITMAP C39RenderBitmap(IMPMARK *m,LPSTR t);
void C39Init(IMPMARK *m);
BOOL CALLBACK C39MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _C39_H
