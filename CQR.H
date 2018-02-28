/***************************************************************************
    NAME : CQR.H
    DESC : Header for CQR.C
***************************************************************************/
#ifndef _CQR_H
#define _CQR_H

extern BOOL QRCoderIsAvailable;

BOOL QRCoderCheck();
void CQRInit(IMPMARK *m);
HBITMAP CQRRenderBitmap(IMPMARK *m,LPSTR t,BOOL showerr);
BOOL CALLBACK CQRMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

#endif // _CQR_H
