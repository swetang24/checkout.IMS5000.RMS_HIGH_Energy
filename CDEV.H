/***************************************************************************
    NAME : CDEV.H
    DESC : Header for CDEV.C
***************************************************************************/
#ifndef _CDEV_H
#define _CDEV_H

#include <windows.h>
/***************************************************************************
	PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
void CDevSysInit();
void CDevReloadSysSettings();
void CDevSysQuit();
void CDEVInit(IMPMARK *m);
BOOL CDevMarkUsesFont(IMPMARK *m,int idf);
void CDevMarkDecGreaterFonts(IMPMARK *m,int idf);
BOOL CDevMapFonts(char *text,WORD *fontmap,int nfonts);
BOOL CALLBACK CDEVMARKEDITDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);


#endif // _CDEV_H
