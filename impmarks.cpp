#include "stdafx.h"
#include "imp.h"
#include "imsio.h"

/******************************************************************************
	IMPMARK CLASS
******************************************************************************/
IMPMARK::IMPMARK()
	{
	origin.x=0.0f;
	origin.y=0.0f;
	rotation=0.0f;
	slant=0.0f;
	zflight=0.0f;
	justx=mjxLEFT;
	justy=mjyTOP;
	rep=0;
	}

IMPMARK::~IMPMARK(){
}

/* Initializes the type members - Update this function if new fields are added */
void IMPMARK::initByType(){
	switch(type){
	case mtTEXT:
		t.ijfmt.font=0;
		t.ijfmt.ms=1;
		t.ijfmt.xflip=0;
		t.ijfmt.yflip=0;
		t.ch=0.15;
		t.cw=0.10;
		t.text[0]=0;
		break;
	case mtDEVBC: 
		devbc.ijfmt.font=0;
		devbc.ijfmt.ms=1;
		devbc.ijfmt.xflip=0;
		devbc.ijfmt.yflip=0;
		devbc.ch=0.25;
		devbc.cw=0.20;
		devbc.text[0]=0;
		break;
	case mtBITMAP:  
		b.xres=100;
		b.yres=100;
		b.ms=1;
		b.neg=0;
		b.fn[0]=0;
		break;
	case mtC128:  
		c128.xres=100;
		c128.yres=100;
		c128.dx=1;
		c128.dy=24;
		c128.qz=0;
		c128.ms=1;
		c128.neg=0;
		c128.overlayhr=HRNONE;
		c128.text[0]=0;
		c128.hrpict[0]=0;
		c128.cset=C128OPT;
		c128.reserved=0;
		break;
	case mtCUPCEAN:   
		cupcean.xres=100;
		cupcean.yres=100;
		cupcean.dx=1;
		cupcean.dy=24;
		cupcean.qz=0;
		cupcean.ms=1;
		cupcean.neg=0;
		cupcean.subtype=0;
		cupcean.overlayhr=HRNONE;
		cupcean.hrpict[0]=0;
		cupcean.text[0]=0;
		break;
	case mtC93:  
		c93.xres=100;
		c93.yres=100;
		c93.dx=1;
		c93.dy=24;
		c93.qz=0;
		c93.ms=1;
		c93.neg=0;
		c93.text[0]=0;
		break;
	case mtMULTILINE:		
		ml.ijfmt.font=0;
		ml.ijfmt.ms=1;
		ml.ijfmt.xflip=0;
		ml.ijfmt.yflip=0;
		ml.ch=0.15;
		ml.cw=0.10;
		ml.text[0]=0;
		break;
	//case uid12:                            //------------------Linghai-11-21-08---
	//case uid6:                             //------------------Linghai-11-21-08--
	//case uid5:                             //------------------Linghai-11-21-08--
	case mtUID:                             //--------------Linghai-4-9-09----
		uid.xres=100;
		uid.yres=100;
		uid.ms=1;
		uid.text[0]=0;
		uid.dminfo.neg=0;
		uid.dminfo.xmul=1;
		uid.dminfo.ymul=1;
		uid.dminfo.xqz=1;
		uid.dminfo.yqz=1;
		uid.dminfo.rotation=0;
		uid.dminfo.xflip=0;
		uid.dminfo.yflip=0;
		uid.dminfo.format=5;
		uid.dminfo.eccval=DM_ECC_000;
		uid.dminfo.rows=0;
		uid.dminfo.cols=0;
		uid.dminfo.style=DM_NORMAL_IMAGE;
		uid.dminfo.border=1;
		uid.dminfo.encrypt=DM_NO_ENCRYPTION;
		uid.dminfo.data=NULL;
		break;
	case mtCDM:  
		cdm.xres=100;
		cdm.yres=100;
		cdm.ms=1;
		cdm.text[0]=0;
		cdm.dminfo.neg=0;
		cdm.dminfo.xmul=1;
		cdm.dminfo.ymul=1;
		cdm.dminfo.xqz=1;
		cdm.dminfo.yqz=1;
		cdm.dminfo.rotation=0;
		cdm.dminfo.xflip=0;
		cdm.dminfo.yflip=0;
		cdm.dminfo.format=5;
		cdm.dminfo.eccval=DM_ECC_000;
		cdm.dminfo.rows=0;
		cdm.dminfo.cols=0;
		cdm.dminfo.style=DM_NORMAL_IMAGE;
		cdm.dminfo.border=1;
		cdm.dminfo.encrypt=DM_NO_ENCRYPTION;
		cdm.dminfo.data=NULL;
		break;
	case mtWINFONT: 
		winfont.xres=100;
		winfont.yres=100;
		winfont.ms=1;
		winfont.neg=0;
		winfont.text[0]=0;
		winfont.sef=0;
		winfont.yorgm=0;
		winfont.yincm=0;
		winfont.logfont.lfHeight=-8;
		winfont.logfont.lfWidth=0;
		winfont.logfont.lfEscapement=0;
		winfont.logfont.lfOrientation=0;
		winfont.logfont.lfWeight=0;
		winfont.logfont.lfItalic=0;
		winfont.logfont.lfCharSet=0;
		winfont.logfont.lfUnderline=0;
		winfont.logfont.lfStrikeOut=0;
		winfont.logfont.lfOutPrecision=0;
		winfont.logfont.lfClipPrecision=0;
		winfont.logfont.lfQuality=0;
		winfont.logfont.lfPitchAndFamily=0;
		lstrcpy(winfont.logfont.lfFaceName,"System");
		break;
	case mtWMTRX: 
		wmtrx.xres=100;
		wmtrx.yres=100;
		wmtrx.ms=1;
		wmtrx.neg=0;

		wmtrx.srcxs=4;
		wmtrx.srcys=4;
		wmtrx.subxo=0;
		wmtrx.subyo=0;
		wmtrx.subxs=4;
		wmtrx.subys=4;
		wmtrx.celxs=24;
		wmtrx.celys=24;
		wmtrx.dotxs=20;
		wmtrx.dotys=20;
		wmtrx.text[0]=0;
		wmtrx.hex=false;
		wmtrx.flags=0;
		wmtrx.chtext[0]=0;
		wmtrx.rhtext[0]=0;
		wmtrx.logfont.lfHeight=-8;
		wmtrx.logfont.lfWidth=0;
		wmtrx.logfont.lfEscapement=0;
		wmtrx.logfont.lfOrientation=0;
		wmtrx.logfont.lfWeight=0;
		wmtrx.logfont.lfItalic=0;
		wmtrx.logfont.lfCharSet=0;
		wmtrx.logfont.lfUnderline=0;
		wmtrx.logfont.lfStrikeOut=0;
		wmtrx.logfont.lfOutPrecision=0;
		wmtrx.logfont.lfClipPrecision=0;
		wmtrx.logfont.lfQuality=0;
		wmtrx.logfont.lfPitchAndFamily=0;
		lstrcpy(wmtrx.logfont.lfFaceName,"Lucida Console");
		break;
	case mtCQR: 
		cqr.xres=100;
		cqr.yres=100;
		cqr.ms=1;
		cqr.neg=0;
		cqr.rot=0;
		cqr.deg=0;
		cqr.xflip=false;
		cqr.yflip=false;
		cqr.xmul=1;
		cqr.ymul=1;
		cqr.xqz=0;
		cqr.yqz=0;
		cqr.enc=IMPCQRENCA;
		cqr.cor=IMPCQRCORL;
		cqr.ver=1;
		cqr.text[0]=0;
		break;

	case mtPATCH: 
		patch.xres=100;
		patch.yres=100;
		patch.ms=1;
		patch.neg=0;
		patch.size.x=20;
		patch.size.y=20;
		patch.res1=0;
		patch.res2=0;
		patch.res3=0;
		patch.res4.x=0;
		patch.res4.y=0;
		break;	
	case mtC39:  
		c39.xres=100;
		c39.yres=100;
		c39.dx=1;
		c39.dy=24;
		c39.qz=0;
		c39.ms=1;
		c39.neg=0;
		c39.overlayhr=HRNONE;
		c39.fa=0;
		c39.addsum=0;
		c39.text[0]=0;
		c39.hrpict[0]=0;
		break;
	case mtC2OF5:  
		c2of5.xres=100;
		c2of5.yres=100;
		c2of5.dx=1;
		c2of5.dy=24;
		c2of5.qz=0;
		c2of5.ms=1;
		c2of5.neg=0;
		c2of5.overlayhr=HRNONE;
		c2of5.interleave=TRUE;
		c2of5.bbars=0;
		c2of5.addsum=0;
		c2of5.text[0]=0;
		c2of5.hrpict[0]=0;
		break;
	case mtCURE:  
		cure.xsize=20;
		cure.vel=0;
		cure.keepon=0;
		break;
	case mtCDEV:   // size: 147 (Includes the 8 pointers for bsms)
		cdev.bijfmt.font=0;
		cdev.bijfmt.ms=1;
		cdev.bijfmt.xflip=0;
		cdev.bijfmt.yflip=0;
		cdev.bfi=0;
		cdev.xres=100;
		cdev.yres=100;
		cdev.text[0]=0;
		cdev.nbsm=0;
		// cdev.bsm already initialized
		break;
	case mtDEVSYM:
		devsym.ijfmt.font=0;
		devsym.ijfmt.ms=1;
		devsym.ijfmt.xflip=0;
		devsym.ijfmt.yflip=0;
		devsym.width=0.3f;
		devsym.height=0.3f;
		devsym.xdots=0;
		devsym.text[0]=0;
		break;
  default:;
	}

}

BOOL IMPMARK::read(HANDLE fh,WORD ver){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver);
	else 
		return translate(fh,ver);
}

// Reads native versions 4.00 and 4.01
BOOL IMPMARK::readnative(HANDLE fh,WORD ver){
	// Origin
	if (!readDouble(fh,&origin.x))
		return FALSE;
	if (!readDouble(fh,&origin.y))
		return FALSE;
	// Rotation
	if (!readDouble(fh,&rotation))
		return FALSE;
	// Rotation degree	--sam	10/7/16
	//if (!readDouble(fh,&deg.t))
		//return FALSE;
	// Slant
	if (!readDouble(fh,&slant))
		return FALSE;
	// Z flight
	if (!readDouble(fh,&zflight))
		return FALSE;
	// Justification
	if (!readByte(fh,&justx))
		return FALSE;
	if (!readByte(fh,&justy))
		return FALSE;
	// Rep
	if (!readWord(fh,&rep))
		return FALSE;
	// Type
	if (!readInt(fh,(int*)&type))
		return FALSE;
	// Init the type members
	initByType();
	// Type specific info
	switch (type){
	case mtTEXT:
		// ijfmt
		if (!readIJFMT(fh,&t.ijfmt))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&t.cw))
			return FALSE;
		if (!readDouble(fh,&t.ch))
			return FALSE;
		// text
		if (!readString(fh,t.text,IMPLTEXT+1))
			return FALSE;
		break;
	case mtDEVBC:
		// ijfmt
		if (!readIJFMT(fh,&devbc.ijfmt))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&devbc.cw))
			return FALSE;
		if (!readDouble(fh,&devbc.ch))
			return FALSE;
		// text
		if (!readString(fh,devbc.text,IMPLDEVBC+1))
			return FALSE;
		break;
	case mtBITMAP:
		//xres, yres
		if (!readDouble(fh,&b.xres))
			return FALSE;
		if (!readDouble(fh,&b.yres))
			return FALSE;
		//ms
		if (!readWord(fh,&b.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&b.neg))
			return FALSE;
		//fn
		if (!readString(fh,b.fn,IMPLBMFN+1))
			return FALSE;
		break;
	case mtC128:
		//xres, yres
		if (!readDouble(fh,&c128.xres))
			return FALSE;
		if (!readDouble(fh,&c128.yres))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c128.dx))
			return FALSE;
		if (!readInt(fh,&c128.dy))
			return FALSE;
		//qz
		if (!readInt(fh,&c128.qz))
			return FALSE;
		//ms
		if (!readWord(fh,&c128.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c128.neg))
			return FALSE;
		//overlayhr
		if (!readInt(fh,(int*) &c128.overlayhr))
			return FALSE;
		//text
		if (!readString(fh,c128.text,IMPLC128+1))
			return FALSE;
		//hrpict
		if (!readString(fh,c128.hrpict,IMPLC128HRPICTURE+1))
			return FALSE;
		//cset
		if (!readByte(fh,&c128.cset))
			return FALSE;
		//reserved
		if (!readByte(fh,&c128.reserved))
			return FALSE;
		break;
	case mtCUPCEAN:
		//xres, yres
		if (!readDouble(fh,&cupcean.xres))
			return FALSE;
		if (!readDouble(fh,&cupcean.yres))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&cupcean.dx))
			return FALSE;
		if (!readInt(fh,&cupcean.dy))
			return FALSE;
		//qz
		if (!readInt(fh,&cupcean.qz))
			return FALSE;
		//ms
		if (!readWord(fh,&cupcean.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&cupcean.neg))
			return FALSE;
		//subtype
		if (!readInt(fh,&cupcean.subtype))
			return FALSE;
		//overlayhr
		if (!readInt(fh,&cupcean.overlayhr))
			return FALSE;
		//hrpict
		if (!readString(fh,cupcean.hrpict,IMPLCUPCEANHRPICTURE+1))
			return FALSE;
		//text
		if (!readString(fh,cupcean.text,IMPLCUPCEAN+1))
			return FALSE;
		break;
	case mtC93:
		//xres, yres
		if (!readDouble(fh,&c93.xres))
			return FALSE;
		if (!readDouble(fh,&c93.yres))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c93.dx))
			return FALSE;
		if (!readInt(fh,&c93.dy))
			return FALSE;
		//qz
		if (!readInt(fh,&c93.qz))
			return FALSE;
		//ms
		if (!readWord(fh,&c93.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c93.neg))
			return FALSE;
		//text
		if (!readString(fh,c93.text,IMPLC93+1))
			return FALSE;
		break;
	case mtMULTILINE:
		// ijfmt
		if (!readIJFMT(fh,&ml.ijfmt))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&ml.cw))
			return FALSE;
		if (!readDouble(fh,&ml.ch))
			return FALSE;
		// text
		if (!readString(fh,ml.text,IMPLMLTEXT+1))
			return FALSE;
		break;
	//case uid12:                             //-----------------Linghai-11-21-8--
	//case uid6:                              //-----------------Linghai-11-21-8--
	//case uid5:                              //-----------------Linghai-11-21-08--
	case mtUID:
		//xres, yres
		if (!readDouble(fh,&uid.xres))
			return FALSE;
		if (!readDouble(fh,&uid.yres))
			return FALSE;
		//ms
		if (!readWord(fh,&uid.ms))
			return FALSE;
		//dminfo
		if (!readDMINFO(fh,&uid.dminfo))
			return FALSE;
		//text
		if (!readString(fh,uid.text,IMPLCDM+1))
			return FALSE;
		break;
	case mtCDM:
		//xres, yres
		if (!readDouble(fh,&cdm.xres))
			return FALSE;
		if (!readDouble(fh,&cdm.yres))
			return FALSE;
		//ms
		if (!readWord(fh,&cdm.ms))
			return FALSE;
		//dminfo
		if (!readDMINFO(fh,&cdm.dminfo))
			return FALSE;
		//text
		if (!readString(fh,cdm.text,IMPLCDM+1))
			return FALSE;
		break;
	case mtWINFONT:
		//xres, yres
		if (!readDouble(fh,&winfont.xres))
			return FALSE;
		if (!readDouble(fh,&winfont.yres))
			return FALSE;
		//ms
		if (!readWord(fh,&winfont.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&winfont.neg))
			return FALSE;
		//font
		if (!readLOGFONT(fh,&winfont.logfont))
			return FALSE;
		//text
		if (!readString(fh,winfont.text,IMPLWINFONT+1))
			return FALSE;
		//sef,yorgm, yincm
		if (!readByte(fh,&winfont.sef))
			return FALSE;
		if (!readInt(fh,&winfont.yorgm))
			return FALSE;
		if (!readInt(fh,&winfont.yincm))
			return FALSE;

		break;
	case mtWMTRX:
		if (!readDouble(fh,&wmtrx.xres)) return FALSE;
		if (!readDouble(fh,&wmtrx.yres)) return FALSE;
		if (!readWord(fh,&wmtrx.ms)) return FALSE;
		if (!readInt(fh,&wmtrx.neg)) return FALSE;
		if (!readInt(fh,&wmtrx.srcxs)) return FALSE;
		if (!readInt(fh,&wmtrx.srcys)) return FALSE;
		if (!readInt(fh,&wmtrx.subxo)) return FALSE;
		if (!readInt(fh,&wmtrx.subyo)) return FALSE;
		if (!readInt(fh,&wmtrx.subxs)) return FALSE;
		if (!readInt(fh,&wmtrx.subys)) return FALSE;
		if (!readInt(fh,&wmtrx.celxs)) return FALSE;
		if (!readInt(fh,&wmtrx.celys)) return FALSE;
		if (!readInt(fh,&wmtrx.dotxs)) return FALSE;
		if (!readInt(fh,&wmtrx.dotys)) return FALSE;

		if (!readInt(fh,&wmtrx.matxo)) return FALSE;
		if (!readInt(fh,&wmtrx.matyo)) return FALSE;
		if (!readInt(fh,&wmtrx.chxo)) return FALSE;
		if (!readInt(fh,&wmtrx.chyo)) return FALSE;
		if (!readInt(fh,&wmtrx.rhxo)) return FALSE;
		if (!readInt(fh,&wmtrx.rhyo)) return FALSE;

		if (!readInt(fh,&wmtrx.hex)) return FALSE;
		if (!readString(fh,wmtrx.text,IMPLWMTRX+1)) return FALSE;
		if (!readByte(fh,&wmtrx.flags)) return FALSE;

		if (!readString(fh,wmtrx.chtext,IMPLWMTRXCHTEXT+1)) return FALSE;
		if (!readString(fh,wmtrx.rhtext,IMPLWMTRXRHTEXT+1)) return FALSE;
		if (!readLOGFONT(fh,&wmtrx.logfont)) return FALSE;

		break;

	case mtCQR:
		if (!readDouble(fh,&cqr.xres)) return FALSE;
		if (!readDouble(fh,&cqr.yres)) return FALSE;
		if (!readWord(fh,&cqr.ms)) return FALSE;
		if (!readInt(fh,&cqr.neg)) return FALSE;
		if (!readInt(fh,&cqr.rot)) return FALSE;
		//rotation degree	--sam	10/10/16
		//if(!HideOptRotaryDEG || EditHasRotDeg)
			if (!readDouble(fh,&cqr.deg)) return FALSE;
		if (!readInt(fh,&cqr.xflip)) return FALSE;
		if (!readInt(fh,&cqr.yflip)) return FALSE;
		if (!readInt(fh,&cqr.xmul)) return FALSE;
		if (!readInt(fh,&cqr.ymul)) return FALSE;
		if (!readInt(fh,&cqr.xqz)) return FALSE;
		if (!readInt(fh,&cqr.yqz)) return FALSE;
		if (!readInt(fh,(int*)&cqr.enc)) return FALSE;
		if (!readInt(fh,(int*)&cqr.cor)) return FALSE;
		if (!readInt(fh,&cqr.ver)) return FALSE;
		if (!readString(fh,cqr.text,IMPLCQR+1)) return FALSE;
		break;

	case mtPATCH:
		//xres, yres
		if (!readDouble(fh,&patch.xres))
			return FALSE;
		if (!readDouble(fh,&patch.yres))
			return FALSE;
		//ms
		if (!readWord(fh,&patch.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&patch.neg))
			return FALSE;
		//size
		if (!readInt(fh,(int*) &patch.size.x))
			return FALSE;
		if (!readInt(fh,(int*) &patch.size.y))
			return FALSE;
		//res1, res2, res3
		if (!readWord(fh,&patch.res1))
			return FALSE;
		if (!readWord(fh,&patch.res2))
			return FALSE;
		if (!readWord(fh,&patch.res3))
			return FALSE;
		//res4
		if (!readInt(fh,(int*) &patch.res4.x))
			return FALSE;
		if (!readInt(fh,(int*) &patch.res4.y))
			return FALSE;
		break;
	case mtC39:
		//xres, yres
		if (!readDouble(fh,&c39.xres))
			return FALSE;
		if (!readDouble(fh,&c39.yres))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c39.dx))
			return FALSE;
		if (!readInt(fh,&c39.dy))
			return FALSE;
		//qz
		if (!readInt(fh,&c39.qz))
			return FALSE;
		//ms
		if (!readWord(fh,&c39.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c39.neg))
			return FALSE;
		//overlayhr
		if (!readInt(fh,(int*)&c39.overlayhr))
			return FALSE;
		//fa
		if (!readInt(fh,&c39.fa))
			return FALSE;
		//addsum
		if (!readInt(fh,&c39.addsum))
			return FALSE;
		//text
		if (!readString(fh,c39.text,IMPLC39+1))
			return FALSE;
		//hrpict
		if (!readString(fh,c39.hrpict,IMPLC39HRPICTURE+1))
			return FALSE;
		break;
	case mtC2OF5:
		if (!readDouble(fh,&c2of5.xres))
			return FALSE;
		if (!readDouble(fh,&c2of5.yres))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c2of5.dx))
			return FALSE;
		if (!readInt(fh,&c2of5.dy))
			return FALSE;
		//qz
		if (!readInt(fh,&c2of5.qz))
			return FALSE;
		//ms
		if (!readWord(fh,&c2of5.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c2of5.neg))
			return FALSE;
		//overlayhr
		if (!readInt(fh,(int*)&c2of5.overlayhr))
			return FALSE;
		//interleave
		if (!readInt(fh,&c2of5.interleave))
			return FALSE;
		//bbars
		if (!readInt(fh,&c2of5.bbars))
			return FALSE;
		//addsum
		if (!readInt(fh,&c2of5.addsum))
			return FALSE;
		//text
		if (!readString(fh,c2of5.text,IMPLC2OF5+1))
			return FALSE;
		//hrpict
		if (!readString(fh,c2of5.hrpict,IMPLC2OF5HRPICTURE+1))
			return FALSE;
		break;
	case mtCURE:
		//xsize
		if (!readDouble(fh,&cure.xsize))
			return FALSE;
		//vel
		if (!readDouble(fh,&cure.vel))
			return FALSE;
		//keepon
		if (!readInt(fh,&cure.keepon))
			return FALSE;
		break;
	case mtCDEV:
		//bijfmt
		if (!readIJFMT(fh,&cdev.bijfmt))
			return FALSE;
		//bfi
		if (!readInt(fh,&cdev.bfi))
			return FALSE;
		//res
		if (!readDouble(fh,&cdev.xres))
			return FALSE;
		if (!readDouble(fh,&cdev.yres))
			return FALSE;
		//text
		if (!readString(fh,cdev.text,IMPLCDEVTEXT+1))
			return FALSE;
		//nbsm
		if (!readInt(fh,&cdev.nbsm))
			return FALSE;
		//bsms
		{
			for (int bi=0;bi<IMPMAXCDEVBSM;bi++){
				if (!readInt(fh,(int*)&cdev.bsm[bi]))
					return FALSE;
			}
		}
		break;
	case mtDEVSYM:
		// ijfmt
		if (!readIJFMT(fh,&devsym.ijfmt))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&devsym.width))
			return FALSE;
		if (!readDouble(fh,&devsym.height))
			return FALSE;
		// text
		if (!readString(fh,devsym.text,IMPLDEVSYM+1))
			return FALSE;
		// xdots: added 8/18/2004
		if (ver == 0)  
		{
			if (!readWord(fh,&devsym.xdots))
				return FALSE;
		}
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
		}
		return TRUE;
}

BOOL IMPMARK::write(HANDLE fh){
	// Origin
	if (!writeDouble(fh,&origin.x))
		return FALSE;
	if (!writeDouble(fh,&origin.y))
		return FALSE;
	// Rotation
	if (!writeDouble(fh,&rotation))
		return FALSE;
	// Rotation degree	--sam	10/7/16
	
	//if (!writeDouble(fh,&deg.t))
		//return FALSE;
	// Slant
	if (!writeDouble(fh,&slant))
		return FALSE;
	// Z flight
	if (!writeDouble(fh,&zflight))
		return FALSE;
	// Justification
	if (!writeByte(fh,&justx))
		return FALSE;
	if (!writeByte(fh,&justy))
		return FALSE;
	// Rep
	if (!writeWord(fh,&rep))
		return FALSE;
	// Type
	if (!writeInt(fh,(int*)&type))
		return FALSE;
	// Type specific info
	switch (type){
	case mtTEXT:
		// ijfmt
		if (!writeIJFMT(fh,&t.ijfmt))
			return FALSE;
		// cw,ch
		if (!writeDouble(fh,&t.cw))
			return FALSE;
		if (!writeDouble(fh,&t.ch))
			return FALSE;
		// text
		if (!writeString(fh,t.text,IMPLTEXT+1))
			return FALSE;
		break;
	case mtDEVBC:
		// ijfmt
		if (!writeIJFMT(fh,&devbc.ijfmt))
			return FALSE;
		// cw,ch
		if (!writeDouble(fh,&devbc.cw))
			return FALSE;
		if (!writeDouble(fh,&devbc.ch))
			return FALSE;
		// text
		if (!writeString(fh,devbc.text,IMPLDEVBC+1))
			return FALSE;
		break;
	case mtBITMAP:
		if (!writeDouble(fh,&b.xres))
			return FALSE;
		if (!writeDouble(fh,&b.yres))
			return FALSE;
		//ms
		if (!writeWord(fh,&b.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&b.neg))
			return FALSE;
		//fn
		if (!writeString(fh,b.fn,IMPLBMFN+1))
			return FALSE;
		break;
	case mtC128:
		//xres, yres
		if (!writeDouble(fh,&c128.xres))
			return FALSE;
		if (!writeDouble(fh,&c128.yres))
			return FALSE;
		//dx, dy
		if (!writeInt(fh,&c128.dx))
			return FALSE;
		if (!writeInt(fh,&c128.dy))
			return FALSE;
		//qz
		if (!writeInt(fh,&c128.qz))
			return FALSE;
		//ms
		if (!writeWord(fh,&c128.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&c128.neg))
			return FALSE;
		//overlayhr
		if (!writeInt(fh,(int*) &c128.overlayhr))
			return FALSE;
		//text
		if (!writeString(fh,c128.text,IMPLC128+1))
			return FALSE;
		//hrpicture
		if (!writeString(fh,c128.hrpict,IMPLC128HRPICTURE+1))
			return FALSE;
		//cset
		if (!writeByte(fh,&c128.cset))
			return FALSE;
		//reserved
		if (!writeByte(fh,&c128.reserved))
			return FALSE;
		break;
	case mtCUPCEAN:
		//xres, yres
		if (!writeDouble(fh,&cupcean.xres))
			return FALSE;
		if (!writeDouble(fh,&cupcean.yres))
			return FALSE;
		//dx, dy
		if (!writeInt(fh,&cupcean.dx))
			return FALSE;
		if (!writeInt(fh,&cupcean.dy))
			return FALSE;
		//qz
		if (!writeInt(fh,&cupcean.qz))
			return FALSE;
		//ms
		if (!writeWord(fh,&cupcean.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&cupcean.neg))
			return FALSE;
		//subtype
		if (!writeInt(fh,&cupcean.subtype))
			return FALSE;
		//overlayhr
		if (!writeInt(fh,&cupcean.overlayhr))
			return FALSE;
		//hrpict
		if (!writeString(fh,cupcean.hrpict,IMPLCUPCEANHRPICTURE+1))
			return FALSE;
		//text
		if (!writeString(fh,cupcean.text,IMPLCUPCEAN+1))
			return FALSE;
		break;
	case mtC93:
		//xres, yres
		if (!writeDouble(fh,&c93.xres))
			return FALSE;
		if (!writeDouble(fh,&c93.yres))
			return FALSE;
		//dx, dy
		if (!writeInt(fh,&c93.dx))
			return FALSE;
		if (!writeInt(fh,&c93.dy))
			return FALSE;
		//qz
		if (!writeInt(fh,&c93.qz))
			return FALSE;
		//ms
		if (!writeWord(fh,&c93.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&c93.neg))
			return FALSE;
		//text
		if (!writeString(fh,c93.text,IMPLC93+1))
			return FALSE;
		break;
	case mtMULTILINE:
		if (!writeIJFMT(fh,&ml.ijfmt))
			return FALSE;
		// cw,ch
		if (!writeDouble(fh,&ml.cw))
			return FALSE;
		if (!writeDouble(fh,&ml.ch))
			return FALSE;
		// text
		if (!writeString(fh,ml.text,IMPLMLTEXT+1))
			return FALSE;
		break;
	//case uid12:                             //-----------------Linghai-11-21-8--
	//case uid6:                              //-----------------Linghai-11-21-8--
	//case uid5:                              //-----------------Linghai-11-21-08--
	case mtUID:                               //--------------Linghai-4-9-09----
		//xres, yres
		if (!writeDouble(fh,&uid.xres))
			return FALSE;
		if (!writeDouble(fh,&uid.yres))
			return FALSE;
		//ms
		if (!writeWord(fh,&uid.ms))
			return FALSE;
		//dminfo
		if (!writeDMINFO(fh,&uid.dminfo))
			return FALSE;
		//text
		if (!writeString(fh,uid.text,IMPLCDM+1))
			return FALSE;
		break;
	case mtCDM:
		//xres, yres
		if (!writeDouble(fh,&cdm.xres))
			return FALSE;
		if (!writeDouble(fh,&cdm.yres))
			return FALSE;
		//ms
		if (!writeWord(fh,&cdm.ms))
			return FALSE;
		//dminfo
		if (!writeDMINFO(fh,&cdm.dminfo))
			return FALSE;
		//text
		if (!writeString(fh,cdm.text,IMPLCDM+1))
			return FALSE;
		break;
	case mtWINFONT:
		//xres, yres
		if (!writeDouble(fh,&winfont.xres))
			return FALSE;
		if (!writeDouble(fh,&winfont.yres))
			return FALSE;
		//ms
		if (!writeWord(fh,&winfont.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&winfont.neg))
			return FALSE;
		//font
		if (!writeLOGFONT(fh,&winfont.logfont))
			return FALSE;
		//text
		if (!writeString(fh,winfont.text,IMPLWINFONT+1))
			return FALSE;
		//sef
		if (!writeByte(fh,&winfont.sef))
			return FALSE;
		//yorgm, yincm
		if (!writeInt(fh,&winfont.yorgm))
			return FALSE;
		if (!writeInt(fh,&winfont.yincm))
			return FALSE;
		break;

	case mtWMTRX:
		if (!writeDouble(fh,&wmtrx.xres)) return FALSE;
		if (!writeDouble(fh,&wmtrx.yres)) return FALSE;
		if (!writeWord(fh,&wmtrx.ms)) return FALSE;
		if (!writeInt(fh,&wmtrx.neg)) return FALSE;
		if (!writeInt(fh,&wmtrx.srcxs)) return FALSE;
		if (!writeInt(fh,&wmtrx.srcys)) return FALSE;
		if (!writeInt(fh,&wmtrx.subxo)) return FALSE;
		if (!writeInt(fh,&wmtrx.subyo)) return FALSE;
		if (!writeInt(fh,&wmtrx.subxs)) return FALSE;
		if (!writeInt(fh,&wmtrx.subys)) return FALSE;
		if (!writeInt(fh,&wmtrx.celxs)) return FALSE;
		if (!writeInt(fh,&wmtrx.celys)) return FALSE;
		if (!writeInt(fh,&wmtrx.dotxs)) return FALSE;
		if (!writeInt(fh,&wmtrx.dotys)) return FALSE;

		if (!writeInt(fh,&wmtrx.matxo)) return FALSE;
		if (!writeInt(fh,&wmtrx.matyo)) return FALSE;
		if (!writeInt(fh,&wmtrx.chxo)) return FALSE;
		if (!writeInt(fh,&wmtrx.chyo)) return FALSE;
		if (!writeInt(fh,&wmtrx.rhxo)) return FALSE;
		if (!writeInt(fh,&wmtrx.rhyo)) return FALSE;

		if (!writeInt(fh,&wmtrx.hex)) return FALSE;
		if (!writeString(fh,wmtrx.text,IMPLWMTRX+1)) return FALSE;
		if (!writeByte(fh,&wmtrx.flags)) return FALSE;

		if (!writeString(fh,wmtrx.chtext,IMPLWMTRXCHTEXT+1)) return FALSE;
		if (!writeString(fh,wmtrx.rhtext,IMPLWMTRXRHTEXT+1)) return FALSE;
		if (!writeLOGFONT(fh,&wmtrx.logfont)) return FALSE;

		break;

	case mtCQR:
		if (!writeDouble(fh,&cqr.xres)) return FALSE;
		if (!writeDouble(fh,&cqr.yres)) return FALSE;
		if (!writeWord(fh,&cqr.ms)) return FALSE;
		if (!writeInt(fh,&cqr.neg)) return FALSE;
		if (!writeInt(fh,&cqr.rot)) return FALSE;
		//rotation degree	--sam	10/10/16
		//if(!HideOptRotaryDEG || EditHasRotDeg)
		if (!writeDouble(fh,&cqr.deg)) return FALSE;
		if (!writeInt(fh,&cqr.xflip)) return FALSE;
		if (!writeInt(fh,&cqr.yflip)) return FALSE;
		if (!writeInt(fh,&cqr.xmul)) return FALSE;
		if (!writeInt(fh,&cqr.ymul)) return FALSE;
		if (!writeInt(fh,&cqr.xqz)) return FALSE;
		if (!writeInt(fh,&cqr.yqz)) return FALSE;
		if (!writeInt(fh,(int*)&cqr.enc)) return FALSE;
		if (!writeInt(fh,(int*)&cqr.cor)) return FALSE;
		if (!writeInt(fh,&cqr.ver)) return FALSE;
		if (!writeString(fh,cqr.text,IMPLCQR+1)) return FALSE;
		break;
	case mtPATCH:
		//xres, yres
		if (!writeDouble(fh,&patch.xres))
			return FALSE;
		if (!writeDouble(fh,&patch.yres))
			return FALSE;
		//ms
		if (!writeWord(fh,&patch.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&patch.neg))
			return FALSE;
		//size
		if (!writeInt(fh,(int*) &patch.size.x))
			return FALSE;
		if (!writeInt(fh,(int*) &patch.size.y))
			return FALSE;
		//res1, res2, res3
		if (!writeWord(fh,&patch.res1))
			return FALSE;
		if (!writeWord(fh,&patch.res2))
			return FALSE;
		if (!writeWord(fh,&patch.res3))
			return FALSE;
		//res4
		if (!writeInt(fh,(int*) &patch.res4.x))
			return FALSE;
		if (!writeInt(fh,(int*) &patch.res4.y))
			return FALSE;
		break;
	case mtC39:
		if (!writeDouble(fh,&c39.xres))
			return FALSE;
		if (!writeDouble(fh,&c39.yres))
			return FALSE;
		//dx, dy
		if (!writeInt(fh,&c39.dx))
			return FALSE;
		if (!writeInt(fh,&c39.dy))
			return FALSE;
		//qz
		if (!writeInt(fh,&c39.qz))
			return FALSE;
		//ms
		if (!writeWord(fh,&c39.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&c39.neg))
			return FALSE;
		//overlayhr
		if (!writeInt(fh,(int*)&c39.overlayhr))
			return FALSE;
		//fa
		if (!writeInt(fh,&c39.fa))
			return FALSE;
		//addsum
		if (!writeInt(fh,&c39.addsum))
			return FALSE;
		//text
		if (!writeString(fh,c39.text,IMPLC39+1))
			return FALSE;
		//hrpict
		if (!writeString(fh,c39.hrpict,IMPLC39HRPICTURE+1))
			return FALSE;
		break;
	case mtC2OF5:
		if (!writeDouble(fh,&c2of5.xres))
			return FALSE;
		if (!writeDouble(fh,&c2of5.yres))
			return FALSE;
		//dx, dy
		if (!writeInt(fh,&c2of5.dx))
			return FALSE;
		if (!writeInt(fh,&c2of5.dy))
			return FALSE;
		//qz
		if (!writeInt(fh,&c2of5.qz))
			return FALSE;
		//ms
		if (!writeWord(fh,&c2of5.ms))
			return FALSE;
		//neg
		if (!writeInt(fh,&c2of5.neg))
			return FALSE;
		//overlayhr
		if (!writeInt(fh,(int*)&c2of5.overlayhr))
			return FALSE;
		//interleave
		if (!writeInt(fh,&c2of5.interleave))
			return FALSE;
		//bbars
		if (!writeInt(fh,&c2of5.bbars))
			return FALSE;
		//addsum
		if (!writeInt(fh,&c2of5.addsum))
			return FALSE;
		//text
		if (!writeString(fh,c2of5.text,IMPLC2OF5+1))
			return FALSE;
		//hrpict
		if (!writeString(fh,c2of5.hrpict,IMPLC2OF5HRPICTURE+1))
			return FALSE;
		break;
	case mtCURE:
		//xsize
		if (!writeDouble(fh,&cure.xsize))
			return FALSE;
		//vel
		if (!writeDouble(fh,&cure.vel))
			return FALSE;
		//keepon
		if (!writeInt(fh,&cure.keepon))
			return FALSE;
		break;
	case mtCDEV:
		//bijfmt
		if (!writeIJFMT(fh,&cdev.bijfmt))
			return FALSE;
		//bfi
		if (!writeInt(fh,&cdev.bfi))
			return FALSE;
		//res
		if (!writeDouble(fh,&cdev.xres))
			return FALSE;
		if (!writeDouble(fh,&cdev.yres))
			return FALSE;
		//text
		if (!writeString(fh,cdev.text,IMPLCDEVTEXT+1))
			return FALSE;
		//nbsm
		if (!writeInt(fh,&cdev.nbsm))
			return FALSE;
		//bsms
		{
			for (int bi=0;bi<IMPMAXCDEVBSM;bi++){
				if (!writeInt(fh,(int*)&cdev.bsm[bi]))
					return FALSE;
			}
		}
		break;
		case mtDEVSYM:
		// ijfmt
		if (!writeIJFMT(fh,&devsym.ijfmt))
			return FALSE;
		// cw,ch
		if (!writeDouble(fh,&devsym.width))
			return FALSE;
		if (!writeDouble(fh,&devsym.height))
			return FALSE;
		// text
		if (!writeString(fh,devsym.text,IMPLDEVSYM+1))
			return FALSE;
		// xdots  -- New field on 8/18/2004
		if (!writeWord(fh,&devsym.xdots))
			return FALSE;
		break;
	default:
		IDError = IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;		
}

BOOL IMPMARK::translate(HANDLE fh, WORD ver){
	if (ver >341 && ver <=368)
		return translateVers343Thru368(fh,ver);	 
	else if (ver == 341)
		return translateVer341(fh,FALSE);	
	else if (ver == 339)
		return translateVer341(fh,TRUE);	
	else if (ver == 338)
		return translateVer338(fh);	
	else if (ver == 330 || ver == 300)
		return translateOriginalVersion(fh);	
	else
		return FALSE;
}

/***********************************************************************

  Versions 3.43-3.68 - These are Win16 versions, so set 3rd argument of 
  the ReadInt and ReadDouble functions to read them from the buffer as
  shorts and floats - o/w same format as native

************************************************************************/
BOOL IMPMARK::translateVers343Thru368(HANDLE fh, WORD ver){

	// Origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	// Rotation
	if (!readDouble(fh,&rotation,TRUE))
		return FALSE;
	// Rotation degree	--sam	10/7/16
	//	if (!readDouble(fh,&deg.t,TRUE))
		//return FALSE;
	// Slant
	if (!readDouble(fh,&slant,TRUE))
		return FALSE;
	// Z flight
	if (!readDouble(fh,&zflight,TRUE))
		return FALSE;
	// Justification
	if (!readByte(fh,&justx))
		return FALSE;
	if (!readByte(fh,&justy))
		return FALSE;
	// Rep
	if (!readWord(fh,&rep))
		return FALSE;
	// Type
	if (!readInt(fh,(int*)&type,TRUE))
		return FALSE;
	char buf[200];
	// Init the type members
	initByType();
	// Read type specific info
	switch (type){
	// Old union size is 166
	case mtTEXT:  // size: 145
		// ijfmt  - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&t.ijfmt,TRUE))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&t.cw,TRUE))
			return FALSE;
		if (!readDouble(fh,&t.ch,TRUE))
			return FALSE;
		// text
		if (!readString(fh,t.text,IMPLTEXT+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,21))  // 21
			return FALSE;
		break;
	case mtDEVBC:  // size: 81
		// ijfmt
		if (!readIJFMT(fh,&devbc.ijfmt,TRUE))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&devbc.cw,TRUE))
			return FALSE;
		if (!readDouble(fh,&devbc.ch,TRUE))
			return FALSE;
		// text
		if (!readString(fh,devbc.text,IMPLDEVBC+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,85))  // 85
			return FALSE;
		break;
	case mtBITMAP:  // size: 141
		//xres, yres
		if (!readDouble(fh,&b.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&b.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&b.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&b.neg,TRUE))
			return FALSE;
		//fn
		if (!readString(fh,b.fn,IMPLBMFN+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,25)) 
			return FALSE;
		break;
	case mtC128:  // size: 166
		//xres, yres
		if (!readDouble(fh,&c128.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&c128.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c128.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c128.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c128.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c128.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c128.neg,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,(int*) &c128.overlayhr,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c128.text,IMPLC128+1))
			return FALSE;
		//hrpict
		if (!readString(fh,c128.hrpict,IMPLC128HRPICTURE+1))
			return FALSE;
		IMPV343C128CSET cs;
		//cset and reserved used to be 3 and 5 bits
		if (!readByte(fh,(BYTE*) &cs))
			return FALSE;
		//cset, reserved
		c128.cset = cs.cset;
		c128.reserved = cs.reserved;
		break;
	case mtCUPCEAN:   // size: 136
		//xres, yres
		if (!readDouble(fh,&cupcean.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&cupcean.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&cupcean.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&cupcean.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&cupcean.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cupcean.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&cupcean.neg,TRUE))
			return FALSE;
		//subtype
		if (!readInt(fh,&cupcean.subtype,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,&cupcean.overlayhr,TRUE))
			return FALSE;
		//hrpict
		if (!readString(fh,cupcean.hrpict,IMPLCUPCEANHRPICTURE+1))
			return FALSE;
		//text
		if (!readString(fh,cupcean.text,IMPLCUPCEAN+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,30))  // 30
			return FALSE;
		break;
	case mtC93:  // size: 83
		//xres, yres
		if (!readDouble(fh,&c93.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&c93.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c93.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c93.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c93.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c93.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c93.neg,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c93.text,IMPLC93+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,83))  // 83
			return FALSE;
		break;
	case mtMULTILINE:		// size: 145
		// ijfmt - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&ml.ijfmt,TRUE))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&ml.cw,TRUE))
			return FALSE;
		if (!readDouble(fh,&ml.ch,TRUE))
			return FALSE;
		// text
		if (!readString(fh,ml.text,IMPLMLTEXT+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,21))  // 21
			return FALSE;
		break;
	//case uid12:                             //-----------------Linghai-11-21-8--
	//case uid6:                              //-----------------Linghai-11-21-8--
	//case uid5:                              //-----------------Linghai-11-21-08--
	//case mtUID:                               //-----------------Linghai-4-9-09---
	case mtCDM:  // size: 144
		//xres, yres
		if (!readDouble(fh,&cdm.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&cdm.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cdm.ms))
			return FALSE;
		//dminfo
		if (!readDMINFO(fh,&cdm.dminfo,TRUE))
			return FALSE;
		// 4 more bytes for dminfo.data pointer
		if (!readString(fh,buf,4))  // 4
			return FALSE;
		//text
		if (!readString(fh,cdm.text,IMPLCDM+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,22))  // 22
			return FALSE;
		break;
	case mtWINFONT:  //.size: 166
		//xres, yres
		if (!readDouble(fh,&winfont.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&winfont.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&winfont.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&winfont.neg,TRUE))
			return FALSE;
		//font
		if (!readLOGFONT(fh,&winfont.logfont,TRUE))
			return FALSE;
		//text
		if (!readString(fh,winfont.text,IMPLWINFONT+1))
			return FALSE;
		//sef, yorgm, yincm
		if (ver>347){  // Earlier versions: no sef,yincm,yorgm
			if (!readByte(fh,&winfont.sef))
				return FALSE;
			if (!readInt(fh,&winfont.yorgm,TRUE))
				return FALSE;
			if (!readInt(fh,&winfont.yincm,TRUE))
				return FALSE;
		} else {  // Read past the 5 bytes from the absence of sef,yincm,yorgm
			if (!readString(fh,buf,5))
				return FALSE;
		}
		// Versions 3.43 and 3.47 - sef,yincm,yorgm set by initByType()
		break;
	case mtPATCH:  // size: 26
		//xres, yres
		if (!readDouble(fh,&patch.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&patch.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&patch.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&patch.neg,TRUE))
			return FALSE;
		//size
		if (!readInt(fh,(int*) &patch.size.x,TRUE))
			return FALSE;
		if (!readInt(fh,(int*) &patch.size.y,TRUE))
			return FALSE;
		//res1, res2, res3
		if (!readWord(fh,&patch.res1))
			return FALSE;
		if (!readWord(fh,&patch.res2))
			return FALSE;
		if (!readWord(fh,&patch.res3))
			return FALSE;
		//res4
		if (!readInt(fh,(int*) &patch.res4.x,TRUE))
			return FALSE;
		if (!readInt(fh,(int*) &patch.res4.y,TRUE))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,140))  // 140
			return FALSE;
		break;
	case mtC39:  // size: 147
		//xres, yres
		if (!readDouble(fh,&c39.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&c39.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c39.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c39.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c39.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c39.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c39.neg,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,(int*)&c39.overlayhr,TRUE))
			return FALSE;
		//fa
		if (!readInt(fh,&c39.fa,TRUE))
			return FALSE;
		//addsum
		if (!readInt(fh,&c39.addsum,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c39.text,IMPLC39+1))
			return FALSE;
		//hrpict
		if (!readString(fh,c39.hrpict,IMPLC39HRPICTURE+1))
			return FALSE;
		
		// Read past rest of union
		if (!readString(fh,buf,17))  // 17 
			return FALSE;
		break;
	case mtC2OF5:  // size: 151
		if (!readDouble(fh,&c2of5.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&c2of5.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c2of5.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c2of5.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c2of5.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c2of5.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c2of5.neg,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,(int*)&c2of5.overlayhr,TRUE))
			return FALSE;
		//interleave
		if (!readInt(fh,&c2of5.interleave,TRUE))
			return FALSE;
		//bbars
		if (!readInt(fh,&c2of5.bbars,TRUE))
			return FALSE;
		//addsum
		if (!readInt(fh,&c2of5.addsum,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c2of5.text,IMPLC2OF5+1))
			return FALSE;
		//hrpict
		if (!readString(fh,c2of5.hrpict,IMPLC2OF5HRPICTURE+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,15))  // 15 
			return FALSE;
		break;
	case mtCURE:  // size: 10
		//xsize
		if (!readDouble(fh,&cure.xsize,TRUE))
			return FALSE;
		//vel
		if (!readDouble(fh,&cure.vel,TRUE))
			return FALSE;
		//keepon
		if (!readInt(fh,&cure.keepon,TRUE))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,156))  // 156 
			return FALSE;
		break;
	case mtCDEV:   // old size: 163
		//bijfmt
		if (!readIJFMT(fh,&cdev.bijfmt,TRUE))
			return FALSE;
		//bfi
		if (!readInt(fh,&cdev.bfi,TRUE))
			return FALSE;
		//res
		if (!readDouble(fh,&cdev.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&cdev.yres,TRUE))
			return FALSE;
		//text
		if (!readString(fh,cdev.text,IMPLCDEVTEXT+1))
			return FALSE;
		//nbsm
		if (!readInt(fh,&cdev.nbsm,TRUE))
			return FALSE;
		//bsms
		{
			for (int bi=0;bi<IMPMAXCDEVBSM;bi++){
				if (!readInt(fh,(int*)&cdev.bsm[bi]))  // Pointers were longs
					return FALSE;
			}
		}
		// Read past rest of union
		if (!readString(fh,buf,3))  
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
		}
		return TRUE;
}


/*
// Version 3.41 - Set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats

 --------- OLD STRUCTURES ----------
/*
typedef enum {mtV341TEXT,mtV341DEVBC,mtV341BITMAP,mtV341C128,mtV341CUPCEAN,
	mtV341C93,mtV341UCC128,mtV341MULTILINE,mtV341CDM,mtV341WINFONT,mtV341PATCH} IMPV341MARKTYPE;


typedef WORD IMPV341MARKJUST;

	{
	IJFMT ijfmt;           // Inkjet format (font,ms,xflip,yflip)
	float cw;              // Character width
	float ch;              // Character cell height //RESERVED
	char text[128+1]; // Mark text
	} IMPV341TEXTMARK;

	{
	IJFMT ijfmt;             // Inkjet format (font,ms,xflip,yflip)
	float cw;                // Character width
	float ch;              // Character cell height //RESERVED
	char text[64+1]; // DEVBC text
	} IMPV341DEVBCMARK;

	{
	float xres;           // X resolution in drops/inch (not including ms)
	float yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative code128 flag
	char text[63+1]; // C128 text
	BYTE cset : 2;         // 2 lsb of last byte
	BYTE reserved : 6;     // remainder of last byte reserved
	} IMPV341C128MARK;

	{
	float xres;           // X resolution in drops/inch (not including ms)
	float yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative flag
	int  subtype;         // UPCA|UPCE|EAN8
	BOOL overlayhr;       // Overlay human readable
	char hrpict[48+1]; // Human readable picture "(X)-XC" 
	char text[64+1]; // UPCEAN text
	} IMPV341CUPCEANMARK;

	{
	float xres;           // X resolution in drops/inch (not including ms)
	float yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative flag
	BOOL overlayhr;       // Overlay human readable
	char hrpict[48+1]; // Human readable picture "(X) XC" 
	char text[32+1]; // UCC128 text
	} IMPV341UCC128MARK;

	{
	float xres;           // X resolution in drops/inch (not including ms)
	float yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative code93 flag
	char text[64+1]; // C93 text
	} IMPV341C93MARK;

	{
	float xres;           // X resolution in drops/inch (not including ms)
	float yres;           // Y resolution in drops/inch
	WORD ms;              // Multistroke factor
	DMINFO dminfo;        // DM001 Info
	char text[105+1]; // uncompiled text
	} IMPV341CDMMARK;

	{
	float xres;      // X resolution in drops/inch (not including ms)
	float yres;      // Y resolution in drops/inch
	WORD ms;         // Multistroke factor
	BOOL neg;        // Negative image
	LOGFONT logfont; // The Windows logical font
	char text[80+1]; // uncompiled text
	BYTE sef;        // space expansion factor
	} IMPV341WINFONTMARK;

	{
	float xres;      // X resolution in drops/inch (not including ms)
	float yres;      // Y resolution in drops/inch
	WORD ms;         // Multistroke factor
	BOOL neg;        // Negative image
	POINT size;      // Patch size in pixels x and y
	WORD  res1;      // Reserved for fill style enum
	WORD  res2;      // Reserved for pen style enum
	WORD  res3;      // Reserved for type Rect Ellipse RoundRect
	POINT res4;      // Reserved for round rect corner ellipse
	} IMPV341PATCHMARK;

	{
	float xres;            // X resolution in drops/inch (not including ms)
	float yres;            // Y resolution in drops/inch
	WORD ms;               // Multistroke 1..IMPMAXMS (not supported!)
	BOOL neg;              // Negative bitmap flag
	char fn[128+1];   // Bitmap filename
	} IMPV341BITMAPMARK;

	{
	IJFMT ijfmt;             // Inkjet format (font,ms,xflip,yflip)
	float cw;                // Character cell width
	float ch;                // Character cell height //RESERVED
	char text[128+1]; // Mark text (with \n seperating lines)
	} IMPV341MULTILINEMARK;

	{
	P2 origin;              // Mark origin in program coordinates
	float rotation;         // Mark rotation in degrees from FaceCS //RESERVED
	float slant;            // Mark y axis slant in degrees from rotated FaceCS //RESERVED
	float zflight;          // ZFlight override if >0 //RESERVED
	IMPV341MARKJUST just;
	WORD rep;               // + in 341
	IMPV341MARKTYPE type;
	union
		{
		IMPV341TEXTMARK          t;
		IMPV341BITMAPMARK        b;
		IMPV341DEVBCMARK         devbc;
		IMPV341C128MARK       c128;
		IMPV341C93MARK         c93;
		IMPV341UCC128MARK   ucc128;
		IMPV341CUPCEANMARK cupcean;
		IMPV341CDMMARK         cdm; // + in 341
		IMPV341WINFONTMARK winfont; // + in 341
		IMPV341PATCHMARK     patch; // + in 341
		IMPV341MULTILINEMARK    ml;
		};
	} IMPV341MARK;
*/

BOOL IMPMARK::translateVer341(HANDLE fh, BOOL v339){
	

	BOOL bUCC = FALSE;  // UCC128 or C128
	
	// Origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	// Rotation
	if (!readDouble(fh,&rotation,TRUE))
		return FALSE;
	// Rotation degree	--sam	10/7/16
	//if (!readDouble(fh,&deg.t,TRUE))
		//return FALSE;
	// Slant
	if (!readDouble(fh,&slant,TRUE))
		return FALSE;
	// Z flight
	if (!readDouble(fh,&zflight,TRUE))
		return FALSE;
	
	// Justification
	WORD wtemp;
	if (!readWord(fh,&wtemp))
		return FALSE;
	justx = (BYTE) wtemp;
	// Rep
	if (!readWord(fh,&rep))
		return FALSE;
	// Type
	IMPV341MARKTYPE ty;
	if (!readInt(fh,(int*)&ty,TRUE))
		return FALSE;
	switch(ty){
	case mtV341TEXT:
		type = mtTEXT;
		break;
	case mtV341DEVBC:
		type = mtDEVBC;
		break;
	case mtV341BITMAP:
		type = mtBITMAP; 
		break;
	case mtV341C128:
		type = mtC128;
		break;
	case mtV341CUPCEAN:
		type = mtCUPCEAN;
		break;
	case mtV341C93:
		type = mtC93;
		break;
	case mtV341UCC128:
		bUCC=TRUE;
		type = mtC128;
		break;
	case mtV341MULTILINE:
		type = mtMULTILINE;
		break;
	case mtV341CDM:
		type = mtCDM;
		break;
	case mtV341WINFONT:
		type = mtWINFONT;
		break;
	case mtV341PATCH:
		type = mtPATCH;
		break;
	default:
		return FALSE;
	}
	char buf[200];
	// Init the type members
	initByType();
	// Read the type specific info
	switch (type){
		// Old union size is 145
	case mtTEXT:  // old size: 145
		// ijfmt  - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&t.ijfmt,TRUE))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&t.cw,TRUE))
			return FALSE;
		if (!readDouble(fh,&t.ch,TRUE))
			return FALSE;
		// text
		if (!readString(fh,t.text,128+1))
			return FALSE;
		break;
	case mtDEVBC:  // old size: 81
		// ijfmt
		if (!readIJFMT(fh,&devbc.ijfmt,TRUE))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&devbc.cw,TRUE))
			return FALSE;
		if (!readDouble(fh,&devbc.ch,TRUE))
			return FALSE;
		// text
		if (!readString(fh,devbc.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,64))  
			return FALSE;
		break;
	case mtBITMAP:  // old size: 141
		//xres, yres
		if (!readDouble(fh,&b.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&b.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&b.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&b.neg,TRUE))
			return FALSE;
		//fn
		if (!readString(fh,b.fn,IMPLBMFN+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,4)) 
			return FALSE;
		break;
	case mtC128:
        // if bUCC
		//    xres,yres,dx,dy,qz,ms,neg,overlayhr,hrpict(49),text(33) 
		//    old size = 102
		// otherwise
		//    xres,yres,dx,dy,qz,ms,neg,text(64),cset/reserved
		//    old size = 83

		if (bUCC) // v.341 UCC128 mark
		{
			//xres, yres
			if (!readDouble(fh,&c128.xres,TRUE))
				return FALSE;
			if (!readDouble(fh,&c128.yres,TRUE))
				return FALSE;
			//dx, dy
			if (!readInt(fh,&c128.dx,TRUE))
				return FALSE;
			if (!readInt(fh,&c128.dy,TRUE))
				return FALSE;
			//qz
			if (!readInt(fh,&c128.qz,TRUE))
				return FALSE;
			//ms
			if (!readWord(fh,&c128.ms))
				return FALSE;
			//neg
			if (!readInt(fh,&c128.neg,TRUE))
				return FALSE;
			//overlayhr
			if (!readInt(fh,(int*)&c128.overlayhr,TRUE))
				return FALSE;
			//hrpict
			if (!readString(fh,b.fn,48+1))
				return FALSE;
			//hrtext
			if (!readString(fh,b.fn,32+1))
				return FALSE;
			// Other fields are initialized by initByType()
			
			// Read past rest of union
			if (!readString(fh,buf,43)) 
				return FALSE;

		} else {
			//xres, yres
			if (!readDouble(fh,&c128.xres,TRUE))
				return FALSE;
			if (!readDouble(fh,&c128.yres,TRUE))
				return FALSE;
			//dx, dy
			if (!readInt(fh,&c128.dx,TRUE))
				return FALSE;
			if (!readInt(fh,&c128.dy,TRUE))
				return FALSE;
			//qz
			if (!readInt(fh,&c128.qz,TRUE))
				return FALSE;
			//ms
			if (!readWord(fh,&c128.ms))
				return FALSE;
			//neg
			if (!readInt(fh,&c128.neg,TRUE))
				return FALSE;
			//text
			if (!readString(fh,c128.text,63+1))
				return FALSE;
			//cset and reserved used to be 2 and 6 bits in v341 (not v339)
			//cset, reserved
			if (v339){
//				BYTE bt;
				if (!readByte(fh,&c128.cset))
					return FALSE;
			} else {
				IMPV341C128CSET cs;
				if (!readByte(fh,(BYTE*) &cs))
					return FALSE;
				c128.cset = cs.cset;	// 2 bits	
				c128.reserved = cs.reserved; // 6 bits
			}
			// Other fields are initialized by initByType()

			// Read past rest of union
			if (!readString(fh,buf,62))
				return FALSE;
		}
		break;
	case mtCUPCEAN:   // old size: 136
		//xres, yres
		if (!readDouble(fh,&cupcean.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&cupcean.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&cupcean.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&cupcean.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&cupcean.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cupcean.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&cupcean.neg,TRUE))
			return FALSE;
		//subtype
		if (!readInt(fh,&cupcean.subtype,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,&cupcean.overlayhr,TRUE))
			return FALSE;
		//hrpict
		if (!readString(fh,cupcean.hrpict,48+1))
			return FALSE;
		//text
		if (!readString(fh,cupcean.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,9)) 
			return FALSE;
		break;
	case mtC93:  // old size: 83
		//xres, yres
		if (!readDouble(fh,&c93.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&c93.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c93.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c93.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c93.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c93.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c93.neg,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c93.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,62))
			return FALSE;
		break;
	case mtMULTILINE:		// size: 145
		// ijfmt - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&ml.ijfmt,TRUE))
			return FALSE;
		// cw,ch
		if (!readDouble(fh,&ml.cw,TRUE))
			return FALSE;
		if (!readDouble(fh,&ml.ch,TRUE))
			return FALSE;
		// text
		if (!readString(fh,ml.text,128+1))
			return FALSE;
		break;
	//case uid12:                             //-----------------Linghai-11-21-8--
	//case uid6:                              //-----------------Linghai-11-21-8--
	//case uid5:                              //-----------------Linghai-11-21-08--
	//case uid:                                //-----------------Linghai-4-9-09--
	case mtCDM:  // old size: 144
		//xres, yres
		if (!readDouble(fh,&cdm.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&cdm.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cdm.ms))
			return FALSE;
		//dminfo
		if (!readDMINFO(fh,&cdm.dminfo,TRUE))
			return FALSE;
		// 4 more bytes for dminfo.data pointer
		if (!readString(fh,buf,4))  // 4
			return FALSE;
		//text
		if (!readString(fh,cdm.text,105+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,1)) 
			return FALSE;
		break;
	case mtWINFONT:  //old size: 144
		//xres, yres
		if (!readDouble(fh,&winfont.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&winfont.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&winfont.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&winfont.neg,TRUE))
			return FALSE;
		//font
		if (!readLOGFONT(fh,&winfont.logfont,TRUE))
			return FALSE;
		//text
		if (!readString(fh,winfont.text,80+1))
			return FALSE;
		//sef
		if (v339){   // no sef in v3.39
			// read past empty byte
			if (!readString(fh,buf,1))
				return FALSE;
		}
		// v 3.41 sef
		if (!readByte(fh,&winfont.sef))
			return FALSE;
		// one more byte in union
		if (!readString(fh,buf,1))
			return FALSE;
		//yorgm,yincm set by initByType()
		break;
	case mtPATCH:  // old size: 26
		//xres, yres
		if (!readDouble(fh,&patch.xres,TRUE))
			return FALSE;
		if (!readDouble(fh,&patch.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&patch.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&patch.neg,TRUE))
			return FALSE;
		//size
		if (!readInt(fh,(int*) &patch.size.x,TRUE))
			return FALSE;
		if (!readInt(fh,(int*) &patch.size.y,TRUE))
			return FALSE;
		//res1, res2, res3
		if (!readWord(fh,&patch.res1))
			return FALSE;
		if (!readWord(fh,&patch.res2))
			return FALSE;
		if (!readWord(fh,&patch.res3))
			return FALSE;
		//res4
		if (!readInt(fh,(int*) &patch.res4.x,TRUE))
			return FALSE;
		if (!readInt(fh,(int*) &patch.res4.y,TRUE))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,119)) 
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
		}
	return TRUE;
}

/*
// Version 3.38 - Set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats

 --------- OLD STRUCTURES ---------
/*
typedef enum {mtV338TEXT,mtV338DEVBC,mtV338BITMAP,mtV338C128,mtV338CUPCEAN,
	mtV338C93,mtV338UCC128,mtV338MULTILINE,
	// + in 338
	mtV338CDM,mtV338WINFONT,mtV338PATCH} IMPV338MARKTYPE;

typedef WORD IMPV338MARKJUST;

typedef struct
	{
	IJFMT ijfmt;
	float cw;
	char text[128+1];
	} IMPV338TEXTMARK;

typedef struct
	{
	IJFMT ijfmt;
	float cw;
	char text[64+1];
	} IMPV338DEVBCMARK;

typedef struct
	{
	int xres;
	int yres;
	int dx;
	int dy;
	int qz;
	WORD ms;
	BOOL neg;
	char text[64+1];
	} IMPV338C128MARK;

typedef struct
	{
	int xres;
	int yres;
	int dx;
	int dy;
	int qz;
	WORD ms;
	BOOL neg;
	int  subtype;
	BOOL overlayhr;
	char hrpict[48+1];
	char text[64+1];
	} IMPV338CUPCEANMARK;

typedef struct
	{
	int xres;
	int yres;
	int dx;
	int dy;
	int qz;
	WORD ms;
	BOOL neg;
	BOOL overlayhr;
	char hrpict[48+1];
	char text[32+1];
	} IMPV338UCC128MARK;

typedef struct
	{
	int xres;
	int yres;
	int dx;
	int dy;
	int qz;
	WORD ms;
	BOOL neg;
	char text[64+1];
	} IMPV338C93MARK;

// + in 338
typedef struct
	{
	int xres;
	int yres;
	WORD ms;
	DMINFO dminfo;
	char text[105+1];
	} IMPV338CDMMARK;

// + in 338
typedef struct
	{
	int xres;
	int yres;
	WORD ms;
	BOOL neg;
	LOGFONT logfont;
	char text[81+1];
	} IMPV338WINFONTMARK;

// + in 338
typedef struct
	{
	int xres;
	int yres;
	WORD ms;
	BOOL neg;
	POINT size;
	WORD  res1;
	WORD  res2;
	WORD  res3;
	POINT res4;
	} IMPV338PATCHMARK;

typedef struct
	{
	int xres;
	int yres;
	WORD ms;
	BOOL neg;
	char fn[128+1];
	} IMPV338BITMAPMARK;

typedef struct
	{
	IJFMT ijfmt;
	float cw;
	char text[128+1];
	} IMPV338MULTILINEMARK;

typedef struct
	{
	P2 origin;
	float rotation;         // + in 338
	float slant;            // + in 338
	IMPV338MARKJUST just;
	WORD rep;               // + in 338
	IMPV338MARKTYPE type;
	union
		{
		IMPV338TEXTMARK          t;
		IMPV338BITMAPMARK        b;
		IMPV338DEVBCMARK         devbc;
		IMPV338C128MARK       c128;
		IMPV338C93MARK         c93;
		IMPV338UCC128MARK   ucc128;
		IMPV338CUPCEANMARK cupcean;
		IMPV338CDMMARK         cdm; // + in 338
		IMPV338WINFONTMARK winfont; // + in 338
		IMPV338PATCHMARK     patch; // + in 338
		IMPV338MULTILINEMARK    ml;
		};
	} IMPV338MARK;
*/

BOOL IMPMARK::translateVer338(HANDLE fh){
	
	BOOL bUCC = FALSE;  // UCC128 or C128
	
	// Origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	// Rotation
	if (!readDouble(fh,&rotation,TRUE))
		return FALSE;
	// Rotation degree	--sam	10/7/16
		//if (!readDouble(fh,&deg.t,TRUE))
		//return FALSE;
	// Slant
	if (!readDouble(fh,&slant,TRUE))
		return FALSE;
	// Justification
	WORD wtemp;
	if (!readWord(fh,&wtemp))
		return FALSE;
	justx = (BYTE) wtemp;
	// Rep
	if (!readWord(fh,&rep))
		return FALSE;
	// Type
	IMPV338MARKTYPE ty;
	if (!readInt(fh,(int*)&ty,TRUE))
		return FALSE;
	switch(ty){
	case mtV338TEXT:
		type = mtTEXT;
		break;
	case mtV338DEVBC:
		type = mtDEVBC;
		break;
	case mtV338BITMAP:
		type = mtBITMAP; 
		break;
	case mtV338C128:
		type = mtC128;
		break;
	case mtV338CUPCEAN:
		type = mtCUPCEAN;
		break;
	case mtV338C93:
		type = mtC93;
		break;
	case mtV338UCC128:
		bUCC=TRUE;
		type = mtC128;
		break;
	case mtV338MULTILINE:
		type = mtMULTILINE;
		break;
	case mtV338CDM:
		type = mtCDM;
		break;
	case mtV338WINFONT:
		type = mtWINFONT;
		break;
	case mtV338PATCH:
		type = mtPATCH;
		break;
	default:
		return FALSE;
	}
	char buf[200];
	// Init the type members
	initByType();
	// Read the type specific info
	switch (type){
		// Old union size is 141
	case mtTEXT:  // old size: 141
		// ijfmt  - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&t.ijfmt,TRUE))
			return FALSE;
		// cw
		if (!readDouble(fh,&t.cw,TRUE))
			return FALSE;
		// text
		if (!readString(fh,t.text,128+1))
			return FALSE;
		
		break;
	case mtDEVBC:  // old size: 77
		// ijfmt
		if (!readIJFMT(fh,&devbc.ijfmt,TRUE))
			return FALSE;
		// cw
		if (!readDouble(fh,&devbc.cw,TRUE))
			return FALSE;
		// text
		if (!readString(fh,devbc.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,64))
			return FALSE;
		break;
	case mtBITMAP:  // old size: 137
		//xres, yres
		if (!readInt(fh,(int*)&b.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&b.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&b.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&b.neg,TRUE))
			return FALSE;
		//fn
		if (!readString(fh,b.fn,IMPLBMFN+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,4))
			return FALSE;
		break;
	case mtC128:
        // if bUCC
		//    xres,yres,dx,dy,qz,ms,neg,overlayhr,hrpict(49),text(33) 
		//    old size = 98
		// otherwise
		//    xres,yres,dx,dy,qz,ms,neg,text(64)
		//    old size = 79

		if (bUCC) // v.341 UCC128 mark
		{
			//xres, yres
			if (!readInt(fh,(int*)&c128.xres,TRUE))
				return FALSE;
			if (!readInt(fh,(int*)&c128.yres,TRUE))
				return FALSE;
			//dx, dy
			if (!readInt(fh,&c128.dx,TRUE))
				return FALSE;
			if (!readInt(fh,&c128.dy,TRUE))
				return FALSE;
			//qz
			if (!readInt(fh,&c128.qz,TRUE))
				return FALSE;
			//ms
			if (!readWord(fh,&c128.ms))
				return FALSE;
			//neg
			if (!readInt(fh,&c128.neg,TRUE))
				return FALSE;
			//overlayhr
			if (!readInt(fh,(int*)&c128.overlayhr,TRUE))
				return FALSE;
			//hrpict
			if (!readString(fh,c128.hrpict,48+1))
				return FALSE;
			//text
			if (!readString(fh,c128.text,32+1))
				return FALSE;
			// Other fields are initialized by initByType()
			
			// Read past rest of union
			if (!readString(fh,buf,43))
				return FALSE;

			
		} else {
			//xres, yres
			if (!readInt(fh,(int*)&c128.xres,TRUE))
				return FALSE;
			if (!readInt(fh,(int*)&c128.yres,TRUE))
				return FALSE;
			//dx, dy
			if (!readInt(fh,&c128.dx,TRUE))
				return FALSE;
			if (!readInt(fh,&c128.dy,TRUE))
				return FALSE;
			//qz
			if (!readInt(fh,&c128.qz,TRUE))
				return FALSE;
			//ms
			if (!readWord(fh,&c128.ms))
				return FALSE;
			//neg
			if (!readInt(fh,&c128.neg,TRUE))
				return FALSE;
			//text
			if (!readString(fh,c128.text,63+1))
				return FALSE;
			
			// Other fields are initialized by initByType()

			// Read past rest of union
			if (!readString(fh,buf,62))
				return FALSE;
		}
		break;
	case mtCUPCEAN:   // old size: 132
		//xres, yres
		if (!readInt(fh,(int*)&cupcean.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&cupcean.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&cupcean.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&cupcean.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&cupcean.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cupcean.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&cupcean.neg,TRUE))
			return FALSE;
		//subtype
		if (!readInt(fh,&cupcean.subtype,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,&cupcean.overlayhr,TRUE))
			return FALSE;
		//hrpict
		if (!readString(fh,cupcean.hrpict,48+1))
			return FALSE;
		//text
		if (!readString(fh,cupcean.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,9)) 
			return FALSE;
		break;
	case mtC93:  // old size: 79
		//xres, yres
		if (!readInt(fh,(int*)&c93.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&c93.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c93.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c93.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c93.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c93.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c93.neg,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c93.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,62))
			return FALSE;
		break;
	case mtMULTILINE:		// size: 141
		// ijfmt - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&ml.ijfmt,TRUE))
			return FALSE;
		// cw
		if (!readDouble(fh,&ml.cw,TRUE))
			return FALSE;
		// text
		if (!readString(fh,ml.text,128+1))
			return FALSE;
		break;
	//case uid12:                             //-----------------Linghai-11-21-8--
	//case uid6:                              //-----------------Linghai-11-21-8--
	//case uid5:                              //-----------------Linghai-11-21-08--
	//case uid:                               //--------------Linghai-4-9-09---
	case mtCDM:  // old size: 140
		//xres, yres
		if (!readInt(fh,(int*)&cdm.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&cdm.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cdm.ms))
			return FALSE;
		//dminfo
		if (!readDMINFO(fh,&cdm.dminfo,TRUE))
			return FALSE;
		// 4 more bytes for dminfo.data pointer
		if (!readString(fh,buf,4))  // 4
			return FALSE;
		//text
		if (!readString(fh,cdm.text,105+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,1)) 
			return FALSE;
		break;
	case mtWINFONT:  //old size: 140

		//xres, yres
		if (!readInt(fh,(int*)&winfont.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&winfont.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&winfont.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&winfont.neg,TRUE))
			return FALSE;
		//font
		if (!readLOGFONT(fh,&winfont.logfont,TRUE))
			return FALSE;
		//text
		if (!readString(fh,winfont.text,80+1))
			return FALSE;
		
		// read to end of union
		if (!readString(fh,buf,1)) 
			return FALSE;

		// other fields set by initByType()
		break;
	case mtPATCH:  // old size: 22
		//xres, yres
		if (!readInt(fh,(int*)&patch.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&patch.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&patch.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&patch.neg,TRUE))
			return FALSE;
		//size
		if (!readInt(fh,(int*) &patch.size.x,TRUE))
			return FALSE;
		if (!readInt(fh,(int*) &patch.size.y,TRUE))
			return FALSE;
		//res1, res2, res3
		if (!readWord(fh,&patch.res1))
			return FALSE;
		if (!readWord(fh,&patch.res2))
			return FALSE;
		if (!readWord(fh,&patch.res3))
			return FALSE;
		//res4
		if (!readInt(fh,(int*) &patch.res4.x,TRUE))
			return FALSE;
		if (!readInt(fh,(int*) &patch.res4.y,TRUE))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,119)) 
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
		}
	return TRUE;
}

/*
// Version 3.30 and earlier - Set 3rd argument of 
// the ReadInt and ReadDouble functions to read them from the buffer as
// shorts and floats

 --------- OLD STRUCTURES ---------
/*
typedef enum {mtV330TEXT,mtV330DEVBC,mtV330BITMAP,mtV330C128,mtV330CUPCEAN,
	mtV330C93,mtV330UCC128,mtV330MULTILINE} IMPV330MARKTYPE;

typedef WORD IMPV330MARKJUST;

typedef struct
	{
	IJFMT ijfmt;           
	float cw;              
	char text[128+1]; 
	} IMPV330TEXTMARK;

typedef struct
	{
	IJFMT ijfmt;             
	float cw;                
	char text[64+1]; 
	} IMPV330DEVBCMARK;

typedef struct
	{
	int xres;             
	int yres;             
	int dx;               
	int dy;               
	int qz;               
	WORD ms;              
	BOOL neg;             
	char text[64+1]; 
	} IMPV330C128MARK;

typedef struct
	{
	int xres;             
	int yres;             
	int dx;               
	int dy;               
	int qz;               
	WORD ms;              
	BOOL neg;             
	int  subtype;         
	BOOL overlayhr;       
	char hrpict[48+1]; 
	char text[64+1]; 
	} IMPV330CUPCEANMARK;

typedef struct
	{
	int xres;             
	int yres;             
	int dx;               
	int dy;               
	int qz;               
	WORD ms;              
	BOOL neg;             
	BOOL overlayhr;       
	char hrpict[48+1]; 
	char text[32+1]; 
	} IMPV330UCC128MARK;

typedef struct
	{
	int xres;             
	int yres;             
	int dx;               
	int dy;               
	int qz;               
	WORD ms;              
	BOOL neg;             
	char text[64+1]; 
	} IMPV330C93MARK;

typedef struct
	{
	int xres;              
	int yres;              
	WORD ms;               
	BOOL neg;              
	char fn[128+1];   
	} IMPV330BITMAPMARK;

typedef struct
	{
	IJFMT ijfmt;             
	float cw;                
	char text[128+1]; 
	} IMPV330MULTILINEMARK;

typedef struct
	{
	P2 origin;              
	IMPV330MARKJUST just;       
	IMPV330MARKTYPE type;       
	union
		{
		IMPV330TEXTMARK          t; 
		IMPV330BITMAPMARK        b; 
		IMPV330DEVBCMARK         devbc; 
		IMPV330C128MARK       c128; 
		IMPV330C93MARK         c93; 
		IMPV330UCC128MARK   ucc128; 
		IMPV330CUPCEANMARK cupcean; 
		IMPV330MULTILINEMARK    ml; 
		};
	} IMPV330MARK;
*/

BOOL IMPMARK::translateOriginalVersion(HANDLE fh){
	
	BOOL bUCC = FALSE;  // UCC128 or C128
	
	// Origin
	if (!readDouble(fh,&origin.x,TRUE))
		return FALSE;
	if (!readDouble(fh,&origin.y,TRUE))
		return FALSE;
	// Justification
	WORD wtemp;
	if (!readWord(fh,&wtemp))
		return FALSE;
	justx = (BYTE) wtemp;
	// Type
	IMPV330MARKTYPE ty;
	if (!readInt(fh,(int*)&ty,TRUE))
		return FALSE;
	switch(ty){
	case mtV330TEXT:
		type = mtTEXT;
		break;
	case mtV330DEVBC:
		type = mtDEVBC;
		break;
	case mtV330BITMAP:
		type = mtBITMAP; 
		break;
	case mtV330C128:
		type = mtC128;
		break;
	case mtV330CUPCEAN:
		type = mtCUPCEAN;
		break;
	case mtV330C93:
		type = mtC93;
		break;
	case mtV330UCC128:
		bUCC=TRUE;
		type = mtC128;
		break;
	case mtV330MULTILINE:
		type = mtMULTILINE;
		break;
	default:
		return FALSE;
	}
	char buf[200];
	// Init the type members
	initByType();
	// Read the type specific info
	switch (type){
		// Old union size is 141
	case mtTEXT:  // old size: 141
		// ijfmt  - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&t.ijfmt,TRUE))
			return FALSE;
		// cw
		if (!readDouble(fh,&t.cw,TRUE))
			return FALSE;
		// text
		if (!readString(fh,t.text,128+1))
			return FALSE;
		
		break;
	case mtDEVBC:  // old size: 77
		// ijfmt
		if (!readIJFMT(fh,&devbc.ijfmt,TRUE))
			return FALSE;
		// cw
		if (!readDouble(fh,&devbc.cw,TRUE))
			return FALSE;
		// text
		if (!readString(fh,devbc.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,64))
			return FALSE;
		break;
	case mtBITMAP:  // old size: 137
		//xres, yres
		if (!readInt(fh,(int*)&b.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&b.yres,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&b.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&b.neg,TRUE))
			return FALSE;
		//fn
		if (!readString(fh,b.fn,IMPLBMFN+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,4))
			return FALSE;
		break;
	case mtC128:
        // if bUCC
		//    xres,yres,dx,dy,qz,ms,neg,overlayhr,hrpict(49),text(33) 
		//    old size = 98
		// otherwise
		//    xres,yres,dx,dy,qz,ms,neg,text(64)
		//    old size = 79

		if (bUCC) // v.341 UCC128 mark
		{
			//xres, yres
			if (!readInt(fh,(int*)&c128.xres,TRUE))
				return FALSE;
			if (!readInt(fh,(int*)&c128.yres,TRUE))
				return FALSE;
			//dx, dy
			if (!readInt(fh,&c128.dx,TRUE))
				return FALSE;
			if (!readInt(fh,&c128.dy,TRUE))
				return FALSE;
			//qz
			if (!readInt(fh,&c128.qz,TRUE))
				return FALSE;
			//ms
			if (!readWord(fh,&c128.ms))
				return FALSE;
			//neg
			if (!readInt(fh,&c128.neg,TRUE))
				return FALSE;
			//overlayhr
			if (!readInt(fh,(int*)&c128.overlayhr,TRUE))
				return FALSE;
			//hrpict
			if (!readString(fh,c128.hrpict,48+1))
				return FALSE;
			//text
			if (!readString(fh,c128.text,32+1))
				return FALSE;
			// Other fields are initialized by initByType()
			
			// Read past rest of union
			if (!readString(fh,buf,43))
				return FALSE;

			
		} else {
			//xres, yres
			if (!readInt(fh,(int*)&c128.xres,TRUE))
				return FALSE;
			if (!readInt(fh,(int*)&c128.yres,TRUE))
				return FALSE;
			//dx, dy
			if (!readInt(fh,&c128.dx,TRUE))
				return FALSE;
			if (!readInt(fh,&c128.dy,TRUE))
				return FALSE;
			//qz
			if (!readInt(fh,&c128.qz,TRUE))
				return FALSE;
			//ms
			if (!readWord(fh,&c128.ms))
				return FALSE;
			//neg
			if (!readInt(fh,&c128.neg,TRUE))
				return FALSE;
			//text
			if (!readString(fh,c128.text,64+1))
				return FALSE;
			
			// Other fields are initialized by initByType()

			// Read past rest of union
			if (!readString(fh,buf,62))
				return FALSE;
		}
		break;
	case mtCUPCEAN:   // old size: 132
		//xres, yres
		if (!readInt(fh,(int*)&cupcean.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&cupcean.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&cupcean.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&cupcean.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&cupcean.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&cupcean.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&cupcean.neg,TRUE))
			return FALSE;
		//subtype
		if (!readInt(fh,&cupcean.subtype,TRUE))
			return FALSE;
		//overlayhr
		if (!readInt(fh,&cupcean.overlayhr,TRUE))
			return FALSE;
		//hrpict
		if (!readString(fh,cupcean.hrpict,48+1))
			return FALSE;
		//text
		if (!readString(fh,cupcean.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,9)) 
			return FALSE;
		break;
	case mtC93:  // old size: 79
		//xres, yres
		if (!readInt(fh,(int*)&c93.xres,TRUE))
			return FALSE;
		if (!readInt(fh,(int*)&c93.yres,TRUE))
			return FALSE;
		//dx, dy
		if (!readInt(fh,&c93.dx,TRUE))
			return FALSE;
		if (!readInt(fh,&c93.dy,TRUE))
			return FALSE;
		//qz
		if (!readInt(fh,&c93.qz,TRUE))
			return FALSE;
		//ms
		if (!readWord(fh,&c93.ms))
			return FALSE;
		//neg
		if (!readInt(fh,&c93.neg,TRUE))
			return FALSE;
		//text
		if (!readString(fh,c93.text,64+1))
			return FALSE;
		// Read past rest of union
		if (!readString(fh,buf,62))
			return FALSE;
		break;
	case mtMULTILINE:		// size: 141
		// ijfmt - 3rd param= TRUE: Read as 16-bit version 
		if (!readIJFMT(fh,&ml.ijfmt,TRUE))
			return FALSE;
		// cw
		if (!readDouble(fh,&ml.cw,TRUE))
			return FALSE;
		// text
		if (!readString(fh,ml.text,128+1))
			return FALSE;
		break;
	default:
		IDError = IDE_READFILE;
		return FALSE;
		}
	return TRUE;
}



/******************************************************************************
	IMPMARKS CLASS
******************************************************************************/
IMPMARKS::IMPMARKS()
{
	count = 0;
	capac = 0;
	mark = NULL;
}

IMPMARKS::IMPMARKS(int initCapac)
{
	count = 0;
	capac = initCapac;

	if (initCapac>0) 
		mark = new IMPMARK[initCapac];
	else
		mark = NULL;
}

IMPMARKS::~IMPMARKS()
	{
		free();
	}

void IMPMARKS::free(){
	if (mark){
		delete [] mark;
		mark=0;
	}
}

BOOL IMPMARKS::realloc(int ncount){
	if (ncount <= 0)
	{
		free();
		count=0;
		capac=0;
		return FALSE; 
	}
	
	// Create the new array
	IMPMARK * newArr = new IMPMARK[ncount];
	
	count=min(count,ncount);
	capac=ncount;
	
	for(int i=0;i<count;i++){
		newArr[i] = mark[i];
	}

	// Free the old array and assign new one
	free();

	mark = newArr;
	return (mark!=NULL);
}

BOOL IMPMARKS::write(HANDLE fh){
	
	DWORD bw;
	// Write count
	WriteFile(fh,(LPSTR)&count,sizeof(WORD),&bw,NULL);
	if (bw != sizeof(WORD))
		return FALSE;
	
	// Write the array
	for (int i=0;i<count;i++){
		if (!mark[i].write(fh))
			return FALSE;
	}
	
	return TRUE;
}

IMPMARKS::IMPMARKS(const IMPMARKS & src)
	{
	count = src.count;
	capac = src.capac;
	mark = NULL;

	if (capac>0){
		mark = new IMPMARK[capac];
	} 

	for (int i=0;i<count;i++){
		mark[i] = src.mark[i];
	}

	}

IMPMARKS& IMPMARKS::operator = (const IMPMARKS & src)
{
	free();

	count = src.count;
	capac = src.capac;

	if (capac>0){
		mark = new IMPMARK[capac];
	} 
	
	for (int i=0;i<count;i++){
		mark[i] = src.mark[i];
	}
	
	return *this;
}

BOOL IMPMARKS::read(HANDLE fh,WORD ver,WORD cnt){
	if (IMPIsNativeVersion(ver))
		return readnative(fh,ver,cnt);
	else 
		return translate(fh,ver,cnt);
}



BOOL IMPMARKS::readnative(HANDLE fh,WORD ver,WORD cnt){
	for (int i=0;i<cnt;i++){
		if (!mark[i].readnative(fh,ver))
			return FALSE;
	}
	count=cnt;
	return TRUE;
}

BOOL IMPMARKS::translate(HANDLE fh,WORD ver,WORD cnt){
	for (int i=0;i<cnt;i++){
		if (!mark[i].translate(fh,ver))
			return FALSE;
	}
	count=cnt;
	return TRUE;
}



