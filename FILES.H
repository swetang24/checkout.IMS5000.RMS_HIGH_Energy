#ifndef _FILES_H
#define _FILES_H

/***************************************************************************
    NAME : FILES.H
    DESC : Header for files window module for IMS
***************************************************************************/

/***************************************************************************
    PUBLIC FUNCTION PROTOTYPES
***************************************************************************/
void FilesInit(void);
void FilesShutdown(void);
HWND CreateFilesWnd(HWND parent,WORD id,int x,int y,int w,int h);
void CreateInputWnd(HWND parent);
BOOL OpenFile(LPSTR fileName);
void ReloadFileOptions();
void formatTime(FILETIME ft,LPSTR tbuf);
#endif // _FILES_H
