/***************************************************************************
    NAME : C2OF5.H
    DESC : Header for C2OF5.C
***************************************************************************/
#ifndef _C2OF5_H
#define _C2OF5_H

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
HBITMAP C2OF5RenderBitmap(IMPMARK *m,LPSTR t);
void C2OF5Init(IMPMARK *m);
BOOL CALLBACK C2OF5MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _C2OF5_H
