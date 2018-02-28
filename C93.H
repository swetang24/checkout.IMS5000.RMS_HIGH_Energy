/***************************************************************************
    NAME : C93.H
    DESC : Header for C93.C
***************************************************************************/
#ifndef _C93_H
#define _C93_H

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
HBITMAP C93RenderBitmap(IMPMARK *m,LPSTR t);
void C93Init(IMPMARK *m);
BOOL CALLBACK C93MARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _C93_H
