/******************************************************************************
	NAME : CLAMP
	DESC : Active Clamp Fixture Support.
		A clamping fixture can be controlled by a DIO bit called Clamp.
		It is locked and released at appropriate times in the marking
		and editing cycles and is enabled when the bit Clamp is defined
		and the UDHAc Clamp is defined and has a value other than 0.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <stddef.h>
#include <IM5000\IM5000.H>
#include "ims5000.h"
#include "imp.h"
#include "clamp.h"

/******************************************************************************
	PUBLIC VARS
******************************************************************************/
int ClampActive=FALSE; // Indicates clamp is in use >1 if no autorelease

/******************************************************************************
	STATIC VARS
******************************************************************************/
static HBIT bitClamp=0;
static HBIT bitPC2Clamp=0;
static char sClamp[]="Clamp";
static char sPC2Clamp[]="PC2Clamp";
static BOOL ClampLocked=FALSE;

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	ClampInit(void)

	Maps bitClamp.
******************************************************************************/
void ClampInit(void)
	{
	bitClamp=IMMapOutBit(sClamp);
	}

/******************************************************************************
	ClampInit(void)

	Maps bitClamp.
******************************************************************************/
void PC2ClampInit(void)
	{
	bitPC2Clamp=IMMapOutBit(sPC2Clamp);
	}


/******************************************************************************
	ClampStart(pimp,pudhas)

	If Clamp bit exists, Searches for UDHA Clamp and if found
	sets ClampActive.

	PIMP pimp = IMP with UDHAs to search if !pudhas
	IMPUDHAS *pudhas = UHDAS to search
******************************************************************************/
void ClampStart(PIMP pimp,IMPUDHAS *pudhas)
	{
	if(!bitClamp)
		{
		ClampActive=FALSE;
		return;
		}
	if(pudhas)
		ClampActive=_IMPGetUDHAInt(pudhas,sClamp,0);
	else
		ClampActive=IMPGetUDHAInt(pimp,sClamp,0);
	if(!ClampActive)
		ClampRelease();
	ClampLocked=FALSE;
	}

/******************************************************************************
	ClampLock(void)

	Locks the clamp if active.
******************************************************************************/
void ClampLock(void)
	{
	if(ClampActive)
		{
		IMSetOutBit(bitClamp,TRUE);
		ClampLocked=TRUE;
		}
	}

/******************************************************************************
	ClampRelease(void)

	Releases the clamp if the bit is defined even if not active
******************************************************************************/
void ClampRelease(void)
	{
	if(bitClamp)
		IMSetOutBit(bitClamp,FALSE);
	ClampLocked=FALSE;
	}

/******************************************************************************
	ClampToggle(void)

	Toggles the ClampState
******************************************************************************/
void ClampToggle(void)
	{
	if(ClampLocked)
		ClampRelease();
	else
		ClampLock();
	}
/******************************************************************************
	PC2ClampRelease(void)

	Releases the PC2 clamp if the bit is defined even if not active
******************************************************************************/
void PC2ClampRelease(void)
	{
	if(bitPC2Clamp)
		IMSetOutBit(bitPC2Clamp,TRUE);
	//ClampLocked=FALSE;
	}

