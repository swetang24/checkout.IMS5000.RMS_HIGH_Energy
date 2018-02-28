/******************************************************************************
	Name : DDESRVR.H
	Desc : DDE Server API declarations
******************************************************************************/
/******************************************************************************
	PUBLIC DEFINES
******************************************************************************/
// Message to owner
#ifndef _DDESRVR_H
#define _DDESRVR_H


#define WM_DDERUN WM_USER // wP=DRNCODE : Sent to owner

// DDERUN Execute codes (sent to owner in wP of WM_DDERUN)
#define DRM_QUIT        0 // Quits the DDERUN mode and closes the IMS window.
#define DRM_LOCK        1 // lP=rlb :  Locks the RUN interface
#define DRM_UNLOCK      2 // lP=rlb : Unlocks the RUN interface
#define DRM_LOCKSTART   3 // Locks the Start button
#define DRM_UNLOCKSTART 4 // Allows the start button to unlock when ready
#define DRM_NEXTVALS    5 // Causes IMS vals to compute there next values
#define DRM_UPDATE      6 // Causes the vals window to update
#define DRM_SETFOCUS    7 // Causes the IMSWnd to receive focus
#define DRM_SETNLOADED  8 // Sets the number of loaded parts to lP
#define DRM_JOG         9 // Starts jog mode
#define DRM_INVALIMF   10 // Invalidates the IMF run window
#define DRM_LOCKNESTSONCHANGE 11 // sets flag to lock nests on any change to lP
#define DRM_START     12  // Starts the marking cycle if the im is ready
#define DRM_MVE     13  //----sam----testing for rotary

#define DDERUNMAXFN    128 // Maximum filename length

/******************************************************************************
	PUBLIC TYPES
******************************************************************************/
typedef enum
	{dreREADY,dreSTART,dreDONE,dreCANCEL,dreQUIT,dreERROR,
	dreJOGSTART,dreJOGEND,dreNOTREADY,dreNBFCHANGE,
	dreCYCLEQUIT,dreCYCLEPASS,dreCYCLEFAIL,dreCONTINUE,
	drePDONE,dreBUSY,dreREADSTART,dreREADEND,dreLOCSTART,dreLOCEND,dreEXTRA1} DDERUNEVENT;

/******************************************************************************
	PUBLIC VARIABLES
******************************************************************************/
extern USERLEVEL DDEUserLevel;         // DDE User's access level
extern char DDEUserName[LUSERNAME+1];  // DDE Users name
extern char DDEUserID[LUSERID+1];      // DDE Users ID
extern char DDERunFilename[DDERUNMAXFN+1]; // DDERUN file to open
extern BOOL Extra1IsDDE;               // True when EXTRA1 button is set DDE

/******************************************************************************
	API FUNCTION DECLARATIONS
******************************************************************************/
BOOL DDEServerInit(void);
BOOL DDEServerShutdown(void);
void DDERunInit(HWND hwnd);
void DDERunShutdown(void);
void DDERunPostEvent(DDERUNEVENT dre,int param);
void DDERunStart(void);
void DDERunIMStateAdvise(void);
void ReloadDDESettings();

#endif //end DDESRVR