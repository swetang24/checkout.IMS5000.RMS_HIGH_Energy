#include "stdafx.h"
#include "imf.h"
#include "imsio.h"

/******************************************************************************
	IMFHEADER CLASS
******************************************************************************/
// Fill in header members with defaults (except ofs)
IMFHEADER::IMFHEADER()
	{
	origin.x=0.0;
	origin.y=0.0;
	extent.x=10.0;
	extent.y=10.0;
	extent.z=0.0;
	originrot.x=0.0;//sam
	originrot.y=0.0;//sam
	extentrot.d=10.0;	//--sam	10/6/16
	extentrot.z=0.0;	//--sam	10/6/16
	desc[0]=0;
	xymode=imXY;
	ccwplus=FALSE;
	rfdrv[0]=0;
	rotation=0.0f;
	}

BOOL IMFHEADER::write(HANDLE fh)
	{
	
	// extent
	if(HideOptRotaryIMF)
	{
		// origin
		if (!writeDouble(fh,&origin.x))
			return FALSE;
		if (!writeDouble(fh,&origin.y))
			return FALSE;
		if (!writeDouble(fh,&extentrot.d))
			return FALSE;
		if (!writeDouble(fh,&extentrot.z))
			return FALSE;

	}
	else
	{
		// origin
		if (!writeDouble(fh,&originrot.x))
			return FALSE;
		if (!writeDouble(fh,&originrot.y))
			return FALSE;
        
		// extent for rotary	--sam 10/6/16
		if (!writeDouble(fh,&extentrot.d))
			return FALSE;
		if (!writeDouble(fh,&extentrot.z))
			return FALSE;
	}
	// desc
	if (!writeString(fh,desc,IMFLDESC+1))
		return FALSE;
	// xymode
	if (!writeInt(fh,(int*)&xymode))
		return FALSE;
	// ccwplus
	if (!writeInt(fh,&ccwplus))
		return FALSE;
	// rfdrv
	if (!writeString(fh,rfdrv,IMFLRFDRV+1))
		return FALSE;
	// rotation
	if (!writeDouble(fh,&rotation))
		return FALSE;

	return TRUE;
	}

BOOL IMFHEADER::read(HANDLE fh, WORD ver)
	{
	if (ver==0)
		return readnative(fh,ver);
	else 
		return translate(fh,ver);
	}

BOOL IMFHEADER::readnative(HANDLE fh, WORD ver)
	{
	
	if(HideOptRotaryIMF)
	{
		// origin
		if (!readDouble(fh,&origin.x))
			return FALSE;
		if (!readDouble(fh,&origin.y))
			return FALSE;
        
		// extent for rotary		--sam 10/6/16
		if (!readDouble(fh,&extentrot.d))
			return FALSE;
		if (!readDouble(fh,&extentrot.z))
			return FALSE;
		

	}
	else
	{
		// origin
		if (!readDouble(fh,&originrot.x))
			return FALSE;
		if (!readDouble(fh,&originrot.y))
			return FALSE;
        
		// extent for rotary		--sam 10/6/16
		if (!readDouble(fh,&extentrot.d))
			return FALSE;
		if (!readDouble(fh,&extentrot.z))
			return FALSE;
		
		
	}
	
	// desc
	if (!readString(fh,desc,IMFLDESC+1))
		return FALSE;
	// xymode
	if (!readInt(fh,(int*)&xymode))
		return FALSE;
	// ccwplus
	if (!readInt(fh,&ccwplus))
		return FALSE;
	// rfdrv
	if (!readString(fh,rfdrv,IMFLRFDRV+1))
		return FALSE;
	// rotation
	if (!readDouble(fh,&rotation))
		return FALSE;

	return TRUE;
	}

BOOL IMFHEADER::translate(HANDLE fh, WORD ver)	
	{
	if (ver >338 && ver <=357){
		return translateVers345Thru357(fh,ver);	
	} else if (ver == 338){
		return translateVer338(fh);	
	} else if (ver == 330 || ver == 300)
		return translateOriginalVersion(fh);	
	return TRUE;
	}

// These are from Win16, so read doubles as floats and ints as shorts
BOOL IMFHEADER::translateVers345Thru357(HANDLE fh, WORD ver)	
	{
	if(HideOptRotaryIMF)
	{
		// origin
		if (!readDouble(fh,&origin.x,TRUE))     // 3rd param TRUE = Read as float
			return FALSE;
		if (!readDouble(fh,&origin.y,TRUE))
			return FALSE;
		// extent for rotary	--sam	10/6/16
	    if (!readDouble(fh,&extentrot.d,TRUE))
			return FALSE;
		if (!readDouble(fh,&extentrot.z,TRUE))
		return FALSE;
	
	}
	else
	{
		// origin
		if (!readDouble(fh,&originrot.x,TRUE))     // 3rd param TRUE = Read as float
			return FALSE;
		if (!readDouble(fh,&originrot.y,TRUE))
			return FALSE;

		// extent for rotary	--sam	10/6/16
	    if (!readDouble(fh,&extentrot.d,TRUE))
			return FALSE;
		if (!readDouble(fh,&extentrot.z,TRUE))
		return FALSE;
	
	}
	// desc
	if (!readString(fh,desc,IMFLDESC+1))
		return FALSE;
	// xymode
	if (!readInt(fh,(int*)&xymode,TRUE))    // 3rd param TRUE = Read as short
		return FALSE;
	// ccwplus
	if (!readInt(fh,&ccwplus,TRUE))
		return FALSE;
	// rfdrv
	if (!readString(fh,rfdrv,IMFLRFDRV+1))
		return FALSE;
	// rotation
	if (ver > 345){
		if (!readDouble(fh,&rotation,TRUE))
			return FALSE;
	} else {  // No rotation in 345
		rotation=0.0;
	}

	return TRUE;
	}

// V 3.38 translator
/*
typedef struct
	{
	P2 origin;             // The fixture origin
	P3 extent;             // The fixtures outer dimensions.
	char desc[63+1]; // Description text
	} IMFV338HEADER;
*/
BOOL IMFHEADER::translateVer338(HANDLE fh)	
	{
	if(HideOptRotaryIMF)
	{	
	// origin
	if (!readDouble(fh,&origin.x,TRUE))     // 3rd param TRUE = Read as float
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;

		// extent for rotary	--sam	10/6/16
		if (!readDouble(fh,&extentrot.d,TRUE))
			return FALSE;
		if (!readDouble(fh,&extentrot.z,TRUE))
			return FALSE;


	}
	else
	{
	// origin
	if (!readDouble(fh,&originrot.x,TRUE))     // 3rd param TRUE = Read as float
		return FALSE;
	if (!readDouble(fh,&originrot.y,TRUE))
		return FALSE;

		// extent for rotary	--sam	10/6/16
		if (!readDouble(fh,&extentrot.d,TRUE))
			return FALSE;
		if (!readDouble(fh,&extentrot.z,TRUE))
			return FALSE;

		

	}

	// desc
	if (!readString(fh,desc,IMFLDESC+1))
		return FALSE;

	return TRUE;
	}

// V 3.30 and original version have the same header
/*
typedef struct // Inkjet Marking Fixture
	{
	P3 extent;       // The fixtures outer dimensions.
	} 
	IMF4000HEADER;
*/
BOOL IMFHEADER::translateOriginalVersion(HANDLE fh)	
	{
		if(HideOptRotaryIMF)
		{
			// extent
			if (!readDouble(fh,&extent.x,TRUE))  // 3rd param TRUE = Read as float
				return FALSE;
			if (!readDouble(fh,&extent.y,TRUE))
				return FALSE;
			if (!readDouble(fh,&extent.z,TRUE))
				return FALSE;

		}
		else
		{
			// extend for rotary	--sam 10/6/16
			if (!readDouble(fh,&extentrot.d,TRUE))  // 3rd param TRUE = Read as float
				return FALSE;
			if (!readDouble(fh,&extentrot.z,TRUE))
				return FALSE;

		}
	return TRUE;
	}

