/******************************************************************************
	Name : IMP.C

	Desc : Implementation of IMS Inkjet Marking Program and fixture
		   structures and the functions to support them.
******************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#include <imslib32\imslib32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "login.h"
#include "imp.h"
#include "imf.h"
#include "xload.h"
#include "c2of5.h"
#include "c39.h"
#include "c93.h"
#include "c128.h"
#include "cupcean.h"
#include "cdm.h"
#include "winfont.h"
#include "patch.h"
#include "WMTRX.H"
#include "cqr.h"
#include "lbmp.h"
#include "cure.h"
#include "cdev.h"
#include "bsmfixup.h"
#include "resource.h"

/******************************************************************************
	PRIVATE DEFINES
******************************************************************************/
#define LSIGIMP     (sizeof(sigIMP)-1)
#define LSIGIMPV400 (sizeof(sigIMPV400)-1)
#define LSIGIMPV368 (sizeof(sigIMPV368)-1)
#define LSIGIMPV364 (sizeof(sigIMPV364)-1)
#define LSIGIMPV355 (sizeof(sigIMPV355)-1)
#define LSIGIMPV350 (sizeof(sigIMPV350)-1)
#define LSIGIMPV347 (sizeof(sigIMPV347)-1)
#define LSIGIMPV343 (sizeof(sigIMPV343)-1)
#define LSIGIMPV341 (sizeof(sigIMPV341)-1)
#define LSIGIMPV339 (sizeof(sigIMPV339)-1)
#define LSIGIMPV338 (sizeof(sigIMPV338)-1)
#define LSIGIMPV330 (sizeof(sigIMPV330)-1)
#define LSIGIMP4001 (sizeof(sigIMP4001)-1)

#define PIX2 6.283185307f  // 2*PI
typedef struct {double t,xf,yf;} RADE; // Radius Exp (compiled) r=t+xf*mx+yf*my


/******************************************************************************
	STATIC FUNCTION PROTOTYPES
******************************************************************************/
static PIMSTEP IMPCompile(PIMP pimp,IMPVALS *pvals,IMXYMODE xymode,int nesti,BOOL cure);
static int FindCharInSet(char c,LPSTR set);
static LPSTR vnRead(LPSTR s,BOOL showerr);
static int vnFind(IMPVARS *vars);
static int haFind(IMPUDHAS *pudh);
static int vnSysExpand(LPSTR buf);
static IMPHEADER *IMPHEADERNew(void);
static UINT IMPUDHASNew(IMPUDHAS & u);
//static IMPVALS *IMPVALSNew(void);
//static IMPMARKS *IMPMARKSNew(void);
//static IMPFONTS *IMPFONTSNew(void);
//static IMPVARS *IMPVARSNew(void);

//static BOOL LoadNativeIMP(HANDLE fh,PIMP pimp,int ver);
static void MarkDecGreaterFonts(IMPMARK *m,int idf);
static BOOL MarkUsesFont(IMPMARK *m,int idf);
static BOOL ContainsOnlySpaces(LPSTR s);
static void JustifyOrigin(BYTE justx,BYTE justy,P2 *ext,P2 *jorg);
static void Replace0withO(char *s);
static int FindBasesIndex(LPSTR name);
static void AppendRFScale(PIMSTEP imsteps,IMXYMODE xymode,P2 *origin,
	RADE *rade);
static void BuildRADE(RADE *rade,IMXYMODE xymode,IMPUDHAS *pudhas);
static LPM3 M3RotSlant(LPM3 ans,double arot,double aslant);
static void Rotmark(double ans,double deg);
static BOOL ValidateCodeChars(LPSTR data,IMPMARKTYPE ty);

//static void V355VarToV364(IMPVAR *dst,IMPV355VAR *src);

/******************************************************************************
	PUBLIC VARS
******************************************************************************/
// Basic Header Attribute names
char *IMPBHANames[IMPNBHAS]={
	"Desc","Author","Fixture","Part Diameter & Height","Inkjet","Origin","Rotation","Init","Link","IncOnLink",
	"Inspector","InspectPos","InspectPrep",
	"ConveyorStart","LinkAutoStart","SkipOnLinkFail","ChainLinkFail",
	"Reader","ReadPos","ReadPrep","ReadVar","Part Diameter & Height"};

// System var names
char *IMPSysVarNames[IMPNSYSVARS]={
	"@DATE","@TIME","@USERID","@USERNAME","@CYCLESTATUS","@UIDFORMAT5","@UIDFORMAT6","@UIDSEPARATOR","@UIDEND"};

//UID var names
//char *IMPUIDVarNames[IMPUIDVARS]={
//	"#CAGE","#DUNS","#GS1","#DODAAC","#OTHER","#SERNEI","#SERNPIN","#Part Num",
//	"#Lot/Batch","#Comp UII","#UIIIAC","#VIN","#ESN","#GRAI","#GIAI","#CPN","#LOTBA"};             //--------------Linghai-4-20-09--

// bases : list of basename and set from [BASES] in the ini file
#define MAXBASES 10
static struct
	{
	LPSTR name;
	LPSTR set;
	} bases[MAXBASES];
static int nbases=0; // number of bases currently defined
static char sBASES[]="BASES"; // ini section

/******************************************************************************
	STATIC VARS
******************************************************************************/
// File type signitures
static char sigIMP[]	="JETIMP5000V401\x1a"; // Native IMP 
static char sigIMPV400[]="JETIMP5000V400\x1a"; // Native IMP v 4.00
static char sigIMPV368[]="JETIMP4001V368\x1a"; // old IMP 3.68
static char sigIMPV364[]="JETIMP4001V364A\x1a"; // old IMP 3.64A
static char sigIMPV355[]="JETIMP4001V355\x1a";  // old IMP 3.55
static char sigIMPV350[]="JETIMP4001V350\x1a";  // old IMP 3.50
static char sigIMPV347[]="JETIMP4001V347\x1a";  // old IMP 3.47
static char sigIMPV343[]="JETIMP4001V343\x1a";  // old IMP 3.43
static char sigIMPV341[]="JETIMP4001V341\x1a";  // old IMP 3.41
static char sigIMPV339[]="JETIMP4001V339\x1a";  // old IMP 3.39
static char sigIMPV338[]="JETIMP4001V338\x1a";  // old IMP 3.38
static char sigIMPV330[]="JETIMP4001V330\x1a";  // old IMP 3.30
static char sigIMP4001[]="JETIMP4001\x1a";      // pre IMP 3.30

// Variable Name or Header Attribute Name in parser
static char vnbuf[IMPLVNAME+1]; // variable name buffer (HA's too)
static WORD vnlen;              // length of vnbuf

// Some strings
static char sYes[]="Yes";
static char sNo[]="No";


/******************************************************************************
	IMP class
******************************************************************************/
IMP::IMP()
	{
	pheader=NULL;
//	pudhas=NULL;
//	pvars=NULL;
//	pgvars=NULL;
//	pgvals=NULL;
//	pmarks=NULL;
//	pfonts=NULL;
	changed=FALSE;
	aesetup=NULL;
	}

/******************************************************************************
	IMPVALS CLASS
******************************************************************************/
IMPVALS::IMPVALS()
{
	count = 0;
	capac = 0;
	val = NULL;
}

IMPVALS::IMPVALS(int initCapac)
{
	count = 0;
	capac = initCapac;
	if (initCapac>0) 
		val = new IMPVAL[capac];
	else
		val = NULL;
}

IMPVALS::~IMPVALS()
	{
		free();
	}

void IMPVALS::free(){
	if (val){
		delete [] val;
		val=0;
	}
}

IMPVALS::IMPVALS(const IMPVALS & src)
	{
	count = src.count;
	capac = src.capac;
	val = NULL;

	if (capac>0){
		val = new IMPVAL[capac];
	} 

	for (int i=0;i<count;i++){
		val[i] = src.val[i];
	}
	
	}

IMPVALS& IMPVALS::operator = (const IMPVALS & src)
{
	free();

	count = src.count;
	capac = src.capac;

	if (capac>0){
		val = new IMPVAL[capac];
	} 
	
	for (int i=0;i<count;i++){
		val[i] = src.val[i];
	}
	
	return *this;
}

/******************************************************************************
	IMCORRS CLASS
******************************************************************************/
// Allocates memory for an IMCORRS collection for ncount IMCORRs.
// Allocate IMCORRS with this operator only!

IMCORRS::IMCORRS()
	{
	init();
	}
// Copy constructor used by IMCORRS::realloc
// Assumes this has enough capacity for src->count objects
IMCORRS::IMCORRS(IMCORRS *src)
	{
	memcpy(this,src,sizeof(IMCORRS));
	}


// Looks for a IMCORR with a matching id
IMCORR *IMCORRS::find(int mid)
	{
	if(mid<0)
		return NULL;
	for(int i=0;i<IMPMAXMARKS;i++)
		if(imc[i].id==mid)
			return &imc[i];
	return NULL;
	}
IMCORR *IMCORRS::getempty(void)
	{
	for(int i=0;i<IMPMAXMARKS;i++)
		if(imc[i].id==-1)
			return &imc[i];
	return NULL;
	}

void IMCORRS::init(void)
	{
	for(int i=0;i<IMPMAXMARKS;i++)
		imc[i].init();
	}

BOOL IMCORRS::read(HANDLE fh)
	{
	init();
	// Check for IM Correction Extension
	IMCORR tmpcorr;
	WORD imcver;
	if(tmpcorr.readsig(fh,imcver))
		{
		// Construct IMCORRS
		if(_lread32(fh,(LPSTR)&ncorrs,sizeof(ncorrs))!=sizeof(ncorrs)) // Read count
			{
			IDError=IDE_READFILE;
			return FALSE;
			}
		if(ncorrs>IMPMAXMARKS) // Range check
			{
			IDError=IDE_READFILE;
			return FALSE;
			}
		for(int i=0;i<ncorrs;i++)
			{
			if(!imc[i].read(fh,imcver))
				{
				IDError=IDE_READFILE;
				return FALSE;
				}
			}
		}
	return TRUE;
	}

/******************************************************************************
	PUBLIC FUNCTIONS
******************************************************************************/
/******************************************************************************
	ok = IMPLoad(pimp)

	Reads the file signiture and dispatches the correct loader.  The file
	is identified by the ofs in the imp.

	PIMP pimp = IMP structure to fill in (and ofs of file).
	BOOL ok = TRUE on success.
******************************************************************************/
BOOL  IMPLoad(PIMP pimp)
	{
	HANDLE fh;         // file handle
	BOOL ok;        // return from LoadNativeIMP

	// Initialize all IMP handles to 0
	pimp->pheader=NULL;
	pimp->udhas=IMPUDHAS();
	pimp->vars=IMPVARS();
	pimp->gvars=IMPVARS();
	pimp->gvals=IMPVALS();
	pimp->marks=IMPMARKS();
	pimp->fonts=IMPFONTS();
	pimp->aesetup=NULL;

	// ReOpen the file
	fh = CreateFile(pimp->fn,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE) // On open error
		{
		IDError=IDE_OPENIMP;
		return FALSE;
		}

	// Read the signiture
	_lread32(fh,tbuf,MAXSIGNITURE);

	WORD ver;

	// Dispatch loader
	if(!memcmp(tbuf,sigIMP,LSIGIMP))
		{
		_llseek32(fh,LSIGIMP,0);
	    ver=0;
		}
	else if(!memcmp(tbuf,sigIMPV400,LSIGIMPV400))
		{
		_llseek32(fh,LSIGIMP,0);
	    ver=400;
		}
	else if(!memcmp(tbuf,sigIMPV368,LSIGIMPV368))
		{
		_llseek32(fh,LSIGIMPV368,0);
		ver=368;
		}
	else if(!memcmp(tbuf,sigIMPV364,LSIGIMPV364))
		{
		_llseek32(fh,LSIGIMPV364,0);
		ver=364;
		}
	else if(!memcmp(tbuf,sigIMPV355,LSIGIMPV355))
		{
		_llseek32(fh,LSIGIMPV355,0);
		ver=355;
		}
	else if(!memcmp(tbuf,sigIMPV350,LSIGIMPV350))
		{
		_llseek32(fh,LSIGIMPV350,0);
		ver=350;
		}
	else if(!memcmp(tbuf,sigIMPV347,LSIGIMPV347))
		{
		_llseek32(fh,LSIGIMPV347,0);
		ver=347;
		}
	else if(!memcmp(tbuf,sigIMPV343,LSIGIMPV343))
		{
		_llseek32(fh,LSIGIMPV343,0);
		ver=343;
		}
	else if(!memcmp(tbuf,sigIMPV341,LSIGIMPV341))
		{
		_llseek32(fh,LSIGIMPV341,0);
		ver=341;
		}
	else if(!memcmp(tbuf,sigIMPV339,LSIGIMPV339))
		{
		_llseek32(fh,LSIGIMPV339,0);
		ver=339;
		}
	else if(!memcmp(tbuf,sigIMPV338,LSIGIMPV338))
		{
		_llseek32(fh,LSIGIMPV338,0);
		ver=338;
		}
	else if(!memcmp(tbuf,sigIMPV330,LSIGIMPV330))
		{
		_llseek32(fh,LSIGIMPV330,0);
		ver=330;
		}
	else if(!memcmp(tbuf,sigIMP4001,LSIGIMP4001))
		{
		_llseek32(fh,LSIGIMP4001,0);
		ver=300;
		}
	else
		{ // Signiture not recognized
		IDError=IDE_FILETYPE;
		CloseHandle(fh);
		return FALSE;
		}

	WORD count;

	pimp->pheader = new IMPHEADER();
	if(!pimp->pheader->read(fh,ver)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	// ----------- UDHAS -------------- //
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the udhaCount
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	if(count>IMPMAXVARS)              // Check range
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	pimp->udhas=IMPUDHAS(count); // Allocate pudhas and read it
	if(!pimp->udhas.udha && count>0)
	{
		IDError=IDE_NOMEM;
		ok=FALSE;
	}
	if(!pimp->udhas.read(fh,ver,count)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	char xygDir[128]; // mps rotary test udha
	strcpy(xygDir,IMSDir); // mps rotary test udha
	strncat (xygDir, "\\XYGRoty.ini", 12); // mps rotary test udha
	
	char xygDir7[128]; //Rotarytest udha
	strcpy(xygDir7,IMSDir); // Rotarytest udha
	strncat (xygDir7, "\\XYGRottary.ini", 12); //Rotarytest udha   --sam

	char strtmp[128];
	strcpy(strtmp,IMSDir);
	strncat (strtmp, "\\IM5000.ini", 12);

	if (count>0) {
		int mps; // mps rotary test udha
		mps=IMPGetUDHAInt(pimp,"RotaryGalil",0); // mps test udha
		int jet3PrintGoDelay; // mps test udha
		jet3PrintGoDelay=IMPGetUDHAInt(pimp,"RotaryGalilJet3Delay",10); // mps test udha
		int jet3_UP_Down; // mps rotary test udha
		jet3_UP_Down=IMPGetUDHAInt(pimp,"RotaryGalilUpsideDown",0); // overwrite up down property of jet 3, mps test udha
		int Rotary; // mps rotary test udha
		Rotary=IMPGetUDHAInt(pimp,"Rotary",0); //Rotary test udha
		int RotarySpeed; // mps rotary test udha
		RotarySpeed=IMPGetUDHAInt(pimp,"RotarySpeed",0); // overwrite RotarySpeed, mps test udha ---sam
		int RotaryAcclExtra; // mps rotary test udha
		RotaryAcclExtra=IMPGetUDHAInt(pimp,"RotaryAcclExtra",0); // overwrite RotaryAcclExtra, mps test udha ---sam
		int RotaryDcclExtra; // mps rotary test udha
		RotaryDcclExtra=IMPGetUDHAInt(pimp,"RotaryDcclExtra",0); // overwrite RotaryDcclExtra, mps test udha  ---sam
		 
		if (mps)
		{ 
			char buffer [33];
			char buffer2 [33];
			itoa (jet3PrintGoDelay,buffer,10);
			itoa (jet3_UP_Down,buffer2,10);
			WritePrivateProfileString("XYGRoty","RotaryGalil","1",xygDir);
			WritePrivateProfileString("XYGRoty","RotaryGalilJet3Delay",buffer,xygDir);
			WritePrivateProfileString("XYGRoty","RotaryGalilUpsideDown",buffer2,xygDir);
			}
		else 
		{
			WritePrivateProfileString("XYGRoty","RotaryGalil","0",xygDir);
		}

		if (Rotary)
		{ 
			
			char buffer3 [50],buffer4 [50],buffer5 [50];
			
			itoa (RotarySpeed,buffer3,10);
			itoa (RotaryAcclExtra,buffer4,10);
			itoa (RotaryDcclExtra,buffer5,10);

			WritePrivateProfileString("XYGRottary","Rotary","1",xygDir); //sam 06/16
			WritePrivateProfileString("XYGRottary","RotarySpeed",buffer3,xygDir);  //sam 06/16
			WritePrivateProfileString("XYGRottary","RotaryAcclExtra",buffer4,xygDir);   //sam 06/16
			WritePrivateProfileString("XYGRottary","RotaryDcclExtra",buffer5,xygDir);    //sam 06/16

		}
		else 
		{
			WritePrivateProfileString("XYGRottary","Rotary","0",xygDir); //sam 06/16
		
		}
	}

	// ------------ VARS -------------- //
	if (ver != 300){
		if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the varsCount
		{
			IDError=IDE_READFILE;
			ok=FALSE;
		}
		if(count>IMPMAXVARS)              // Check range
		{
			IDError=IDE_READFILE;
			ok=FALSE;
		}
		pimp->vars=IMPVARS(count); // Allocate vars and read it
		if(!pimp->vars.var && count>0)
		{
			IDError=IDE_NOMEM;
			ok=FALSE;
		}
		if(!pimp->vars.read(fh,ver,count)){
			IDError=IDE_READFILE;
			ok=FALSE;
		}
	} else {
		// No instance vars original version
	}
	// ------------ GVARS -------------- //
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the gvarsCount
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	if(count+pimp->vars.count>IMPMAXVARS) // Check range
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	pimp->gvars=IMPVARS(count); // Allocate gvars and read it
	if(!pimp->gvars.var && count>0)
	{
		IDError=IDE_NOMEM;
		ok=FALSE;
	}
	if(!pimp->gvars.read(fh,ver,count)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	// ------------ MARKS -------------- //
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the markCount
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	if(count>IMPMAXMARKS || count<1) // Check range
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	pimp->marks=IMPMARKS(count); // Allocate marks and read it
	if(!pimp->marks.mark && count>0)
	{
		IDError=IDE_NOMEM;
		ok=FALSE;
	}
	if(!pimp->marks.read(fh,ver,count)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	// bsms
	if(ver==0||ver>355) // Load and link CDEV child marks
		{
		BSMFIXUPS fixups(&pimp->marks); // builds identity index map
		if(!fixups.readbsms(fh,ver))  // alloc and read bsms,update fixups
			{
			IDError=IDE_READFILE;
			ok=FALSE;
			}
		fixups.topointer();       // Fixup the parents BSM pointers
		}


	// ------------ FONTS -------------- //
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the markCount
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}
	if(count>IMPMAXFONTS) // Check range
	{
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	pimp->fonts=IMPFONTS(count); // Allocate marks and read it
	if(!pimp->fonts.name && count>0)
	{
		IDError=IDE_NOMEM;
		ok=FALSE;
	}
	if(!pimp->fonts.read(fh,ver,count)){
		IDError=IDE_READFILE;
		ok=FALSE;
	}

	// Check for Alignment Extension
	AESETUP tempae;
	WORD aever;
	if(tempae.readsig(fh,aever))
		{
		// Allocate aesetup and load it
		pimp->aesetup=new AESETUP();
		if(!pimp->aesetup)
			{
			IDError=IDE_NOMEM;
			return FALSE;
			}
		if(!pimp->aesetup->load(fh,aever))
			return FALSE;
		}

	// read optional imcorrs
	
	if(!pimp->imcorrs.read(fh))
		return FALSE;
    

	CloseHandle(fh);
	pimp->changed=FALSE;

	// On error free the successfully allocated parts
	if(!ok)
		IMPFree(pimp);

	return ok;

  }

/******************************************************************************
	error = IMPSave(pimp)

	Saves an IMP memory object to its disk file in sigIMP format.

	BOOL error = True on error.
	PIMP pimp = IMP to save
******************************************************************************/
BOOL  IMPSave(PIMP pimp)
	{

	DWORD wlen;
	BOOL ok=TRUE;

	//ReCreate the file
	//HANDLE fh=OpenFile(NULL,&pimp->ofs,OF_REOPEN|OF_CREATE|OF_WRITE);
	HANDLE fh=CreateFile(pimp->fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE)
		{
		ShowError(IDE_CREATEFILE);
		ok=FALSE;
		goto EXIT;
		}

	// Write signiture
	wlen=LSIGIMP;
	if(_lwrite32(fh,sigIMP,wlen)!=wlen)
		{
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
		}

	// Write Header
	if(!pimp->pheader->write(fh)){
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
	}

    // Write udhas 

	if(!pimp->udhas.write(fh)){
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
	}

	// Vars
	wlen=sizeof(IMPVARS)+pimp->vars.count*sizeof(IMPVAR);
	
	if(!pimp->vars.write(fh)){
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
	}

	// Gvars
	if(!pimp->gvars.write(fh)){
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
	}
	// Marks
	{ // fixups lives in this block
	BSMFIXUPS fixups(&pimp->marks); // Build BSM fixups table for the marks
	fixups.toindex();    // Convert the marks BSM pointers to fixup indexes
	
	// Write the parent marks
	if(!pimp->marks.write(fh)){
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
	}

	fixups.topointer();  // Restore the marks BSM pointers
	if(!fixups.writebsms(fh)) // Write any BSMs
		{
		ok=FALSE;
		goto EXIT;
		}
	}

	// fonts
	//wlen=sizeof(IMPFONTS)+pimp->fonts.count*sizeof(IJFONTNAME);
	//if(_lwrite32(fh,(LPCSTR)pimp->pfonts,wlen)!=wlen)
	//	{
	//	ShowError(IDE_WRITEFILE);
	//	ok=FALSE;
	//	goto EXIT;
	//	}
	if(!pimp->fonts.write(fh)){
		ShowError(IDE_WRITEFILE);
		ok=FALSE;
		goto EXIT;
	}

	if(pimp->aesetup)
		if(!pimp->aesetup->save(fh))
			ok=FALSE;

	pimp->imcorrs.ncorrs=0;
	int i;
	for(i=0;i<IMPMAXMARKS;i++)
		{
		if(pimp->imcorrs.imc[i].id>-1)
			pimp->imcorrs.ncorrs++;
		}
	if(pimp->imcorrs.ncorrs)
		{
		if(!IMCORR::writesig(fh))
			{
			ok=FALSE;
			goto EXIT;
			}
		wlen=sizeof(pimp->imcorrs.ncorrs);
		if(_lwrite32(fh,(LPCSTR)&pimp->imcorrs.ncorrs,wlen)!=wlen)
			{
			ShowError(IDE_WRITEFILE);
			ok=FALSE;
			goto EXIT;
			}
		wlen=sizeof(IMCORR);
		for(i=0;i<IMPMAXMARKS;i++)
			{
			if(pimp->imcorrs.imc[i].id>-1)
				{				
				if(!pimp->imcorrs.imc[i].save(fh))
					{
					ok=FALSE;
					goto EXIT;
					}
				}
			}
		}

	pimp->changed=FALSE;

EXIT: // close file, return result
	if(fh!=INVALID_HANDLE_VALUE)
		CloseHandle(fh);
	return !ok;
	}

/******************************************************************************
	error = IMPNew(pimp)

	Creates a new IMP memory object.  The ofs is initially blank.  The changed
	flag is initially false.

	BOOL error = true on error.
	IMP1 *IMP = IMP to initialize.
******************************************************************************/
BOOL  IMPNew(PIMP pimp)
	{
	// Initialize to empty
	pimp->pheader=NULL;
	pimp->udhas=IMPUDHAS();
	pimp->vars=IMPVARS();
	pimp->gvars=IMPVARS();
	pimp->gvals=IMPVALS();
	pimp->marks=IMPMARKS();
	pimp->fonts=IMPFONTS();
	pimp->changed=FALSE;
	pimp->aesetup=NULL;
	pimp->fn[0]=0;
//	memset(&pimp->ofs,0,sizeof(OFSTRUCT));

	// header
	pimp->pheader=IMPHEADERNew();
	if(!pimp->pheader)
		return TRUE;

	// udhas
	UINT err = IMPUDHASNew(pimp->udhas);
	if(err!=0)
		{
		IMPFree(pimp);
		return TRUE;
		}

	// vars
//	pimp->pvars=IMPVARSNew();
//	if(!pimp->pvars)
//		{
//		IMPFree(pimp);
//		return TRUE;
//		}

	// gvars
//	pimp->pgvars=IMPVARSNew();
//	if(!pimp->pgvars)
//		{
//		IMPFree(pimp);
//		return TRUE;
//		}
	
	// marks
//	pimp->pmarks=IMPMARKSNew();
//	if(!pimp->pmarks)
//		{
//		IMPFree(pimp);
//		return TRUE;
//		}

	// fonts
//	pimp->pfonts=IMPFONTSNew();
//	if(!pimp->pfonts)
//		{
//		IMPFree(pimp);
//		return TRUE;
//		}

	return FALSE;
	}

/******************************************************************************
	IMPFree(pimp)

	Frees memory alllocated to each handle in an IMP and sets it to 0.
	Only frees when the handle itself is not 0.

	PIMP pimp = IMP to free allocated memory for.
******************************************************************************/
void  IMPFree(PIMP pimp)
	{
	if(pimp->marks.mark) // Free any sub marks before freeing top level marks
		{
		for(int mi=0;mi<pimp->marks.count;mi++)
			{
			IMPMARK &m=pimp->marks.mark[mi];
			if(m.type==mtCDEV)
				for(int bi=0;bi<m.cdev.nbsm;bi++)
					delete m.cdev.bsm[bi];
			}
		}

	// Free the top level objects
	DELZ(pimp->pheader);
//	DELZ(pimp->pudhas);
//	DELZ(pimp->pvars);
//	DELZ(pimp->pgvars);
//	DELZ(pimp->pgvals);
//	DELZ(pimp->pmarks);
//	DELZ(pimp->pfonts);
	DELZ(pimp->aesetup);
	}

/******************************************************************************
	valid = IMPValid(pimp)

	Checks an IMP for validity.  An IMP is valid if non of its required
	pointers are NULL and if there is at least one mark.

	BOOL valid = True if imp is valid.
	PIMP pimp = IMP to check.
******************************************************************************/
BOOL  IMPValid(PIMP pimp)
	{
	// Check for null required pointers
	if((!pimp->pheader)
		||(!pimp->udhas.udha)
		||(!pimp->vars.var)
		||(!pimp->gvars.var)
		||(!pimp->fonts.name)
		||(!pimp->marks.mark))
		return FALSE;

	if(pimp->marks.count<1) // It must have at least one mark.
		return FALSE;

	return TRUE; // It looks valid.
	}

/******************************************************************************
	IMPVarToTbuf(pvar)

	Copies the contents of a var to tbuf in a format sutable for
	display in the editor listbox.

	IMPVAR *var = var to copy.
******************************************************************************/
void  IMPVarToTbuf(IMPVAR *pvar)
	{
	tlen=wsprintf(tbuf,"%s",(LPSTR)pvar->name);
	}

/******************************************************************************
	IMPVarValToTbuf(pvar,pval)

	Copies the contents of a var and val to tbuf in a format sutable for
	display in the run listbox.

	IMPVAR *var = var to copy.
	IMPVAL *val = value for var.
******************************************************************************/
void  IMPVarValToTbuf(IMPVAR *pvar,IMPVAL *pval)
	{
	char format[12];
	char precision[5];

	if(pvar->type==vtKEYED) // Keyed var
		{
		tlen=wsprintf(tbuf,"%s:%s",(LPSTR)pvar->name,(LPSTR)pval->k);
		return;
		}

	if(pvar->type==vtDATE) // Date var
		{
		tlen=wsprintf(tbuf,"%s:",(LPSTR)pvar->name);
		if(pval->d.override[0]) // If overridden
			lstrcpy(tbuf+tlen,pval->d.override);
		else
			GetDate(tbuf+tlen,pvar->d.format,pvar->d.offset,pvar->d.base);
		return;
		}
	if(pvar->type==vtTIME) // Time var
		{
		tlen=wsprintf(tbuf,"%s:",(LPSTR)pvar->name);
		if(pval->t.override[0]) // If overridden
			lstrcpy(tbuf+tlen,pval->t.override);
		else
			GetTime(tbuf+tlen,
				pvar->t.useNames,
				pvar->t.is24,
				pvar->t.isLZ,
				pvar->t.seperator,
				pvar->t.amSuffix,
				pvar->t.pmSuffix,
				pvar->t.showSeconds,
				pvar->t.offset,
				pvar->t.base);
		return;
		}

	if(pvar->type==vtBASED) // BASED var
		{
		tlen=wsprintf(tbuf,"%s:%s",(LPSTR)pvar->name,(LPSTR)pval->b);
		return;
		}

	// Serial var

	// build precision string
	if(pvar->lpad)
		wsprintf(precision,"0.%u",pvar->width);
	else
		precision[0]=0;

	// build format string
	lstrcpy(format,"%s:%");
	lstrcat(format,precision);
	lstrcat(format,"lu");

	// build output string
	tlen=wsprintf(tbuf,format,(LPSTR)pvar->name,pval->s);
	}

/******************************************************************************
	IMPVarNValToTbuf(pvar,n,pval,occupied)

	Copies the contents of a var and val to tbuf in a format sutable for
	display in the run listbox for multi part fixtures.

	IMPVAR *var = var to copy.
	int n = var instance number to display after .
	IMPVAL *val = value for var.
	bool occupied = flag set when the nest is occupied.
******************************************************************************/
void  IMPVarNValToTbuf(IMPVAR *pvar,int n,IMPVAL *pval,bool occupied)
	{
	char format[15];
	char precision[5];

	if(!occupied)
		{
		tlen=wsprintf(tbuf,"%s.%d:",(LPSTR)pvar->name,n);
		return;
		}
	if(pvar->type==vtKEYED) // Keyed var
		{
		tlen=wsprintf(tbuf,"%s.%d:%s",(LPSTR)pvar->name,n,(LPSTR)pval->k);
		return;
		}

	if(pvar->type==vtDATE) // Date var
		{
		tlen=wsprintf(tbuf,"%s.%d:",(LPSTR)pvar->name,n);
		if(pval->d.override[0]) // If overridden
			lstrcpy(tbuf+tlen,pval->d.override);
		else
			GetDate(tbuf+tlen,pvar->d.format,pvar->d.offset,pvar->d.base);
		return;
		}

	if(pvar->type==vtTIME) // Time var
		{
		tlen=wsprintf(tbuf,"%s.%d:",(LPSTR)pvar->name,n);
		if(pval->t.override[0]) // If overridden
			lstrcpy(tbuf+tlen,pval->t.override);
		else
			GetTime(tbuf+tlen,
				pvar->t.useNames,
				pvar->t.is24,
				pvar->t.isLZ,
				pvar->t.seperator,
				pvar->t.amSuffix,
				pvar->t.pmSuffix,
				pvar->t.showSeconds,
				pvar->t.offset,
				pvar->t.base);

		return;
		}

	if(pvar->type==vtBASED) // based var
		{
		tlen=wsprintf(tbuf,"%s.%d:%s",(LPSTR)pvar->name,n,(LPSTR)pval->b);
		return;
		}

	// Serial var

	// build precision string
	if(pvar->lpad)
		wsprintf(precision,"0.%u",pvar->width);
	else
		precision[0]=0;

	// build format string
	lstrcpy(format,"%s.%d:%");
	lstrcat(format,precision);
	lstrcat(format,"lu");

	// build output string
	tlen=wsprintf(tbuf,format,(LPSTR)pvar->name,n,pval->s);
	}

/******************************************************************************
	IMPVarValToIni(pvar,pval,sect,ini)

	Copies the contents of a var and val to to an ini init var
******************************************************************************/
void  IMPVarValToIni(IMPVAR *pvar,IMPVAL *pval,LPSTR sect,LPSTR ini)
	{
	char format[12];
	char precision[5];
	char key[IMPLVNAME+8];

	wsprintf(key,"%s",(LPSTR)pvar->name);

	if(pvar->type==vtKEYED) // Keyed var
		{
		tlen=wsprintf(tbuf,"%s",(LPSTR)pval->k);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	if(pvar->type==vtDATE) // Date var
		{
		if(pval->d.override[0]) // If overridden
			lstrcpy(tbuf,pval->d.override);
		else
			GetDate(tbuf,pvar->d.format,pvar->d.offset,pvar->d.base);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}


	if(pvar->type==vtTIME) // Time var
		{
		if(pval->t.override[0]) // If overridden
			lstrcpy(tbuf,pval->t.override);
		else
			GetTime(tbuf,
				pvar->t.useNames,
				pvar->t.is24,
				pvar->t.isLZ,
				pvar->t.seperator,
				pvar->t.amSuffix,
				pvar->t.pmSuffix,
				pvar->t.showSeconds,
				pvar->t.offset,
				pvar->t.base);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	if(pvar->type==vtBASED) // BASED var
		{
		tlen=wsprintf(tbuf,"%s",(LPSTR)pval->b);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	// Serial var

	// build precision string
	if(pvar->lpad)
		wsprintf(precision,"0.%u",pvar->width);
	else
		precision[0]=0;

	// build format string
	lstrcpy(format,"%");
	lstrcat(format,precision);
	lstrcat(format,"lu");

	// build output string
	tlen=wsprintf(tbuf,format,pval->s);
	WritePrivateProfileString(sect,key,tbuf,ini);
	}

/******************************************************************************
	IMPVarNValToIni(pvar,n,pval,sect,ini)

	Copies the contents of a var and val to tbuf in a format sutable for
	display in the run listbox for multi part fixtures.

	IMPVAR *var = var to copy.
	int n = var instance number to display after .
	IMPVAL *val = value for var.
******************************************************************************/
void  IMPVarNValToIni(IMPVAR *pvar,int n,IMPVAL *pval,LPSTR sect,LPSTR ini)
	{
	char format[15];
	char precision[5];
	char key[IMPLVNAME+8];

	wsprintf(key,"%s.%d",(LPSTR)pvar->name,n);

	if(pvar->type==vtKEYED) // Keyed var
		{
		tlen=wsprintf(tbuf,"%s",(LPSTR)pval->k);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	if(pvar->type==vtDATE) // Date var
		{
		if(pval->d.override[0]) // If overridden
			lstrcpy(tbuf,pval->d.override);
		else
			GetDate(tbuf,pvar->d.format,pvar->d.offset,pvar->d.base);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	if(pvar->type==vtTIME) // Time var
		{
		if(pval->t.override[0]) // If overridden
			lstrcpy(tbuf,pval->t.override);
		else
			GetTime(tbuf,
				pvar->t.useNames,
				pvar->t.is24,
				pvar->t.isLZ,
				pvar->t.seperator,
				pvar->t.amSuffix,
				pvar->t.pmSuffix,
				pvar->t.showSeconds,
				pvar->t.offset,
				pvar->t.base);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	if(pvar->type==vtBASED) // based var
		{
		tlen=wsprintf(tbuf,"%s",(LPSTR)pval->b);
		WritePrivateProfileString(sect,key,tbuf,ini);
		return;
		}

	// Serial var

	// build precision string
	if(pvar->lpad)
		wsprintf(precision,"0.%u",pvar->width);
	else
		precision[0]=0;

	// build format string
	lstrcpy(format,"%");
	lstrcat(format,precision);
	lstrcat(format,"lu");

	// build output string
	tlen=wsprintf(tbuf,format,pval->s);
	WritePrivateProfileString(sect,key,tbuf,ini);
	}


/******************************************************************************
	IMPVarValFromIni(pvar,pval,sect,ini)

	Loads the contents of a val from an ini init var
******************************************************************************/
void  IMPVarValFromIni(IMPVAR *pvar,IMPVAL *pval,LPSTR sect,LPSTR ini)
	{
	char key[IMPLVNAME+8];

	wsprintf(key,"%s",(LPSTR)pvar->name);

	switch(pvar->type)
		{
		case vtKEYED:
			tlen=GetPrivateProfileString(sect,key,"",tbuf,TSIZE,ini);
			wsprintf(pval->k,"%s",(LPSTR)tbuf);
			break;
		case vtSERIAL:
			GetPrivateProfileString(sect,key,"",tbuf,TSIZE,ini);
			ParseLong(tbuf,(LPLONG)&pval->s);
			break;
		// No vtDATE
		// No vtTIME
		case vtBASED:
			GetPrivateProfileString(sect,key,"",tbuf,TSIZE,ini);
			wsprintf(pval->b,"%s",(LPSTR)tbuf);
			break;
		}
	}

/******************************************************************************
	IMPVarNValFromIni(pvar,n,pval,sect,ini)

	IMPVAR *var = var to find
	int n = var instance number to display after .
	IMPVAL *val = value for var to load.
******************************************************************************/
void  IMPVarNValFromIni(IMPVAR *pvar,int n,IMPVAL *pval,LPSTR sect,LPSTR ini)
	{
	char key[IMPLVNAME+8];

	wsprintf(key,"%s.%d",(LPSTR)pvar->name,n);

	switch(pvar->type)
		{
		case vtKEYED:
			tlen=GetPrivateProfileString(sect,key,"",tbuf,TSIZE,ini);
			wsprintf(pval->k,"%s",(LPSTR)tbuf);
			break;
		case vtSERIAL:
			GetPrivateProfileString(sect,key,"",tbuf,TSIZE,ini);
			ParseLong(tbuf,(LPLONG)&pval->s);
			break;
		// No vtDATE
		// No vtTIME
		case vtBASED:
			tlen=GetPrivateProfileString(sect,key,"",tbuf,TSIZE,ini);
			wsprintf(pval->b,"%s",(LPSTR)tbuf);
			break;
		}
	}

/******************************************************************************
	IMPMarkToTbuf(pm)

	Copies the contents of a mark to tbuf in a format sutable for display
	in the editor listbox.

	IMPMARK *pm = mark to add.
******************************************************************************/
void  IMPMarkToTbuf(IMPMARK *pm,IMPFONTS *pf)
	{
	char just[3];
	char sOrigin[24];
	char sFloat[12];
	char sxres[12],syres[12];
	double v[2];

	char *p;

    // Fonts without the description
	IMPFONTS fnd = IMPFONTS(pf->count);
	for(int i=0;i<pf->count;i++){
		lstrcpy(fnd.name[i],pf->name[i]);
		p = strchr(fnd.name[i],':');
		if(p)
			*p=0;
		
	}

	switch(pm->justx)
		{
		case mjxLEFT:   just[0]='L'; break;
		case mjxCENTER: just[0]='C'; break;
		case mjxRIGHT:  just[0]='R'; break;
		default: just[0]=' ';
		}
	switch(pm->justx)
		{
		case mjyTOP:    just[1]='T'; break;
		case mjyMIDDLE: just[1]='M'; break;
		case mjyBOTTOM: just[1]='B'; break;
		default: just[1]=' ';
		}
	just[2]=0;

	switch(pm->type)
		{
		case mtTEXT:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->t.cw));
			VectToString(sFloat,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Text at(%s) just(%s) cw(%s) "
				"font(%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sFloat,
				(LPSTR)&fnd.name[pm->t.ijfmt.font],
				pm->t.ijfmt.ms,
				(LPSTR)pm->t.text);
			break;
		case mtDEVBC:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->t.cw));
			VectToString(sFloat,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"DevBarcode at(%s) just(%s) cw(%s) "
				"font(%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sFloat,
				(LPSTR)&fnd.name[pm->devbc.ijfmt.font],
				pm->devbc.ijfmt.ms,
				(LPSTR)pm->devbc.text);
			break;
		case mtBITMAP:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->b.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->b.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"BITMAP at(%s) just(%s) res(%s,%s) "
				"neg(%d) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->b.neg ? 1 : 0,
				pm->b.ms,
				(LPSTR)pm->b.fn);
			break;
		case mtC128:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c128.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c128.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Code128 at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->c128.ms,
				(LPSTR)pm->c128.text);
			break;
		case mtCUPCEAN:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cupcean.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cupcean.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"UPC/EAN at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->cupcean.ms,
				(LPSTR)pm->cupcean.text);
			break;
		case mtC93:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c93.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c93.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Code93 at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->c93.ms,
				(LPSTR)pm->c93.text);
			break;
		case mtMULTILINE:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->ml.cw));
			VectToString(sFloat,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"MULTILINE at(%s) just(%s) ccw(%s) "
				"font(%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sFloat,
				(LPSTR)&pf->name[pm->ml.ijfmt.font],
				pm->ml.ijfmt.ms,
				(LPSTR)pm->ml.text);
			break;
		//case uid12:                             //-----------------Linghai-11-21-8--
		//case uid6:                              //-----------------Linghai-11-21-8--
		//case uid5:                              //-----------------Linghai-11-21-08--
		case mtUID:                                   //--------------Linghai-4-9-09---UID
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->uid.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->uid.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"UID  at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->uid.ms,
				(LPSTR)pm->uid.text);
			break;
			
		case mtCDM:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cdm.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cdm.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"DataMatrix at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->cdm.ms,
				(LPSTR)pm->cdm.text);
			break;
		case mtWINFONT:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->winfont.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->winfont.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"WinFont at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->winfont.ms,
				(LPSTR)pm->winfont.text);
			break;
		case mtWMTRX:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->wmtrx.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->wmtrx.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Matrix at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->wmtrx.ms,
				(LPSTR)pm->wmtrx.text);
			break;
		case mtCQR:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cqr.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cqr.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"QR Code at(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->cqr.ms,
				(LPSTR)pm->cqr.text);
			break;
		case mtPATCH:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->patch.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->patch.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Patch at(%s) just(%s) res(%s,%s) ms(%d) size(%d,%d)",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->patch.ms,
				pm->patch.size.x,
				pm->patch.size.y);
			break;
		case mtC2OF5:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c2of5.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c2of5.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Code 2of5 at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->c2of5.ms,
				(LPSTR)pm->c2of5.text);
			break;
		case mtC39:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c39.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->c39.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"Code39 at(%s) just(%s) res(%s,%s) ms(%d) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sxres,
				(LPSTR)syres,
				pm->c39.ms,
				(LPSTR)pm->c39.text);
			break;
		case mtCURE:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			tlen=wsprintf(tbuf,"CureStep at(%s)",(LPSTR)sOrigin);
			break;
		case mtCDEV:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cdev.xres));
			VectToString(sxres,(LPDOUBLE)&v,1,"");
			YMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->cdev.yres));
			VectToString(syres,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,"CDev at(%s) just(%s) bfi(%d) res(%s,%s) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				pm->cdev.bfi,
				(LPSTR)sxres,
				(LPSTR)syres,
				(LPSTR)pm->cdev.text);
			break;
		case mtDEVSYM:
			XYMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->origin));
			VectToString(sOrigin,(LPDOUBLE)&v,2,",");
			XMScaleVect((LPDOUBLE)&v,(LPDOUBLE)&(pm->devsym.width));
			VectToString(sFloat,(LPDOUBLE)&v,1,"");
			tlen=wsprintf(tbuf,
				"DevSymbol at(%s) just(%s) width(%s) "
				"font(%s) \"%s\"",
				(LPSTR)sOrigin,
				(LPSTR)just,
				(LPSTR)sFloat,
				(LPSTR)&fnd.name[pm->devsym.ijfmt.font],
				(LPSTR)pm->devsym.text);
			break;
		}
	}

/******************************************************************************
	IMPHeaderAttributeToTbuf(ph,pudh,i)

	Loads tbuf with the listbox entry for a given index.
	Works with both BHA and UDHA.

	IMPHEADER *ph = Header
	IMPUDHAS *pudh = U D Header
	WORD i = Header entry index
******************************************************************************/
void  IMPHeaderAttributeToTbuf(IMPHEADER *ph,IMPUDHAS *pudh,DWORD i)
	{
	int l;

	if(i>=(DWORD)(pudh->count+IMPNBHAS))
		{
		tbuf[0]=0; // Error invalid index
		return;
		}
	l=wsprintf(tbuf,"%s: ",(i<IMPNBHAS) ? 
		(LPSTR)IMPBHANames[i] : 
		(LPSTR)(pudh->udha[i-IMPNBHAS].name));
	HAToBuf(ph,pudh,i,&tbuf[l]);
	}

BOOL  IMPHAIsHidden(DWORD i)
	{
	switch(i)
		{
		case 3: // Part SIze
			return !HideOptRotaryDEG;
		case 6: // Rotation
			return HideOptRotation;
		case 7: // Init
			return HideOptInit;
		case 8: // Link
		case 9: // IncOnLink
			return HideOptLink;
		case 10: // Inspector
		case 11: // InspectPos
		case 12: // InspectPrep
			return HideOptInspect;
		case 13: // ConveyorStart
			return HideOptConveyor;
		case 14: // LinkAutoStart
		case 15: // SkipOnLinkFail
		case 16: // ChainLinkFail
			return HideOptLink;
		case 17: // Reader
		case 18: // ReadPos
		case 19: // ReadPrep
		case 20: // ReadVar
			return HideOptRead;
		case 21:
			return HideOptRotaryDEG;
		case 22:
			return HideOptRotaryDEG;
		}
	return FALSE;
	}

/*****************************************************************************
	ok = IMPInitVals(pvals,pvars)

	Initializes the values for the variables.

	BOOL ok = TRUE if no errors
	IMPVALS *pvals = vals to initialize
	IMPVARS *pvars = vars that coorispond with the vals
******************************************************************************/
BOOL  IMPInitVals(IMPVALS *pvals,IMPVARS *pvars)
	{
	WORD vcount,i;

	vcount=pvars->count;
	for(i=0;i<vcount;i++)
		if(!IMPInitVal(&(pvals->val[i]),&(pvars->var[i])))
			return FALSE;
	return TRUE;
	}

/*****************************************************************************
	ok = IMPInitVal(pval,pvar)

	Initializes the value for the variable.

	BOOL ok = TRUE if no errors
	IMPVAL *pval = val to initialize
	IMPVAR *pvar = var that coorispond with the val
******************************************************************************/
BOOL  IMPInitVal(IMPVAL *pval,IMPVAR *pvar)
	{
	WORD j;
	int bi; // based index

	switch(pvar->type)
		{
		case vtKEYED:
			pval->k[0]=0;
			break;
		case vtDATE:
			pval->d.override[0]=0;
			break;
		case vtTIME:
			pval->t.override[0]=0;
			break;
		case vtBASED:
			bi=FindBasesIndex(pvar->b.base);
			if(bi==-1)
				{
				ShowError(IDE_BADBASE);
				return FALSE;
				}
			j=0;
			if(bi>=0)
				while(j<pvar->width)
					{
					pval->b[j]=bases[bi].set[0];
					j++;
					}
			pval->b[j]=0;
			break;
		case vtSERIAL:
			pval->s=pvar->s.dirpos ? pvar->s.minval : pvar->s.maxval;
			pval->c=0;
			break;
		}
	return TRUE;
	}

/*****************************************************************************
	err = IMPNextVals(vals,vars,showerror)

	Updates serial number variable values to there next values.

	IMPVALS *vals = Values to increment
	IMPVARS *vars = Vars that values map to.
******************************************************************************/
BOOL  IMPNextVals(IMPVALS *vals,IMPVARS *vars,BOOL showerror)
	{
	int i;
	IMPVAL *val;
	IMPVAR *var;
	BOOL svdone,bvdone; // counter done error flags

	svdone=FALSE;
	bvdone=FALSE;
	for(i=0;i<vars->count;i++)
		{
		var=&(vars->var[i]);
		val=&(vals->val[i]);
		if(var->type==vtSERIAL)
			{
			if(!IMPNextSerialVal(val,var))
				svdone=TRUE;
			}
		else if(var->type==vtBASED)
			{
			if(!IMPNextBasedVal(val,var))
				bvdone=TRUE;
			}
		}
	if(showerror && bvdone)
		ShowError(IDE_BVALDONE);
	if(showerror && svdone)
		ShowError(IDE_SVALDONE);
	return svdone||bvdone;
	}

/******************************************************************************
	ok = IMPNextSerialVal(pval,pvar)
	
	Increments a serial value.
******************************************************************************/
BOOL  IMPNextSerialVal(IMPVAL *pval,IMPVAR *pvar)
	{
	if(pval->c < pvar->s.repeat) // If more repeats
		pval->c++;
	else
		{
		pval->c=0;
		if(pvar->s.dirpos)
			{
			pval->s+=pvar->s.step;
			if(pval->s > pvar->s.maxval)
				{
				if(pvar->s.loop)
					pval->s=pvar->s.minval;
				else
					{
					pval->s=pvar->s.maxval;
					return FALSE;
					}
				}
			}
		else
			{
			pval->s-=pvar->s.step;
			if(pval->s < pvar->s.minval)
				{
				if(pvar->s.loop)
					pval->s=pvar->s.maxval;
				else
					{
					pval->s=pvar->s.minval;
					return FALSE;
					}
				}
			}
		}
	return TRUE;
	}

/******************************************************************************
	ok = IMPNextBasedVal(pval,pvar)

	Increments a based value.
******************************************************************************/
BOOL  IMPNextBasedVal(IMPVAL *pval,IMPVAR *pvar)
	{
	int j,lset;
	int bi; // bases index;
	int i;

	bi=FindBasesIndex(pvar->b.base);
	if(bi==-1)
		return FALSE;

	lset=lstrlen(bases[bi].set);
	j=pvar->width;
	while(j--)
		{
		i=FindCharInSet(pval->b[j],bases[bi].set);
		i++;
		i%=lset;
		pval->b[j]=bases[bi].set[i];
		if(i>0)
			break; // no carry
		}

	if(j==-1 && i==0 && !pvar->b.loop) // Overflow?
		{
		for(j=0;j<pvar->width;j++) // restore to max value
			pval->b[j]=bases[bi].set[lset-1];
		return FALSE;
		}

	return TRUE;
	}

/******************************************************************************
	IMPLoadBases()

	Loads the bases table from the [BASES] ini section.
******************************************************************************/
void  IMPLoadBases(void)
	{
	char keys[256];
	BOOL tendef;
	char *p;
	if(nbases)
		return; // allready loaded!

	tendef=FALSE; // base 10 not defined yet!

	// get keys
	GetPrivateProfileString(sBASES,NULL,"",keys,sizeof(keys),IMSIni);

	// enum over keys adding to bases
	p=keys;
	while(*p && nbases<MAXBASES)
		{
		tlen=GetPrivateProfileString(sBASES,p,"",tbuf,TSIZE,IMSIni);
		if(tlen)
			{
			bases[nbases].name=new char[lstrlen(p)+1];
			bases[nbases].set=new char[tlen+1];
			lstrcpy(bases[nbases].name,p);
			lstrcpy(bases[nbases].set,tbuf);
			if(!lstrcmp(p,"10"))
				tendef=TRUE;
			nbases++;
			}
		while(*p++); // advance p to next list entry or terminator
		}
	if(nbases<MAXBASES && !tendef)
		{
		bases[nbases].name=new char[3];
		bases[nbases].set=new char[11];
		lstrcpy(bases[nbases].name,"10");
		lstrcpy(bases[nbases].set,"0123456789");
		nbases++;
		}
	}

/******************************************************************************
	IMPFreeBases()

	Frees the bases list
******************************************************************************/
void  IMPFreeBases(void)
	{
	int i;
	for(i=0;i<nbases;i++)
		{
		delete [] bases[i].name;
		delete [] bases[i].set;
		}
	nbases=0;
	}

/******************************************************************************
	ok = IMPValidateBasedVal(pval,pvar)

	Checks the validity of a based value.

	BOOL ok = True if its valid.
	IMPVAL *pval = val to check
	IMPVAR *pvar = var to check
******************************************************************************/
BOOL  IMPValidateBasedVal(IMPVAL *pval,IMPVAR *pvar)
	{
	int i,bi;

	bi=FindBasesIndex(pvar->b.base);
	if(bi==-1)
		return FALSE;

	i=pvar->width;
	while(i--)
		if(FindCharInSet(pval->b[i],bases[bi].set)==-1)
			break;
	return (i==-1);
	}

/*****************************************************************************
	IMPAddBasesToCB(hctrl,minlen,maxlen)

	Adds the base names to a comobobox list.  Only bases with length
	>=minlen and <= maxlen will be added

	int minlen = minimum length of base set or 0 for no checking
	int maxlen = max length of base set or 0 for no checking
	HWND hctrl = handle of comobox to add base names to.
*****************************************************************************/
void  IMPAddBasesToCB(HWND hctrl,int minlen,int maxlen)
	{
	int bi;
	int l;
	for(bi=0;bi<nbases;bi++)
		{
		l=lstrlen(bases[bi].set);
		if(minlen && l<minlen)
			continue;
		if(maxlen && l>maxlen)
			continue;
		SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)bases[bi].name);
		}
	}

/*****************************************************************************
	imsteps = IMPCompileMarks(pimp,pvals,xymode,cure)

	Compiles the marks of an IMP into an IMSTEP sequence.

	PIMSTEP imsteps = Pointer of new IMSTEP sequence or NULL on error.
	IMPVALS *pvals = Values for variables
	PIMP pimp = IMP to compile.
	IMXYMODE xymode = The mode the IM. XY XS SY STATIONARY
******************************************************************************/
PIMSTEP  IMPCompileMarks(PIMP pimp,IMPVALS *pvals,IMXYMODE xymode)
	{
	return IMPCompile(pimp,pvals,xymode,-1,FALSE);
	}

/*****************************************************************************
	imsteps = IMPCompileCureSteps(pimp,xymode,nesti)

	Compiles the CureSteps of an IMP into an IMSTEP sequence.

	PIMSTEP imsteps = Pointer of new IMSTEP sequence or NULL on error.
	PIMP pimp = IMP to compile.
	IMXYMODE xymode = The mode the IM. XY XS SY STATIONARY
	int nesti = nest index to mark cure step with
******************************************************************************/
PIMSTEP  IMPCompileCureSteps(PIMP pimp,IMXYMODE xymode,int nesti)
	{
	return IMPCompile(pimp,NULL,xymode,nesti,TRUE);
	}

/*****************************************************************************
	len = IMPExpandTextToBuf(buf,bufsize,text,vars,vals,gvars,gvals,
		ph,pudhas,showerr)

	Expands any variable refrences in text and puts the result in
	buf returning then length len.  Searches globals first then
	instance second.

	int len = length of resultant string in buf or 0 on error
	LPSTR buf = buffer to receive compiled result
	int bufsize = size of buf (including space for null)
	LPSTR text = text to expand.
	IMPVARS *vars = vars for expansion.
	IMPVALS *vals = vals for expansion. (NULL for prevals)
	IMPVARS *gvars = global vars for expansion.
	IMPVALS *gvals = global vals for expansion. (NULL for prevals)
	IMPHEADER *ph = header for expansion
	IMPUDHAS *pudhas = user defined header entries for expansion
	BOOL showerr = FALSE to suppress display of errors.
******************************************************************************/
int  IMPExpandTextToBuf(LPSTR buf,int bufsize,LPSTR text,IMPVARS *vars,IMPVALS *vals,
	IMPVARS *gvars,IMPVALS *gvals,IMPHEADER *ph,IMPUDHAS *pudhas,BOOL showerr)
	{
	char c;
	LPSTR s;
	int i;
	long l;
	int len=0;

	s=text;
	while(1)
		{
		c=*s;
		switch(c)
			{
			case '\r' : // skip \r (for multiline marks)
				s++;
				continue;
			case 0: // end
				break;
			case '%': // start of variable name or %%
				s++;
				s=vnRead(s,showerr);
				if(!s)
					return 0;
				if(!vnlen) // if %%
					{
					buf[len++]='%';
					if(len>=bufsize)
						{
						if(showerr)
							ShowError(IDE_COMPBUF);
						return 0;
						}

					continue;
					}
				if(vnbuf[0]=='@') // if variable has system flag
					{
					i=vnSysExpand(((LPSTR)buf)+len);
					if(!i)
						{
						if(showerr)
							ShowError(IDE_SYSVARNOTFOUND);
						return 0;
						}
					len+=i;
					if(len>=bufsize)
						{
						if(showerr)
							ShowError(IDE_COMPBUF);
						return 0;
						}
					continue;
					}
				if(vnbuf[0]=='!') // if variable has header flag
					{
					i=haFind(pudhas);
					if(i==-1)
						{
						if(showerr)
							ShowError(IDE_HANOTFOUND);
						return 0;
						}
					len+=HAToBuf(ph,pudhas,i,&buf[len]);
					if(len>=bufsize)
						{
						if(showerr)
							ShowError(IDE_COMPBUF);
						return 0;
						}
					continue;
					}

				// Not system or header.

				// Global ?
				i=vnFind(gvars);
				if(i>=0)
					{
					len+=ValToBuf(gvals,i,gvars,((LPSTR)buf)+len);
					if(len>=bufsize)
						{
						if(showerr)
							ShowError(IDE_COMPBUF);
						return 0;
						}
					continue;
					}

				// Instance ?
				if(vars)
					{
					i=vnFind(vars);
					if(i>=0)
						{
						len+=ValToBuf(vals,i,vars,((LPSTR)buf)+len);
						if(len>=bufsize)
							{
							if(showerr)
								ShowError(IDE_COMPBUF);
							return 0;
							}
						continue;
						}
					}

				// %nnn% raw code insert
				for(i=0;i<vnlen;i++)
					if(vnbuf[i]<'0' || vnbuf[i]>'9')
						break;
				if(i==vnlen)
					{
					ParseLong(vnbuf,&l);
					if(l>=0L && l<=255L)
						{
						buf[len++]=(char)(l & 255);
						if(len>=bufsize)
							{
							if(showerr)
								ShowError(IDE_COMPBUF);
							return 0;
							}
						continue;
						}
					}

				// Not found!
				if(showerr)
					ShowError(IDE_VARNOTFOUND);
				return 0;

			default:
				s++;
				buf[len++]=c;
				if(len>=bufsize)
					{
					if(showerr)
						ShowError(IDE_COMPBUF);
					return 0;
					}
				continue;
			}
		break;
		}
	buf[len]=0;
	return len;
	}

/*****************************************************************************
	ok = IMPExpandTextToTbuf(text,vars,vals,gvars,gvals,ph,pudhas)

	Calls ExpandTextToBuf to compile the text to tbuf and set tlen
	to its length. Any errors are displayed when encountered.

	See IMPExpandTextToBuf for arguments except:
	BOOL ok = TRUE on success.
******************************************************************************/
BOOL  IMPExpandTextToTbuf(LPSTR text,IMPVARS *vars,IMPVALS *vals,
	IMPVARS *gvars,IMPVALS *gvals,IMPHEADER *ph,IMPUDHAS *pudhas)
	{
	tlen=IMPExpandTextToBuf(tbuf,TSIZE,text,vars,vals,gvars,gvals,
		ph,pudhas,TRUE);
	return (tlen>0);
	}


/******************************************************************************
	IMPPurgeFonts(pimp)

	Scans the marks and fonts of the imp and deletes any fonts that
	have no refrences in the marks.  The holes created by the deltion
	are filled by shifting the remianing fonts down and then deccrementing
	all the font refrences to them.

	PIMP pimp = imp to purge unused fonts from.
******************************************************************************/
void  IMPPurgeFonts(PIMP pimp)
	{
	int idf;
	int idm;
	WORD frem;

	idf=0;
	while(idf<pimp->fonts.count) // Go through all the fonts
		{
		// Search for mark that refrences idf
		idm=pimp->marks.count;
		while(idm--)
			if(MarkUsesFont(&pimp->marks.mark[idm],idf)) // Break if refd
				break;
		if(idm==-1) // If not found
			{
			frem=pimp->fonts.count-idf-1; // Comp remaining fonts after idf
			if(frem) // If any
				{
				// shift them down
				memmove(&pimp->fonts.name[idf],
					&pimp->fonts.name[idf+1],
					frem*sizeof(IJFONTNAME));
				// Fix all shifted refrences
				for(idm=0;idm<pimp->marks.count;idm++)
					MarkDecGreaterFonts(&pimp->marks.mark[idm],idf);
				}
			pimp->fonts.count--;
			continue;
			}
		idf++;
		}
	}

/******************************************************************************
	len = ValToBuf(vals,i,vars,buf)

	Renders a variable value to a buffer.

	WORD len = length of result placed in buf
	IMPVALS *vals = values ( or NULL t use preval)
	WORD i = index of variable to render
	IMPVARS *vars = variables
	LPSTR buf = place to store result.
******************************************************************************/
WORD  ValToBuf(IMPVALS *vals,WORD i,IMPVARS *vars,LPSTR buf)
	{
	WORD len;
	char fmt[10];

	len=0;
	switch(vars->var[i].type)
		{
		case vtKEYED:
			lstrcpy(buf,vals ? vals->val[i].k : vars->var[i].preval.k);
			len=lstrlen(buf);
			break;
		case vtDATE:
			if(vals && vals->val[i].d.override[0]) // If overridden
				lstrcpy(buf,vals->val[i].d.override);
			else
				GetDate(buf,vars->var[i].d.format,vars->var[i].d.offset,
					vars->var[i].d.base);
			len=lstrlen(buf);
			break;

		case vtTIME:
			if(vals && vals->val[i].t.override[0]) // If overridden
				lstrcpy(buf,vals->val[i].t.override);
			else
				GetTime(buf,
					vars->var[i].t.useNames,
					vars->var[i].t.is24,
					vars->var[i].t.isLZ,
					vars->var[i].t.seperator,
					vars->var[i].t.amSuffix,
					vars->var[i].t.pmSuffix,
					vars->var[i].t.showSeconds,
					vars->var[i].t.offset,
					vars->var[i].t.base);
			len=lstrlen(buf);
			break;
		case vtBASED:
			lstrcpy(buf,vals ? vals->val[i].b : vars->var[i].preval.b);
			len=lstrlen(buf);
			break;
		case vtSERIAL:
			if(vars->var[i].lpad)
				wsprintf(fmt,"%%0%ulu",vars->var[i].width);
			else
				lstrcpy(fmt,"%lu");
			len=wsprintf(buf,fmt,vals ? vals->val[i].s : vars->var[i].preval.s);
			break;
		}
	return len;
	}

/******************************************************************************
	len = HAToBuf(ph,pudha,i,buf)

	Renders a Header Attribute value to a buffer.  I can be with < IMPNBHAS
	for the basic header or >=IMPNBHAS for the User Defined Header Attributes.

	WORD len = length of result placed in buf
	IMPHEADER *ph = Basic header.
	IMPUDHAS *pudha = User defined header.
	WORD i = Index
	LPSTR buf = place to store result.
******************************************************************************/
WORD  HAToBuf(IMPHEADER *ph,IMPUDHAS *pudha,DWORD i,LPSTR buf)
	{
	double v[3],r[2];

	switch(i)
		{
		case 0: // Desc
			return wsprintf(buf,"%s",(LPSTR)ph->desc);
		case 1: // Author
			return wsprintf(buf,"%s",(LPSTR)ph->author);
		case 2: // Fixture
			return wsprintf(buf,"%s",(LPSTR)ph->fixture);
		case 3: // Part Size
			v[0]=ph->partDiameter.di;
			v[1]=ph->partDiameter.hi;
			MScaleVect((LPDOUBLE)&v,2,FALSE);
			VectToString(buf,(LPDOUBLE)&v,2," , ");
			return lstrlen(buf);
			
			/*r[0]=ph->partDiameter.di;
			r[1]=ph->partDiameter.hi;
			
			ph->partSize.x=r[0]/2;
			ph->partSize.y=r[0]/2;
			ph->partSize.z=r[1];
			
			v[0]=ph->partSize.x;
			v[1]=ph->partSize.y;
			v[2]=ph->partSize.z;
			
			MScaleVect((LPDOUBLE)&v,3,FALSE);
			VectToString(buf,(LPDOUBLE)&v,3," x ");
			return lstrlen(buf);*/
		case 4: // Inkjet
			return wsprintf(buf,"%s",(LPSTR)ph->inkjet);
		case 5: // Origin
			v[0]=ph->origin.x;
			v[1]=ph->origin.y;
			MScaleVect((LPDOUBLE)&v,2,FALSE);
			VectToString(buf,(LPDOUBLE)&v,2,",");
			return lstrlen(buf);
		case 6: // Rotation--Y settings--sam
			VectToString(buf,(LPDOUBLE)&(ph->rotation),1,"");
			return lstrlen(buf);
		case 7: // Init
			return wsprintf(buf,"%s",(LPSTR)ph->initsect);
		case 8: // Link
			return wsprintf(buf,"%s",(LPSTR)ph->link);
		case 9: // IncOnLink
			return wsprintf(buf,"%s",(LPSTR)(ph->inconlink ? sYes : sNo));
		case 10: // Inspector
			return wsprintf(buf,"%s",(LPSTR)ph->inspector);
		case 11: // InspectPos
			v[0]=ph->inspectpos.x;
			v[1]=ph->inspectpos.y;
			v[2]=ph->inspectpos.z;
			MScaleVect((LPDOUBLE)&v,3,FALSE);
			VectToString(buf,(LPDOUBLE)&v,3,",");
			return lstrlen(buf);
		case 12: // InspectPrep
			return wsprintf(buf,"%s",(LPSTR)ph->inspectprep);
		case 13: // ConveyorStart
			return wsprintf(buf,"%s",(LPSTR)(ph->conveyorstart ? sYes : sNo));
		case 14: // LinkAutoStart
			return wsprintf(buf,"%s",(LPSTR)(ph->linkautostart ? sYes:sNo));
		case 15: // SkipOnLinkFail
			return wsprintf(buf,"%s",(LPSTR)(ph->skiponlinkfail ? sYes:sNo));
		case 16: // ChainLinkFail
			return wsprintf(buf,"%s",(LPSTR)(ph->chainlinkfail ? sYes:sNo));
		case 17: // Reader
			return wsprintf(buf,"%s",(LPSTR)ph->reader);
		case 18: // ReadPos
			v[0]=ph->readpos.x;
			v[1]=ph->readpos.y;
			v[2]=ph->readpos.z;
			MScaleVect((LPDOUBLE)&v,3,FALSE);
			VectToString(buf,(LPDOUBLE)&v,3,",");
			return lstrlen(buf);
		case 19: // ReadPrep
			return wsprintf(buf,"%s",(LPSTR)ph->readprep);
		case 20: // ReadVar
			return wsprintf(buf,"%s",(LPSTR)ph->readvar);
		case 21: // Part Diameter
			v[0]=ph->partDiameter.di;
			v[1]=ph->partDiameter.hi;
			MScaleVect((LPDOUBLE)&v,2,FALSE);
			VectToString(buf,(LPDOUBLE)&v,2," , ");
			return lstrlen(buf);
		
			
		}
   // Look it up in the UDHAs 
   i-=IMPNBHAS; // Map i to UDHA

	if(i>=pudha->count) // Error if out of range
		return 0;

	return wsprintf(buf,"%s",(LPSTR)pudha->udha[i].val);
	}

/******************************************************************************
	ldesc = IMPLoadDesc(fname,fdesc)

	Extracts the file description from an IMP file.

	int ldesc = length of desc or < 0 on error.
	char *fname = name of file to get desc from.
	char *fdesc = place to store file description if found.
******************************************************************************/
int  IMPLoadDesc(char *fname,char *fdesc)
	{
	HANDLE fh;
//	OFSTRUCT ofs;
	IMPHEADER header;

	char dbuf[100];
	
	strcpy(fdesc,"");
	fh=CreateFile(fname,GENERIC_READ,0,NULL,OPEN_EXISTING,
	  FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh==INVALID_HANDLE_VALUE)
		return -1;
	_lread32(fh,tbuf,MAXSIGNITURE);
	if((!memcmp(tbuf,sigIMP,LSIGIMP))||      // Native
		(!memcmp(tbuf,sigIMPV400,LSIGIMPV400))|| // 368 (same desc offset)
		(!memcmp(tbuf,sigIMPV368,LSIGIMPV368))|| // 368 (same desc offset)
		(!memcmp(tbuf,sigIMPV355,LSIGIMPV355))|| // 355 (same desc offset)
		(!memcmp(tbuf,sigIMPV350,LSIGIMPV350))|| // 350 (same desc offset)
		(!memcmp(tbuf,sigIMPV347,LSIGIMPV347))|| // 347 same header
		(!memcmp(tbuf,sigIMPV343,LSIGIMPV343)))  // 343 same header
		{
		_llseek32(fh,LSIGIMP,0);
		if(_lread32(fh,(LPSTR)&dbuf,IMPLDESC+1)==IMPLDESC+1)
			lstrcpy(fdesc,dbuf);
		} 
	else if(!memcmp(tbuf,sigIMPV364,LSIGIMPV364)) // 364 deferrent desc offset
		{
		_llseek32(fh,LSIGIMPV364,0);
		if(_lread32(fh,(LPSTR)&dbuf,IMPLDESC+1)==IMPLDESC+1)
			lstrcpy(fdesc,header.desc);
		}
	else if(!memcmp(tbuf,sigIMPV341,LSIGIMPV341)) // Inline 341 xlate
		{
		_llseek32(fh,LSIGIMPV341,0);
		XLoadDescIMPV341(fh,fdesc);
		}
	else if(!memcmp(tbuf,sigIMPV339,LSIGIMPV339)) // Inline 339 xlate
		{
		_llseek32(fh,LSIGIMPV339,0);
		XLoadDescIMPV341(fh,fdesc);
		}
	else if(!memcmp(tbuf,sigIMPV338,LSIGIMPV338))  // XLOAD
		{
		_llseek32(fh,LSIGIMPV338,0);
		XLoadDescIMPV338(fh,fdesc);
		}
	else if(!memcmp(tbuf,sigIMPV330,LSIGIMPV330))  // XLOAD
		{
		_llseek32(fh,LSIGIMPV330,0);
		XLoadDescIMPV330(fh,fdesc);
		}
	else if(!memcmp(tbuf,sigIMP4001,LSIGIMP4001))   // XLOAD
		{
		_llseek32(fh,LSIGIMPV330,0);
		XLoadDescIMP4001(fh,fdesc);
		}
	CloseHandle(fh);			
	return lstrlen(fdesc);
	}

/***************************************************************************
	match = IMPSigMatch(sig)

	Compares a signiture against the set of IMP signitures.

	BOOL match = TRUE if sig is an IMP signiture.
	LPSTR sig = signiture to match.
***************************************************************************/
BOOL  IMPSigMatch(LPSTR sig)
	{
	return	(!memcmp(sig,sigIMP,LSIGIMP))||
			(!memcmp(sig,sigIMPV400,LSIGIMPV400))||
			(!memcmp(sig,sigIMPV368,LSIGIMPV368))||
			(!memcmp(sig,sigIMPV364,LSIGIMPV364))||
			(!memcmp(sig,sigIMPV355,LSIGIMPV355))||
			(!memcmp(sig,sigIMPV350,LSIGIMPV350))||
			(!memcmp(sig,sigIMPV347,LSIGIMPV347))||
			(!memcmp(sig,sigIMPV343,LSIGIMPV343))||
			(!memcmp(sig,sigIMPV341,LSIGIMPV341))||
			(!memcmp(sig,sigIMPV339,LSIGIMPV339))||
			(!memcmp(sig,sigIMPV338,LSIGIMPV338))||
			(!memcmp(sig,sigIMPV330,LSIGIMPV330))||
			(!memcmp(sig,sigIMP4001,LSIGIMP4001));
	}

/******************************************************************************
	set = IMPGetBasedSet(basename)

	Provides access to the bases for external modules.
	used by GetDate and GetTime in IMS5000.CPP.

	LPCSTR set = points to a set string or NULL if the name is not found.
	LPSTR basename = the base name to look for.
******************************************************************************/
LPCSTR  IMPGetBasedSet(LPSTR basename)
	{
	int bi;
	bi=FindBasesIndex(basename);
	if(bi==-1)
		return NULL;
	return bases[bi].set;
	}

/******************************************************************************
	r=_IMPGetUDHAInt(pu,key,def)

	int r = return int.
	IMPUDHAS *pu = UDHAS to search
	LPSTR key = key string
	int def = default value if not found or not integer.
******************************************************************************/
int  _IMPGetUDHAInt(IMPUDHAS *pu,LPSTR key,int def)
	{
	IMPUDHA *udha;
	int i,r;

	udha=NULL;
	for(i=0;i<pu->count;i++)
		{
		udha=&(pu->udha[i]);
		if(!lstrcmp(udha->name,key))
			break;
		udha=NULL;
		}
	r=def;
	if(udha) // if found
		{
		if(!ParseInt(udha->val,&r))
			r=def;
		}
	return r;
	}

/******************************************************************************
	r=IMPGetUDHAInt(pimp,key,def)

	int r = return int.
	PIMP pimp = imp owning udhas to search
	LPSTR key = key string
	int def = default value if not found or not integer.
******************************************************************************/
int  IMPGetUDHAInt(PIMP pimp,LPSTR key,int def)
	{
	/// this used to lock unlock handle udhas the _ version should be
	/// renamed to this and all calls to _ changed to complete the cleanup
	return _IMPGetUDHAInt(&pimp->udhas,key,def);
	}

/******************************************************************************
	count = IMPCountMarks(pimp)

	Counts the IMPMarks that are not IMPCureSteps in the mark list of the imp.

	int count = number of non curesteps found in imp
	PIMP = pimp to count cure steps in.
******************************************************************************/
int  IMPCountMarks(PIMP pimp)
	{
	int count=0;
	for(int i=pimp->marks.count;i--;)
		if(pimp->marks.mark[i].type!=mtCURE)
			count++;
	return count;
	}


/******************************************************************************
	count = IMPCountCureSteps(pimp)

	Counts the IMPCureSteps in the mark list of the imp.

	int count = number of curesteps found in imp
	PIMP = pimp to count cure steps in.
******************************************************************************/
int  IMPCountCureSteps(PIMP pimp)
	{
	int count=0;
	for(int i=pimp->marks.count;i--;)
		if(pimp->marks.mark[i].type==mtCURE)
			count++;
	return count;
	}

/******************************************************************************
	hbitmap IMPCompileRaster(m,
		pvars,pvals,pgvars,pgvals,pheader,pudhas,
		bfi,xres,yres,ms,neg,BOOL showerr)

	Compiles any raster class marks into a bitmap and also copies out
	the bfi,xres,yres,ms and neg attributes so that the caller can treat
	all raster class marks the same. Does not use tbuf.

	HBITMAP hbitmap= Handle to compiled bitmap or NULL on error.
	IMPMARK *m = mark to compile
	IMPVARS *pvars= instance vars
	IMPVALS *pvals= instance values
	IMPVARS *pgvars= global vars
	IMPVALS *pgvals= global values
	IMPHEADER *pheader= header
	IMPUDHAS *pudhas= udhas

	int *bfi = filled in with marks bfi if not NULL
	double *xres= filled in with marks xres if not NULL
	double *yres= filled in with marks yres if not NULL
	WORD *ms= filled in with marks ms if not NULL
	BOOL *neg= filled in with mark neg if not NULL
	BOOL showerr = TRUE to allow display of error messages.
******************************************************************************/
HBITMAP  IMPCompileRaster(IMPMARK *m,
	IMPVARS *pvars,IMPVALS *pvals,IMPVARS *pgvars,IMPVALS *pgvals,
	IMPHEADER *pheader,IMPUDHAS *pudhas,
	int *bfi,double *xres,double *yres,WORD *ms,BOOL *neg,BOOL showerr)
	{
	char cbuf[384];       // compilation buffer 1
	char hrbuf[384];      // compilation buffer for barcode Human Readable Text
	char bmpdsc[TSIZE+5]; // bitmap descriptor when rendering bitmap
	WORD ldsc;            // length of bmpdsc
	HBITMAP hbitmap;
	switch(m->type)
		{
		case mtBITMAP:
			if(m->b.ms==0) // Original 4001 files might have 0 here
				m->b.ms=1;
			// Expand vars in bitmap filename to cbuf
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->b.fn,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			// Build Bitmap Descriptor
			ldsc=1+wsprintf(bmpdsc,"[BMP]%s",(LPSTR)cbuf);
			hbitmap=LoadedBitmapsFind(bmpdsc,ldsc); // Check for pre loaded
			if(!hbitmap) // If not allready loaded
				{
				strfnextend(cbuf,"BMP");
				if(BMPDir[0] &&  // If no absolute path defined, prefix it
					!((cbuf[0]=='\\') || (cbuf[1]==':' && cbuf[2]=='\\')))
					{
					int clen=lstrlen(cbuf);
					int lbmpdir=lstrlen(BMPDir);
					memmove(cbuf+lbmpdir,cbuf,clen+1);
					memmove(cbuf,BMPDir,lbmpdir);
					}
				// Load bitmap
				hbitmap=LoadBMP(cbuf,showerr);
				if(hbitmap)
					LoadedBitmapsAdd(bmpdsc,ldsc,hbitmap);
				}
			if(xres)
				*xres=m->b.xres;
			if(yres)
				*yres=m->b.yres;
			if(ms)
				*ms=m->b.ms;
			if(neg)
				*neg=m->b.neg; // IM must invert the bitmap
			if(bfi)
				*bfi=0; /// update when bitmaps have bfi
			return hbitmap;
		case mtC128:
			if(m->c128.ms==0) // Original 4001 files might have 0 here
				m->c128.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->c128.text,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			IMPExpandTextToBuf(hrbuf,sizeof(hrbuf),m->c128.hrpict,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr);
			hbitmap=C128RenderBitmap(m,cbuf,hrbuf);
			if(xres)
				*xres=m->c128.xres;
			if(yres)
				*yres=m->c128.yres;
			if(ms)
				*ms=m->c128.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTEND LATER
			return hbitmap;
		case mtCUPCEAN:
			{
			if(m->cupcean.ms==0)
				m->cupcean.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->cupcean.text,pvars,
				pvals,pgvars,pgvals,pheader,pudhas,showerr))
				return 0;

			// UPCA must be 11 digits and EAN8 must be 7
			int len = lstrlen(cbuf);

			if ((len!=11 && m->cupcean.subtype==UPCA) || (len!=7 && m->cupcean.subtype==EAN8)){
				if (IMSMode==mRUN){
					ShowError(IDE_BADUPC);
					return 0;
				}
			}

			if (IMSMode==mRUN){
				for (int i=0;i<len;i++){
					char ch = cbuf[i];
					if (ch<'0' || ch>'9'){
						ShowError(IDE_BADUPC);
						return 0;
					}
				}
			}
			
			IMPExpandTextToBuf(hrbuf,sizeof(hrbuf),m->cupcean.hrpict,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr);
			hbitmap=CUPCEANRenderBitmap(m,cbuf);
//zzz			hbitmap=CUPCEANRenderBitmap(m,cbuf,hrbuf);
			if(xres)
				*xres=m->cupcean.xres;
			if(yres)
				*yres=m->cupcean.yres;
			if(ms)
				*ms=m->cupcean.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTEND LATER
			return hbitmap;
			}
		case mtC93:
			if(m->c93.ms==0) // Original 4001 files might have 0 here
				m->c93.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->c93.text,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			if (!ValidateCodeChars(cbuf,m->type) && IMSMode==mRUN){
				ShowError(IDE_BADC93);
				return 0;
			}
			IMPExpandTextToBuf(hrbuf,sizeof(hrbuf),m->cupcean.hrpict,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr);
			hbitmap=C93RenderBitmap(m,cbuf);
//zzz			hbitmap=C93RenderBitmap(m,cbuf,hrbuf);
			if(xres)
				*xres=m->c93.xres;
			if(yres)
				*yres=m->c93.yres;
			if(ms)
				*ms=m->c93.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTENT LATER
			return hbitmap;
		//case uid12:                                     //--------------------Linghai-11-21-08--
		//case uid6:
		//case uid5:                                      //--------------------Linghai-11-21-08--
		case mtUID:                                        //-------------Linghai-4-9-09----UID
			if(!DM001IsLinked) // Error on DM001 marks if no DLL linked
				return 0;
			if(m->uid.ms==0)
				m->uid.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->uid.text,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			hbitmap=CDMRenderBitmap(m,cbuf,showerr);
			if(xres)
				*xres=m->uid.xres;
			if(yres)
				*yres=m->uid.yres;
			if(ms)
				*ms=m->uid.ms;
			if(neg)
				*neg=FALSE; // CDM has no neg
			if(bfi)
				*bfi=0; /// EXTENT LATER
			return hbitmap;
			
		case mtCDM:
			if(!DM001IsLinked) // Error on DM001 marks if no DLL linked
				return 0;
			if(m->cdm.ms==0)
				m->cdm.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->cdm.text,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			hbitmap=CDMRenderBitmap(m,cbuf,showerr);
			if(xres)
				*xres=m->cdm.xres;
			if(yres)
				*yres=m->cdm.yres;
			if(ms)
				*ms=m->cdm.ms;
			if(neg)
				*neg=FALSE; // CDM has no neg
			if(bfi)
				*bfi=0; /// EXTENT LATER
			return hbitmap;
		case mtWINFONT:
			if(m->winfont.ms==0)
				m->winfont.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->winfont.text,pvars,
				pvals,pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			hbitmap=WINFONTRenderBitmap(m,cbuf);
			if(xres)
				*xres=m->winfont.xres;
			if(yres)
				*yres=m->winfont.yres;
			if(ms)
				*ms=m->winfont.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTENT LATER
			return hbitmap;
		case mtWMTRX:
			if(m->wmtrx.ms==0)
				m->wmtrx.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->wmtrx.text,pvars,
				pvals,pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			hbitmap=WMTRXRenderBitmap(m,cbuf,false);
			if(xres)
				*xres=m->wmtrx.xres;
			if(yres)
				*yres=m->wmtrx.yres;
			if(ms)
				*ms=m->wmtrx.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTENT LATER
			return hbitmap;

		case mtCQR:
			if(m->cqr.ms==0)
				m->cqr.ms=1;
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->cqr.text,pvars,
				pvals,pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			hbitmap=CQRRenderBitmap(m,cbuf,showerr);
			if(xres)
				*xres=m->cqr.xres;
			if(yres)
				*yres=m->cqr.yres;
			if(ms)
				*ms=m->cqr.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTENT LATER
			return hbitmap;

		case mtPATCH:
			if(m->patch.ms==0)
				m->patch.ms=1;
			hbitmap=PATCHRenderBitmap(m);
			if(xres)
				*xres=m->patch.xres;
			if(yres)
				*yres=m->patch.yres;
			if(ms)
				*ms=m->patch.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTEND LATER
			return hbitmap;
		case mtC2OF5:
			{
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->c2of5.text,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			
			int texlen = lstrlen(cbuf);
			if (IMSMode == mRUN){
				for (int i=0;i<texlen;i++){
					char ch = cbuf[i];
					if (ch<'0' || ch>'9'){
						ShowError(IDE_BADC2OF5);
						return 0;
					}
				}
			}
			
			IMPExpandTextToBuf(hrbuf,sizeof(hrbuf),m->c2of5.hrpict,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr);


			hbitmap=C2OF5RenderBitmap(m,cbuf);
//zzz			hbitmap=C2OF5RenderBitmap(m,cbuf,hrbuf);
			if(xres)
				*xres=m->c2of5.xres;
			if(yres)
				*yres=m->c2of5.yres;
			if(ms)
				*ms=m->c2of5.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTEND LATER
			return hbitmap;
			}
		case mtC39:
			if(!IMPExpandTextToBuf(cbuf,sizeof(cbuf),m->c39.text,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr))
				return 0;
			if (!ValidateCodeChars(cbuf,m->type) && IMSMode==mRUN){
				ShowError(IDE_BADC39);
				return 0;
			}
			IMPExpandTextToBuf(hrbuf,sizeof(hrbuf),m->c39.hrpict,pvars,pvals,
				pgvars,pgvals,pheader,pudhas,showerr);
			hbitmap=C39RenderBitmap(m,cbuf);
//zzz			hbitmap=C39RenderBitmap(m,cbuf,hrbuf);
			if(xres)
				*xres=m->c39.xres;
			if(yres)
				*yres=m->c39.yres;
			if(ms)
				*ms=m->c39.ms;
			if(neg)
				*neg=FALSE; // Allready rendered
			if(bfi)
				*bfi=0; /// EXTEND LATER
			return hbitmap;
		}
	return 0; // Invalid
	}

BOOL IMPDeviceMarkIJFMTINFO(IMP* pimp,IMPMARK *m,IJFMTINFO *fmti)
{
	int fnum,dfi;
	switch (m->type){

	case mtTEXT:
		fnum = m->t.ijfmt.font;
		dfi=SendMessage(EditorIJDev,IJM_FINDFONT,0,(LPARAM)(LPSTR)pimp->fonts.name[fnum]);
		fmti->fmt.font=dfi;
		fmti->fmt.ms=1;
		SendMessage(EditorIJDev,IJM_GETFMTINFO,0,(LPARAM)(LPSTR)fmti);
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL IMPIsNativeVersion(WORD ver){
	if (ver==400 || ver==0)
		return true;
	else
		return false;
}


/******************************************************************************
	STATIC FUNCTIONS
******************************************************************************/
/*****************************************************************************
	imsteps = IMPCompile(pimp,pvals,xymode,cure)

	Compiles an IMP into an IMSTEP sequence.
	Called by IMPCompileMarks or IMPCompileCureSteps

	PIMSTEP imsteps = Pointer of new IMSTEP sequence or NULL on error.
	IMPVALS *pvals = Values for variables
	PIMP pimp = IMP to compile.
	IMXYMODE xymode = The mode the IM. XY XS SY STATIONARY
	int nesti = nest index to mark curesteps only
	BOOL cure;
******************************************************************************/
static PIMSTEP IMPCompile(PIMP pimp,IMPVALS *pvals,IMXYMODE xymode,int nesti,BOOL cure)
	{
	PIMSTEP imsteps;
	IMPMARK  *m;     // Current mark
	IMPHEADER *h;// header value ---sam 06/12
	IMCORR *c; // Current mark correction
	M3 m3;
	IJFMT ijfmt;
	WORD mi;          // Marks index
	HBITMAP hbitmap;
	HIJ hij;         // inkjet
	WORD fontmap[IMPMAXFONTS];
	int i;  // General looping
	P2 porigin; // Mark origin pushed
	P2 morigin; // Non pushed part of mark origin (for rf S coordinate)
	P2 extent; // Bitmap or Text extent for justification
	P2 poriginrot; // Mark origin pushed---sam
	P2 moriginrot; // Non pushed part of mark origin (for rf S coordinate)---sam
	P3 pdegree; // Bitmap or Text extent for justification---sam
	int cmc; // compiled mark count (skipped do not count)
	RADE rade; // Radius Expression
	double cw,ch; // Char Cell Width and Height computed for CDev marks
	double xres,yres; // Raster mark info
	WORD ms;
	BOOL neg;
	int bfi;
	HBITMAP *hbmps=NULL;
	int nhbmps=0;
	double partdia = h->partDiameter.di;
	static int mystaticint=8;

	// mps rotary test udha start
		char xygDir[128]; 
		strcpy(xygDir,IMSDir); 
		strncat (xygDir, "\\XYGRoty.ini", 12); 
		int mps; 
		mps=IMPGetUDHAInt(pimp,"RotaryGalil",0); 
		if (mps) { 
			WritePrivateProfileString("XYGRoty","RotaryGalil","1",xygDir);
		}else {
			WritePrivateProfileString("XYGRoty","RotaryGalil","0",xygDir);
		}

			// Rottary rotary test udha start
		char xygDir7[128]; 
		strcpy(xygDir7,IMSDir); 
		strncat (xygDir7, "\\XYGRottary.ini", 12); 
		int Rottary; 
		Rottary=IMPGetUDHAInt(pimp,"Rotary",0); 
		if (Rottary) { 
			WritePrivateProfileString("XYGRottary","Rotary","1",xygDir7);
		}else {
			WritePrivateProfileString("XYGRottary","Rotary","0",xygDir7);
		}


	BuildRADE(&rade,xymode,&pimp->udhas);


	// Find HIJ for the inkjet if it exists.
	hij=(int)SendMessage(IMWnd,IMM_FINDIJ,0,(LONG)(LPSTR)&pimp->pheader->inkjet);
	if(hij==HIJERROR) // If IJ not found
		{
		ShowError(IDE_FINDIJ);
		return NULL;
		}
	// Select th ij now to build fonttable
	SendMessage(IMWnd,IMM_SELIJ,hij,1);
	// Build fontmap
	for(i=0;i<pimp->fonts.count;i++)
		{
		fontmap[i]=(short)SendMessage(IMWnd,IMM_FINDFONT,
			0,(LONG)(LPSTR)pimp->fonts.name[i]);
		if(fontmap[i]==-1)
			{
			ShowError(IDE_NOTIJFONT);
			return NULL;
			}
		}


	if(HideOptRotaryIMF)
	{
	// Build imsteps
	// First step is origin

	//	double xorigin= RunIMF.originrot.x- (pimp->pheader->partDiameter.di)/2;
	m3 = M3(TRANSLATE,pimp->pheader->origin.x,pimp->pheader->origin.y);  //zzz - May be wrong
	imsteps=IMPUSHSTEPNew(&m3);
	// Part rotation
	m3 = M3(ROTATE,0,0,pimp->pheader->rotation*DEG);  //zzz - May be wrong
	IMSTEPAppend(imsteps,IMPUSHSTEPNew(&m3));
	}
	if(cure)
		IMSTEPAppend(imsteps,IMMSGSTEPNew("CURING"));
	cmc=0;
	for(mi=0;mi<pimp->marks.count;mi++) // Add for each mark
		{
		
		if(HideOptRotaryIMF)
		{	
			m=&(pimp->marks.mark[mi]);
			c=pimp->imcorrs.find(mi);
			// Compute pushed and nonpushed mark origins
			porigin=m->origin;
			morigin.x=morigin.y=0.0f;
			if(xymode==imXS)
				{
				morigin.y=porigin.y;
				porigin.y=0.0f;
				}
			else if(xymode==imSY)
				{
				morigin.x=porigin.x;
				porigin.x=0.0f;
				}
			// push mark x,y
			m3 = M3(TRANSLATE,porigin.x,porigin.y); //zzz - May be wrrong
	//		IMSTEPAppend(imsteps,IMPUSHSTEPNew(
	//			M3Translate(&m3,porigin.x,porigin.y)));
			IMSTEPAppend(imsteps,IMPUSHSTEPNew(&m3));
		}

/*		else
		{
			m=&(pimp->marks.mark[mi]);
			c=pimp->imcorrs.find(mi);
			// Compute pushed and nonpushed mark origins
			pdegree.x=(RunIMF.originrot.x)-((pimp->pheader->partDiameter.di)/2);
			pdegree.y=RunIMF.originrot.y;
			// push mark x,y
			m3 = M3(TRANSLATE,pdegree.x,pdegree.y); //zzz - May be wrrong
	//		IMSTEPAppend(imsteps,IMPUSHSTEPNew(
	//			M3Translate(&m3,porigin.x,porigin.y)));
			IMSTEPAppend(imsteps,IMPUSHSTEPNew(&m3));
		}  */

		// push mark rotation/slant
		IMSTEPAppend(imsteps,IMPUSHSTEPNew(
			M3RotSlant(&m3,m->rotation*DEG,m->slant*DEG)));
		
		switch(m->type)
			{
			case mtTEXT:
				if(cure) // Ignore marks if compiling for cure
					break;
				if(!IMPExpandTextToTbuf(m->t.text,&pimp->vars,pvals,&pimp->gvars,&pimp->gvals,
					pimp->pheader,&pimp->udhas))
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}

				if(ReplaceZeros)
					Replace0withO(tbuf);

				ijfmt.font=fontmap[m->t.ijfmt.font];
				ijfmt.ms=m->t.ijfmt.ms;
				ijfmt.xflip=m->t.ijfmt.xflip;
				ijfmt.yflip=m->t.ijfmt.yflip;

				// Justify
				IMTextExtent(&extent,hij,&ijfmt,m->t.cw,0,m->t.ch,tbuf,0,0,NULL);
				JustifyOrigin(m->justx,m->justy,&extent,&morigin);
				AppendRFScale(imsteps,xymode,&morigin,&rade);

				IMSTEPAppend(imsteps,IMTEXTSTEPNew(&morigin,m->zflight,partdia,
					&ijfmt,m->t.cw,0,m->t.ch,m->rep,tbuf,
					0,0,NULL,c));
				//zlast=m->zflight;
				zlast=m->zflight;
				cmc++;
				break;
			case mtDEVBC:
				if(cure) // Ignore marks if compiling for cure
					break;
				if(!IMPExpandTextToTbuf(m->devbc.text,&pimp->vars,pvals,&pimp->gvars,&pimp->gvals,
					pimp->pheader,&pimp->udhas))
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}
				ijfmt=m->devbc.ijfmt;
				ijfmt.font=fontmap[ijfmt.font];

				// Justify
				IMTextExtent(&extent,hij,&ijfmt,m->devbc.cw,0,m->devbc.ch,tbuf,
					0,0,NULL);
				JustifyOrigin(m->justx,m->justy,&extent,&morigin);
				AppendRFScale(imsteps,xymode,&morigin,&rade);
				IMSTEPAppend(imsteps,IMTEXTSTEPNew(&morigin,m->zflight,partdia,
						&ijfmt,m->devbc.cw,0,m->devbc.ch,m->rep,tbuf,
						0,0,NULL,c));
				zlast=m->zflight;
				cmc++;
				break;
			case mtDEVSYM: // zzz Very important devsym code
		
				if(cure) // Ignore marks if compiling for cure
					break;
				if(!IMPExpandTextToTbuf(m->devsym.text,&pimp->vars,pvals,&pimp->gvars,&pimp->gvals,
					pimp->pheader,&pimp->udhas))
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}

				if(ReplaceZeros)
					Replace0withO(tbuf);

				ijfmt.font=fontmap[m->devsym.ijfmt.font];
				ijfmt.ms=m->devsym.ijfmt.ms;
				ijfmt.xflip=m->devsym.ijfmt.xflip;
				ijfmt.yflip=m->devsym.ijfmt.yflip;

				cw = m->devsym.width;
				ch = m->devsym.height;

				// Justify
				IMTextExtent(&extent,hij,&ijfmt,cw,m->devsym.xdots,ch,tbuf,0,0,NULL);


				JustifyOrigin(m->justx,m->justy,&extent,&morigin);
				AppendRFScale(imsteps,xymode,&morigin,&rade);

				// zzz What kind of step do we append here?  Text, Bitmap ??
				IMSTEPAppend(imsteps,IMTEXTSTEPNew(&morigin,m->zflight,partdia,
					&ijfmt,cw,m->devsym.xdots,ch,m->rep,tbuf,
					0,0,NULL,c));
				zlast=m->zflight;
				cmc++;
				break;

			case mtBITMAP: // All raster marks
			case mtC128:
			case mtCUPCEAN:
			case mtC93:
			case mtCDM:
			case mtUID:                                                  //-------------Linghai-4-9-09---
			case mtWINFONT:
			case mtWMTRX:
			case mtCQR:
			case mtPATCH:
			case mtC2OF5:
			case mtC39:
				if(cure) // Ignore marks if compiling for cure
					break;
				hbitmap=IMPCompileRaster(m,
					&pimp->vars,pvals,&pimp->gvars,&pimp->gvals,pimp->pheader,&pimp->udhas,
					&bfi,&xres,&yres,&ms,&neg,TRUE);
				if(!hbitmap)
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}
				// 
				IMBitmapExtent(&extent,hij,xres,yres,hbitmap);
				JustifyOrigin(m->justx,m->justy,&extent,&morigin);
				AppendRFScale(imsteps,xymode,&morigin,&rade);
				IMSTEPAppend(imsteps,IMBITMAPSTEPNew(&morigin,
					zmarker+m->zflight,partdia,xres,yres,neg,ms,
					bfi,hbitmap,m->rep,c));
				zlast=m->zflight;
				cmc++;
				break; // IMSTEPS owns the bitmap now

			case mtMULTILINE:
				if(cure) // Ignore marks if compiling for cure
					break;
				if(!IMPExpandTextToTbuf(m->ml.text,&pimp->vars,pvals,
					&pimp->gvars,&pimp->gvals,pimp->pheader,&pimp->udhas))
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}
				ijfmt=m->ml.ijfmt;
				ijfmt.font=fontmap[ijfmt.font];

				if(ReplaceZeros)
					Replace0withO(tbuf);

				// Justify
				IMTextExtent(&extent,hij,&ijfmt,m->ml.cw,0,m->ml.ch,tbuf,
					0,0,NULL);
				JustifyOrigin(m->justx,m->justy,&extent,&morigin);
				AppendRFScale(imsteps,xymode,&morigin,&rade);
				IMSTEPAppend(imsteps,IMTEXTSTEPNew(&morigin,m->zflight,partdia,
					&ijfmt,m->ml.cw,0,m->ml.ch,m->rep,tbuf,
					0,0,NULL,c));
				zlast=m->zflight;
				cmc++;
				break;
			case mtCURE:
				if(!cure) // Ignore cure if not in compiling for cure
					break;
				morigin.x=morigin.y=0.0f;
				AppendRFScale(imsteps,xymode,&morigin,&rade);
				IMSTEPAppend(imsteps,IMCURESTEPNew(&morigin,m->zflight,
					m->cure.xsize,m->cure.vel,m->cure.keepon,m->rep,nesti));
				cmc++;
				break;
			case mtCDEV:
				if(cure) // Ignore marks if compiling for cure
					break;
				ijfmt=m->cdev.bijfmt;
				ijfmt.font=fontmap[ijfmt.font];

				if(!IMPExpandTextToTbuf(m->cdev.text,&pimp->vars,pvals,
					&pimp->gvars,&pimp->gvals,pimp->pheader,&pimp->udhas))
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}
				// map \033Fn fonts in text to device number
				if(!CDevMapFonts(tbuf,fontmap,pimp->fonts.count))
					{
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}
				if(ReplaceZeros)
					Replace0withO(tbuf);
				hbmps=NULL;
				nhbmps=m->cdev.nbsm;
				if(nhbmps)
					{
					hbmps=new HBITMAP [nhbmps];
					for(i=0;i<nhbmps;i++)
						{
						hbmps[i]=IMPCompileRaster(m->cdev.bsm[i],
							&pimp->vars,pvals,&pimp->gvars,&pimp->gvals,pimp->pheader,&pimp->udhas,
							&bfi,&xres,&yres,&ms,&neg,TRUE);
						if(!hbmps[i]) // ERROR
							{
							IMSTEPFree(imsteps,1),imsteps=NULL;
							return NULL;
							}
						}
					}

				// Compute cw and ch given the bfi and res
				cw=IMGetCW(hij,&ijfmt,m->cdev.xres);
				ch=IMGetCH(hij,m->cdev.bfi,m->cdev.yres);
				if(cw==0.0f || ch==0.0f)
					{
					IERROR;
					ShowError(0);
					IMSTEPFree(imsteps,1),imsteps=NULL;
					return NULL;
					}

				// Justify
				IMTextExtent(&extent,hij,&ijfmt,cw,0,ch,tbuf,
					m->cdev.bfi,nhbmps,hbmps);
				JustifyOrigin(m->justx,m->justy,&extent,&morigin);

				AppendRFScale(imsteps,xymode,&morigin,&rade);
				IMSTEPAppend(imsteps,IMTEXTSTEPNew(&morigin,m->zflight,partdia,
					&ijfmt,cw,0,ch,m->rep,tbuf,m->cdev.bfi,nhbmps,hbmps,c));
				zlast=m->zflight;
				
				cmc++;
				delete [] hbmps;
				break;
			default:
				IMSTEPFree(imsteps,1),imsteps=NULL;
				return NULL;
			}
		IMSTEPAppend(imsteps,IMPOPSTEPNew()); // Pop mark rotslant
		IMSTEPAppend(imsteps,IMPOPSTEPNew()); // Pop mark origin
		}

	// Error if no compiled marks
	if(cmc==0)
		{
		ShowError(IDE_ALLMARKSBLANK);
		IMSTEPFree(imsteps,1),imsteps=NULL;
		return NULL;
		}

	// Error if cure and curesteps found but the cure dev does not exist
	if(cure && !IMCureExists())
		{
		ShowError(IDE_NOCURE);
		IMSTEPFree(imsteps,1),imsteps=NULL;
		return NULL;
		}

	IMSTEPAppend(imsteps,IMPOPSTEPNew()); // Pop part rotation
	IMSTEPAppend(imsteps,IMPOPSTEPNew()); // Pop origin

return imsteps;
	}

/******************************************************************************
	i = FindCharInSet(c,LPSTR set)

	Finds the set Index for character c.

	int i = index or -1 if not found
	char c = character to search set for.
	LPSTR set = set to search
******************************************************************************/
static int FindCharInSet(char c,LPSTR set)
	{
	int i;
	i=lstrlen(set);
	while(i--)
		if(c==set[i])
			break;
	return i;
	}

/******************************************************************************
	ss = vnRead(s,showerr)

	Reads variable name into vnbuf and its length into vnlen.

	LPSTR ss = Updated position after read or NULL on error.
	LPSTR s = String pointer to start reading at.
	BOOL showerr= TRUE to enable display of error messages.
*******************************************************************************/
static LPSTR vnRead(LPSTR s,BOOL showerr)
	{
	char c;

	vnlen=0;

	while(1)
		{
		c=*s;
		switch(c)
			{
			case 0: // end
				if(showerr)
					ShowError(IDE_VNEARLYEND);
				return NULL;
			case '%': // End of name marker
				s++;
				break;
			default:
				if(vnlen==IMPLVNAME) // If vnbuf full
					{
					if(showerr)
						ShowError(IDE_VNFULL);
					return NULL;
					}
				s++;
				vnbuf[vnlen++]=c;
				continue;
			}
		break;
		}
	vnbuf[vnlen]=0;
	return s;
	}


/******************************************************************************
	i = vnFind(vars)

	Finds the index of the variable named by vnbuf.

	int i = index of found variable or -1 if not found.
******************************************************************************/
static int vnFind(IMPVARS *vars)
	{
	int i;

	// Search for a var matching vnbuf
	i=0;
	while(i<vars->count)
		{
		if(!lstrcmp(vnbuf,vars->var[i].name))
			break;
		i++;
		}

	// If variable not found
	if(i==vars->count)
		return -1;

	// Return index
	return i;
	}

/******************************************************************************
	i = haFind(pudh)

	Finds the index of header attribute named by vnbuf.

	int i = index of found variable or -1 if not found.
	IMPUDHAS *pudh = User defined header
******************************************************************************/
static int haFind(IMPUDHAS *pudh)
	{
	int i;

	// Search for a ha matching basic header.
	for(i=0;i<IMPNBHAS;i++)
		if(!lstrcmp(&vnbuf[1],IMPBHANames[i]))
			return i;

	// Search for a ha matching the udhas
	for(i=0;i<pudh->count;i++)
		if(!lstrcmp(&vnbuf[1],pudh->udha[i].name))
			return i+IMPNBHAS;

	return -1; // Not found
	}

/******************************************************************************
	len = vnSysExpand(buf)

	Expands the system variable named by vnbuf to buf and returns
	the length of the expansion or 0 if not found or error.

	Supported system vars:
		@DATE = current date in default format.
		@TIME = current time in default format.
		@USERID = current user ID
		@USERNAME = current user name
		@CYCLESTATUS = current CycleFailed ? "Fail" : "Pass"
		@LQTY = current .AUX label quantity (AuxLQty)

	int len = length of expanded system variable or 0 on error or not found
	LPSTR buf = buffer to store expansion to.
******************************************************************************/
static int vnSysExpand(LPSTR buf)
	{
	int len;

	len=0;
	if(!lstrcmp(vnbuf,"@DATE")) // Date in default format
		len=GetDate(buf,"MM/DD/YY",0,NULL);
	else if(!lstrcmp(vnbuf,"@TIME")) // Time in default format
		len=GetTime(buf,FALSE,FALSE,FALSE,':',NULL,NULL,FALSE,0,NULL);
	else if(!lstrcmp(vnbuf,"@USERID"))
		len=wsprintf(buf,"%s",(LPSTR)UserID);
	else if(!lstrcmp(vnbuf,"@USERNAME"))
		len=wsprintf(buf,"%s",(LPSTR)UserName);
	else if(!lstrcmp(vnbuf,"@LQTY"))
		len=wsprintf(buf,"%u",AuxLQty);
	else if(!lstrcmp(vnbuf,"@CYCLESTATUS"))
		len=wsprintf(buf,"%s",CycleFailed ? (LPSTR)"Fail" : (LPSTR)"Pass");
	else if(!lstrcmp(vnbuf,"@UIDFORMAT5"))
		len=wsprintf(buf,"[)>\x1e""05\x1d");
	else if(!lstrcmp(vnbuf,"@UIDFORMAT6"))
		len=wsprintf(buf,"[)>\x1e""06\x1d");
	else if(!lstrcmp(vnbuf,"@UIDSEPARATOR"))
		len=wsprintf(buf,"\x1d");
	else if(!lstrcmp(vnbuf,"@UIDEND"))
		len=wsprintf(buf,"\x04");
	return len;
	}

/******************************************************************************
	ph = IMPHEADERNew()

	Creates and initializes a new IMPHEADER.

	IMPHEADER *ph = pointer to new IMPHEADER or NULL on error.
******************************************************************************/
static IMPHEADER *IMPHEADERNew(void)
	{
	IMPHEADER *ph=new IMPHEADER;
	if(!ph)
		{
		ShowError(IDE_NOMEM);
		return NULL;
		}
	// Initialize
	ph->desc[0]=0;                              // desc=""
	memcpy(&(ph->author),UserName,IMPLAUTHOR); // author=UserName
	ph->author[IMPLAUTHOR]=0;
	ph->fixture[0]=0;                           // fixture=""
	ph->partSize.x=1.0;                         // partSize=(1,1,1)
	ph->partSize.y=1.0;
	ph->partSize.z=1.0;
    ph->partDiameter.di=3.0;       //sam
    ph->partDiameter.hi=3.0;       //sam
   //ph->partSize.x=3.0;       //sam
    
	// Default to currently selected inkjet.
	HIJ hij=SendMessage(IMWnd,IMM_LOCKIJ,1,0);
	SendMessage(IMWnd,IMM_LOCKIJ,0,0);
	SendMessage(IMWnd,IMM_GETIJNAME,hij,(LONG)(LPSTR)ph->inkjet);

	ph->origin.x=0.0;
	ph->origin.y=0.0;
	ph->rotation=0.0;
	ph->degree=0.0;	//Degree	-- 10/2016
	ph->inconlink=FALSE;
	ph->initsect[0]=0;
	ph->link[0]=0;

	ph->inspector[0]=0;

	ph->inspectpos.x=0.0;
	ph->inspectpos.y=0.0;
	ph->inspectpos.z=1.0;

	ph->inspectprep[0]=0;

	ph->conveyorstart=FALSE;

	ph->linkautostart=FALSE;

	ph->skiponlinkfail=FALSE;
	ph->chainlinkfail=FALSE;

	ph->reader[0]=0;

	ph->readpos.x=0.0;
	ph->readpos.y=0.0;
	ph->readpos.z=1.0;

	ph->readprep[0]=0;

	ph->readvar[0]=0;

	// Unlock
	return ph;
	}

/******************************************************************************
	err = IMPUDHASNew(pu)

	Creates and initializes a new IMPUDHAS.  The initial set of UDHAs comes
	from the [UDHA] section of the ini file.

    UINT err = Error code if failed
	IMPUDHAS & pu = New initialized IMPUDHAS.
******************************************************************************/
static UINT IMPUDHASNew(IMPUDHAS & pu)
	{
	
	int count,i,j;
	char key[3];

	// Count UDHAs in INI file
	count=0;
	for(i=0;i<IMPMAXUDHAS;i++)
		{
		wsprintf(key,"%d",i+1);
		tlen=GetPrivateProfileString("UDHA",key,"",tbuf,TSIZE,IMSIni);
		if(tlen<1)
			continue;
		count++;
		}

	// Allocate the UDHAS
	pu=IMPUDHAS(count); // zzz - Constructor
	if(!pu.udha && count>0)
		{
		return IDE_NOMEM;
		}

	// Initialize
//	pu->count=count;
	// reread the ini (its sitting in the cache anyway)
	j=0;
	for(i=0;i<IMPMAXUDHAS;i++)
		{
		wsprintf(key,"%d",i+1);
		// reread the ini (its sitting in the cache anyway)
		tlen=GetPrivateProfileString("UDHA",key,"",
			tbuf,IMPLUDHANAME+1,IMSIni);
		if(tlen<1)
			continue;
		lstrcpy((LPSTR)&(pu.udha[j].name),tbuf[0]==':'
			? (LPSTR)&tbuf[1] : (LPSTR)&tbuf[0]);
		pu.udha[j].redit=tbuf[0]==':';
		pu.udha[j].val[0]=0;
		j++;
		}
	return 0;
	}

/******************************************************************************
	pv = IMPVALSNew()

	Creates and initializes a new IMPVALS.

	IMPVALS *pv = Pointer to new IMPVALS or NULL on error.
******************************************************************************/
/*
static IMPVALS *IMPVALSNew(void)
	{
	IMPVALS *pv=new IMPVALS; // for 0 vals
	if(!pv)
		{
		ShowError(IDE_NOMEM);
		return 0;
		}
	// Initialize
	pv->count=0;
	return pv;
	}
*/
/******************************************************************************
	pm = IMPMARKSNew()

	Creates and initializes a new IMPMARKS.

	IMPMARKS *pm = Pointer to new IMPMARKS or NULL on error.
******************************************************************************/
/*
static IMPMARKS *IMPMARKSNew(void)
	{
	IMPMARKS *pm=new IMPMARKS; // for 0 marks
	if(!pm)
		{
		ShowError(IDE_NOMEM);
		return 0;
		}
	pm->count=0;
	return pm;
	}
*/
/******************************************************************************
  pf = IMPFONTSNew()

	Creates and initializes a new IMPFONTS.

	IMPFONTS *pf = Pointer to new IMPFONTS or NULL on error.
******************************************************************************/
/*
static IMPFONTS *IMPFONTSNew(void)
	{
	IMPFONTS *pf=new IMPFONTS; // for 0 fonts
	if(!pf)
		{
		ShowError(IDE_NOMEM);
		return 0;
		}
	pf->count=0;
	return pf;
	}

 */
/******************************************************************************
	pv = IMPVARSNew()

	Creates and initializes a new IMPVARS.

	IMPVARS *pv = Pointer to new IMPVARS or NULL on error.
******************************************************************************/
/*
static IMPVARS *IMPVARSNew(void)
	{
	IMPVARS *pv=new IMPVARS; // for 0 vars
	if(!pv)
		{
		ShowError(IDE_NOMEM);
		return 0;
		}
	pv->count=0;
	return pv;
	}

*/
/******************************************************************************
	ok = LoadNativeIMP(fh,pimp,ver)

	NO LONGER USED!!!!!

	Loads the native IMP file into the IMP structure.
	IMPLoad should free all allocations if not ok

	HANDLE fh = File handle of open file.
	PIMP pimp = IMP structure to load.
	BOOL ok = TRUE on success.
	int ver = 0 or an older version number (343 347 350 355 364)
******************************************************************************/
/*
static BOOL LoadNativeIMP(HANDLE fh,PIMP pimp,int ver)
	{
	WORD count;
	WORD rlen;      // Read length

	// Construct header
	pimp->pheader=new IMPHEADER; // Allocate it
	if(!pimp->pheader)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}
//	_lread32(fh,tbuf,4);
	rlen=sizeof(IMPHEADER);
	// if ver>0 && ver<=350 then load the old style header into
	// the front of the block and set defaults for the new members
	if(ver>0 && ver<=350)
		rlen-=3+IMPLFNBASE+sizeof(P3)+IMPLVISPREP+IMPLVNAME;
	if(_lread32(fh,(LPSTR)pimp->pheader,rlen)!=rlen) // read it
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	if(ver>0 && ver<=350) // Fill in defaults for members added after 350
		{
		pimp->pheader->reader[0]=0;
		pimp->pheader->readpos.x=0.0f;
		pimp->pheader->readpos.y=0.0f;
		pimp->pheader->readpos.z=1.0f;
		pimp->pheader->readprep[0]=0;
		pimp->pheader->readvar[0]=0;
		}

	// Construct udhas
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read the udhaCount
		{
		IDError=IDE_READFILE;
		return FALSE;
		}

	if(count>IMPMAXVARS)              // Check range
		{
		IDError=IDE_READFILE;
		return FALSE;
		}

	pimp->udhas=IMPUDHAS(count); // Allocate pudhas
	if(!pimp->udhas.udha && count>0)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}

	rlen = count*sizeof(IMPUDHA);
	if(_lread32(fh,(LPSTR)pimp->udhas.udha,rlen)!=rlen) // Read the udha list
		{
		IDError=IDE_READFILE;
		return FALSE;
		}

	pimp->udhas.count = count;

	// Construct vars
	if(_lread32(fh,(LPSTR)&count,2)!=2)    // read the count
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	if(count>IMPMAXVARS)               // Range check
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	pimp->vars= IMPVARS(count);    // Allocate it
	if(!pimp->vars.var && count > 0)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}

	// Initialize it
	if(ver>0 && ver<=355) // 355 and before had no preval per var
		{
		
		IMPV355VAR oldvar;
		rlen=sizeof(IMPV355VAR);
		for(int vi=0;vi<count;vi++)
			{
			if(_lread32(fh,(LPSTR)&oldvar,rlen)!=rlen)
				{
				IDError=IDE_READFILE;
				return FALSE;
				}
			V355VarToV364(&pimp->vars.var[vi],&oldvar);
			}
			
		}
	else // Read native >=364 vars
		{
		rlen=count*sizeof(IMPVAR);            // Read it
		if(_lread32(fh,(LPSTR)pimp->vars.var,rlen)!=rlen)
			{
			IDError=IDE_READFILE;
			return FALSE;
			}
		}

	pimp->vars.count=count;

	// Construct gvars
	if(_lread32(fh,(LPSTR)&count,2)!=2)    // read the count
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	if(count+pimp->vars.count>IMPMAXVARS)               // Range check
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	pimp->gvars=IMPVARS(count);           // Allocate it
	if(!pimp->gvars.var && count>0)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}
	                // Initialize it
	if(ver>0 && ver<=355) // 355 and before had no preval per var
		{
		
		IMPV355VAR oldvar;
		rlen=sizeof(IMPV355VAR);
		for(int vi=0;vi<count;vi++)
			{
			if(_lread32(fh,(LPSTR)&oldvar,rlen)!=rlen)
				{
				IDError=IDE_READFILE;
				return FALSE;
				}
			V355VarToV364(&pimp->gvars.var[vi],&oldvar);
			}
			
		}
	else // Read native >=364 gvars
		{
		rlen=count*sizeof(IMPVAR);            // Read it
		if(_lread32(fh,(LPSTR)pimp->gvars.var,rlen)!=rlen)
			{
			IDError=IDE_READFILE;
			return FALSE;
			}
		}

	pimp->gvars.count=count; 

	// Construct marks
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read count
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	if((count>IMPMAXMARKS)||(count < 1)) // Range check
		{
		IDError=IDE_READFILE;
		return FALSE;
		}

	pimp->marks=IMPMARKS(count);  // Allocate it
	if(!pimp->marks.mark && count>0)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}
	pimp->marks.count=count;                     // Initialize it
	rlen=count*sizeof(IMPMARK);              // Read it
	if(_lread32(fh,(LPSTR)(pimp->marks.mark),rlen)!=rlen)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}

	if(ver==343 || ver==347)
		for(int i=0;i<count;i++)
			if(pimp->marks.mark[i].type==mtWINFONT)
				{
				pimp->marks.mark[i].winfont.sef=0;
				pimp->marks.mark[i].winfont.yincm=0;
				pimp->marks.mark[i].winfont.yorgm=0;
				}

	if(ver==0||ver>355) // Load and link CDEV child marks
		{
		BSMFIXUPS fixups(&pimp->marks); // builds identity index map
		if(!fixups.readbsms(fh,ver))  // alloc and read bsms,update fixups
			{
			IDError=IDE_READFILE;
			return FALSE;
			}
		fixups.topointer();       // Fixup the parents BSM pointers
		}

	// Construct fonts
	if(_lread32(fh,(LPSTR)&count,2)!=2) // Read count
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	if(count>IMPMAXFONTS)             // Range check it
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	pimp->fonts=IMPFONTS(count); // Allocate it
	if(!pimp->fonts.name && count>0)
		{
		IDError=IDE_NOMEM;
		return FALSE;
		}
	
	// Initialize it
	rlen=count*sizeof(IJFONTNAME);       // Read it
	if(_lread32(fh,(LPSTR)pimp->fonts.name,rlen)!=rlen)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}

	pimp->fonts.count=count; 

	// Check for Alignment Extension
	if(AESETUP::readsig(fh))
		{
		// Allocate aesetup and load it
		pimp->aesetup=new AESETUP();
		if(!pimp->aesetup)
			{
			IDError=IDE_NOMEM;
			return FALSE;
			}
		if(!pimp->aesetup->load(fh))
			return FALSE;
		}

	// read optional imcorrs
	if(!pimp->imcorrs.read(fh))
		return FALSE;

	return TRUE; // success
	}

*/

/******************************************************************************
	MarkDecGreaterFonts(m,idf)

	Decrements the font id of the mark if it is greater than idf.  This is
	used by IMPPurgeFonts to adjust the font ids when it deletes fonts.

	IMPMARK *m = mark to decrement font in.
	int idf = font ids must be greater than idf or they stay the same.
******************************************************************************/
static void MarkDecGreaterFonts(IMPMARK *m,int idf)
	{
	switch(m->type)
		{
		case mtTEXT:
			if(m->t.ijfmt.font>idf)
				m->t.ijfmt.font--;
			break;
		case mtDEVBC:
			if(m->devbc.ijfmt.font>idf)
				m->devbc.ijfmt.font--;
			break;
		case mtDEVSYM:
			if(m->devsym.ijfmt.font>idf)
				m->devsym.ijfmt.font--;
			break;
		case mtMULTILINE:
			if(m->ml.ijfmt.font>idf)
				m->ml.ijfmt.font--;
			break;
		case mtCDEV:
			CDevMarkDecGreaterFonts(m,idf);
			break;
		}
	}

/******************************************************************************
	uses = MarkUsesFont(m,idf)

	Returns true if mark m uses font idf.  May be true for multiple idf.

	BOOL uses = True if m refrences idf.
	int idf = Font id to check use of.
	IMPMARK *m = mark to look for idf in.
******************************************************************************/
static BOOL MarkUsesFont(IMPMARK *m,int idf)
	{
	switch(m->type)
		{
		case mtTEXT:
			return (m->t.ijfmt.font==idf);
		case mtDEVBC:
			return (m->devbc.ijfmt.font==idf);
		case mtDEVSYM:
			return (m->devsym.ijfmt.font==idf);
		case mtMULTILINE:
			return (m->ml.ijfmt.font==idf);
		case mtCDEV:
			return CDevMarkUsesFont(m,idf);
		}
	return FALSE;
	}

/******************************************************************************
	ret = ContainsOnlySpaces(s)

	Returns true if the string s contains only spaces.

	BOOL ret = True if s contains only spaces.
	LPSTR s = String to test.
******************************************************************************/
static BOOL ContainsOnlySpaces(LPSTR s)
	{
	int i;
	i=lstrlen(s);
	while(i--)
		if(s[i]!=' ')
			break;
	return (i<0);
	}

/******************************************************************************
	JustifyOrigin(justx,justy,ext,jorg)

	Modifies an origin to be a justified origin given the 
	justification,mark origin and extent in mark space.

	IMPMARKJUSTX justx = X Justification type.
	IMPMARKJUSTY justy = Y Justification type.
	P2 *ext = Extent of the mark.
	P2 *jorg = origin to be justified.
******************************************************************************/
static void JustifyOrigin(BYTE justx,BYTE justy,
P2 *ext,P2 *jorg)
	{
	switch(justx)
		{
		case mjxCENTER:
			jorg->x-=ext->x/2.0;
			break;
		case mjxRIGHT:
			jorg->x-=ext->x;
		}
	switch(justy)
		{
		case mjyMIDDLE:
			jorg->y-=ext->y/2.0;
			break;
		case mjyBOTTOM:
			jorg->y-=ext->y;
		}
	}

/******************************************************************************
	Replace0withO(s)

	Replaces all occurances of '0' in a string with 'O'.
	This function is called for TEXT and MULTILINE mark texts if ReplaceZeros
	is set in the IMS5000.INI file.  Its purpose is to elliminate the slash
	in the zero.

	char *s = string to modify.
******************************************************************************/
static void Replace0withO(char *s)
	{
	while(*s)
		{
		if(*s=='0')
			*s='O';
		s++;
		}
	}

/******************************************************************************
	bi = FindBasesIndex(name)

	Finds the index of a base in the bases array if it exists.

	bi = bases index or -1 if not found.
	LPSTR name = base name to search for.
******************************************************************************/
static int FindBasesIndex(LPSTR name)
 	{
	int bi;
	for(bi=0;bi<nbases;bi++)
		if(!lstrcmp(bases[bi].name,name))
			return bi;
	return -1;
	}

/******************************************************************************
	AppendRFScale(imsteps,xymode,origin,rade)

	Appens an RFSCALE step to imsteps if xymode is imXS or imSY.
		Computes the scale based on origin and t,xf,yf
******************************************************************************/
static void AppendRFScale(PIMSTEP imsteps,IMXYMODE xymode,P2 *origin,
	RADE *rade)
	{
	double r,scale;
	if(xymode==imXS||xymode==imSY)
		{
		r=rade->t+rade->xf*origin->x+rade->yf*origin->y;
		scale=r==0.0 ? 1.0 : 1.0/(PIX2*r);
		IMSTEPAppend(imsteps,IMSETRFSCALESTEPNew(scale));
		}
	}

/******************************************************************************
	BuildRADE(rade,xymode,pudhas)
	Builds the radius expression by compiling the contents of UDHA "Radius"
	If no "Radius" expression is defined then the 0,0,0 RADE is used.
******************************************************************************/
static void BuildRADE(RADE *rade,IMXYMODE xymode,IMPUDHAS *pudhas)
	{
	IMPUDHA *pudha;
	LPSTR p; // parsing pointer
	int i;

	rade->t=0.0f; // Init rade to default
	rade->xf=0.0f;
	rade->yf=0.0f;

	if(xymode==imXS||xymode==imSY)
		{
		for(i=0;i<pudhas->count;i++)
			{
			pudha=&(pudhas->udha[i]);
			if(!lstrcmp(pudha->name,"Radius"))
				break;
			}
		if(i<pudhas->count)
			{
			p=pudha->val;
			while(*p)
				{
				if(!memcmp(p,"-X",2))
					{
					rade->xf=-1.0f;
					p+=2;
					continue;
					}
				if(!memcmp(p,"-Y",2))
					{
					rade->yf=-1.0f;
					p+=2;
					continue;
					}
				if(!memcmp(p,"+X",2))
					{
					rade->xf=1.0f;
					p+=2;
					continue;
					}
				if(p==pudha->val && *p=='X')
					{
					rade->xf=1.0f;
					p++;
					continue;
					}
				if(!memcmp(p,"+Y",2))
					{
					rade->yf=1.0f;
					p+=2;
					continue;
					}
				if(p==pudha->val && *p=='Y')
					{
					rade->yf=1.0f;
					p++;
					continue;
					}
				p=ParseDouble(p,&rade->t);
				if(!p)
					break;
				}
			}
		}
	}

static LPM3 M3RotSlant(LPM3 ans,double arot,double aslant)
    {
	*ans = M3(IDENTITY);
//    M3Identity(ans);
    ans->m3[0][0]=cos(arot);
    ans->m3[0][1]=sin(arot);
    ans->m3[1][0]=-sin(arot+aslant);
    ans->m3[1][1]=cos(arot+aslant);
    return ans;
    }

// Check if the bar code (C39 or C93) has all valid characters
static BOOL ValidateCodeChars(LPSTR data,IMPMARKTYPE ty){
	switch (ty){
	case mtC39:
	case mtDEVBC:
		{
		int len = lstrlen(data);
		for (int i=0;i<len;i++)
			{
				char c = data[i];
				if (!(c>='A' && c<='Z') && !(c>='0' && c<='9') && c!=' ' && 
					c!='-' && c!='.' && c!='$' && c!='/' && c!='+' && c!= '%' )
					return FALSE;		
			}
		return TRUE;
		}
		break;
	case mtC93:
		return TRUE;
	default:
		return FALSE;
	}
	
}

/******************************************************************************
	V355VartoV364
	Converts a V355 var to 364 format. Called by IMP LoadNative in 364 to
	translate old vars without preval to new format
******************************************************************************/
/*
static void V355VarToV364(IMPVAR *dst,IMPV355VAR *src)
	{
	memcpy(dst->name,src->name,IMPLVNAME+1);
	dst->width=src->width;
	dst->lpad=src->lpad;
	dst->type=src->type;
	dst->unique=src->unique;
	switch(src->type)
		{
		case vtKEYED:  dst->k=src->k; break;
		case vtSERIAL: dst->s=src->s; break;
		case vtDATE:   dst->d=src->d; break;
		case vtTIME:   dst->t=src->t; break;
		case vtBASED:  dst->b=src->b;
		};
	IMPInitVal(&dst->preval,dst);
	}
*/