/***************************************************************************
    NAME : C128.H
    DESC : Header for C128.C
***************************************************************************/
#ifndef _C128_H
#define _C128_H

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
HBITMAP C128RenderBitmap(IMPMARK *m,LPSTR t,LPSTR hrt);
void C128Init(IMPMARK *m);
BOOL CALLBACK C128MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _C128_H
