#ifndef _AUX4001_H
#define _AUX4001_H

/******************************************************************************
    Name : AUX5000
    Desc : Definitions for AUX5000 system.
******************************************************************************/

/******************************************************************************
    PUBLIC DEFINES
******************************************************************************/
// Messages
#define AM_DOAUX     WM_USER // lP=LPIMP

// Notifications
#define AN_TAB       0x1000
#define AN_BKTAB     0x1001
#define AN_CLICKED   0x1002
#define AN_SETFOCUS  0x1003

/******************************************************************************
    PUBLIC FUNCTION PROTOTYPES
******************************************************************************/
int AUXInit(void);
HWND AUXCreate(HWND parent,WORD id,int x,int y,int w,int h);

#endif // _AUX4001_H
