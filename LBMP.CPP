/******************************************************************************
	Name : LBMP.C
	Desc : Definitions for LoadedBitmap Cache functions
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <memory.h>
#include "lbmp.h"

#define LBMPALLOC 32    // size of each alloc

typedef struct // Loaded bitmaps cache entry
	{
	void *dsc;
	WORD ldsc;
	HBITMAP hbmp;
	} LBMP;

static LBMP *lbmp=NULL; // loaded bitmaps or NULL if none
static WORD nlbmp=0;         // Number of loaded bitmaps
static WORD sizelbmp=0;      // Current size of block for lbmp

/******************************************************************************
	hbmp = LoadedBitmapsFind(dsc,ldsc)

	Searches the loaded bitmaps cache for an entry matching the supplied
	descriptor.
	If found the associated bitmap handle is returned.  If not found
	0 is returned.

	void _far *dsc = descriptor block
	WORD ldsc = length of dsc
	HBITMAP hbmp = handle of found bitmap or 0 if not found.
******************************************************************************/
HBITMAP LoadedBitmapsFind(void *dsc,WORD ldsc)
	{
	int i;
	// Search lbmp for allready loaded bitmap
	for(i=0;i<nlbmp;i++)
		if(lbmp[i].ldsc==ldsc)
			if(!memcmp(lbmp[i].dsc,dsc,ldsc))
				return lbmp[i].hbmp;
	return 0;
	}

/******************************************************************************
	LodaedBitmapsAdd(dsc,ldsc,hbmp)

	Adds a discriptor->hbitmap association to the loaded bitmaps cache.

	HBITMAP = bitmaps handle.
	void _far *dsc = descriptor block
	WORD ldsc = length of dsc
******************************************************************************/
void LoadedBitmapsAdd(void *dsc,WORD ldsc,HBITMAP hbmp)
	{
	if(!lbmp) // initial alloc?
		{
		nlbmp=0;
		sizelbmp=LBMPALLOC;
		lbmp=new LBMP[sizelbmp];
		}
	if(nlbmp==sizelbmp) // Grow time?
		{
		WORD newsize=sizelbmp+LBMPALLOC;
		LBMP *newlbmp=new LBMP[newsize];
		for(int i=0;i<nlbmp;i++)
			newlbmp[i]=lbmp[i];
		delete [] lbmp;
		lbmp=newlbmp;
		sizelbmp=newsize;
		}
	lbmp[nlbmp].dsc=new char[ldsc];
	memcpy(lbmp[nlbmp].dsc,dsc,ldsc);
	lbmp[nlbmp].ldsc=ldsc;
	lbmp[nlbmp].hbmp=hbmp;
	nlbmp++;
	}

/******************************************************************************
	LoadedBitmapsClear()

	Clears the loaded bitmaps cache.

	The associated bitmaps are not freed (this is the responsability of the
	owner of the IMSTEP sequence.
******************************************************************************/
void LoadedBitmapsClear(void)
	{
	int i;

	if(lbmp)
		{
		for(i=0;i<nlbmp;i++)
			delete [] lbmp[i].dsc;
		delete [] lbmp;
		}
	sizelbmp=0;
	nlbmp=0;
	lbmp=NULL;
	}
