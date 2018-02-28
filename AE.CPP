/******************************************************************************
	Name : AE
	Desc : Alignment Extensions for IMS5000
******************************************************************************/

#include "stdafx.h"
#include "imsio.h"
#include "dirent.h"

#define WMU_NEWPREP1 WM_APP+1
#define WMU_NEWPREP2 WM_APP+2

HWND AEWnd=0; // Set by AESETUP on openedit to its hwnd
              // Cleared by editors when they close
              // Allows app to track single window open state
              // and to relay im notifications.

static BOOL AEWinPosLocked=FALSE;
static char dbuf[512];

AESETUP::AESETUP()
	{
	memset(this,0,sizeof(AESETUP));
	maxrotation=360.0f;
	maxlendelta=0.1f;
	nrefpos=0;	//nrefpos=1;
	nacqpos=1;
	prepsteps=0;
	}

AESETUP::~AESETUP()
	{
//	if(editdp)
//		FreeProcInstance(editdp);
	if(prepsteps)
		IMSTEPFree(prepsteps,FALSE);
	}

PIMSTEP AESETUP::compileprep(PIMP pimp,IMPVALS *pvals,LPSTR preptext)
	{
	// Expand locateprep to tbuf
	if(nesti>=0) // If per part
		{
		if(!IMPExpandTextToTbuf(preptext,&pimp->vars,pvals,&pimp->gvars,
			&pimp->gvals,pimp->pheader,&pimp->udhas))
			return 0;
		}
	else
		lstrcpy(tbuf,preptext); // Fixture aligns get no prep expansion
	return IMPREPLOCATORSTEPNew(tbuf);
	}

PIMSTEP AESETUP::compile(PIMF pimf,int ni,IMPVALS *pvals)
	{
	// Prep locator
	PIMSTEP steps=NULL;
	steps=compileprep(&pimf->imp,pvals,locateprep[0]);
		if(!steps)
			{
			ShowError(IDE_BADALIGNPREP);
			return NULL;
			}
		if(DebugMode){
			OutputDebugString("AESETUP::compile() nacqpos=");
			sprintf(dbuf,"%d imf extent.z:%0.4f",nacqpos,pimf->extent.z);
			OutputDebugString(dbuf);
			if(pimf->imp.pheader)
				sprintf(dbuf," imp partsize.z:%0.4f\n",pimf->imp.pheader->partSize.z);
			else
				lstrcpy(dbuf,"\n");
			OutputDebugString(dbuf);
			}
	for(int ai=0;ai<nacqpos;ai++) // 0 or more acquires
		{                         // with interleaved locates
		zvis=pimf->extent.z+acqpos[ai].z;
		if(ni>=0) // Per part
			zvis+=pimf->imp.pheader->partSize.z;
		if(DebugMode){
			sprintf(dbuf,"AESETUP::compile() append step zvis=%0.4f\n",zvis);
			OutputDebugString(dbuf);
			sprintf(dbuf," acqpos ={%0.4f,%0.4f}\n",acqpos[ai].x,acqpos[ai].y);
			OutputDebugString(dbuf);
			}
		if (ai>0){// change to new locator and prep string if needed				//...d.dixon 11/29/13
			IMSTEPAppend(steps,IMSELLOCATORSTEPNew(locator[ai]));
			IMSTEPAppend(steps,IMMSGSTEPNew("LOCATING"));
			IMSTEPAppend(steps,IMPREPLOCATORSTEPNew(locateprep[ai]));
			}
		IMSTEPAppend(steps,IMXYMOVESTEPNew((LPP2)&acqpos[ai],zvis,xymcsLOCATOR));
		IMSTEPAppend(steps,IMLOCATESTEPNew(ai,ni));
		}//end for loop....

	if(nacqpos==0)                // or a signel positionless locate
		IMSTEPAppend(steps,IMLOCATESTEPNew(-1,ni));
	return steps;
	}

// rxm=getxm(xmt,xmr,info,ni)
// rxm = xm | NULL on error
LPM3 AESETUP::getxm(LPM3 xmt,LPM3 xmr,AEINFO *info,int ni)
	{
	switch(nrefpos)
		{
		case 0:
			{
			return xmt;	//No ref is used ....3/17/14...ddd
			}
		case 1:
			{
			M3 m1(IDENTITY,0,0,0);
			*xmr=m1;
			return getxm1pt(xmt,info,ni);
			}
		case 2:
			{
			int xmi=GetPrivateProfileInt(IMSName,"GetXM",2,IMSIni);
			switch(xmi)
				{
				case 2:
					return getxm2pt_2(xmt,xmr,info,ni);
				case 1:
					return getxm2pt_1(xmt,xmr,info,ni);
				default:
					return getxm2pt_2(xmt,xmr,info,ni);
				}
			}
		}
	IERROR;
	ShowError(0);
	return NULL;
	}

// create 1pt alignemnt matrix
// assumes nrefpos==1
LPM3 AESETUP::getxm1pt(LPM3 xm,AEINFO *info,int ni)
//new
	{
	P3 ar; // reference pont
	P3 al; // located point

	ar=refpos[0];
	al=info->pos[ni];
	// xm <-- t(al-ar)
	M3 tempxm(TRANSLATE,(al.x-ar.x),(al.y-ar.y),0);//M3Translate(xm,al.x-ar.x,al.y-ar.y);
	*xm=tempxm;
	if(DebugMode)
		{
		OutputDebugString("AESETUP::getxm1pt()\n  refpos=");
		sprintf(dbuf,"{%0.4f,%0.4f,%0.4f}\n",ar.x,ar.y,ar.z);
		OutputDebugString(dbuf);
		OutputDebugString("  locpos=");
		sprintf(dbuf,"{%0.4f,%0.4f,%0.4f}\n",al.x,al.y,al.z);
		OutputDebugString(dbuf);
		OutputDebugString("      xm=");
		DebugMatrix(xm);
		}
	return xm;
	}

// create 2 point alignment matrix
// assumes nrefpos==2
LPM3 AESETUP::getxm2pt_2(LPM3 xmt,LPM3 xmr,AEINFO *info,int ni)
	{
	P3 refPosA,refPosB; // reference points a and b
	P3 locPosA,locPosB; // located points a and b

	refPosA=refpos[0];
	refPosB=refpos[1];
	locPosA=info->pos[(ni<<1)];
	locPosB=info->pos[(ni<<1)+1];
	if(adjusta) // Swap A and B if adjusta
		{
		P3 t;
		t=refPosA;refPosA=refPosB;refPosB=t;
		t=locPosA;locPosA=locPosB;locPosB=t;
		}

	V2 refVector;             // reference vector
	refVector.v2[0]=refPosB.x-refPosA.x;
	refVector.v2[1]=refPosB.y-refPosA.y;
	V2 locVector;             // located vector
	locVector.v2[0]=locPosB.x-locPosA.x;
	locVector.v2[1]=locPosB.y-locPosA.y;

	// get the length of reference vector
	double refVectorLen=sqrt(refVector.v2[0]*refVector.v2[0]+refVector.v2[1]*refVector.v2[1]);
	// get the length of located vector
	double locVectorLen=sqrt(locVector.v2[0]*locVector.v2[0]+locVector.v2[1]*locVector.v2[1]);

	// get the absolute vector length value
	double lendelta=fabs(refVectorLen-locVectorLen);
	// make sure it's within tolerance
	if(lendelta>maxlendelta)
		{
		ShowError(IDE_AELENDELTA);
		return NULL;
		}
	if(refVectorLen==0.0f || locVectorLen==0.0f)
		{
		ShowError(IDE_AE0DELTA);
		return NULL;
		}

	// get the angle of each vector
	double refAngle=(double)atan2(refVector.v2[0],refVector.v2[1])/DEG;
	double locAngle=(double)atan2(locVector.v2[0],locVector.v2[1])/DEG;

	// determine absolute delta rotation
	double rotation=fabs(locAngle-refAngle);
	if(rotation>maxrotation)
		{
		ShowError(IDE_AEROTDELTA);
		return NULL;
		}
	// get the delta rotation including direction
	rotation=-1*(locAngle-refAngle);

	if(DebugMode)
		{
		sprintf(dbuf,"AESETUP::getxm2pt_2() rotation=%0.4f lendelta=%0.4f\n",rotation,lendelta);
		OutputDebugString(dbuf);

		sprintf(dbuf,"  refpos {%0.4f,%0.4f,%0.4f} {%0.4f,%0.4f,%0.4f}\n",refpos[0].x,refpos[0].y,refpos[0].z,refpos[1].x,refpos[1].y,refpos[1].z);
		OutputDebugString(dbuf);
		sprintf(dbuf,"  locpos {%0.4f,%0.4f,%0.4f} {%0.4f,%0.4f,%0.4f}\n",info->pos[0].x,info->pos[0].y,info->pos[0].z,info->pos[1].x,info->pos[1].y,info->pos[1].z);
		OutputDebugString(dbuf);
		}

	// create rotation matrix
	M3 m1(ROTATE,0,0,rotation*DEG);

	if(DebugMode)
		{
		double sinrot=(double)sin(rotation*DEG);
		double cosrot=(double)cos(rotation*DEG);
		sprintf(dbuf,"  \n ROTATION MATRIX\n  angle=%0.4f\n    sin=%0.4f\n    cos=%0.4f\n",rotation,sinrot,cosrot);
		OutputDebugString(dbuf);
		OutputDebugString("  m1 before mod=\n");
		DebugMatrix(&m1);
		OutputDebugString("\n");
		}

/*	if(m1.m3[0][0]!=0.0f)
		m1.m3[0][1]/=m1.m3[0][0];
	if(m1.m3[1][1]!=0.0f)
		m1.m3[1][0]/=m1.m3[1][1];
	m1.m3[0][0]=1.0f;
	m1.m3[1][1]=1.0f;*/
	/*
	m1.m3[2][0]=locPosA.x-refPosA.x;
	m1.m3[2][1]=locPosA.y-refPosA.y;

	*xm=m1;
*/
	*xmr=m1;

	// create translation matrix
	M3 m2(TRANSLATE,locPosA.x-refPosA.x,locPosA.y-refPosA.y,0);

	*xmt=m2;

	if(maskxrot)
		{
		(xmr->m3)[0][0]=1.0f;
		(xmr->m3)[0][1]=0.0f;
		(xmr->m3)[0][2]=0.0f;
		}
	if(maskyrot)
		{
		(xmr->m3)[1][0]=0.0f;
		(xmr->m3)[1][1]=1.0f;
		(xmr->m3)[1][2]=0.0f;
		}

	if(DebugMode)
		{
		OutputDebugString("  xmt=\n");
		DebugMatrix(xmt);
		OutputDebugString("  xmr=\n");
		DebugMatrix(xmr);
		}

	return xmt;
	}

// create 2 point alignment matrix
// assumes nrefpos==2
LPM3 AESETUP::getxm2pt_1(LPM3 xmt,LPM3 xmr,AEINFO *info,int ni)
	{
	P3 ar,br; // reference points a and b
	P3 al,bl; // located points a and b

	ar=refpos[0];
	br=refpos[1];
	al=info->pos[(ni<<1)];
	bl=info->pos[(ni<<1)+1];
	if(adjusta) // Swap A and B if adjusta
		{
		P3 t;
		t=ar;ar=br;br=t;
		t=al;al=bl;bl=t;
		}

	V2 abr;             // reference vector
	abr.v2[0]=br.x-ar.x;
	abr.v2[1]=br.y-ar.y;
	V2 abl;             // located vector
	abl.v2[0]=bl.x-al.x;
	abl.v2[1]=bl.y-al.y;

	double labr=sqrt(abr.v2[0]*abr.v2[0]+abr.v2[1]*abr.v2[1]); // length of reference vector
	double labl=sqrt(abl.v2[0]*abl.v2[0]+abl.v2[1]*abl.v2[1]); // length of located vector

	double lendelta=fabs(labr-labl);
	if(lendelta>maxlendelta)
		{
		ShowError(IDE_AELENDELTA);
		return NULL;
		}
	if(labr==0.0f || labl==0.0f)
		{
		ShowError(IDE_AE0DELTA);
		return NULL;
		}
	V2 dabr; // Direction of reference vector
	dabr.v2[0]=abr.v2[0]/labr;
	dabr.v2[1]=abr.v2[1]/labr;

	V2 dabl; // Direction of location vector
	dabl.v2[0]=abl.v2[0]/labl;
	dabl.v2[1]=abl.v2[1]/labl;

	V2 ydabr;//={dabr.v2[0],-dabr.v2[1]}; // y flipped direction of reference vector
	ydabr.v2[0]=dabr.v2[0];
	ydabr.v2[1]=-dabr.v2[1];
	M3 m1(TRANSLATE,-ar.x,-ar.y,0),m2,m3;  // translation matrix of negative reference vector

	//M3Translate(&m1,-ar.x,-ar.y);  // Trans -ar

	m2.m3[0][0]=ydabr.v2[0];               // Rotate so dabr is x
	m2.m3[0][1]=ydabr.v2[1];
	m2.m3[0][2]=0.0f;
	m2.m3[1][0]=-m2.m3[0][1];
	m2.m3[1][1]=m2.m3[0][0];
	m2.m3[1][2]=0.0f;
	m2.m3[2][0]=0.0f;
	m2.m3[2][1]=0.0f;
	m2.m3[2][2]=1.0f;

	m3.m3[0][0]=dabl.v2[0];               // Rotate x to dabl
	m3.m3[0][1]=dabl.v2[1];
	m3.m3[0][2]=0.0f;
	m3.m3[1][0]=-m3.m3[0][1];
	m3.m3[1][1]=m3.m3[0][0];
	m3.m3[1][2]=0.0f;
	m3.m3[2][0]=0.0f;
	m3.m3[2][1]=0.0f;
	m3.m3[2][2]=1.0f;

	M3 m4(TRANSLATE,al.x,al.y,0);
	//M3Translate(&m4,al.x,al.y);   // Translate to al

	// xm=m1*m2*m3*m4
	M3 m12,m123;
	m12=m1*m2;//M3Mul(&m12,&m1,&m2);
	m123=m12*m3;//M3Mul(&m123,&m12,&m3);
	*xmt=m123*m4;//M3Mul(xm,&m123,&m4);

	M3 m5(IDENTITY,0,0,0);
	*xmr=m5;
	
	// determine delta rotation
	double rotation=(double)atan2((xmt->m3)[0][1],(xmt->m3)[0][0])/DEG;
	if(rotation>maxrotation)
		{
		ShowError(IDE_AEROTDELTA);
		return NULL;
		}

	if(maskxrot)
		{
		(xmt->m3)[0][0]=1.0f;
		(xmt->m3)[0][1]=0.0f;
		(xmt->m3)[0][2]=0.0f;
		}
	if(maskyrot)
		{
		(xmt->m3)[1][0]=0.0f;
		(xmt->m3)[1][1]=1.0f;
		(xmt->m3)[1][2]=0.0f;
		}

	if(DebugMode)
		{
		sprintf(dbuf,"AESETUP::getxm2pt_1() rotation=%0.4f lendelta=%0.4f\n",rotation,lendelta);
		OutputDebugString(dbuf);

		sprintf(dbuf,"  refpos {%0.4f,%0.4f,%0.4f} {%0.4f,%0.4f,%0.4f}\n",refpos[0].x,refpos[0].y,refpos[0].z,refpos[1].x,refpos[1].y,refpos[1].z);
		OutputDebugString(dbuf);
		sprintf(dbuf,"  locpos {%0.4f,%0.4f,%0.4f} {%0.4f,%0.4f,%0.4f}\n",info->pos[0].x,info->pos[0].y,info->pos[0].z,info->pos[1].x,info->pos[1].y,info->pos[1].z);
		OutputDebugString(dbuf);

		OutputDebugString("  xmt=\n");
		sprintf(dbuf,"         {%0.4f,%0.4f,%0.4f}\n",xmt->m3[0][0],xmt->m3[0][1],xmt->m3[0][2]);
		OutputDebugString(dbuf);
		sprintf(dbuf,"         {%0.4f,%0.4f,%0.4f}\n",xmt->m3[1][0],xmt->m3[1][1],xmt->m3[1][2]);
		OutputDebugString(dbuf);
		sprintf(dbuf,"         {%0.4f,%0.4f,%0.4f}\n",xmt->m3[2][0],xmt->m3[2][1],xmt->m3[2][2]);
		OutputDebugString(dbuf);
		}
	return xmt;
	}

// ok=save(fh) : saves this on fh (does not close unless error)
BOOL AESETUP::save(HANDLE fh)
	{
	if(_lwrite32(fh,sigAE,LSIGAE)!=LSIGAE) // Write the signiture
		{
		CloseHandle(fh);
		ShowError(IDE_WRITEFILE);
		return FALSE;
		}

	//if(_lwrite32(fh,(LPSTR)this,sizeof(AEFILESETUP))!=sizeof(AEFILESETUP))
	//	{
	//	CloseHandle(fh);
	//	ShowError(IDE_WRITEFILE);
	//	return FALSE;
	//	}

	// Write the other data 

    // locators												//...d.dixon...11/25/13. protect legacy imp settings
	if (strlen(locator[1])){
		strcat(locator[0],"+");
		if (!writeString(fh,locator[0],IMPLFNBASE+1)){
			return FALSE;
			}													
		if (!writeString(fh,locator[1],IMPLFNBASE+1)){
			return FALSE;
			}													
		char * pch = strrchr(locator[0],'+');				// ok, now restore locator by
		if (pch != NULL){									// replacing the '+' char. with 'end of string' char in Locator[0]
			*pch ='\0';
			}							
		}
	else{
		if (!writeString(fh,locator[0],IMPLFNBASE+1)){
			return FALSE;
			}													
		}
	// Locate Preps
	if (strlen(locateprep[1])){								// Check for second locateprep
		strcat(locateprep[0],"+");
		if (!writeString(fh,locateprep[0],IMPLVISPREP+1)){	
			return FALSE;
			}
		if (!writeString(fh,locateprep[1],IMPLVISPREP+1)){	
			return FALSE;
			}
		char * pch = strrchr(locateprep[0],'+');			// ok, now restore locateprep by
		if (pch != NULL){									// replacing the '+' char. with 'end of string' char in Locateprep[0]
			*pch ='\0';
			}								
		}
	else{
		if (!writeString(fh,locateprep[0],IMPLVISPREP+1)){	//...d.dixon.... remember to check for legacy imp settings
			return FALSE;
			}
		}
	// Locate vision processtype
	if (strlen(processtype[1])){								// Check for second processtype
		strcat(processtype[0],"+");
		if (!writeString(fh,processtype[0],IMPLVISPREP+1)){	
			return FALSE;
			}
		if (!writeString(fh,processtype[1],IMPLVISPREP+1)){	
			return FALSE;
			}
		char * pch = strrchr(processtype[0],'+');			// ok, now restore processtype by
		if (pch != NULL){									// replacing the '+' char. with 'end of string' char in processtype[0]
			*pch ='\0';
			}								
		}
	else{
		if (!writeString(fh,processtype[0],IMPLVISPREP+1)){	//...d.dixon.... remember to check for legacy imp settings
			return FALSE;
			}
		}
	// refpos
	if (!writeDouble(fh,&refpos[0].x)){
		return FALSE;
	}
	if (!writeDouble(fh,&refpos[0].y)){
		return FALSE;
	}
	if (!writeDouble(fh,&refpos[0].z)){
		return FALSE;
	}
	if (!writeDouble(fh,&refpos[1].x)){
		return FALSE;
	}
	if (!writeDouble(fh,&refpos[1].y)){
		return FALSE;
	}
	if (!writeDouble(fh,&refpos[1].z)){
		return FALSE;
	}
	// nacqpos, nrefpos
	if (!writeByte(fh,(BYTE*)&nacqpos)){
		return FALSE;
	}
	if (!writeByte(fh,(BYTE*)&nrefpos)){
		return FALSE;
	}
	// acqpos
	if (!writeDouble(fh,&acqpos[0].x)){
		return FALSE;
	}
	if (!writeDouble(fh,&acqpos[0].y)){
		return FALSE;
	}
	if (!writeDouble(fh,&acqpos[0].z)){
		return FALSE;
	}
	if (!writeDouble(fh,&acqpos[1].x)){
		return FALSE;
	}
	if (!writeDouble(fh,&acqpos[1].y)){
		return FALSE;
	}
	if (!writeDouble(fh,&acqpos[1].z)){
		return FALSE;
	}
	// max rotation, max lendelta
	if (!writeDouble(fh,&maxrotation)){
		return FALSE;
	}
	if (!writeDouble(fh,&maxlendelta)){
		return FALSE;
	}
	// maskxrot, maskyrot, adjusta
	if (!writeInt(fh,&maskxrot)){
		return FALSE;
	}
	if (!writeInt(fh,&maskyrot)){
		return FALSE;
	}
	if (!writeInt(fh,&adjusta)){
		return FALSE;
	}

	return TRUE;
	}

// ok=load(fh) // assumes file pointer is just passed the sigAE
BOOL AESETUP::load(HANDLE fh,WORD ver)
	{
//	WORD rlen=sizeof(AEFILESETUP);
//	if(_lread32(fh,(LPSTR)this,rlen)!=rlen)
//		{
//		ShowError(IDE_READFILE);
//		return FALSE;
//		}

	if (ver==0){ // Win32
		//Get the first locator 
		if (!readString(fh,locator[0],IMPLFNBASE+1)){
			return FALSE;			
		}
		//check for an additional locator indicator
		char * pch = strrchr(locator[0],'+');			
		if (pch != NULL){
			*pch ='\0';										// replace the '+' char. with Null char	in locator[0]			
			if (!readString(fh,locator[1],IMPLFNBASE+1)){	// Now read in locator[1]
				return FALSE;
			}
		}
		//Get the first locator prep string
		if (!readString(fh,locateprep[0],IMPLVISPREP+1)){	//...d.dixon.... remember to check for legacy imp settings
			return FALSE;
		}
		//check for an additional prep string indicator
		pch = strrchr(locateprep[0],'+');
		if (pch != NULL){
			*pch ='\0';											// replace the '+' char. with Null char	in Locateprep[0]			
			if (!readString(fh,locateprep[1],IMPLVISPREP+1)){	// Now read in Locateprep[1]
				return FALSE;
			}
		}
		//Get the first processtype 
		if (!readString(fh,processtype[0],IMPLVISPREP+1)){
			return FALSE;			
		}
		//check for an additional processtype indicator
		pch = strrchr(processtype[0],'+');			
		if (pch != NULL){
			*pch ='\0';										// replace the '+' char. with Null char	in locator[0]			
			if (!readString(fh,processtype[1],IMPLVISPREP+1)){	// Now read in processtype[1]
				return FALSE;
			}
		}
		// refpos
		if (!readDouble(fh,&refpos[0].x)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[0].y)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[0].z)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[1].x)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[1].y)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[1].z)){
			return FALSE;
		}
		// nacqpos, nrefpos
		if (!readByte(fh,(BYTE*)&nacqpos)){
			return FALSE;
		}
		if (!readByte(fh,(BYTE*)&nrefpos)){
			return FALSE;
		}
		// acqpos
		if (!readDouble(fh,&acqpos[0].x)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[0].y)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[0].z)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[1].x)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[1].y)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[1].z)){
			return FALSE;
		}
		// max rotation, max lendelta
		if (!readDouble(fh,&maxrotation)){
			return FALSE;
		}
		if (!readDouble(fh,&maxlendelta)){
			return FALSE;
		}
		// maskxrot, maskyrot, adjusta
		if (!readInt(fh,&maskxrot)){
			return FALSE;
		}
		if (!readInt(fh,&maskyrot)){
			return FALSE;
		}
		if (!readInt(fh,&adjusta)){
			return FALSE;
		}
	} else {    // Win16 versions
		// locator and prep
		if (!readString(fh,locator[0],8+1)){
			return FALSE;
		}
		if (!readString(fh,locateprep[0],IMPLVISPREP+1)){	//...d.dixon.... remember to check for legacy imp settings....Fix!!
			return FALSE;
		}
		// refpos
		if (!readDouble(fh,&refpos[0].x,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[0].y,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[0].z,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[1].x,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[1].y,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&refpos[1].z,TRUE)){
			return FALSE;
		}
		// nacqpos, nrefpos
		if (!readByte(fh,(BYTE*)&nacqpos)){
			return FALSE;
		}
		if (!readByte(fh,(BYTE*)&nrefpos)){
			return FALSE;
		}
		// acqpos
		if (!readDouble(fh,&acqpos[0].x,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[0].y,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[0].z,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[1].x,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[1].y,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&acqpos[1].z,TRUE)){
			return FALSE;
		}
		// max rotation, max lendelta
		if (!readDouble(fh,&maxrotation,TRUE)){
			return FALSE;
		}
		if (!readDouble(fh,&maxlendelta,TRUE)){
			return FALSE;
		}
		// maskxrot, maskyrot, adjusta
		if (!readInt(fh,&maskxrot,TRUE)){
			return FALSE;
		}
		if (!readInt(fh,&maskyrot,TRUE)){
			return FALSE;
		}
		if (!readInt(fh,&adjusta,TRUE)){
			return FALSE;
		}
	}

	if(nrefpos>2){ // Not supported yet
		IERROR; // >2 not supported yet
		ShowError(0);
		return FALSE;
		}
	return TRUE;
	}

// ok = readsig(fh) : reads the sig and returns ok : or not
BOOL AESETUP::readsig(HANDLE fh,WORD & ver)
	{
	UINT cr;
	char sig[LSIGAE]="";
	cr=_lread32(fh,sig,LSIGAE);
	if(cr==HFILE_ERROR)
		return FALSE;
	if(memcmp(sig,sigAE,LSIGAE)==0){
		ver=0;
		return TRUE;
	} else if (memcmp(sig,sigAEV357,LSIGAEV357)==0){
		ver=357;
		return TRUE;
	}
	long sl=cr;
	sl*=-1;
	_llseek32(fh,sl,FILE_CURRENT);
	return FALSE;
	}

// Opens the editor dialog for this aesetup if not allready open
// if ni==-1 edit is FCS level instead of NCS level
// Modal!
HWND AESETUP::openedit(HWND parent,HWND *padwnd,PIMF _pimf,double pz,int ni)
	{
	if(!hwnd)
		{
		DLGPROC editdp=(DLGPROC)AESETUPDP;
		pADWnd=padwnd;
		hparent=parent;
		pimf=_pimf;
		ClampStart(&(pimf->imp),NULL);
		ClampLock();
		// Set all nests empty except ni for NCS or
		// 0 for FCS so that IMFCompileLocates compiles
		// for ni only in NCS and does not return for FCS
		for(int i=0;i<pimf->nests.count;i++)
			{
			IMFNEST *pnest=&(pimf->nests.nest[i]);
			pnest->flags=0;
			if(i==ni || (ni<0 && i==0))
				pnest->flags=nbfOCCUPIED;
			}
		// Compute frame zframe and rotation
		M3 fcs;
		GetFCS(&fcs,pimf,FALSE);
		M3 ncs(IDENTITY,0,0,0);
		if(ni>=0)
			{
			IMFNEST *pnest=&(pimf->nests.nest[ni]);
			GetNCS(&ncs,pimf,pnest,ni,FALSE);
			}
//zzz		else
			//M3Identity(&ncs);
		frame=ncs*fcs;//M3Mul(&frame,&ncs,&fcs);
		rotation=(double)atan2(-frame.m3[1][0],frame.m3[1][1])/DEG;
		nesti=ni;
		partz=pz;
		DialogBoxParam(IMSInst,"AESETUP",parent,editdp,(LPARAM)this);
	}
	return hwnd;
	
}
// Closes the editor dialog for this aesetup if its open
void AESETUP::closeedit(void)
	{
	if(hwnd)
		DestroyWindow(hwnd);
	hwnd=0;
	AEWnd=0;
	}

// AESETUP::AESETUPDP(...) Win level dialog proc for AESETUP
// STATIC
BOOL CALLBACK AESETUP::AESETUPDP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	if(msg==WM_INITDIALOG) // Save this pointer in properties on init
		{
		SetProp(hdlg,"THISH",(HANDLE)HIWORD(lP));
		SetProp(hdlg,"THISL",(HANDLE)LOWORD(lP));
		}
	HANDLE th=GetProp(hdlg,"THISH"); // Get this ponter from properties
	HANDLE tl=GetProp(hdlg,"THISL");
	AESETUP *t=(AESETUP *)MAKELONG(tl,th);
	if(!t)
		return FALSE;
	return t->DP(hdlg,msg,wP,lP); // Displatch to instance
	}

// DP(...) Instance level dialog proc for AESETUP
BOOL AESETUP::DP(HWND hdlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	char locbuf[128];
	switch(msg)
		{
		case WM_INITDIALOG:
			{
			AEWnd=hdlg;
			hwnd=hdlg;
			if(prepsteps)
				IMSTEPFree(prepsteps,FALSE);
			prepsteps=0;
			imok=IMState==IMN_READY || IMState==IMN_PRINTOFF || IMState==IMN_XYONLY || IMState==IMN_WARMUP;
			aeok=IMVisLive(locator[0],TRUE);
			if(aeok)// add second locator check ddd...fix!!
				aeok=IMVisIRTM(&irtm,locator[0],rotation);
			if(aeok)
				aeok=IMVisIRTZ(&irtz,locator[0]);
			injog=FALSE;

			// Turn on vision lamp
			VisionLampOn();

			// Load Locator combobox(cb)								// .... d.dixon...11/17/13
			BOOL valid;
			int nVis,index;
			char visLocator[128];
			// Request the vision devices from IM layer
			nVis = IMVisCount();

			// initialize IDC_cbLOCATOR1
			HWND hvisloc1=GetDlgItem(hdlg,IDC_cbLOCATOR1);
			SendMessage(hvisloc1,CB_ADDSTRING,0,(LONG)(LPSTR)"(None)");
			for (index=0; index<nVis; index++){
				valid = IMVisName(visLocator,index);
				if (valid){
					SendMessage(hvisloc1,CB_ADDSTRING,0,(LONG)(LPSTR)visLocator);
				}	
			}
			// initialize IDC_cbLOCATOR2
			HWND hvisloc2=GetDlgItem(hdlg,IDC_cbLOCATOR2);
			SendMessage(hvisloc2,CB_ADDSTRING,0,(LONG)(LPSTR)"(None)");
			for (index=0; index<nVis; index++){
				valid = IMVisName(visLocator,index);
				if (valid){
					SendMessage(hvisloc2,CB_ADDSTRING,0,(LONG)(LPSTR)visLocator);
				}	
			}
			if (strlen(locator[0]))	// check for previous locator.
				SetDlgItemText(hdlg,IDC_cbLOCATOR1,locator[0]);
			else
				SetDlgItemText(hdlg,IDC_cbLOCATOR1,"(None)");

			if (strlen(locator[1]))	// check for previous locator.
				SetDlgItemText(hdlg,IDC_cbLOCATOR2,locator[1]);						
			else
				SetDlgItemText(hdlg,IDC_cbLOCATOR2,"(None)");

			SendDlgItemMessage(hdlg,IDC_cbLOCATOR1,EM_LIMITTEXT,IMPLFNBASE,0);
			SendDlgItemMessage(hdlg,IDC_cbLOCATOR2,EM_LIMITTEXT,IMPLFNBASE,0);

			// initialize IDC_cbPROCTYPE1...........................................
			if (strlen(locator[0])){
				HWND hvisProc1=GetDlgItem(hdlg,IDC_cbPROCTYPE1);
				getvisobjects(hvisProc1, locator[0]);						// Send handle of combobox and locator name...
				SetDlgItemText(hdlg,IDC_cbPROCTYPE1,processtype[0]);		// Set to current processtype...		
				}
				else
					SetDlgItemText(hdlg,IDC_cbPROCTYPE1,"(None)");
			SendDlgItemMessage(hdlg,IDC_cbPROCTYPE1,EM_LIMITTEXT,IMPLFNBASE,0);

			// initialize IDC_cbPROCTYPE2................................................
			if (strlen(locator[1])){
				HWND hvisProc2=GetDlgItem(hdlg,IDC_cbPROCTYPE2);
				getvisobjects(hvisProc2, locator[1]);						// Send handle of combobox and locator name...
				SetDlgItemText(hdlg,IDC_cbPROCTYPE2,processtype[1]);		// Set to current processtype...				
				}
				else
					SetDlgItemText(hdlg,IDC_cbPROCTYPE2,"None");
			SendDlgItemMessage(hdlg,IDC_cbPROCTYPE2,EM_LIMITTEXT,IMPLFNBASE,0);

			// initialize locatepreps 0 ............................................................
			if (strlen(locateprep[0])){
				char prepstr[IMPLVISPREP+1];
				wsprintf(prepstr,"%s",locateprep[0]);				//	Copy locateprep....

				char * pch0 = strtok(prepstr,"|");					//	Skip the Locator destination		
				pch0 = strtok (NULL, "|\0");						//	Get the prep string
				if (pch0 != NULL){
					wsprintf(locbuf, "%s", pch0);					// show only prep string not destination
					SetDlgItemText(hdlg,IDC_LOCATEPREP1,locbuf);	// .... d.dixon...11/21/13....
				} else SetDlgItemText(hdlg,IDC_LOCATEPREP1,"-n-p001"); // Default
			}
			SendDlgItemMessage(hdlg,IDC_LOCATEPREP1,EM_LIMITTEXT,IMPLVISPREP,0);
			// initialize locatepreps 1 ............................................................
			if (strlen(locateprep[1])){
				char prepstr[IMPLVISPREP+1];
				wsprintf(prepstr,"%s",locateprep[1]);				//	Copy locateprep....

				char * pch1 = strtok(prepstr,"|");			//	Skip the Locator destination		
				pch1 = strtok (NULL, "|\0");						//	Get the prep string			
				if (pch1 != NULL){
					wsprintf(locbuf, "%s", pch1);	
					SetDlgItemText(hdlg,IDC_LOCATEPREP2,locbuf);
				} else SetDlgItemText(hdlg,IDC_LOCATEPREP2,"-n-p001");
			}
			SendDlgItemMessage(hdlg,IDC_LOCATEPREP2,EM_LIMITTEXT,IMPLVISPREP,0);
			// initialize IDC_NACQPOS ............................................................
			HWND hctrl=GetDlgItem(hdlg,IDC_NACQPOS);
			for(int i=0;i<3;i++)
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,nacqpos,0);

			hctrl=GetDlgItem(hdlg,IDC_NREFPOS);
			for(int i=0;i<3;i++)		// Ref  1= A, 2= A and B  ddd...3/16/14
				{
				wsprintf(tbuf,"%d",i);
				SendMessage(hctrl,CB_ADDSTRING,0,(LPARAM)(LPSTR)tbuf);
				}
			SendMessage(hctrl,CB_SETCURSEL,nrefpos,0);

			for(int i=0;i<2;i++)
				{
				P3 p=acqpos[i];
				MScaleVect((LPDOUBLE)&p,3,FALSE);
				VectToString(tbuf,(LPDOUBLE)&p,3,",");
				SetDlgItemText(hdlg,IDC_ACQPOS0+i,tbuf);
				}
		for(int i=0;i<2;i++)
				{
				P3 p=refpos[i];
				MScaleVect((LPDOUBLE)&p,3,FALSE);
				VectToString(tbuf,(LPDOUBLE)&p,3,",");
				SetDlgItemText(hdlg,IDC_REFPOS0+i,tbuf);
				}

			VectToString(tbuf,(LPDOUBLE)&maxrotation,1,"");
			SetDlgItemText(hdlg,IDC_MAXROTATION,tbuf);

			double mld=maxlendelta;
			MScaleVect((LPDOUBLE)&mld,1,FALSE);
			sprintf(tbuf,"%.4f",mld);//VectToString(tbuf,&mld,1,"");
			SetDlgItemText(hdlg,IDC_MAXLENDELTA,tbuf);

			CheckDlgButton(hdlg,IDC_MASKXROT,maskxrot?1:0);
			CheckDlgButton(hdlg,IDC_MASKYROT,maskyrot?1:0);
			CheckDlgButton(hdlg,IDC_ADJUSTA,adjusta?1:0);

			// Restore window position from [IMS5000]AEWinPos in IMS5000.INI
			POINT p;
			if(GetPrivateProfileIntVect(IMSName,"AEWinPos",(int*)&(p.x),2,IMSIni))
				{
				RECT r;
				GetWindowRect(hdlg,&r);
				MoveWindow(hdlg,p.x,p.y,r.right-r.left,r.bottom-r.top,FALSE);
				}
			AEWinPosLocked=(BOOL)GetPrivateProfileInt(IMSName,"AEWinPosLocked",
				0,IMSIni);

			SetFocus(GetDlgItem(hdlg,IDC_LOCATOR));

			updateenables(hdlg);
			PostMessage(hdlg,WMU_NEWPREP1,0,0); 
			PostMessage(hdlg,WMU_NEWPREP2,0,0); 
			return TRUE;
			}

		case WMU_NEWPREP1:
			if(prepsteps)
				IMSTEPFree(prepsteps,FALSE);
			prepsteps=0;
			if(locator[0]&&locateprep[0])
				{
				prepsteps=IMSELLOCATORSTEPNew(locator[0]);
				if(prepsteps)
					IMSTEPAppend(prepsteps,IMPREPLOCATORSTEPNew(locateprep[0]));
				if(prepsteps)
					SendMessage(IMWnd,IMM_MARK,0,(LPARAM)prepsteps);
				}
			break;

		case WMU_NEWPREP2:
			if(prepsteps)
				IMSTEPFree(prepsteps,FALSE);
			prepsteps=0;
			if(locator[1]&&locateprep[1])
				{
				prepsteps=IMSELLOCATORSTEPNew(locator[1]);
				if(prepsteps)
					IMSTEPAppend(prepsteps,IMPREPLOCATORSTEPNew(locateprep[1]));
				if(prepsteps)
					SendMessage(IMWnd,IMM_MARK,0,(LPARAM)prepsteps);
				}
			break;

		case WM_MOVE:
			if(!AEWinPosLocked)
				{
				RECT r;
				GetWindowRect(hdlg,&r);
				wsprintf(tbuf,"%d,%d",r.left,r.top);
				WritePrivateProfileString(IMSName,"AEWinPos",tbuf,IMSIni);
				}
			break;

		case WM_ACTIVATE:
			if(pADWnd) // Allow app to to IsDialogMessage
				*pADWnd=wP ? hdlg : 0;
			return FALSE;

		case WM_CLOSE:
			VisionLampEnd();		// Turn off lamp
			EndDialog(hdlg,FALSE);  // End the dialog
			closeedit();
			return FALSE;

		case WM_COMMAND://Win32 changed
			{
			HWND hctrl=(HWND)lP;
			WORD ncode=HIWORD(wP);
			switch(LOWORD(wP))
				{
				case IDCANCEL:
					EndDialog(hdlg,FALSE);  // End the dialog
					closeedit();
					return FALSE;
				case IDC_cbLOCATOR1:
					if(ncode==CBN_KILLFOCUS)
						{
						HWND hvisloc1=GetDlgItem(hdlg,IDC_cbLOCATOR1);
						int nIndex = SendMessage(hvisloc1, CB_GETCURSEL, 0, 0 );     
						SendMessage(hvisloc1,CB_GETLBTEXT, nIndex,(LPARAM)locator[0]);
 						aeok=IMVisLive(locator[0],TRUE);
						if(aeok)
							aeok=IMVisIRTM(&irtm,locator[0],rotation);
						if(aeok)
							aeok=IMVisIRTZ(&irtz,locator[0]);
						//update locator+prep string
						HWND hvisprep1=GetDlgItem(hdlg, IDC_LOCATEPREP1);
						GetWindowText(hvisprep1, locbuf, IMPLVISPREP+1);	//-----------d.dixon-11-21-13-- locateprep array...
						wsprintf(locateprep[0],"%s|%s|%s",locator[0],locbuf,processtype[0]);
						SendMessage(hvisprep1, EM_SETMODIFY, 0, 0);
						SendMessage(hwnd,WMU_NEWPREP1,0,0);
						// update IDC_cbPROCTYPE1...........................................
						HWND hvisProc1=GetDlgItem(hdlg,IDC_cbPROCTYPE1);
							getvisobjects(hvisProc1, locator[0]);
						}
					return TRUE;
				case IDC_cbLOCATOR2:
					if(ncode==CBN_KILLFOCUS)
						{
						HWND hvisloc2=GetDlgItem(hdlg,IDC_cbLOCATOR2);
						int nIndex = SendMessage( hvisloc2 , CB_GETCURSEL, 0, 0 );     
						SendMessage(hvisloc2,CB_GETLBTEXT, nIndex, (LPARAM)locator[1]);
 						aeok=IMVisLive(locator[1],TRUE);
						if(aeok)
							aeok=IMVisIRTM(&irtm,locator[1],rotation);
						if(aeok)
							aeok=IMVisIRTZ(&irtz,locator[1]);
						//update locator+prep string
						HWND hvisprep2=GetDlgItem(hdlg, IDC_LOCATEPREP2);
						GetWindowText(hvisprep2, locbuf, IMPLVISPREP+1);			//-----------d.dixon-11-21-13-- locateprep array...
						wsprintf(locateprep[1],"%s|%s|%s",locator[1],locbuf,processtype[1]);
						SendMessage(hvisprep2, EM_SETMODIFY, 0, 0);
						SendMessage(hwnd,WMU_NEWPREP2,0,0);							//-----------d.dixon-11-21-13-- wait till modified fix!!!
						// update IDC_cbPROCTYPE2................................................
						HWND hvisProc2=GetDlgItem(hdlg,IDC_cbPROCTYPE2);
							getvisobjects(hvisProc2, locator[1]);
						}
					return TRUE;
				case IDC_cbPROCTYPE1:										
					if(ncode==CBN_KILLFOCUS)
						{
						HWND hproctype1=GetDlgItem(hdlg, IDC_cbPROCTYPE1);
						int nIndex = SendMessage( hproctype1 , CB_GETCURSEL, 0, 0 );     
						SendMessage(hproctype1,CB_GETLBTEXT, nIndex, (LPARAM)processtype[0]);
						SendMessage(hwnd,WMU_NEWPREP1,0,0);
						}
					return TRUE;
				case IDC_cbPROCTYPE2:												
					if(ncode==CBN_KILLFOCUS)
						{
						HWND hproctype2=GetDlgItem(hdlg, IDC_cbPROCTYPE2);
						int nIndex = SendMessage( hproctype2 , CB_GETCURSEL, 0, 0 );     
						SendMessage(hproctype2,CB_GETLBTEXT, nIndex, (LPARAM)processtype[1]);
						SendMessage(hwnd,WMU_NEWPREP2,0,0);
						}
					return TRUE;
				case IDC_LOCATEPREP1:										
					if(ncode==EN_KILLFOCUS)
						{
						HWND hvisprep1=GetDlgItem(hdlg, IDC_LOCATEPREP1);
						if(SendMessage(hvisprep1, EM_GETMODIFY, 0, 0))
							{
							GetWindowText(hvisprep1,locbuf,IMPLVISPREP+1);			//-----------d.dixon-11-21-13-- locateprep array...
							wsprintf(locateprep[0],"%s|%s|%s",locator[0],locbuf,processtype[0]);
							SendMessage(hvisprep1, EM_SETMODIFY, 0, 0);
							SendMessage(hwnd,WMU_NEWPREP1,0,0);
							}
						}
					return TRUE;
				case IDC_LOCATEPREP2:												//-----------d.dixon-11-21-13--
					if(ncode==EN_KILLFOCUS)
						{
						HWND hvisprep2=GetDlgItem(hdlg,IDC_LOCATEPREP2);
						if(SendMessage(hvisprep2, EM_GETMODIFY, 0, 0))
							{
							GetWindowText(hvisprep2,locbuf,IMPLVISPREP+1);			//-----------d.dixon-11-21-13-- locateprep array...
							wsprintf(locateprep[1],"%s|%s|%s",locator[1],locbuf,processtype[1]);
							SendMessage(hvisprep2,EM_SETMODIFY, 0, 0);
							SendMessage(hwnd,WMU_NEWPREP2,0,0);		//-----------d.dixon-11-21-13-- wait till modified fix!!!
							}
						}
					return TRUE;
				case IDC_NACQPOS:
					if(ncode==CBN_SELCHANGE)
						{
						nacqpos=(int)SendMessage(hctrl,CB_GETCURSEL,0,0);
						updateenables(hdlg);
						}
					return TRUE;
				case IDC_ACQPOS0:
				case IDC_ACQPOS1:
					if(ncode==EN_KILLFOCUS)
						if(SendMessage(hctrl,EM_GETMODIFY,0,0))
							{
							WORD i=wP-IDC_ACQPOS0;
							tlen=GetWindowText(hctrl,tbuf,TSIZE);
							P3 p;
							if(ParseVect(tbuf,(double*)&p,3))
								{
								MScaleVect((double*)&p,3,TRUE);
								acqpos[i]=p;
								}
							else
								{
								MessageBeep(MB_ICONEXCLAMATION);
								p=acqpos[i];
								}
							MScaleVect((LPDOUBLE)&p,3,FALSE);
							VectToString(tbuf,(LPDOUBLE)&p,3,",");
							SetWindowText(hctrl,tbuf);
							SendMessage(hctrl,EM_SETMODIFY,0,0);
						}
					return TRUE;
				case IDC_ACQMOVE0:
				case IDC_ACQMOVE1:
					{
					WORD i=wP-IDC_ACQMOVE0;
					SetFocus(GetDlgItem(hdlg,IDC_ACQPOS0+i));
					if(movesteps)
						IMSTEPFree(movesteps,FALSE);
					movesteps=NULL;
					VisPos=acqpos[i];
					if(ZEXISTS)
						zvis=VisPos.z+pimf->extent.z+partz;
					else
						zvis=0;
					movesteps=IMSELLOCATORSTEPNew(locator[i]);
					IMSTEPAppend(movesteps,IMFCompileVPosMove(pimf,nesti,vmLOCATOR));
					SendMessage(IMWnd,IMM_MARK,0,(LPARAM)movesteps);
					return TRUE;
					}
				case IDC_ACQJOG0:
					SetFocus(GetDlgItem(hdlg,IDC_ACQPOS0));
					jogindex=0;								// indicates which location jog is to digitize (see Case IMN_Ready)
					SendMessage(hwnd,WMU_NEWPREP1,0,0);		//-----------d.dixon-11-21-13-- wait till modified fix!!!
					SendMessage(IMWnd,IMM_JOG,0,0);			// start jog mode
					return TRUE;
				case IDC_ACQJOG1:
					jogindex=1;								// indicates which location jog is to digitize (see Case IMN_Ready)
					SetFocus(GetDlgItem(hdlg,IDC_ACQPOS1));
					SendMessage(hwnd,WMU_NEWPREP2,0,0);		//-----------d.dixon-11-21-13-- wait till modified fix!!!
					SendMessage(IMWnd,IMM_JOG,0,0);			// start jog mode
					return TRUE;
				case IDC_NREFPOS:
					if(ncode==CBN_SELCHANGE)
						{
						nrefpos=(int)SendMessage(hctrl,CB_GETCURSEL,0,0);
						updateenables(hdlg);
						}
					return TRUE;
				case IDC_REFPOS0:
				case IDC_REFPOS1:
					if(ncode==EN_KILLFOCUS)
						if(SendMessage(hctrl,EM_GETMODIFY,0,0))
							{
							WORD i=wP-IDC_REFPOS0;
							tlen=GetWindowText(hctrl,tbuf,TSIZE);
							P3 p;
							if(ParseVect(tbuf,(LPDOUBLE)&p,3))
								{
								MScaleVect((LPDOUBLE)&p,3,TRUE);
								refpos[i]=p;
								}
							else
								{
								MessageBeep(MB_ICONEXCLAMATION);
								p=refpos[i];
								}
							MScaleVect((LPDOUBLE)&p,3,FALSE);
							VectToString(tbuf,(LPDOUBLE)&p,3,",");
							SetWindowText(hctrl,tbuf);
							SendMessage(hctrl,EM_SETMODIFY,0,0);
							}
					return TRUE;
				case IDC_LEARNREFPOS0:
					{
					SetFocus(GetDlgItem(hdlg,IDC_REFPOS0));
					if(learnsteps)
						IMSTEPFree(learnsteps,FALSE);
					learnsteps=IMFCompileLocates(pimf);
					if(learnsteps)
						{
						// register notify handler during learn if not
						// allready registered by run
						// Start the sequence
						if(pimf!=&RunIMF) // All imnLocates go to RunIMF.ae*
							{
							RunIMF.aesetup=pimf->aesetup;
							RunIMF.aeinfo=pimf->aeinfo;
							}
						SendMessage(IMWnd,IMM_MARK,0,(LPARAM)learnsteps);
						}
					return TRUE;
					}
				case IDC_LEARNREFPOS1:
					{
					SetFocus(GetDlgItem(hdlg,IDC_REFPOS1));
					if(learnsteps)
						IMSTEPFree(learnsteps,FALSE);
					learnsteps=IMFCompileLocates(pimf);
					if(learnsteps)
						{
						// register notify handler during learn if not
						// allready registered by run
						// Start the sequence
						if(pimf!=&RunIMF) // All imnLocates go to RunIMF.ae*
							{
							RunIMF.aesetup=pimf->aesetup;
							RunIMF.aeinfo=pimf->aeinfo;
							}
						SendMessage(IMWnd,IMM_MARK,0,(LPARAM)learnsteps);
						}
					return TRUE;
					}
				case IDC_COPYREFPOS2CAM:
					for(int i=0;i<2;i++)
							{
							P3 p=refpos[i];
							acqpos[i]=p;
							MScaleVect((LPDOUBLE)&p,3,FALSE);
							VectToString(tbuf,(LPDOUBLE)&p,3,",");
							SetDlgItemText(hdlg,IDC_ACQPOS0+i,tbuf);
							}
					return TRUE;
				case IDC_MAXROTATION:
					if(ncode==EN_KILLFOCUS)
						if(SendMessage(hctrl,EM_GETMODIFY,0,0))
							{
							tlen=GetWindowText(hctrl,tbuf,TSIZE);
							double f;
							if(ParseVect(tbuf,(LPDOUBLE)&f,1))
								if(f>=0.0f && f<=360.0)
									maxrotation=f;
								else
									MessageBeep(MB_ICONEXCLAMATION);
							else
								MessageBeep(MB_ICONEXCLAMATION);
							VectToString(tbuf,(LPDOUBLE)&maxrotation,1,",");
							SetWindowText(hctrl,tbuf);
							SendMessage(hctrl,EM_SETMODIFY,0,0);
							}
					return TRUE;
				case IDC_MAXLENDELTA:
					if(ncode==EN_KILLFOCUS)
						if(SendMessage(hctrl,EM_GETMODIFY,0,0))
							{
							tlen=GetWindowText(hctrl,tbuf,TSIZE);
							double f;
							if(ParseVect(tbuf,(LPDOUBLE)&f,1))
								if(f>=0.0f && f<100.0)
									maxlendelta=f;
								else
									MessageBeep(MB_ICONEXCLAMATION);
							else
								MessageBeep(MB_ICONEXCLAMATION);
							VectToString(tbuf,(LPDOUBLE)&maxlendelta,1,",");
							SetWindowText(hctrl,tbuf);
							SendMessage(hctrl,EM_SETMODIFY,0,0);
							}
					return TRUE;
				case IDC_MASKXROT:
					if(ncode==BN_CLICKED)
						maskxrot=IsDlgButtonChecked(hdlg,IDC_MASKXROT);
					return TRUE;
				case IDC_MASKYROT:
					if(ncode==BN_CLICKED)
						maskyrot=IsDlgButtonChecked(hdlg,IDC_MASKYROT);
					return TRUE;
				case IDC_ADJUSTA:
					if(ncode==BN_CLICKED)
						adjusta=IsDlgButtonChecked(hdlg,IDC_ADJUSTA);
					return TRUE;
				case IDC_IM:
					switch(ncode)//zzz
						{
						case IMN_READY:
							if(injog)
								{
								// get jogtcs
								P3 jogtcs;
								SendMessage(IMWnd,IMM_GETXYPOS,0,(LPARAM)(LPDOUBLE)&jogtcs.x);
								if(ZEXISTS)
									SendMessage(IMWnd,IMM_GETZPOS,0,(LPARAM)(LPDOUBLE)&jogtcs.z);
								P3 aecs; // NCS or FCS of location
								// compute aecs from jogtcs
								V2 temp(jogtcs.x,jogtcs.y);
								//P2Xform((LPP2)&aecs,(LPP2)&jogtcs,&irtm);
								V2 atemp=temp*irtm;
								aecs.x=atemp.v2[0];
								aecs.y=atemp.v2[1];
								aecs.x-=frame.m3[2][0];
								aecs.y-=frame.m3[2][1];
								if(ZEXISTS)
									aecs.z=jogtcs.z+irtz-(pimf->extent.z+partz);
								else
									aecs.z=0.0f;
								acqpos[jogindex]=aecs;
								MScaleVect((LPDOUBLE)&aecs,3,FALSE);
								VectToString(tbuf,(LPDOUBLE)&aecs,3,",");
								SetDlgItemText(hdlg,IDC_ACQPOS0+jogindex,tbuf);
								injog=FALSE;
								}
							// detect end of learn sequence (inlearn)
							// and paste into refpos
							if(learnsteps)
								{
								if(pimf!=&RunIMF) // remove alias refs
									{
									RunIMF.aeinfo=NULL;
									RunIMF.aesetup=NULL;
									}
								for(int i=0;i<nrefpos;i++)
									{
									refpos[i]=pimf->aeinfo->pos[i];
									P3 p=refpos[i];
									MScaleVect((LPDOUBLE)&p,3,FALSE);
									VectToString(tbuf,(LPDOUBLE)&p,3,",");
									SetDlgItemText(hdlg,IDC_REFPOS0+i,tbuf);
									}
								IMSTEPFree(learnsteps,FALSE);
								learnsteps=NULL;
								// Unreg callback unless needed by mRUN
								}
							break;
						case IMN_JOG:
							injog=TRUE;
							break;
						}
					imok=IMState==IMN_READY || IMState==IMN_PRINTOFF || IMState==IMN_XYONLY || IMState==IMN_WARMUP;
					if(!injog)
						aeok=IMVisLive(locator[0],TRUE);
					updateenables(hdlg);
					return 0;
				}
			return FALSE;
			}
		case WM_DESTROY:
			IMVisLive(locator[0],FALSE);
			if(movesteps)
				IMSTEPFree(movesteps,FALSE);
			movesteps=NULL;
			if(learnsteps)
				IMSTEPFree(learnsteps,FALSE);
			learnsteps=NULL;
			VisionLampEnd();		// Turn off lamp
			return 0;
		}
	return FALSE;
	}

// updateenables : updates the enabled state of controls on the editor
// based on nacqpos, nrefpos, aeok and imok
void AESETUP::updateenables(HWND hdlg)
	{
	if(!hdlg)
		return;
	EnableWindow(GetDlgItem(hdlg,IDC_cbLOCATOR2),nacqpos>1);	//....d.dixon.....11/21/13
	EnableWindow(GetDlgItem(hdlg,IDC_cbPROCTYPE2),nacqpos>1);	//....d.dixon.....01/09/14
	EnableWindow(GetDlgItem(hdlg,IDC_LOCATEPREP2),nacqpos>1);	//....d.dixon.....11/21/13
	EnableWindow(GetDlgItem(hdlg,IDC_NREFPOS),nacqpos>0);		//....d.dixon.....3/17/14
	for(int i=0;i<2;i++)
		{
		BOOL enab=i<nacqpos;
		EnableWindow(GetDlgItem(hdlg,IDC_ACQPOS0+i),enab);
		enab=enab && imok;
		EnableWindow(GetDlgItem(hdlg,IDC_ACQMOVE0+i),enab);
		EnableWindow(GetDlgItem(hdlg,IDC_ACQJOG0+i),enab);
		}
	for(int i=0;i<2;i++)
		{
		BOOL enab=i<nrefpos; 
		EnableWindow(GetDlgItem(hdlg,IDC_REFPOS0+i),enab);
		EnableWindow(GetDlgItem(hdlg,IDC_LEARNREFPOS0+i),enab&&aeok&&(imok||nacqpos==0));
		}
	EnableWindow(GetDlgItem(hdlg,IDC_MAXROTATION),nrefpos>1);
	EnableWindow(GetDlgItem(hdlg,IDC_MASKXROT),nrefpos>1);
	EnableWindow(GetDlgItem(hdlg,IDC_MASKYROT),nrefpos>1);
	EnableWindow(GetDlgItem(hdlg,IDC_MAXLENDELTA),nrefpos>1);
	EnableWindow(GetDlgItem(hdlg,IDC_ADJUSTA),nrefpos>1);
	//Disabled at this time....d.dixon .... 1/16/14 - TBD
	EnableWindow(GetDlgItem(hdlg,IDC_COPYREFPOS2CAM),nrefpos>1);	

	}
void AESETUP::getvisobjects(HWND hIDC, char location[])
{
	// Load Process combobox(cb)						// .... d.dixon...1/7/14
	char VisDirectory[128], buf[128];
	DIR *dir;
	struct dirent *visobj;					//visual object name on from disk (ie. num5.o2d etc.)
	SendMessage(hIDC,CB_RESETCONTENT,0,0);
	SendMessage(hIDC,CB_ADDSTRING,0,(LONG)(LPSTR)"(None)");
	//Get Vision Object Directory from IMSini
	if (GetPrivateProfileString(IMSName,"VisObjects","", buf,sizeof(buf),IMSIni)){
		wsprintf(VisDirectory,"%s\\%s",buf,location);
		if ((dir = opendir (VisDirectory)) != NULL) {
			while ((visobj = readdir (dir)) != NULL) {
				if (visobj->d_name[0]!= '.')
					SendMessage(hIDC,CB_ADDSTRING,0,(LONG)(LPSTR)visobj->d_name);
				OutputDebugString(visobj->d_name);
				}
			closedir (dir);
			}
	} else 	MessageBox(NULL,"Failed to access Vision Object Directory.\nPlease verifiy IMS5000 Configuration for Director Locations",NULL,MB_OK);


}
