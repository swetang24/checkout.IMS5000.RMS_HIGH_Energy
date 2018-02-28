#include "stdafx.h"
#include "imp.h"
#include "imsio.h"

/******************************************************************************
	IMPFONTS CLASS
******************************************************************************/
IMPFONTS::IMPFONTS()
{
	count = 0;
	capac = 0;
	name = NULL;
}

IMPFONTS::IMPFONTS(int initCapac)
{
	count = 0;
	capac = initCapac;
	if (initCapac>0) 
		name=new IJFONTNAME[initCapac]; 
	else 
		name=NULL;
}

IMPFONTS::~IMPFONTS()
	{
		free();
	}

void IMPFONTS::free(){
	if (name){
		delete [] name;
		name=0;
	}
}

BOOL IMPFONTS::realloc(int ncount){
	if (ncount <= 0)
	{
		free();
		count=0;
		capac=0;
		return FALSE; 
	}
	
	// Create the new array
	IJFONTNAME * newArr = new IJFONTNAME[ncount];
	
	count=min(count,ncount);
	capac=ncount;
	
	for(int i=0;i<count;i++){
		lstrcpy(newArr[i],name[i]);
	}

	// Free the old array and assign new one
	free();

	name = newArr;
	return (name!=NULL);
}

BOOL IMPFONTS::write(HANDLE fh){
	
	DWORD bw;
	// Write count
	WriteFile(fh,(LPSTR)&count,sizeof(WORD),&bw,NULL);
	if (bw != sizeof(WORD))
		return FALSE;
	
	// Write the array
	for (int i=0;i<count;i++){
		if(!writeString(fh,name[i],sizeof(IJFONTNAME)))
			return FALSE;
	}
    
	return TRUE;
}

IMPFONTS::IMPFONTS(const IMPFONTS & src)
	{
	count = src.count;
	capac = src.capac;
	name = NULL;

	if (capac>0){
		name = new IJFONTNAME[capac];
	} 


	for (int i=0;i<count;i++){
		lstrcpy(name[i],src.name[i]);
	}
	
	}

IMPFONTS& IMPFONTS::operator = (const IMPFONTS & src)
{
	free();
	count = src.count;
	capac = src.capac;

	if (capac>0){
		name = new IJFONTNAME[capac];
	} 
	
	for (int i=0;i<count;i++){
		lstrcpy(name[i],src.name[i]);
	}
	
	return *this;
}

BOOL IMPFONTS::read(HANDLE fh, WORD ver, WORD cnt){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver,cnt);
	else 
		return translate(fh,ver,cnt);
}

BOOL IMPFONTS::readnative(HANDLE fh,WORD ver,WORD cnt){	
	
	for (int i=0;i<cnt;i++){
		if(!readString(fh,name[i],sizeof(IJFONTNAME)))
			return FALSE;
	}

	count = cnt;
	return TRUE;
}

// Everything is the same from 4001 to 3.68
BOOL IMPFONTS::translate(HANDLE fh, WORD ver, WORD cnt){
	for (int i=0;i<cnt;i++){
		if(!readString(fh,name[i],sizeof(IJFONTNAME)))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}


