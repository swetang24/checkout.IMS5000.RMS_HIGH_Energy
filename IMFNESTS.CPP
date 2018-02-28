#include "stdafx.h"
#include "imf.h"
#include "imsio.h"

/******************************************************************************
	IMFNESTS CLASS
******************************************************************************/
IMFNESTS::IMFNESTS()
{
	count = 0;
	capac = 0;
	nest = NULL;
}

IMFNESTS::IMFNESTS(int initCapac)
{
	count = 0;
	capac = initCapac;
	if (initCapac>0) 
		nest = new IMFNEST[capac];
	else
		nest = NULL;
}

IMFNESTS::~IMFNESTS()
	{
		free();
	}

void IMFNESTS::free(){
	if (nest){
		delete [] nest;
		nest=0;
	}
}

BOOL IMFNESTS::realloc(int ncount){
	if (ncount <= 0)
	{
		free();
		count=0;
		capac=0;
		return FALSE; 
	}

	// Create the new array
	IMFNEST * newArr = new IMFNEST[ncount];
	
	count=min(count,ncount);
	capac=ncount;
	
	for(int i=0;i<count;i++){
		newArr[i] = nest[i];
	}

	// Free the old array and assign new one
	free();

	nest = newArr;
	return nest!=NULL;
}

BOOL IMFNESTS::write(HANDLE fh){
	
	DWORD bw;
	// Write count
	WriteFile(fh,(LPSTR)&count,sizeof(WORD),&bw,NULL);
	if (bw != sizeof(WORD))
		return FALSE;
	
	// Write the array
	for (int i=0;i<count;i++){
		if(!nest[i].write(fh))
			return FALSE;
	}
    
	return TRUE;
}

IMFNESTS::IMFNESTS(const IMFNESTS & src)
	{
	count = src.count;
	capac = src.capac;
	nest = NULL;

	if (capac>0){
		nest = new IMFNEST[capac];
	} 

	for (int i=0;i<count;i++){
		nest[i] = src.nest[i];
	}
	
	}

IMFNESTS& IMFNESTS::operator = (const IMFNESTS & src)
{
	free();

	count = src.count;
	capac = src.capac;

	if (capac>0){
		nest = new IMFNEST[capac];
	} 
	
	for (int i=0;i<count;i++){
		nest[i] = src.nest[i];
	}
	
	return *this;
}

BOOL IMFNESTS::read(HANDLE fh, WORD ver, WORD cnt)
{
	if (ver==0)
		return readnative(fh,ver,cnt);
	else 
		return translate(fh,ver,cnt);
}

BOOL IMFNESTS::readnative(HANDLE fh, WORD ver, WORD cnt)
{
	for (int i=0;i<cnt;i++){
		if(!nest[i].readnative(fh,ver))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}

BOOL IMFNESTS::translate(HANDLE fh, WORD ver, WORD cnt)
{
	for (int i=0;i<cnt;i++){
		if(!nest[i].translate(fh,ver))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}

/******************************************************************************
	IMFNEST CLASS
******************************************************************************/
// Fill in nest members with defaults
IMFNEST::IMFNEST()
	{
	origin.x=0.0f;
	origin.y=0.0f;
	originrot.x=0.0f;//sam
	originrot.y=0.0f;//sam
	extent.x=1.0f;
	extent.y=1.0f;
	rotfixextent.d=1.0f;  //sam


	//extent.x=1.0f;
	rotation=0.0f;
	flags=0;
	}

BOOL IMFNEST::readnative(HANDLE fh, WORD ver){
	if(HideOptRotaryIMF)
	{
			//origin
	if (!readDouble(fh,&origin.x))
		return FALSE;
	if (!readDouble(fh,&origin.y))
		return FALSE;

		//extent
	if (!readDouble(fh,&extent.x))
		return FALSE;
	if (!readDouble(fh,&extent.y))
		return FALSE;

	}
	else
	{
		//origin
	if (!readDouble(fh,&originrot.x))
		return FALSE;
	if (!readDouble(fh,&originrot.y))
		return FALSE;
		//Diameter for rotaryIMF	--sam	10/14/16
	if (!readDouble(fh,&rotfixextent.d))
		return FALSE;
	}
		//rotation
	if (!readDouble(fh,&rotation))
		return FALSE;
	//flags
	if (!readWord(fh,&flags))
		return FALSE;
	return TRUE;
}

BOOL IMFNEST::write(HANDLE fh)
{
	


	if(HideOptRotaryIMF)
	{
		//origin
		if (!writeDouble(fh,&origin.x))
		return FALSE;
		if (!writeDouble(fh,&origin.y))
		return FALSE;
		//extent
	if (!writeDouble(fh,&extent.x))
		return FALSE;
	if (!writeDouble(fh,&extent.y))
		return FALSE;
	}
	else
	{
		//origin
		if (!writeDouble(fh,&originrot.x))
		return FALSE;
		if (!writeDouble(fh,&originrot.y))
		return FALSE;
		//Diameter for rotaryIMF	--sam	10/14/16
		if (!writeDouble(fh,&rotfixextent.d))
		return FALSE;

	}
	//rotation
	if (!writeDouble(fh,&rotation))
		return FALSE;
	
	//flags
	if (!writeWord(fh,&flags))
		return FALSE;
	
	return TRUE;
}
BOOL IMFNEST::translate(HANDLE fh, WORD ver){
	if (ver >338 && ver <=357){
		return translateVers345Thru357(fh);	
	} else if (ver == 330 || ver == 338){
		return translateVers330Thru338(fh);
	} else if (ver == 300){
		return translateOriginalVersion(fh);
	}
	return TRUE;
}

// Read the doubles as floats since these were Win16 files 
// v3.45-3.57 translator
BOOL IMFNEST::translateVers345Thru357(HANDLE fh){
	
	if(HideOptRotaryIMF)
	{
		//origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;	
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;

	//extent
	if (!readDouble(fh,&extent.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&extent.y,TRUE))
		return FALSE;

	}
	else
	{
		//origin
	if (!readDouble(fh,&originrot.x,TRUE))
		return FALSE;	
	if (!readDouble(fh,&originrot.y,TRUE))
		return FALSE;

		//Diameter for rotaryIMF	--sam	10/14/16
	if (!readDouble(fh,&rotfixextent.d,TRUE))
		return FALSE;
		
	}
		//rotation
	if (!readDouble(fh,&rotation,TRUE))
		return FALSE;
	//flags
	if (!readWord(fh,&flags))
		return FALSE;
	return TRUE;
}


// v3.30-v3.38 translator 
/*
typedef struct
	{
	P2 origin;
	P2 extent;  // 0 component maps to partSize
	WORD flags; // nest bit flags
	} IMFV330NEST;

*/
// Read the doubles as floats since these were Win16 files 
BOOL IMFNEST::translateVers330Thru338(HANDLE fh){
	if(HideOptRotaryIMF)
	{
		//origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;	
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;

	//extent
	if (!readDouble(fh,&extent.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&extent.y,TRUE))
		return FALSE;

	}
	else
	{
		//origin
	if (!readDouble(fh,&originrot.x,TRUE))
		return FALSE;	
	if (!readDouble(fh,&originrot.y,TRUE))
		return FALSE;

		//Diameter for rotaryIMF	--sam	10/14/16
	if (!readDouble(fh,&rotfixextent.d,TRUE))
		return FALSE;
		
	}

		//rotation
	if (!readDouble(fh,&rotation,TRUE))
		return FALSE;
	//flags
	if (!readWord(fh,&flags))
		return FALSE;
	return TRUE;
}

// Original version translator 
/*
typedef struct
	{
	P2 origin;
	WORD flags; // nest bit flags
	} IMF4000NEST;

*/

// Read the doubles as floats since these were Win16 files 
BOOL IMFNEST::translateOriginalVersion(HANDLE fh){
		if(HideOptRotaryIMF)
	{
		//origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;	
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;

	//extent
	if (!readDouble(fh,&extent.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&extent.y,TRUE))
		return FALSE;

	}
	else
	{
		//origin
	if (!readDouble(fh,&originrot.x,TRUE))
		return FALSE;	
	if (!readDouble(fh,&originrot.y,TRUE))
		return FALSE;

		//Diameter for rotaryIMF	--sam	10/14/16
	if (!readDouble(fh,&rotfixextent.d,TRUE))
		return FALSE;
		
	}
	//rotation
	if (!readDouble(fh,&rotation,TRUE))
		return FALSE;
	//flags
	if (!readWord(fh,&flags))
		return FALSE;

	return TRUE;
}

