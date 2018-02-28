#include "stdafx.h"
#include "imsio.h"

/******************************************************************************
	readInt(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readInt(HANDLE fh,int * res,BOOL bShort)
	{
	IMSINT i;
	int rlen;
	if (bShort){  // Read it as a short
		rlen=_lread32(fh,(LPSTR)&i.sVal,2);
		if(rlen!=2)
		{
			IDError=IDE_READFILE;
			return FALSE;
		}
		*res = i.sVal; 
	} else {  // Read it as a long
		rlen=_lread32(fh,(LPSTR)&i.lVal,4);
		if(rlen!=4)
		{
			IDError=IDE_READFILE;
			return FALSE;
		}
		*res = i.lVal;
	}
	return TRUE;
}

/******************************************************************************
	readWord(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readWord(HANDLE fh,WORD * res)
	{
	int rlen=_lread32(fh,(LPSTR)res,2);
	if(rlen!=2)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	readByte(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readByte(HANDLE fh,BYTE * res)
	{
	int rlen=_lread32(fh,(LPSTR)res,1);
	if(rlen!=1)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	readFloat(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readFloat(HANDLE fh,float* res)
	{
	int rlen=_lread32(fh,(LPSTR)res,4);
	if(rlen!=4)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	readDouble(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readDouble(HANDLE fh,double* res,BOOL bFloat)
	{
	IMSDOUBLE d;
	int rlen;
	if (bFloat){  // Read it as a float
		rlen=_lread32(fh,(LPSTR)&d.fVal,4);
		if(rlen!=4)
		{
			IDError=IDE_READFILE;
			return FALSE;
		}
		*res = d.fVal; 
	} else {  // Read it as a double
		rlen=_lread32(fh,(LPSTR)&d.fVal,8);
		if(rlen!=8)
		{
			IDError=IDE_READFILE;
			return FALSE;
		}
		*res = d.dVal;
	}
	return TRUE;
	}

/******************************************************************************
	readString(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readString(HANDLE fh,void *rbuf,int rsize)
	{
	int rlen=_lread32(fh,(LPSTR)rbuf,rsize);
	if(rlen!=rsize)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	readDoubleWord(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readDoubleWord(HANDLE fh,DWORD * res)
	{
	int rlen=_lread32(fh,(LPSTR)res,4);
	if(rlen!=4)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	readChar(fh,rbuf,rsize)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL readChar(HANDLE fh,char * res)
	{
	int rlen=_lread32(fh,(LPSTR)res,1);
	if(rlen!=1)
		{
		IDError=IDE_READFILE;
		return FALSE;
		}
	return TRUE;
	}


/******************************************************************************
	readIJFMT(fh,rbuf,bShort)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
	
	bShort==TRUE: ints will be read in as short ints 
******************************************************************************/
BOOL readIJFMT(HANDLE fh,IJFMT * res,BOOL bShort)
{
	// Font
	if (!readInt(fh,&res->font,bShort))
		return FALSE;
	// MS
	if (!readWord(fh,&res->ms))
		return FALSE;
	// xflip,yflip
	if (!readInt(fh,&res->xflip,bShort))
		return FALSE;
	if (!readInt(fh,&res->yflip,bShort))
		return FALSE;
	return TRUE;
}

/******************************************************************************
	readDMINFO(fh,res,bShort)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
	
	bShort==TRUE: ints will be read in as short ints 
******************************************************************************/
BOOL readDMINFO(HANDLE fh,DMINFO * res,BOOL bShort)
{
	//neg
	if (!readInt(fh,&res->neg,bShort))
		return FALSE;
	//xmul, ymul
	if (!readInt(fh,&res->xmul,bShort))
		return FALSE;
	if (!readInt(fh,&res->ymul,bShort))
		return FALSE;
	//xqz, yqz
	if (!readInt(fh,&res->xqz,bShort))
		return FALSE;
	if (!readInt(fh,&res->yqz,bShort))
		return FALSE;
	if (!bShort){  // Win16 - No rotation,xflip,yflip
		//rotation
		if (!readInt(fh,&res->rotation,bShort))
			return FALSE;
		//xflip, yflip
		if (!readInt(fh,&res->xflip,bShort))
			return FALSE;
		if (!readInt(fh,&res->yflip,bShort))
			return FALSE;
	} else {
		res->rotation=0; // Win16 - Assign defaults
		res->xflip=0;
		res->yflip=0;
	}
	//format
	if (!readWord(fh,&res->format))
		return FALSE;
	//eccval
	if (!readInt(fh,(int*)&res->eccval,bShort))
		return FALSE;
	//rows, cols
	if (!readWord(fh,&res->rows))
		return FALSE;
	if (!readWord(fh,&res->cols))
		return FALSE;
	//style
	if (!readInt(fh,(int*)&res->style,bShort))
		return FALSE;
	//border
	if (!readWord(fh,&res->border))
		return FALSE;
	//encrypt
	if (!readInt(fh,(int*)&res->encrypt,bShort))
		return FALSE;
	//data (NULL)
	res->data = NULL;
	return TRUE;
}

/******************************************************************************
	readLOGFONT(fh,res,bShort)
	warps the _lread function to detect error and set IDError and simplify
	caller expressions
	 
	bShort==TRUE: ints will be read in as short ints 
******************************************************************************/
BOOL readLOGFONT(HANDLE fh,LOGFONT * res,BOOL bShort)
{
	// w,h
	if (!readInt(fh,(int*)&res->lfHeight,bShort))
		return FALSE;
	if (!readInt(fh,(int*)&res->lfWidth,bShort))
		return FALSE;
	// escapement
	if (!readInt(fh,(int*)&res->lfEscapement,bShort))
		return FALSE;
	// orientation
	if (!readInt(fh,(int*)&res->lfOrientation,bShort))
		return FALSE;
	// wt
	if (!readInt(fh,(int*)&res->lfWeight,bShort))
		return FALSE;
	// italic, underline, strikeout
	if (!readByte(fh,&res->lfItalic))
		return FALSE;
	if (!readByte(fh,&res->lfUnderline))
		return FALSE;
	if (!readByte(fh,&res->lfStrikeOut))
		return FALSE;
	// charset
	if (!readByte(fh,&res->lfCharSet))
		return FALSE;
	// precision
	if (!readByte(fh,&res->lfOutPrecision))
		return FALSE;
	if (!readByte(fh,&res->lfClipPrecision))
		return FALSE;
	// quality
	if (!readByte(fh,&res->lfQuality))
		return FALSE;
	// pitch
	if (!readByte(fh,&res->lfPitchAndFamily))
		return FALSE;
	// face name
	if (!readString(fh,&res->lfFaceName,LF_FACESIZE))
		return FALSE;
	return TRUE;
}

/******************************************************************************
	writeInt(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeInt(HANDLE fh,int * res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,4);
	if(wlen!=4)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeWord(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeWord(HANDLE fh,WORD * res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,2);
	if(wlen!=2)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeDoubleWord(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeDoubleWord(HANDLE fh,DWORD * res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,4);
	if(wlen!=4)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeByte(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeByte(HANDLE fh,BYTE * res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,1);
	if(wlen!=1)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeChar(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeChar(HANDLE fh,char * res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,1);
	if(wlen!=1)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeFloat(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeFloat(HANDLE fh,float* res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,4);
	if(wlen!=4)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeDouble(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeDouble(HANDLE fh,double* res)
	{
	int wlen=_lwrite32(fh,(LPSTR)res,8);
	if(wlen!=8)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeString(fh,rbuf,rsize)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
******************************************************************************/
BOOL writeString(HANDLE fh,void *wbuf,int wsize)
	{
	int wlen=_lwrite32(fh,(LPSTR)wbuf,wsize);
	if(wlen!=wsize)
		{
		IDError=IDE_WRITEFILE;
		return FALSE;
		}
	return TRUE;
	}

/******************************************************************************
	writeIJFMT(fh,rbuf)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
	 
******************************************************************************/
BOOL writeIJFMT(HANDLE fh,IJFMT * res)
	{
	// Font
	if (!writeInt(fh,&res->font))
		return FALSE;
	// MS
	if (!writeWord(fh,&res->ms))
		return FALSE;
	// xflip,yflip
	if (!writeInt(fh,&res->xflip))
		return FALSE;
	if (!writeInt(fh,&res->yflip))
		return FALSE;
	return TRUE;
	}

/******************************************************************************
	writeDMINFO(fh,res)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
	
******************************************************************************/
BOOL writeDMINFO(HANDLE fh,DMINFO * res)
{
	//neg
	if (!writeInt(fh,&res->neg))
		return FALSE;
	//xmul, ymul
	if (!writeInt(fh,&res->xmul))
		return FALSE;
	if (!writeInt(fh,&res->ymul))
		return FALSE;
	//xqz, yqz
	if (!writeInt(fh,&res->xqz))
		return FALSE;
	if (!writeInt(fh,&res->yqz))
		return FALSE;
	//rotation
	if (!writeInt(fh,&res->rotation))
		return FALSE;
	//xflip, yflip
	if (!writeInt(fh,&res->xflip))
		return FALSE;
	if (!writeInt(fh,&res->yflip))
		return FALSE;
	//format
	if (!writeWord(fh,&res->format))
		return FALSE;
	//eccval
	if (!writeInt(fh,(int*)&res->eccval))
		return FALSE;
	//rows, cols
	if (!writeWord(fh,&res->rows))
		return FALSE;
	if (!writeWord(fh,&res->cols))
		return FALSE;
	//style
	if (!writeInt(fh,(int*)&res->style))
		return FALSE;
	//border
	if (!writeWord(fh,&res->border))
		return FALSE;
	//encrypt
	if (!writeInt(fh,(int*)&res->encrypt))
		return FALSE;
	return TRUE;
}

/******************************************************************************
	writeLOGFONT(fh,res)
	warps the _lwrite function to detect error and set IDError and simplify
	caller expressions
	 
******************************************************************************/
BOOL writeLOGFONT(HANDLE fh,LOGFONT * res)
{
	// w,h
	if (!writeInt(fh,(int*)&res->lfHeight))
		return FALSE;
	if (!writeInt(fh,(int*)&res->lfWidth))
		return FALSE;
	// escapement
	if (!writeInt(fh,(int*)&res->lfEscapement))
		return FALSE;
	// orientation
	if (!writeInt(fh,(int*)&res->lfOrientation))
		return FALSE;
	// wt
	if (!writeInt(fh,(int*)&res->lfWeight))
		return FALSE;
	// italic, underline, strikeout
	if (!writeByte(fh,&res->lfItalic))
		return FALSE;
	if (!writeByte(fh,&res->lfUnderline))
		return FALSE;
	if (!writeByte(fh,&res->lfStrikeOut))
		return FALSE;
	// charset
	if (!writeByte(fh,&res->lfCharSet))
		return FALSE;
	// precision
	if (!writeByte(fh,&res->lfOutPrecision))
		return FALSE;
	if (!writeByte(fh,&res->lfClipPrecision))
		return FALSE;
	// quality
	if (!writeByte(fh,&res->lfQuality))
		return FALSE;
	// pitch
	if (!writeByte(fh,&res->lfPitchAndFamily))
		return FALSE;
	// face name
	if (!writeString(fh,&res->lfFaceName,LF_FACESIZE))
		return FALSE;
	return TRUE;
}
