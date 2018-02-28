/******************************************************************************
	BSMFIXUPS
	Fixup table for ponter to index and back mapping of Bitmap Sub Mark
	pointers for file storage and retreval.

	Constructs the BSMFIXUPS for the existing marks collection.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <stddef.h>
#include "ims5000.h"
#include "imp.h"
#include "bsmfixup.h"

/******************************************************************************
	BSMFIXUPS(marks)
	Constructs the BSMFIXUPS for the existing marks collection.
******************************************************************************/
BSMFIXUPS::BSMFIXUPS(IMPMARKS *marks)
	{
	pmarks=marks;

	sizefixups=CountUniqueBSMs();
	nfixups=0;

	// Allocate the fixups array
	fixups=NULL;
	if(sizefixups>0)
		fixups=new IMPMARK *[sizefixups];

	// Load it with pointers (or indexes if just loaded)
	for(int mi=0;mi<pmarks->count;mi++)
		{
		IMPMARK &m=pmarks->mark[mi];
		if(m.type==mtCDEV)
			for(int bi=0;bi<m.cdev.nbsm;bi++)
				add(m.cdev.bsm[bi]);
		}
 	}

/******************************************************************************
	~BSMFIXUPS()

	Frees memory allocated to fixups
******************************************************************************/
BSMFIXUPS::~BSMFIXUPS()
	{
	if(sizefixups)
		delete [] fixups;
	fixups=NULL;
	sizefixups=0;
	}

/******************************************************************************
	BSMFIXUPS::toindex()

	Replaces the bsm pointers in the cdev marks of the parent marks
	with there fixups index.  Use before saving to disk.
	Assumes fixups contains valid pointers.
******************************************************************************/
void BSMFIXUPS::toindex()
	{
	for(int mi=0;mi<pmarks->count;mi++)
		{
		IMPMARK &m=pmarks->mark[mi];
		if(m.type==mtCDEV)
			for(int bi=0;bi<m.cdev.nbsm;bi++){
				m.cdev.bsm[bi]=(IMPMARK *)find(m.cdev.bsm[bi]);
			}

		}
	}

/******************************************************************************
	BSMFIXUPS::topointer()

	Replaces the indexes in the bsm pointers of the cdev marks of the parent
	marks with the pointers in fixups
******************************************************************************/
void BSMFIXUPS::topointer()
	{
	for(int mi=0;mi<pmarks->count;mi++)
		{
		IMPMARK &m=pmarks->mark[mi];
		if(m.type==mtCDEV)
			for(int bi=0;bi<m.cdev.nbsm;bi++)
				m.cdev.bsm[bi]=fixups[(long)(m.cdev.bsm[bi])];
		}
	}

/******************************************************************************
	ok = BSMFIXUPS::writebsms(fh)

	Writes each mark referenced in fixups to the file.

	BOOL ok = TRUE on success
	HANDLE fh = file handle
	
******************************************************************************/
BOOL BSMFIXUPS::writebsms(HANDLE fh)
	{
	for(int fi=0;fi<nfixups;fi++)
		{
		IMPMARK &m=*(fixups[fi]);
//		if(_lwrite32(fh,(LPSTR)&m,sizeof(IMPMARK))!=sizeof(IMPMARK))
		if(!m.write(fh))
			{
			ShowError(IDE_WRITEFILE);
			return FALSE;
			}
		}
	return TRUE;
	}

/******************************************************************************
	ok = BSMFIXUPS::readbsms(fh)

	Allocates and reads the mark and updates the fixup pointer for each
	fixup.
******************************************************************************/
BOOL BSMFIXUPS::readbsms(HANDLE fh,WORD ver)
	{
	for(int fi=0;fi<nfixups;fi++)
	{
		IMPMARK *m=new IMPMARK;
		//		if(_lread32(fh,m,sizeof(IMPMARK))!=sizeof(IMPMARK))
		if(!m->read(fh,ver))
		{
			ShowError(IDE_READFILE);
			return FALSE;
		}
		fixups[fi]=m;
	}
	return TRUE;
	}

/******************************************************************************
	index = add(mark)

	Adds a mark pointer to the fixups if it is not allready in it.
	Returns the index either way.

	int index = fixups index of mark.
	IMPMARK *mark = mark pointer to add.
******************************************************************************/
long BSMFIXUPS::add(IMPMARK *mark)
	{
	long index=find(mark);
	if(index<0)
		{
		index=nfixups++;
		fixups[index]=mark;
		}
	return index;
	}

/******************************************************************************
	index = find(mark)
	Searches for and returns the index of the mark pointer if it is contained
	in fixups.  Returns -1 if not found.

	int index = index of found mark pointer or -1 if not found.
	IMPMARK *mark = mark pointer to search for.
******************************************************************************/
long BSMFIXUPS::find(IMPMARK *mark)
	{
	for(int fi=0;fi<nfixups;fi++)
		if(fixups[fi]==mark)
			return (long)fi;
	return -1L;
	}

/*****************************************************************************
	count = CountUniqueBSMs()

	Counts the unique bsm pointers contained in the parent marks.
	Used prior to allocation of fixup to determine size.

	int count = Count of unique bsm pointers in the marks of pmarks.
******************************************************************************/
int BSMFIXUPS::CountUniqueBSMs()
	{
	int count=0;
	for(int mia=0;mia<pmarks->count;mia++)
		{
		IMPMARK &ma=pmarks->mark[mia];
		if(ma.type!=mtCDEV)
			continue;
		for(int bia=0;bia<ma.cdev.nbsm;bia++)
			{
				int mib;
			for(mib=0;mib<=mia;mib++)
				{
				IMPMARK &mb=pmarks->mark[mib];
				if(mb.type!=mtCDEV)
					continue;
				int nbib=mia==mib ? bia : mb.cdev.nbsm;
				int bib;
				for(bib=0;bib<nbib;bib++)
					{
					if(mb.cdev.bsm[bib]==ma.cdev.bsm[bia])
						break; // Not unique
					}
				if(bib<nbib)
					break; // Not unique
				}
			if(mib>mia)
				count++;
			}
		}
	return count;
	}
