#ifndef IMSIO_H
#define IMSIO_H

BOOL readInt(HANDLE fh,int * res,BOOL bShort=FALSE);
BOOL readWord(HANDLE fh,WORD * res);
BOOL readByte(HANDLE fh,BYTE * res);
BOOL readFloat(HANDLE fh,float* res);
BOOL readDouble(HANDLE fh,double* res,BOOL bFloat=FALSE);
BOOL readString(HANDLE fh,void *rbuf,int rsize);
BOOL readDoubleWord(HANDLE fh,DWORD * res);
BOOL readChar(HANDLE fh,DWORD * res);
BOOL readIJFMT(HANDLE fh,IJFMT * res,BOOL bShort=FALSE);
BOOL readDMINFO(HANDLE fh,DMINFO * res,BOOL bShort=FALSE);
BOOL readLOGFONT(HANDLE fh,LOGFONT * res,BOOL bShort=FALSE);

BOOL writeInt(HANDLE fh,int * res);
BOOL writeWord(HANDLE fh,WORD * res);
BOOL writeByte(HANDLE fh,BYTE * res);
BOOL writeFloat(HANDLE fh,float* res);
BOOL writeDouble(HANDLE fh,double* res);
BOOL writeString(HANDLE fh,void *rbuf,int rsize);
BOOL writeDoubleWord(HANDLE fh,DWORD * res);
BOOL writeChar(HANDLE fh,DWORD * res);
BOOL writeIJFMT(HANDLE fh,IJFMT * res);
BOOL writeDMINFO(HANDLE fh,DMINFO * res);
BOOL writeLOGFONT(HANDLE fh,LOGFONT * res);

union IMSINT{
	short sVal;
	long lVal;
};

union IMSDOUBLE{
	float fVal;
	double dVal;
};

#endif