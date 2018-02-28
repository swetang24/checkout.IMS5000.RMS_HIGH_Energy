#include "stdafx.h"
#include "imp.h"
#include "imsio.h"

IMPHEADER::IMPHEADER()
	{
	desc[0]=0;
	author[0]=0;
	fixture[0]=0;
	partSize.x=0.0f;
	partSize.y=0.0f;
	partSize.z=0.0f;
    partDiameter.di=0.0f;//sam 01/07
	partDiameter.hi=0.0f;//sam 01/09
	inkjet[0]=0;
	origin.x=0.0f;
	origin.y=0.0f;
	rotation=0.0f;
	degree=0.0f;
	initsect[0]=0;
	link[0]=0;
	inconlink=FALSE;
	inspector[0]=0;
	inspectpos.x=0.0f;
	inspectpos.y=0.0f;
	inspectpos.z=1.0f;
	inspectprep[0]=0;
	conveyorstart=FALSE;
	linkautostart=FALSE;
	skiponlinkfail=FALSE;
	chainlinkfail=FALSE;
	zflight=0.0f;
	reader[0]=0;
	readpos.x=0.0f;
	readpos.y=0.0f;
	readpos.z=1.0f;
	readprep[0]=0;
	readvar[0]=0;
	}

BOOL IMPHEADER::write(HANDLE fh){
	
	// desc, author, fixture
	if (!writeString(fh,desc,IMPLDESC+1))
		return FALSE;
	if (!writeString(fh,author,IMPLAUTHOR+1))
		return FALSE;
	if (!writeString(fh,fixture,129))
		return FALSE;
	
		// partdiameter and height---------1/17/2017
	if (!writeDouble(fh,&partDiameter.di))
		return FALSE;
	if (!writeDouble(fh,&partDiameter.hi))
		return FALSE;
	// inkjet
	if (!writeString(fh,inkjet,41))
		return FALSE;
	
	// origin and rotation
	if (!writeDouble(fh,&origin.x))
		return FALSE;
	if (!writeDouble(fh,&origin.y))
		return FALSE;
	/*if (!writeDouble(fh,&rotation))
		return FALSE;
	//Rotation degree	--sam	10/7/16
	if(!HideOptRotaryIMF)
	{
	if (!writeDouble(fh,&degree))
		return FALSE;
	}*/
	
	// initsect, link, inconlink
	if (!writeString(fh,initsect,IMPLFNBASE+1))
		return FALSE;
	if (!writeString(fh,link,IMPLFNBASE+1))
		return FALSE;
	if (!writeInt(fh,&inconlink))
		return FALSE;
	
	// inspector
	if (!writeString(fh,inspector,IMPLFNBASE+1))
		return FALSE;

	// inspectpos
	if (!writeDouble(fh,&inspectpos.x))
		return FALSE;
	if (!writeDouble(fh,&inspectpos.y))
		return FALSE;
	if (!writeDouble(fh,&inspectpos.z))
		return FALSE;
	
	// inspectprep
	if (!writeString(fh,inspectprep,IMPLVISPREP+1))
		return FALSE;
	
	// conveyorstart, linkautostart, skiponlinkfail, chainlinkfail
	if (!writeInt(fh,&conveyorstart))
		return FALSE;
	if (!writeInt(fh,&linkautostart))
		return FALSE;
	if (!writeInt(fh,&skiponlinkfail))
		return FALSE;
	if (!writeInt(fh,&chainlinkfail))
		return FALSE;
	
	// zflight
	if (!writeDouble(fh,&zflight))
		return FALSE;
	
	// reader, readpos, readprep, readvar
	if (!writeString(fh,reader,IMPLFNBASE+1))
		return FALSE;
	if (!writeDouble(fh,&readpos.x))
		return FALSE;
	if (!writeDouble(fh,&readpos.y))
		return FALSE;
	if (!writeDouble(fh,&readpos.z))
		return FALSE;
	if (!writeString(fh,readprep,IMPLVISPREP+1))
		return FALSE;
	if (!writeString(fh,readvar,IMPLVNAME+1))
		return FALSE;

	return TRUE;
}

BOOL IMPHEADER::read(HANDLE fh, WORD ver){
	if (IMPIsNativeVersion(ver))
		return readnative(fh, ver);
	else 
		return translate(fh,ver);
}

BOOL IMPHEADER::readnative(HANDLE fh, WORD ver){
	// desc, author, fixture
	if (!readString(fh,desc,IMPLDESC+1))
		return FALSE;
	if (!readString(fh,author,IMPLAUTHOR+1))
		return FALSE;
	if (!readString(fh,fixture,129))
		return FALSE;
	
	// partdiameter and height---------1/17/2017
	if (!readDouble(fh,&partDiameter.di))
		return FALSE;
	if (!readDouble(fh,&partDiameter.hi))
		return FALSE;
	
	// inkjet
	if (!readString(fh,inkjet,41))
		return FALSE;
	
	// origin and rotation
	if (!readDouble(fh,&origin.x))
		return FALSE;
	if (!readDouble(fh,&origin.y))
		return FALSE;
	// initsect, link, inconlink
	if (!readString(fh,initsect,IMPLFNBASE+1))
		return FALSE;
	if (!readString(fh,link,IMPLFNBASE+1))
		return FALSE;
	if (!readInt(fh,&inconlink))
		return FALSE;
	
	// inspector
	if (!readString(fh,inspector,IMPLFNBASE+1))
		return FALSE;

	// inspectpos
	if (!readDouble(fh,&inspectpos.x))
		return FALSE;
	if (!readDouble(fh,&inspectpos.y))
		return FALSE;
	if (!readDouble(fh,&inspectpos.z))
		return FALSE;
	
	// inspectprep
	if (!readString(fh,inspectprep,IMPLVISPREP+1))
		return FALSE;
	
	// conveyorstart, linkautostart, skiponlinkfail, chainlinkfail
	if (!readInt(fh,&conveyorstart))
		return FALSE;
	if (!readInt(fh,&linkautostart))
		return FALSE;
	if (!readInt(fh,&skiponlinkfail))
		return FALSE;
	if (!readInt(fh,&chainlinkfail))
		return FALSE;
	
	// zflight
	if (!readDouble(fh,&zflight))
		return FALSE;
	
	// reader, readpos, readprep, readvar
	if (!readString(fh,reader,IMPLFNBASE+1))
		return FALSE;
	if (!readDouble(fh,&readpos.x))
		return FALSE;
	if (!readDouble(fh,&readpos.y))
		return FALSE;
	if (!readDouble(fh,&readpos.z))
		return FALSE;
	if (!readString(fh,readprep,IMPLVISPREP+1))
		return FALSE;
	if (!readString(fh,readvar,IMPLVNAME+1))
		return FALSE;

	return TRUE;
}

BOOL IMPHEADER::translate(HANDLE fh, WORD ver){
	if (ver >341 && ver <=368)
		return translateVers343Thru368(fh,ver);	
	else if (ver == 341 || ver == 339)
		return translateVer341(fh);
	else if (ver == 338)
		return translateVer338(fh);
	else if (ver == 330 || ver == 300)
		return translateOriginalVersion(fh);
	else
		return FALSE;
}

// Versions 3.43 thru 3.68 translation - 
// These are Win16 versions, so set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats
BOOL IMPHEADER::translateVers343Thru368(HANDLE fh, WORD ver){
	// desc, author, fixture
	if (!readString(fh,desc,IMPLDESC+1))
		return FALSE;
	if (!readString(fh,author,IMPLAUTHOR+1))
		return FALSE;
	if (!readString(fh,fixture,129))
		return FALSE;
	
	// partdiameter and height---------1/17/2017
	if (!readDouble(fh,&partDiameter.di,TRUE))
		return FALSE;
	if (!readDouble(fh,&partDiameter.hi,TRUE))
		return FALSE;
	
	// inkjet
	if (!readString(fh,inkjet,41))
		return FALSE;
	
	// origin and rotation
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	
	// initsect, link, inconlink
	if (!readString(fh,initsect,9))  
		return FALSE;
	if (!readString(fh,link,9))      
		return FALSE;
	if (!readInt(fh,&inconlink,TRUE))  // Read as short (2 bytes)
		return FALSE;
	
	// inspector
	if (!readString(fh,inspector,9))   
		return FALSE;

	// inspectpos
	if (!readDouble(fh,&inspectpos.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&inspectpos.y,TRUE))
		return FALSE;
	if (!readDouble(fh,&inspectpos.z,TRUE))
		return FALSE;
	
	// inspectprep
	if (!readString(fh,inspectprep,IMPLVISPREP+1))
		return FALSE;
	
	// conveyorstart, linkautostart, skiponlinkfail, chainlinkfail
	if (!readInt(fh,&conveyorstart,TRUE))
		return FALSE;
	if (!readInt(fh,&linkautostart,TRUE))
		return FALSE;
	if (!readInt(fh,&skiponlinkfail,TRUE))
		return FALSE;
	if (!readInt(fh,&chainlinkfail,TRUE))
		return FALSE;
	
	// zflight
	if (!readDouble(fh,&zflight,TRUE))
		return FALSE;
	
	// reader, readpos, readprep, readvar
	if (ver>350){
		if (!readString(fh,reader,8+1))
			return FALSE;
		if (!readDouble(fh,&readpos.x,TRUE))
			return FALSE;
		if (!readDouble(fh,&readpos.y,TRUE))
			return FALSE;
		if (!readDouble(fh,&readpos.z,TRUE))
			return FALSE;
		if (!readString(fh,readprep,IMPLVISPREP+1))
			return FALSE;
		if (!readString(fh,readvar,IMPLVNAME+1))
			return FALSE;
	} 
	else {
		// Fill in defaults for members added after 350
		reader[0]=0;
		readpos.x=0.0f;
		readpos.y=0.0f;
		readpos.z=1.0f;
		readprep[0]=0;
		readvar[0]=0;	
	}
	
	return TRUE;
}


BOOL IMPHEADER::translateVer341(HANDLE fh){
	
	// desc, author, fixture
	if (!readString(fh,desc,63+1))
		return FALSE;
	if (!readString(fh,author,32+1))
		return FALSE;
	if (!readString(fh,fixture,128+1))
		return FALSE;
	
	// partdiameter and height---------1/17/2017
	if (!readDouble(fh,&partDiameter.di,TRUE))
		return FALSE;
	if (!readDouble(fh,&partDiameter.hi,TRUE))
		return FALSE;

	
	// inkjet
	if (!readString(fh,inkjet,41))
		return FALSE;
	
	// origin and rotation
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	
	
	// initsect, link, inconlink
	if (!readString(fh,initsect,8+1))
		return FALSE;
	if (!readString(fh,link,8+1))
		return FALSE;
	if (!readInt(fh,&inconlink,TRUE))  // Read as short (2 bytes)
		return FALSE;
	
	// inspector
	if (!readString(fh,inspector,8+1))
		return FALSE;

	// inspectpos
	if (!readDouble(fh,&inspectpos.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&inspectpos.y,TRUE))
		return FALSE;
	if (!readDouble(fh,&inspectpos.z,TRUE))
		return FALSE;
	
	// inspectprep
	if (!readString(fh,inspectprep,64+1))
		return FALSE;
	
	// conveyorstart, linkautostart, skiponlinkfail, chainlinkfail
	if (!readInt(fh,&conveyorstart,TRUE))
		return FALSE;
	if (!readInt(fh,&linkautostart,TRUE))
		return FALSE;
	if (!readInt(fh,&skiponlinkfail,TRUE))
		return FALSE;
	if (!readInt(fh,&chainlinkfail,TRUE))
		return FALSE;

	// zflight
	if (!readDouble(fh,&zflight,TRUE))
		return FALSE;

	// reader, readpos, readprep, readvar - initialized by IMPHEADER constructor

	return TRUE;
}


BOOL IMPHEADER::translateVer338(HANDLE fh){
	
	// desc, author, fixture
	if (!readString(fh,desc,63+1))
		return FALSE;
	if (!readString(fh,author,32+1))
		return FALSE;
	if (!readString(fh,fixture,8+1))
		return FALSE;
	

	// partdiameter and height---------1/17/2017
	if (!readDouble(fh,&partDiameter.di,TRUE))
		return FALSE;
	if (!readDouble(fh,&partDiameter.hi,TRUE))
		return FALSE;

	
	// inkjet
	if (!readString(fh,inkjet,41))
		return FALSE;
	
	// origin and rotation
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	
	// initsect, link, inconlink
	if (!readString(fh,initsect,8+1))
		return FALSE;
	if (!readString(fh,link,8+1))
		return FALSE;
	if (!readInt(fh,&inconlink,TRUE))  // Read as short (2 bytes)
		return FALSE;
	
	// inspector
	if (!readString(fh,inspector,8+1))
		return FALSE;

	// inspectpos
	if (!readDouble(fh,&inspectpos.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&inspectpos.y,TRUE))
		return FALSE;
	if (!readDouble(fh,&inspectpos.z,TRUE))
		return FALSE;
	
	// inspectprep
	if (!readString(fh,inspectprep,64+1))
		return FALSE;
	
	// conveyorstart, linkautostart, skiponlinkfail, chainlinkfail
	if (!readInt(fh,&conveyorstart,TRUE))
		return FALSE;
	if (!readInt(fh,&linkautostart,TRUE))
		return FALSE;
	if (!readInt(fh,&skiponlinkfail,TRUE))
		return FALSE;
	if (!readInt(fh,&chainlinkfail,TRUE))
		return FALSE;

	// zflight, reader, readpos, readprep, readvar - initialized by IMPHEADER constructor

	return TRUE;
}


BOOL IMPHEADER::translateOriginalVersion(HANDLE fh){
	
	// desc, author, fixture
	if (!readString(fh,desc,63+1))
		return FALSE;
	if (!readString(fh,author,32+1))
		return FALSE;
	if (!readString(fh,fixture,8+1))
		return FALSE;
	

	// partdiameter and height---------1/17/2017
	if (!readDouble(fh,&partDiameter.di,TRUE))
		return FALSE;
	if (!readDouble(fh,&partDiameter.hi,TRUE))
		return FALSE;
	

	// inkjet
	if (!readString(fh,inkjet,41))
		return FALSE;
	
	// origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	
    // All other native attributes initialized by IMPHEADER constructor 
	return TRUE;
}