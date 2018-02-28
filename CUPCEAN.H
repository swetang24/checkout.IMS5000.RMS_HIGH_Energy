/***************************************************************************
    NAME : CUPCEAN.H
    DESC : Header for CUPCEAN.C
***************************************************************************/
#ifndef _CUPCEAN_H
#define _CUPCEAN_H

/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
HBITMAP CUPCEANRenderBitmap(IMPMARK *m,LPSTR t);
void CUPCEANInit(IMPMARK *m);
BOOL CALLBACK CUPCEANMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _CUPCEAN_H
