/******************************************************************************
	Name : LBMP.H
	Desc : Declarations for LoadedBitmap Cache functions
******************************************************************************/
HBITMAP LoadedBitmapsFind(void *dsc,WORD ldsc);
void LoadedBitmapsAdd(void *dsc,WORD ldsc,HBITMAP hbmp);
void LoadedBitmapsClear(void);
