#include "stdafx.h"
#include "imp.h"
#include "imsio.h"

/******************************************************************************
	IMPUDHA CLASS
******************************************************************************/
IMPUDHA::IMPUDHA()
	{
	name[0]=0;
	redit=0;
	val[0]=0;
	}

IMPUDHA::~IMPUDHA()
	{	
	}

/******************************************************************************
	IMPUDHAS CLASS
******************************************************************************/
IMPUDHAS::IMPUDHAS()
	{
	count = 0;
	capac = 0;
	udha = NULL;
	}

IMPUDHAS::IMPUDHAS(int initCapac){
	count = 0;
	capac = initCapac;
	if(initCapac>0) 
		udha = new IMPUDHA[capac];
	else
		udha=NULL;
}

IMPUDHAS::~IMPUDHAS()
	{
		free();
	}

void IMPUDHAS::free()
	{
	if (udha){
		delete [] udha;
		udha=0;
	}
	}

BOOL IMPUDHAS::realloc(int ncount){

	if (ncount <= 0)
	{
		free();
		count=0;
		capac=0;
		return FALSE; 
	}
	
	// Create the new array
	IMPUDHA * newArr = new IMPUDHA[ncount];
	
	count=min(count,ncount);
	capac=ncount;
	
	for(int i=0;i<count;i++){
		newArr[i] = udha[i];
	}

	// Free the old array and assign new one
	free();

	udha = newArr;
	return (udha!=NULL);
}



IMPUDHAS::IMPUDHAS(const IMPUDHAS & src)
	{
	count = src.count;
	capac = src.capac;
	udha = NULL;

	if (capac>0){
		udha = new IMPUDHA[capac];
	} 

	for (int i=0;i<count;i++){
		udha[i] = src.udha[i];
	}
	
	}

IMPUDHAS& IMPUDHAS::operator = (const IMPUDHAS & src)
{
	free();
	
	count = src.count;
	capac = src.capac;

	if (capac>0){
		udha = new IMPUDHA[capac];
	} 
	
	for (int i=0;i<count;i++){
		udha[i] = src.udha[i];
	}
	
	return *this;
	
}

BOOL IMPUDHAS::read(HANDLE fh, WORD ver, WORD cnt){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver,cnt);
	else 
		return translate(fh,ver,cnt);
}

BOOL IMPUDHAS::readnative(HANDLE fh, WORD ver, WORD cnt){
	for (int i=0;i<cnt;i++){
		if (!udha[i].readnative(fh,ver))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}

BOOL IMPUDHAS::translate(HANDLE fh, WORD ver, WORD cnt){
	for (int i=0;i<cnt;i++){
		if (!udha[i].translate(fh,ver))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}

BOOL IMPUDHAS::write(HANDLE fh){
	
	DWORD bw;
	// Write count
	WriteFile(fh,(LPSTR)&count,sizeof(WORD),&bw,NULL);
	if (bw != sizeof(WORD))
		return FALSE;

	// Write the data
	for (int i=0;i<count;i++){
		if (!udha[i].write(fh))
			return FALSE;
	}
	   
	return TRUE;
}

BOOL IMPUDHA::readnative(HANDLE fh,WORD ver){
	if (!readString(fh,name,IMPLUDHANAME+1))
		return FALSE;
	if (!readByte(fh,&redit))
		return FALSE;
	if (!readString(fh,val,IMPLUDHAVAL+1))
		return FALSE;
	return TRUE;
}

BOOL IMPUDHA::write(HANDLE fh){
	if (!writeString(fh,name,IMPLUDHANAME+1))
		return FALSE;
	if (!writeByte(fh,&redit))
		return FALSE;
	if (!writeString(fh,val,IMPLUDHAVAL+1))
		return FALSE;	   
	return TRUE;
}

BOOL IMPUDHA::read(HANDLE fh, WORD ver){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver);
	else 
		return translate(fh,ver);
}

// All versions from pre 3.30 to 3.68 are the same 
BOOL IMPUDHA::translate(HANDLE fh, WORD ver){
	if (!readString(fh,name,31+1))
		return FALSE;
	if (!readByte(fh,&redit))
		return FALSE;
	if (!readString(fh,val,32+1))
		return FALSE;
	// If any new fields are added to IMPUDHAS, the constructor should init them
	return TRUE;
}

