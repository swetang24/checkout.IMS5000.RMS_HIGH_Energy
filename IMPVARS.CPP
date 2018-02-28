#include "stdafx.h"
#include "imp.h"
#include "imsio.h"
/******************************************************************************
	IMPVAR CLASS
******************************************************************************/
IMPVAR::IMPVAR()
	{
	name[0]=0;
	width=20;
	lpad=0;
	unique=0;
	}

IMPVAR::~IMPVAR()
{
}

/* Initializes the type members - Update this function if new fields are added */
void IMPVAR::initByType(){
	switch(type){
	case vtKEYED:
		k.digits = TRUE;
		k.lowers = TRUE;
		k.others = TRUE;
		k.space  = TRUE;
		k.uppers = TRUE;
		break;
	case vtSERIAL:
		s.repno = 0;
		s.step = 1;
		s.dirpos = FALSE;
		s.repeat = 0;
		s.minval = 0;    
		s.maxval = 1;    
		s.loop = FALSE;      
		break;
	case vtBASED:
		//HAC40
		lstrcpy(b.base,"HAC40");
		b.loop = FALSE;
		break;
	case vtTIME:
		lstrcpy(t.amSuffix,"AM");
		lstrcpy(t.base,"10");
		t.canOverride=FALSE;
		t.is24=FALSE;
		t.isLZ=FALSE;
		t.offset=0;
		lstrcpy(t.pmSuffix,"PM");
		t.seperator=':';
		t.showSeconds=TRUE;
		t.useNames=FALSE;
		break;
	case vtDATE:
		lstrcpy(d.base,"10");
		d.canOverride=FALSE;
		lstrcpy(d.format,"M/D/YYYY");
		d.offset=0;
		break;
	default:;
	}
}

BOOL IMPVAR::read(HANDLE fh, WORD ver){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver);
	else 
		return translate(fh,ver);
}

BOOL IMPVAR::readnative(HANDLE fh, WORD ver){
	
	// Name
	if (!readString(fh,name,IMPLVNAME+1))
		return FALSE;
	// Width
	if (!readInt(fh,&width))
		return FALSE;
	// Lpad
	if (!readByte(fh,(BYTE*) &lpad))
		return FALSE;
	// Type
	if (!readInt(fh,(int*) &type))
		return FALSE;		
	// Unique
	if (!readInt(fh,&unique))
		return FALSE;
	
	// Preval
	switch (type){
	case vtKEYED:
		if (!readString(fh,preval.k,IMPLKVVAL+1))  // 49
			return FALSE;
		break;
	case vtSERIAL:
		// s
		if (!readDoubleWord(fh,&preval.s))  // 4
			return FALSE;
		// c
		if (!readWord(fh,&preval.c))     // 2
			return FALSE;
		break;
	case vtBASED:
		if (!readString(fh,preval.b,IMPLBASEDVAL+1))  // 26
			return FALSE;
		break;
	case vtDATE:
		if (!readString(fh,preval.d.override,IMPLDATEOVERRIDE+1))  // 41
			return FALSE;
		break;
	case vtTIME:
		if (!readString(fh,preval.t.override,IMPLTIMEOVERRIDE+1))  // 41
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	} 
	
	// Type-specific info: Init it
	initByType();
	// Read it
	switch (type){
	case vtKEYED:
		// Digits
		if (!readInt(fh,&k.digits))
			return FALSE;
		// Lowers
		if (!readInt(fh,&k.lowers))
			return FALSE;
		// Uppers
		if (!readInt(fh,&k.uppers))
			return FALSE;
		// Space
		if (!readInt(fh,&k.space))
			return FALSE;
		// Others
		if (!readInt(fh,&k.others))
			return FALSE;
		break;
	case vtSERIAL:
		// repno
		if (!readWord(fh,&s.repno))
			return FALSE;
		// step
		if (!readDoubleWord(fh,&s.step))
			return FALSE;
		// dirpos
		if (!readInt(fh,&s.dirpos))
			return FALSE;
		// repeat
		if (!readWord(fh,&s.repeat))
			return FALSE;
		// minval
		if (!readDoubleWord(fh,&s.minval))
			return FALSE;
		// maxval
		if (!readDoubleWord(fh,&s.maxval))
			return FALSE;
		// loop
		if (!readInt(fh,&s.loop))
			return FALSE;
		break;
	case vtBASED:
		// base
		if (!readString(fh,b.base,IMPLBASEDNAME+1))
			return FALSE;
		// loop
		if (!readInt(fh,&b.loop))
			return FALSE;
		break;
	case vtDATE:
		// offset
		if (!readInt(fh,(int*)&d.offset))
			return FALSE;
		// format
		if (!readString(fh,d.format,IMPLDATEFORMAT+1))
			return FALSE;
		// canOverride
		if (!readInt(fh,&d.canOverride))
			return FALSE;
		// base
		if (!readString(fh,d.base,IMPLBASEDNAME+1))
			return FALSE;
		break;
	case vtTIME:
		// offset
		if (!readInt(fh,(int*)&t.offset))
			return FALSE;
		// useNames
		if (!readInt(fh,&t.useNames))
			return FALSE;
		// is24
		if (!readInt(fh,&t.is24))
			return FALSE;
		// isLZ
		if (!readInt(fh,&t.isLZ))
			return FALSE;
		// seperator
		if (!readByte(fh,(BYTE*)&t.seperator))
			return FALSE;
		// amSuffix
		if (!readString(fh,t.amSuffix,IMPLTIMESUFFIX+1))
			return FALSE;
		// pmSuffix
		if (!readString(fh,t.pmSuffix,IMPLTIMESUFFIX+1))
			return FALSE;
		// showSeconds
		if (!readInt(fh,&t.showSeconds))
			return FALSE;
		// canOverride
		if (!readInt(fh,&t.canOverride))
			return FALSE;
		// base
		if (!readString(fh,t.base,IMPLBASEDNAME+1))
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	}

	return TRUE;
}

BOOL IMPVAR::write(HANDLE fh){
	
	// Name
	if (!writeString(fh,name,IMPLVNAME+1))
		return FALSE;
	// Width
	if (!writeInt(fh,&width))
		return FALSE;
	// Lpad
	if (!writeByte(fh,(BYTE*) &lpad))
		return FALSE;
	// Type
	if (!writeInt(fh,(int*) &type))
		return FALSE;		
	// Unique
	if (!writeInt(fh,&unique))
		return FALSE;
	
	// Preval
	switch (type){
	case vtKEYED:
		if (!writeString(fh,preval.k,IMPLKVVAL+1))  // 49
			return FALSE;
		break;
	case vtSERIAL:
		// s
		if (!writeDoubleWord(fh,&preval.s))  // 4
			return FALSE;
		// c
		if (!writeWord(fh,&preval.c))     // 2
			return FALSE;
		break;
	case vtBASED:
		if (!writeString(fh,preval.b,IMPLBASEDVAL+1))  // 26
			return FALSE;
		break;
	case vtDATE:
		if (!writeString(fh,preval.d.override,IMPLDATEOVERRIDE+1))  // 41
			return FALSE;
		break;
	case vtTIME:
		if (!writeString(fh,preval.t.override,IMPLTIMEOVERRIDE+1))  // 41
			return FALSE;
		break;
	default:
		IDError = IDE_WRITEFILE;
		return FALSE;
	} 
	
	// Type-specific info		
	switch (type){
	case vtKEYED:
		// Digits
		if (!writeInt(fh,&k.digits))
			return FALSE;
		// Lowers
		if (!writeInt(fh,&k.lowers))
			return FALSE;
		// Uppers
		if (!writeInt(fh,&k.uppers))
			return FALSE;
		// Space
		if (!writeInt(fh,&k.space))
			return FALSE;
		// Others
		if (!writeInt(fh,&k.others))
			return FALSE;
		break;
	case vtSERIAL:
		// repno
		if (!writeWord(fh,&s.repno))
			return FALSE;
		// step
		if (!writeDoubleWord(fh,&s.step))
			return FALSE;
		// dirpos
		if (!writeInt(fh,&s.dirpos))
			return FALSE;
		// repeat
		if (!writeWord(fh,&s.repeat))
			return FALSE;
		// minval
		if (!writeDoubleWord(fh,&s.minval))
			return FALSE;
		// maxval
		if (!writeDoubleWord(fh,&s.maxval))
			return FALSE;
		// loop
		if (!writeInt(fh,&s.loop))
			return FALSE;
		break;
	case vtBASED:
		// base
		if (!writeString(fh,b.base,IMPLBASEDNAME+1))
			return FALSE;
		// loop
		if (!writeInt(fh,&b.loop))
			return FALSE;
		break;
	case vtDATE:
		// offset
		if (!writeInt(fh,(int*)&d.offset))
			return FALSE;
		// format
		if (!writeString(fh,d.format,IMPLDATEFORMAT+1))
			return FALSE;
		// canOverride
		if (!writeInt(fh,&d.canOverride))
			return FALSE;
		// base
		if (!writeString(fh,d.base,IMPLBASEDNAME+1))
			return FALSE;
		break;
	case vtTIME:
		// offset
		if (!writeInt(fh,(int*)&t.offset))
			return FALSE;
		// useNames
		if (!writeInt(fh,&t.useNames))
			return FALSE;
		// is24
		if (!writeInt(fh,&t.is24))
			return FALSE;
		// isLZ
		if (!writeInt(fh,&t.isLZ))
			return FALSE;
		// seperator
		if (!writeByte(fh,(BYTE*)&t.seperator))
			return FALSE;
		// amSuffix
		if (!writeString(fh,t.amSuffix,IMPLTIMESUFFIX+1))
			return FALSE;
		// pmSuffix
		if (!writeString(fh,t.pmSuffix,IMPLTIMESUFFIX+1))
			return FALSE;
		// showSeconds
		if (!writeInt(fh,&t.showSeconds))
			return FALSE;
		// canOverride
		if (!writeInt(fh,&t.canOverride))
			return FALSE;
		// base
		if (!writeString(fh,t.base,IMPLBASEDNAME+1))
			return FALSE;
		break;
	default:
		IDError = IDE_WRITEFILE;
		return FALSE;
	}
	
	return TRUE;
}

BOOL IMPVAR::translate(HANDLE fh, WORD ver){
	if (ver >341 && ver <=368)
		return translateVers343Thru368(fh,ver);	
	else if (ver == 341 || ver == 339)
		return translateVer341(fh);	
	else if (ver == 330 || ver == 338)
		return translateVers330Thru338(fh);	
	else if (ver == 300)
		return translateOriginalVersion(fh);
	else
		return FALSE;
}

/***********************************************************************

  Versions 3.43-3.68 - These are Win16 versions, so set 3rd argument of 
  the ReadInt and ReadDouble functions to read them from the buffer as
  shorts and floats - o/w same format as native

************************************************************************/
BOOL IMPVAR::translateVers343Thru368(HANDLE fh, WORD ver){
	// Name
	if (!readString(fh,name,IMPLVNAME+1))
		return FALSE;
	// Width
	if (!readInt(fh,&width,TRUE))
		return FALSE;
	// Lpad
	if (!readByte(fh,(BYTE*) &lpad))
		return FALSE;
	IMPV343VARTYPE vt;
	// Type and unique - one of few cases where it is safe to read into a struct
	if (!readInt(fh,(int*) &vt,TRUE)) 
		return FALSE;
	type = (IMPVARTYPE) vt.type;   // Was 15 bits
	unique = vt.unique;			// Was 1 bit
	char buf[50];
	// Preval - after v 3.55
	if (ver>355){
	switch (type){
	case vtKEYED:
		if (!readString(fh,preval.k,IMPLKVVAL+1))  // 49
			return FALSE;

		break;
	case vtSERIAL:
		// s
		if (!readDoubleWord(fh,&preval.s))  // 4
			return FALSE;
		// c
		if (!readWord(fh,&preval.c))     // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,43))  // 43 
			return FALSE;
		break;
	case vtBASED:
		if (!readString(fh,preval.b,IMPLBASEDVAL+1))  // 26
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,23))  // 23 
			return FALSE;
		break;
	case vtDATE:
		if (!readString(fh,preval.d.override,IMPLDATEOVERRIDE+1))  // 41
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,8))  // 8 
			return FALSE;
		break;
	case vtTIME:
		if (!readString(fh,preval.t.override,IMPLTIMEOVERRIDE+1))  // 41
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,8))  // 8 
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	} 
	} else {
		IMPInitVal(&preval,this);
	}
	// Initialize type-specific info
	initByType();
	// Type-specific info (union size: 48)		
	switch (type){
	case vtKEYED:
		// Digits
		if (!readInt(fh,&k.digits,TRUE))  // 2
			return FALSE;
		// Lowers
		if (!readInt(fh,&k.lowers,TRUE))  // 2
			return FALSE;
		// Uppers
		if (!readInt(fh,&k.uppers,TRUE))  // 2
			return FALSE;
		// Space
		if (!readInt(fh,&k.space,TRUE))   // 2
			return FALSE;
		// Others
		if (!readInt(fh,&k.others,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,38))  // 38 
			return FALSE;
		break;
	case vtSERIAL:
		// repno
		if (!readWord(fh,&s.repno))   // 2
			return FALSE;
		// step
		if (!readDoubleWord(fh,&s.step))  // 4
			return FALSE;
		// dirpos
		if (!readInt(fh,&s.dirpos,TRUE))  // 2
			return FALSE;
		// repeat
		if (!readWord(fh,&s.repeat))  // 2
			return FALSE;
		// minval
		if (!readDoubleWord(fh,&s.minval))  // 4
			return FALSE;
		// maxval
		if (!readDoubleWord(fh,&s.maxval))  // 4
			return FALSE;
		// loop
		if (!readInt(fh,&s.loop,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,28))  // 28 
			return FALSE;
		break;
	case vtBASED:
		// base
		if (!readString(fh,b.base,IMPLBASEDNAME+1))  // 21
			return FALSE;
		// loop
		if (!readInt(fh,&b.loop,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,25))  // 25 
			return FALSE;
		break;
	case vtDATE:  // Both of date, time are 48 bytes (union size)
		// offset
		if (!readInt(fh,(int*)&d.offset))  // 4
			return FALSE;
		// format
		if (!readString(fh,d.format,IMPLDATEFORMAT+1))  // 21
			return FALSE;
		// canOverride
		if (!readInt(fh,&d.canOverride,TRUE))  // 2
			return FALSE;
		// base
		if (!readString(fh,d.base,IMPLBASEDNAME+1))  // 21
			return FALSE;
		break;
	case vtTIME:
		// offset
		if (!readInt(fh,(int*)&t.offset))  // 4
			return FALSE;
		// useNames
		if (!readInt(fh,&t.useNames,TRUE))  // 2
			return FALSE;
		// is24
		if (!readInt(fh,&t.is24,TRUE))   // 2
			return FALSE;
		// isLZ
		if (!readInt(fh,&t.isLZ,TRUE))  // 2
			return FALSE;
		// seperator
		if (!readByte(fh,(BYTE*)&t.seperator)) //1
			return FALSE;
		// amSuffix
		if (!readString(fh,t.amSuffix,IMPLTIMESUFFIX+1))  // 6
			return FALSE;
		// pmSuffix
		if (!readString(fh,t.pmSuffix,IMPLTIMESUFFIX+1))  // 6
			return FALSE;
		// showSeconds
		if (!readInt(fh,&t.showSeconds,TRUE))  // 2
			return FALSE;
		// canOverride
		if (!readInt(fh,&t.canOverride,TRUE))  // 2
			return FALSE;
		// base
		if (!readString(fh,t.base,IMPLBASEDNAME+1))  // 21
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	}

	return TRUE;
}

// Version 3.41 - Set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats

//--------- OLD STRUCTURES ----------//
/*
//typedef enum {vtV341KEYED,vtV341SERIAL,vtV341DATE,vtV341TIME,vtV341BASED} IMPV341VARTYPE;

	{
	BOOL digits;
	BOOL lowers;
	BOOL uppers;
	BOOL space;
	BOOL others;
	} IMPV341KEYEDVAR;

	{
	WORD  repno;
	DWORD step;
	BOOL  dirpos;
	WORD  repeat;
	DWORD minval;
	DWORD maxval;
	BOOL  loop;
	} IMPV341SERIALVAR;

	{
	char base[20+1]; // name of base from ini file
	BOOL loop;      // Sequence either loops or stops
	} IMPV341BASEDVAR;

	{
	long offset;                   // date offset in seconds
	char format[20+1]; // Date format string
	BOOL canOverride;  // True if user can override value at runtime
	char base[20+1]; // name of base from ini file (10 if null)
	} IMPV341DATEVAR;

	{
	long offset;    // clock offset in seconds (-86400<offset<86400)
	BOOL useNames;  // Use time names from ini
	BOOL is24;      // True if 24 hour clock (!useNames)
	BOOL isLZ;      // True if leading zero on hour (!useNames)
	char seperator; // seperator character (!useNames)
	char amSuffix[5+1]; // 12hr AM suffix (!useNames)
	char pmSuffix[5+1]; // 12hr PM|24hr suffix (!useNames)
	BOOL showSeconds; // True if seconds should be displayed (!useNames)
	BOOL canOverride; // True if user can override value at runtime
	char base[20+1]; // name of base from ini file (10 if null)
	} IMPV341TIMEVAR;

	{
	char name[32+1];   // Variable name
	WORD width;               // Max width of variable
	char lpad;                // if(lpad) left pad to width with lpad
	IMPV341VARTYPE type;          // Variable type
	union
		{
		IMPV341KEYEDVAR  k;
		IMPV341SERIALVAR s;
		IMPV341DATEVAR   d;
		IMPV341TIMEVAR   t;
		IMPV341BASEDVAR  b;
		};
	} IMPV341VAR;
*/

BOOL IMPVAR::translateVer341(HANDLE fh){
	// Name
	if (!readString(fh,name,32+1))
		return FALSE;
	// Width
	if (!readInt(fh,&width,TRUE))
		return FALSE;
	// Lpad
	if (!readByte(fh,(BYTE*) &lpad))
		return FALSE;

	IMPV341VARTYPE ty;
	// Type
	if (!readInt(fh,(int*) &ty,TRUE))  
		return FALSE;
	
	switch(ty){
	case vtV341KEYED:
		type = vtKEYED;
		break;
	case vtV341SERIAL:
		type = vtSERIAL;
		break;
	case vtV341DATE:
		type = vtDATE;
		break;
	case vtV341TIME:
		type = vtTIME;
		break;
	case vtV341BASED:
		type = vtBASED;
		break;	
	default:
		return FALSE;
	}


	char buf[50];
	
	// Preval - init it
	IMPInitVal(&preval,this);
	// Init the type specific info - update this function if new fields are added
	initByType();
	
	// Read the type specific info (union size: 48)		
	switch (type){
	case vtKEYED:
		// Digits
		if (!readInt(fh,&k.digits,TRUE))  // 2
			return FALSE;
		// Lowers
		if (!readInt(fh,&k.lowers,TRUE))  // 2
			return FALSE;
		// Uppers
		if (!readInt(fh,&k.uppers,TRUE))  // 2
			return FALSE;
		// Space
		if (!readInt(fh,&k.space,TRUE))   // 2
			return FALSE;
		// Others
		if (!readInt(fh,&k.others,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,38))  // 38 
			return FALSE;
		break;
	case vtSERIAL:
		// repno
		if (!readWord(fh,&s.repno))   // 2
			return FALSE;
		// step
		if (!readDoubleWord(fh,&s.step))  // 4
			return FALSE;
		// dirpos
		if (!readInt(fh,&s.dirpos,TRUE))  // 2
			return FALSE;
		// repeat
		if (!readWord(fh,&s.repeat))  // 2
			return FALSE;
		// minval
		if (!readDoubleWord(fh,&s.minval))  // 4
			return FALSE;
		// maxval
		if (!readDoubleWord(fh,&s.maxval))  // 4
			return FALSE;
		// loop
		if (!readInt(fh,&s.loop,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,28))  // 28 
			return FALSE;
		break;
	case vtBASED:
		// base
		if (!readString(fh,b.base,20+1))  // 21
			return FALSE;
		// loop
		if (!readInt(fh,&b.loop,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,25))  // 25 
			return FALSE;
		break;
	case vtDATE:  // Both of date, time are 48 bytes (union size)
		// offset
		if (!readInt(fh,(int*)&d.offset))  // 4
			return FALSE;
		// format
		if (!readString(fh,d.format,20+1))  // 21
			return FALSE;
		// canOverride
		if (!readInt(fh,&d.canOverride,TRUE))  // 2
			return FALSE;
		// base
		if (!readString(fh,d.base,20+1))  // 21
			return FALSE;
		break;
	case vtTIME:
		// offset
		if (!readInt(fh,(int*)&t.offset))  // 4
			return FALSE;
		// useNames
		if (!readInt(fh,&t.useNames,TRUE))  // 2
			return FALSE;
		// is24
		if (!readInt(fh,&t.is24,TRUE))   // 2
			return FALSE;
		// isLZ
		if (!readInt(fh,&t.isLZ,TRUE))  // 2
			return FALSE;
		// seperator
		if (!readByte(fh,(BYTE*)&t.seperator)) //1
			return FALSE;
		// amSuffix
		if (!readString(fh,t.amSuffix,5+1))  // 6
			return FALSE;
		// pmSuffix
		if (!readString(fh,t.pmSuffix,5+1))  // 6
			return FALSE;
		// showSeconds
		if (!readInt(fh,&t.showSeconds,TRUE))  // 2
			return FALSE;
		// canOverride
		if (!readInt(fh,&t.canOverride,TRUE))  // 2
			return FALSE;
		// base
		if (!readString(fh,t.base,20+1))  // 21
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	}

	return TRUE;
}

// Versions 3.30 and 3.38 - Set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats

//--------- OLD STRUCTURES (same format for both versions) ----------//
/*

typedef enum {vtV330KEYED,vtV330SERIAL,vtV330DATE,vtV330TIME,vtV330B40} IMPV330VARTYPE;
typedef enum {vtV338KEYED,vtV338SERIAL,vtV338DATE,vtV338TIME,vtV338B40} IMPV338VARTYPE;

typedef struct
	{
	BOOL digits;
	BOOL lowers;
	BOOL uppers;
	BOOL space;
	BOOL others;
	} IMPV338KEYEDVAR;

typedef struct
	{
	WORD  repno;
	DWORD step;
	BOOL  dirpos;
	WORD  repeat;
	DWORD minval;
	DWORD maxval;
	BOOL  loop;
	} IMPV338SERIALVAR;

typedef struct
	{
	WORD dummy;
	} IMPV338B40VAR;

typedef struct
	{
	long offset;
	char format[20+1];
	BOOL canOverride;
	} IMPV338DATEVAR;

typedef struct
	{
	long offset;
	BOOL useNames;
	BOOL is24;
	BOOL isLZ;
	char seperator;
	char amSuffix[5+1];
	char pmSuffix[5+1];
	BOOL showSeconds;
	BOOL canOverride;
	} IMPV338TIMEVAR;

typedef struct
	{
	char name[32+1];
	WORD width;
	char lpad;
	IMPV338VARTYPE type;
	union
		{
		IMPV338KEYEDVAR  k;
		IMPV338SERIALVAR s;
		IMPV338DATEVAR   d;
		IMPV338TIMEVAR   t;
		IMPV338B40VAR    b40;
		};
	} IMPV338VAR;

*/

BOOL IMPVAR::translateVers330Thru338(HANDLE fh){
	// Name
	if (!readString(fh,name,32+1))
		return FALSE;
	// Width
	if (!readInt(fh,&width,TRUE))
		return FALSE;
	// Lpad
	if (!readByte(fh,(BYTE*) &lpad))
		return FALSE;

	IMPV338VARTYPE ty;
	// Type
	if (!readInt(fh,(int*) &ty,TRUE))  
		return FALSE;
	
	switch(ty){
//	case vtV330KEYED:
	case vtV338KEYED:
		type = vtKEYED;
		break;
//	case vtV330SERIAL:
	case vtV338SERIAL:
		type = vtSERIAL;
		break;
//	case vtV330DATE:
	case vtV338DATE:
		type = vtDATE;
		break;
//	case vtV330TIME:
	case vtV338TIME:
		type = vtTIME;
		break;
//	case vtV330B40:
	case vtV338B40:
		type = vtBASED;
		break;	
	default:
		return FALSE;
	}


	char buf[50];
	
	// Preval - init it
	IMPInitVal(&preval,this);
	// Init the type specific info - update this function if new fields are added
	initByType();
	
	// Read the type specific info (union size: 27)		
	switch (type){
	case vtKEYED:
		// Digits
		if (!readInt(fh,&k.digits,TRUE))  // 2
			return FALSE;
		// Lowers
		if (!readInt(fh,&k.lowers,TRUE))  // 2
			return FALSE;
		// Uppers
		if (!readInt(fh,&k.uppers,TRUE))  // 2
			return FALSE;
		// Space
		if (!readInt(fh,&k.space,TRUE))   // 2
			return FALSE;
		// Others
		if (!readInt(fh,&k.others,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,17))  
			return FALSE;
		break;
	case vtSERIAL:
		// repno
		if (!readWord(fh,&s.repno))   // 2
			return FALSE;
		// step
		if (!readDoubleWord(fh,&s.step))  // 4
			return FALSE;
		// dirpos
		if (!readInt(fh,&s.dirpos,TRUE))  // 2
			return FALSE;
		// repeat
		if (!readWord(fh,&s.repeat))  // 2
			return FALSE;
		// minval
		if (!readDoubleWord(fh,&s.minval))  // 4
			return FALSE;
		// maxval
		if (!readDoubleWord(fh,&s.maxval))  // 4
			return FALSE;
		// loop
		if (!readInt(fh,&s.loop,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,7))  
			return FALSE;
		break;
	case vtBASED:
		// Read the whole union  (Loop and base set by initByType()) 
		if (!readString(fh,buf,27)) 
			return FALSE;
		break;
	case vtDATE:  // Both of date, time are 48 bytes (union size)
		// offset
		if (!readInt(fh,(int*)&d.offset))  // 4
			return FALSE;
		// format
		if (!readString(fh,d.format,20+1))  // 21
			return FALSE;
		// canOverride
		if (!readInt(fh,&d.canOverride,TRUE))  // 2
			return FALSE;
		break;
	case vtTIME:  
		// offset
		if (!readInt(fh,(int*)&t.offset))  // 4
			return FALSE;
		// useNames
		if (!readInt(fh,&t.useNames,TRUE))  // 2
			return FALSE;
		// is24
		if (!readInt(fh,&t.is24,TRUE))   // 2
			return FALSE;
		// isLZ
		if (!readInt(fh,&t.isLZ,TRUE))  // 2
			return FALSE;
		// seperator
		if (!readByte(fh,(BYTE*)&t.seperator)) //1
			return FALSE;
		// amSuffix
		if (!readString(fh,t.amSuffix,5+1))  // 6
			return FALSE;
		// pmSuffix
		if (!readString(fh,t.pmSuffix,5+1))  // 6
			return FALSE;
		// showSeconds
		if (!readInt(fh,&t.showSeconds,TRUE))  // 2
			return FALSE;
		// canOverride
		if (!readInt(fh,&t.canOverride,TRUE))  // 2
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	}

	return TRUE;
}

// Original version (LSIGIMP4001) - Set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats 

// Only difference from 3.30 and 3.38 - No dates and times
/*
typedef enum {vt4001KEYED,vt4001SERIAL,vt4001B40} IMP4001VARTYPE;

typedef struct
    {
    BOOL digits;              // Variable can contain Digits
    BOOL lowers;              // Variable can contain Lowercase Letters
    BOOL uppers;              // Variable can contain Uppercase Letters
    BOOL space;               // Variable can contain Space
    BOOL others;              // Variable can contain other characters
    } IMP4001KEYEDVAR;

typedef struct
    {
    WORD  repno;     // Current repeat number
    DWORD step;      // Step size
    BOOL  dirpos;    // Count direction positive or FALSE for negative
    WORD  repeat;    // Repeat limit 0..9999
    DWORD minval;    // Minimum value 0 to IMPMAXSVVAL
    DWORD maxval;    // Maximum value 0 to IMPMAXSVVAL
    BOOL  loop;      // Program either loops or stops
    } IMP4001SERIALVAR;

typedef struct
	{
	WORD dummy; // unused
	} IMP4001B40VAR;

typedef struct
    {
    char name[32+1];     // Variable name
    WORD width;          // Width of variable
    char lpad;           // if(lpad) left pad to width with lpad
    IMP4001VARTYPE type; // Variable type
    union
        {
        IMP4001KEYEDVAR  k;      // type==vt4001KEYED
        IMP4001SERIALVAR s;      // type==vt4001SERIAL
        IMP4001B40VAR    b40;    // type==vt4001B40
        };
    } IMP4001VAR;
*/

BOOL IMPVAR::translateOriginalVersion(HANDLE fh){
	// Name
	if (!readString(fh,name,32+1))
		return FALSE;
	// Width
	if (!readInt(fh,&width,TRUE))
		return FALSE;
	// Lpad
	if (!readByte(fh,(BYTE*) &lpad))
		return FALSE;

	IMP4001VARTYPE ty;
	// Type
	if (!readInt(fh,(int*) &ty,TRUE))  
		return FALSE;
	
	switch(ty){
	case vt4001KEYED:
		type = vtKEYED;
		break;
	case vt4001SERIAL:
		type = vtSERIAL;
		break;
	case vt4001B40:
		type = vtBASED;
		break;	
	default:
		return FALSE;
	}


	char buf[50];
	
	// Preval - init it
	IMPInitVal(&preval,this);
	// Init the type specific info - update this function if new fields are added
	initByType();
	
	// Read the type specific info (union size: 27)		
	switch (type){
	case vtKEYED:
		// Digits
		if (!readInt(fh,&k.digits,TRUE))  // 2
			return FALSE;
		// Lowers
		if (!readInt(fh,&k.lowers,TRUE))  // 2
			return FALSE;
		// Uppers
		if (!readInt(fh,&k.uppers,TRUE))  // 2
			return FALSE;
		// Space
		if (!readInt(fh,&k.space,TRUE))   // 2
			return FALSE;
		// Others
		if (!readInt(fh,&k.others,TRUE))  // 2
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,10))  
			return FALSE;
		break;
	case vtSERIAL:
		// repno
		if (!readWord(fh,&s.repno))   // 2
			return FALSE;
		// step
		if (!readDoubleWord(fh,&s.step))  // 4
			return FALSE;
		// dirpos
		if (!readInt(fh,&s.dirpos,TRUE))  // 2
			return FALSE;
		// repeat
		if (!readWord(fh,&s.repeat))  // 2
			return FALSE;
		// minval
		if (!readDoubleWord(fh,&s.minval))  // 4
			return FALSE;
		// maxval
		if (!readDoubleWord(fh,&s.maxval))  // 4
			return FALSE;
		// loop
		if (!readInt(fh,&s.loop,TRUE))  // 2
			return FALSE;
		// Read past rest of union
	//	if (!readString(fh,buf,7))  
	//		return FALSE;
		break;
	case vtBASED:
		// Read the whole union  (Loop and base set by initByType()) 
		if (!readString(fh,buf,20))
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
	}

	return TRUE;
}


/******************************************************************************
	IMPVARS CLASS
******************************************************************************/
IMPVARS::IMPVARS()
{
	count = 0;
	capac = 0;
	var = NULL;
}

IMPVARS::IMPVARS(int initCapac)
{
	count = 0;
	capac = initCapac;
	if (initCapac>0) 
		var = new IMPVAR[capac];
	else
		var = NULL;
}

IMPVARS::~IMPVARS()
	{
		free();
	}

void IMPVARS::free(){
	if (var){
		delete [] var;
		var=0;
	}
}

BOOL IMPVARS::realloc(int ncount){
	if (ncount <= 0)
	{
		free();
		count=0;
		capac=0;
		return FALSE; 
	}

	// Create the new array
	IMPVAR * newArr = new IMPVAR[ncount];
	
	count=min(count,ncount);
	capac=ncount;
	
	for(int i=0;i<count;i++){
		newArr[i] = var[i];
	}

	// Free the old array and assign new one
	free();

	var = newArr;
	return var!=NULL;
}



IMPVARS::IMPVARS(const IMPVARS & src)
	{
	count = src.count;
	capac = src.capac;
	var = NULL;

	if (capac>0){
		var = new IMPVAR[capac];
	} 

	for (int i=0;i<count;i++){
		var[i] = src.var[i];
	}
	
	}

IMPVARS& IMPVARS::operator = (const IMPVARS & src)
{
	free();

	count = src.count;
	capac = src.capac;

	if (capac>0){
		var = new IMPVAR[capac];
	} 
	
	for (int i=0;i<count;i++){
		var[i] = src.var[i];
	}
	
	return *this;
}

BOOL IMPVARS::read(HANDLE fh, WORD ver, WORD cnt){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver,cnt);
	else 
		return translate(fh,ver,cnt);
}

BOOL IMPVARS::readnative(HANDLE fh, WORD ver, WORD cnt){
	for (int i=0;i<cnt;i++){
		if (!var[i].readnative(fh,ver))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}

BOOL IMPVARS::translate(HANDLE fh, WORD ver, WORD cnt){
	for (int i=0;i<cnt;i++){
		if (!var[i].translate(fh,ver))
			return FALSE;
	}
	count = cnt;
	return TRUE;
}

BOOL IMPVARS::write(HANDLE fh){
	
	DWORD bw;
	// Write count
	WriteFile(fh,(LPSTR)&count,sizeof(WORD),&bw,NULL);
	if (bw != sizeof(WORD))
		return FALSE;

	// Write the data
	for (int i=0;i<count;i++){
		if (!var[i].write(fh))
			return FALSE;
	}
	   
	return TRUE;
}



