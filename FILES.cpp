/***************************************************************************
	NAME : FILES.C
	DESC : Implementation of Files window for IMS
***************************************************************************/
#include "stdafx.h"

#include <windows.h>
#include <direct.h>
#include <dos.h>
#include <imslib32\imslib32.h>
#include <im5000\im5000.h>
#include "ims5000.h"
#include "imp.h"
#include "imf.h"
#include "imsio.h"
#include "login.h"
#include "editor.h"
#include "run.h"
#include "files.h"
#include "resource.h"

/***************************************************************************
	PRIVATE DEFINES
***************************************************************************/
#define MAXPATH  120
#define MAXFILE   100

// Control IDs
#define IDC_DIRTREE    100
#define IDC_LB2    101
#define IDC_DIR    102

/***************************************************************************
	NON API EXPORT FUNCTION PROTOTYPES
***************************************************************************/
LRESULT WINAPI FILESWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK SERINPUTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK FILENAMEDESCDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK NEWFILEDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);
BOOL CALLBACK RESOLVEFILENAMEDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP);

/***************************************************************************
	STATIC FUNCTION PROTOTYPES
***************************************************************************/
static void FilesRefresh(void);
static BOOL MatchEndPattern(char *s,char *p);
static char ClassifyFileSig(LPSTR fn);
static BOOL FileNameDescDialog(HWND hwnd,LPSTR filename,LPSTR filedesc,
	LPSTR caption);
static BOOL FindFilenameByDesc(HWND hwnd,LPSTR desc,LPSTR name);
static BOOL ResolveFilenameByDesc(HWND hwnd,LPSTR desc,LPSTR name);
static void ParseFileNameDesc(LPSTR ibuf,LPSTR fname,LPSTR fdesc);
static int InitDirectoryTree();
static BOOL AddDrive(LPSTR strDrive);
static LRESULT OnDirectoryTreeExpand(LPNMHDR pnmh);
static void GetPathFromTreeItem(HTREEITEM hitem,LPSTR path);
static void InitializeDirectorySelection(LPSTR path);
static void SetExpansionState(HTREEITEM hitem,LPSTR path);

/***************************************************************************
	STATIC VARIABLES
***************************************************************************/
static HWND dirTree,lb2,dir;
static char FilesClass[]="IMSFILES";
static WORD FocusID;
static int dirTreeCurSel;
static int LB2CurSel;
static BOOL SaveLB2CurSel=FALSE; // True if LB2CurSel should be reused in next run window

// INI vars
static char szFILES[]="FILES"; // ini section
static int iniShowDesc;
static int iniShowIMFsLast;
static BOOL iniSuppressFilePopup;

// FILENAMEDESC Dialog I/O
static LPSTR fndName;
static LPSTR fndDesc;
static LPSTR fndCaption;

// RESOLVEFILENAME Dialog I/O
static LPSTR rfName;
static LPSTR rfDesc;

static BOOL bInitTree = FALSE;
BOOL inputornot = TRUE;
/***************************************************************************
	PUBLIC FUNCTIONS
***************************************************************************/
void FilesInit(void)
	{
	}

void FilesShutdown(void)
	{
	}

/**********************************************
Siemens input window
CreateInputWnd(parent)
***********************************************/
void CreateInputWnd(HWND parent)
{
	DialogBox(IMSInst,"FILESERINPUT",parent,(DLGPROC)SERINPUTDP);
}

/**********************************************
Siemens open file
***********************************************/
BOOL OpenFile(LPSTR fileName,HWND hwnd)
{
///CodeRay Note : It appears that this is never called

	int i;
			OFSTRUCT ofs;
			char fname[MAX_PATH];
			char fntemp[MAX_PATH];
			char fdesc[IMPLDESC+1];				

						if(!(iniSuppressFilePopup && fileName[0]))
							if(!FileNameDescDialog(hwnd,fileName,fdesc,"Open"))
								return 0;
					//	if(OpenFile(fname,&ofs,OF_READ|OF_EXIST)==-1)
					  HANDLE fh;
					  fh = CreateFile(fileName,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);

					  if(fh == INVALID_HANDLE_VALUE) 
							{
							ShowError(IDE_FNF);
							return 0;
							}
					  CloseHandle(fh);
					  LPTSTR pPath;
					  GetFullPathName(fileName,sizeof(fntemp),fntemp,&pPath);
					   //  lstrcpy(ofs.szPathName,fname);
						// Could be a Part or a Fixture or unknown
						switch(ClassifyFileSig(fntemp))
							{
							case 'P':
								lstrcpy(EditorIMP.fn,fntemp);
								if(!IMPLoad(&EditorIMP))
									{
									ShowError(0);
									return 0;
									}
								LB2CurSel=i;
								SaveLB2CurSel=TRUE;
								SetMode(mEDIT);
								return 0;
							case 'F':
								lstrcpy(EditIMF.fn,fntemp);
								if(!IMFLoad(&EditIMF))
									{
									ShowError(0);
									return 0;
									}
								LB2CurSel=i;
								SaveLB2CurSel=TRUE;
								SetMode(mIMFEDIT);
								return 0;
							}
						return 0;
}//OpenFile

/************************************************************************
SERINPUTDP
SIEMENS FILE SER INPUT PROCESS
*************************************************************************/
BOOL CALLBACK SERINPUTDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	static WORD focusid=0;
	int n=0;
	char fname[MAX_PATH];
	char sectionSer[] = "VARIABLE";
	switch(message)
		{
		case WM_INITDIALOG:  // Initialization
		///CodeRay Note : He should of just put the caption in the resource
			{
			SetWindowText(hdlg,"Input Window");
			focusid=0;
			return TRUE;
			}

		case WM_COMMAND:     // Command - changed Win32
//-------------------------
			if(HIWORD(wP)==EN_SETFOCUS)
				focusid=LOWORD(wP);
			else
//--------------------------
			switch(LOWORD(wP))
				{
				case IDOK:
					// If focus was on first fields just tab
					if(focusid==IDC_FILENAME||focusid==IDC_NAME)
						{
						PostMessage(hdlg,WM_NEXTDLGCTL,0,0);
						return true;
						}

					n = GetDlgItemText(hdlg,IDC_FILENAME,fname,sizeof(fname));
					strfnextend(fname,"imp");
					//OpenFile();
					if(!GetIMFIMP(&RunIMF,NULL,fname))
							{
							ShowError(0);
							return 0;
							}
						LB2CurSel=1;

///CodeRay Note:He did not use proper symantics when copying code
///IDC_DESC IS VAR1 SN
///IDC_NAME IS VAR2 LOT
						
					var1len=GetPrivateProfileString(sectionSer,"VAR1","SN",var1,TSIZE,IMSIni);
					var1vallen = GetDlgItemText(hdlg,IDC_DESC,var1val,sizeof(var1val));

					var2len=GetPrivateProfileString(sectionSer,"VAR2","LOT",var2,TSIZE,IMSIni);
					//CodeRay get val2 stripping any leading and trailing non digits
						{
						char vbuf2[TSIZE];
						int len2=GetDlgItemText(hdlg,IDC_NAME,vbuf2,sizeof(vbuf2));
						char *p=vbuf2;
						for(;*p && !isdigit(*p);p++); // advance to first digit or null
						var2vallen=0;
						while(isdigit(*p))
							var2val[var2vallen++]=*p++;
						}
					updateVar = TRUE;
					SaveLB2CurSel=TRUE;
					RunMode=rmRUN;
					RunLinked=FALSE;
					SetMode(mRUN);
					EndDialog(hdlg,TRUE);  // End the dialog
					return TRUE;
				case IDC_QUITPG:
					PostMessage(IMSWnd,WM_COMMAND,IDM_QUIT,0);
					EndDialog(hdlg,FALSE);
					return TRUE;
				case IDC_EXITPG:
					if(UserLevel>2)
					{
					PostMessage(IMSWnd,WM_COMMAND,IDM_EXIT,0);
					EndDialog(hdlg,FALSE);
					}
					return TRUE;
				case IDCANCEL:
					if(UserLevel == 4)
					{
					EndDialog(hdlg,FALSE);
					}
					return TRUE;
			}
		}
	return FALSE;            // Message not handled
	}

/***************************************************************************
	hwnd = CreateFilesWnd(parent,id,x,y,w,h)

	Creates a files child window.

	HWND hwnd = Handle of new files window or NULL on error.
	HWND parent = Handle of parent for new files window.
	WORD id = Child window ID for new files window.
	int x = Window x coordinate
	int y = Window y coordinate
	int w = Window width
	int h = Window height
***************************************************************************/
HWND CreateFilesWnd(HWND parent,WORD id,int x,int y,int w,int h)
	{
	static BOOL registered=FALSE;
	WNDCLASS wc;

	if(!registered)
		{
		wc.style=CS_VREDRAW|CS_HREDRAW;
		wc.lpfnWndProc=FILESWP;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=IMSInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=HBRUSH(COLOR_WINDOW+1); 
		wc.lpszMenuName=NULL;
		wc.lpszClassName=FilesClass;
		if(!RegisterClass(&wc))
			return NULL;
		registered=TRUE;
		}

	// Read INI vars
	iniShowDesc=GetPrivateProfileInt(szFILES,
		"ShowDescriptions",1,IMSIni);

	iniShowIMFsLast=GetPrivateProfileInt(szFILES,
		"ShowIMFsLast",1,IMSIni);

	iniSuppressFilePopup=(BOOL)GetPrivateProfileInt(szFILES,
		"SuppressFilePopup",0,IMSIni);
	HWND abc = CreateWindow(FilesClass,
		NULL,
		WS_CHILD|WS_VISIBLE,
		x,y,w,h,
		parent,(HMENU)id,IMSInst,NULL);
//DialogBox(IMSInst,"FILENAMEDESC",abc,(DLGPROC)FILENAMEDESCDP);
	return abc;


	//return CreateWindow(FilesClass,
	//	NULL,
	//	WS_CHILD|WS_VISIBLE,
	//	x,y,w,h,
	//	parent,(HMENU)id,IMSInst,NULL);
	

	}

/***************************************************************************
	NON API EXPORT FUNCTIONS
***************************************************************************/
/***************************************************************************
	FILESWP

	Files Window Process.
***************************************************************************/
LRESULT CALLBACK FILESWP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP)
	{
	LOGFONT lf;
	HFONT font;
	
	static COLORREF cr = RGB(240,240,240);

	switch(message)
		{
		case WM_CREATE:
			InitCommonControls();
/*			lb1=CreateWindow("XLISTBOX",NULL,
				WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|
				LBS_NOINTEGRALHEIGHT|LBS_SORT|WS_BORDER,
				0,0,0,0,hwnd,(HMENU)IDC_LB1,IMSInst,NULL);
/*
			lb2=CreateWindow("XLISTBOX",NULL,
				WS_CHILD|WS_VISIBLE|WS_HSCROLL|LBS_NOTIFY|LBS_SORT|
				LBS_NOINTEGRALHEIGHT|LBS_MULTICOLUMN|WS_BORDER,
				0,0,0,0,hwnd,(HMENU)IDC_LB2,IMSInst,NULL);
			SendMessage(lb2,WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT),0);
*/
			dirTree = CreateWindow ("XTREEVIEW", "", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|
				TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_LINESATROOT, 
				0,0,0,0,hwnd,(HMENU)IDC_DIRTREE,IMSInst,NULL);
			
			font = (HFONT) SendMessage(dirTree,WM_GETFONT,0,0);
			if (font){
				GetObject(font, sizeof(LOGFONT), &lf);
				
				lf.lfHeight-=2;
						lf.lfWeight = FW_BOLD;
				font = CreateFontIndirect(&lf);
				
				SendMessage(dirTree,WM_SETFONT,(WPARAM) font,0);
			}
			CreateTreeViewImageList(dirTree,IMSInst,IDB_DIRIMAGES);

			InitDirectoryTree();

			lb2 = CreateWindow (/*WC_LISTVIEW*/ "XLISTVIEW", "", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|
				LVS_SORTASCENDING|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS, 
				0,0,0,0,hwnd,(HMENU)IDC_LB2,IMSInst,NULL);
		
			font = (HFONT) SendMessage(lb2,WM_GETFONT,0,0);
			if (font){
				GetObject(font, sizeof(LOGFONT), &lf);
				
				lf.lfHeight-=2;
						lf.lfWeight = FW_BOLD;
				font = CreateFontIndirect(&lf);
				
				SendMessage(lb2,WM_SETFONT,(WPARAM) font,0);
			}

			if (font)
				DeleteObject(font);

			AddListViewColumn(lb2,"Filename",0,150);
			AddListViewColumn(lb2,"Description",1,200);
//			AddListViewColumn(lb2,"Last Processed",2,150);
			AddListViewColumn(lb2,"Last Modified",3,150);
			CreateListViewImageLists(lb2,IMSInst,IDI_IMF,2);

			ListView_SetExtendedListViewStyle(lb2,LVS_EX_FULLROWSELECT);
			ListView_SetTextBkColor(lb2,cr);
			ListView_SetBkColor(lb2,cr);

			dir=CreateWindow("static",NULL,
				WS_CHILD|WS_VISIBLE|SS_LEFT|WS_BORDER,
				0,0,0,0,hwnd,(HMENU)IDC_DIR,IMSInst,NULL);
			FocusID=IDC_LB2;
			dirTreeCurSel=0;
			if(!SaveLB2CurSel){
				LB2CurSel=0;
			}
			FilesRefresh();
//DialogBox(IMSInst,"FILENAMEDESC",hwnd,(DLGPROC)FILENAMEDESCDP);//----------------------------------------
			return 0;

		case WM_SIZE:
			{
			int xs,ys,ysd;

			xs=LOWORD(lP);
			ys=HIWORD(lP);
			ysd=sftm.tmHeight+2;

			MoveWindow(dir,0,0,xs,ysd,TRUE);
			MoveWindow(dirTree,0,ysd,xs/3,ys-ysd,TRUE);
			MoveWindow(lb2,xs/3,ysd,xs-xs/3,ys-ysd,TRUE);
			return 0;
			}

		case WM_SETFOCUS:
			SetFocus(FocusID ? GetDlgItem(hwnd,FocusID) : IMWnd);
			
			return 0;
		case WM_NOTIFY:
			{ 
				
				LPNMHDR pnmh = (LPNMHDR) lP;
				
				switch (pnmh->idFrom){
				case IDC_DIRTREE:
					{
						switch (pnmh->code){
							case TVN_ITEMEXPANDING:
							{
							    OnDirectoryTreeExpand(pnmh);
								return 0;
							} 
							case TVN_SELCHANGED:
							{
								
								if (!bInitTree){
									HTREEITEM sel = TreeView_GetSelection(dirTree);
									HTREEITEM parent;
									char path[MAX_PATH];
									
									if(sel==NULL)
										return 0;
									
									parent = TreeView_GetParent(dirTree,sel);
									GetPathFromTreeItem(sel,path);
									NM_TREEVIEW* pnmtv = (NM_TREEVIEW*) lP;
									tbuf[lstrlen(tbuf)-1]=0; // replace ']' with '\0'
									if(!SetCurrentDirectory(path)){ // Change dir
										
										HTREEITEM oldItem = pnmtv->itemOld.hItem;
										MessageBox(hwnd,"Error changing directory",IMSName,MB_ICONEXCLAMATION);
										if (oldItem){
											TreeView_SelectItem(dirTree,oldItem);
										}
										return 0;
									}

									FilesRefresh();
									LB2CurSel=0;
									dirTreeCurSel=0;
									 
								}
								//	SendMessage(dirTree,LB_SETCURSEL,0,0L);
								return 0;
							}
						}
					}
					break;
				case IDC_LB2:
					{
						switch (pnmh->code){
						case LVN_ITEMACTIVATE:
							{
							int selItem = ListView_GetNextItem(lb2,-1,LVNI_SELECTED);
							SendMessage(hwnd,WM_COMMAND,IDM_RUN,0);
							return 0;
							}
						}  
					}
					break;
				}
				return 0;
				

			}

		case WM_COMMAND:  // Win32 changed
			{
			int i;
			OFSTRUCT ofs;
			char fname[MAX_PATH];
			char fntemp[MAX_PATH];
			char fdesc[IMPLDESC+1];
//			IMPHEADER *ph;
			if(!lP) // If from menu
				{
				switch(LOWORD(wP))
					{
					case IDM_NEW:
						if (UserLevel>2)
							DialogBox(IMSInst,"NEWFILE",hwnd,(DLGPROC)NEWFILEDP);
						return 0;
					case IDM_QUIT:
						// zzz - Just Exit for now since I keep forgetting the 'Quit' button
						//       does not close the program
					//	PostMessage(IMSWnd,WM_COMMAND,IDM_EXIT,0);
						// zzz
						inputornot = TRUE;
						SaveLB2CurSel=FALSE;
						SetMode(mLOGIN);
						return 0;
					case IDM_OPEN:
						if (UserLevel<2)
							return 0;
					//	i=(int)(WORD)SendMessage(lb2,LB_GETCURSEL,0,0L);
						i=(int)ListView_GetNextItem(lb2,-1,LVNI_SELECTED);
						if(i==-1) // No selection
							{
							fname[0]=0;
							fdesc[0]=0;
							}
						else
							{
							ListView_GetItemText(lb2,i,0,fname,sizeof(fname));
							ListView_GetItemText(lb2,i,1,fdesc,sizeof(fdesc));

							//SendMessage(lb2,LB_GETTEXT,i,(LONG)(LPSTR)tbuf);
							//ParseFileNameDesc(tbuf,fname,fdesc);
							}
						if(!(iniSuppressFilePopup && fname[0]))
							if(!FileNameDescDialog(hwnd,fname,fdesc,"Open"))
								return 0;
					//	if(OpenFile(fname,&ofs,OF_READ|OF_EXIST)==-1)
					  HANDLE fh;
					  fh = CreateFile(fname,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);

					  if(fh == INVALID_HANDLE_VALUE) 
							{
							ShowError(IDE_FNF);
							return 0;
							}
					  CloseHandle(fh);
					  LPTSTR pPath;
					  GetFullPathName(fname,sizeof(fntemp),fntemp,&pPath);
					   //  lstrcpy(ofs.szPathName,fname);
						// Could be a Part or a Fixture or unknown
						switch(ClassifyFileSig(fntemp))
							{
							case 'P':
								lstrcpy(EditorIMP.fn,fntemp);
								if(!IMPLoad(&EditorIMP))
									{
									ShowError(0);
									return 0;
									}
								LB2CurSel=i;
								SaveLB2CurSel=TRUE;
								SetMode(mEDIT);
								return 0;
							case 'F':
								lstrcpy(EditIMF.fn,fntemp);
								if(!IMFLoad(&EditIMF))
									{
									ShowError(0);
									return 0;
									}
								LB2CurSel=i;
								SaveLB2CurSel=TRUE;
								SetMode(mIMFEDIT);
								return 0;
							}
						return 0;
					case IDM_NEWIMP:
						EditorIMP.fn[0]=0;
						if(!IMPNew(&EditorIMP))
							{
							SaveLB2CurSel=FALSE;
							SetMode(mEDIT);
							}
						return 0;
					case IDM_NEWIMF:
						if(IMFNew(&EditIMF))
							{
							SaveLB2CurSel=FALSE;
							SetMode(mIMFEDIT);
							}
						return 0;
/*					case ID_RECENTF1:
						GetPrivateProfileString("recent","1","NA",fname,sizeof(fname),IMSIni);
						IMPLoadDesc(fname,fdesc);
						if(!FileNameDescDialog(hwnd,fname,fdesc,"Run"))
								return 0;
						strfnextend(fname,"IMP");
						if(!GetIMFIMP(&RunIMF,NULL,fname))
						{
							ShowError(0);
							return 0;
						}
						LB2CurSel=i;
						SaveLB2CurSel=TRUE;
						RunMode=rmRUN;
						RunLinked=FALSE;
						SetMode(mRUN);
						return 0;
						break;
					case ID_RECENTF2:
						GetPrivateProfileString("recent","2","NA",fname,sizeof(fname),IMSIni);
						IMPLoadDesc(fname,fdesc);
						if(!FileNameDescDialog(hwnd,fname,fdesc,"Run"))
								return 0;
						strfnextend(fname,"IMP");
						if(!GetIMFIMP(&RunIMF,NULL,fname))
						{
							ShowError(0);
							return 0;
						}
						LB2CurSel=i;
						SaveLB2CurSel=TRUE;
						RunMode=rmRUN;
						RunLinked=FALSE;
						SetMode(mRUN);
						return 0;
						break;
					case ID_RECENTF3:
						GetPrivateProfileString("recent","3","NA",fname,sizeof(fname),IMSIni);
						IMPLoadDesc(fname,fdesc);
						if(!FileNameDescDialog(hwnd,fname,fdesc,"Run"))
								return 0;
						strfnextend(fname,"IMP");
						if(!GetIMFIMP(&RunIMF,NULL,fname))
						{
							ShowError(0);
							return 0;
						}
						LB2CurSel=i;
						SaveLB2CurSel=TRUE;
						RunMode=rmRUN;
						RunLinked=FALSE;
						SetMode(mRUN);
						return 0;
						break;
					case ID_RECENTF4:
						GetPrivateProfileString("recent","4","NA",fname,sizeof(fname),IMSIni);
						IMPLoadDesc(fname,fdesc);
						if(!FileNameDescDialog(hwnd,fname,fdesc,"Run"))
								return 0;
						strfnextend(fname,"IMP");
						if(!GetIMFIMP(&RunIMF,NULL,fname))
						{
							ShowError(0);
							return 0;
						}
						LB2CurSel=i;
						SaveLB2CurSel=TRUE;
						RunMode=rmRUN;
						RunLinked=FALSE;
						SetMode(mRUN);
						return 0;
						break;
*/					case IDM_RUN:
						{
						i=(int)ListView_GetNextItem(lb2,-1,LVNI_SELECTED);
						if(i==-1) // No selection
							{
							fname[0]=0;
							fdesc[0]=0;
							}
						else
							{
							ListView_GetItemText(lb2,i,0,fname,sizeof(fname));
							ListView_GetItemText(lb2,i,1,fdesc,sizeof(fdesc));

						//	SendMessage(lb2,LB_GETTEXT,i,(LONG)(LPSTR)tbuf);
						//	ParseFileNameDesc(tbuf,fname,fdesc);
							}
						if(!(iniSuppressFilePopup && fname[0]))
							if(!FileNameDescDialog(hwnd,fname,fdesc,"Run"))
								return 0;
						if(!GetIMFIMP(&RunIMF,NULL,fname))
							{
							ShowError(0);
							return 0;
							}
						LB2CurSel=i;
						SaveLB2CurSel=TRUE;
						RunMode=rmRUN;
						RunLinked=FALSE;
						SetMode(mRUN);
						return 0;
						}
					case IDM_USERS:
						Users();
						return 0;
					case IDM_DELETE:
						{
						i=(int)ListView_GetNextItem(lb2,-1,LVNI_SELECTED);
					//	i=(int)(WORD)SendMessage(lb2,LB_GETCURSEL,0,0L);
						if(i==-1) // No selection
							return 0;

					//	SendMessage(lb2,LB_GETTEXT,i,(LONG)(LPSTR)tbuf);
					//	ParseFileNameDesc(tbuf,fname,fdesc);

						ListView_GetItemText(lb2,i,0,fname,sizeof(fname));
						ListView_GetItemText(lb2,i,1,fdesc,sizeof(fdesc));

						tlen=wsprintf(tbuf,"Delete %s",
							(LPSTR)fname);
						if(fdesc[0])
							wsprintf(tbuf+tlen," | %s",(LPSTR)fdesc);
						if(MessageBox(hwnd,tbuf,IMSName,
							 MB_ICONQUESTION|MB_YESNO)!=IDYES)
							 return 0;
						if(OpenFile(fname,&ofs,OF_WRITE|OF_EXIST)==-1)
							{
							ShowError(IDE_FILEDELFAILED);
							return 0;
							}
						OpenFile(NULL,&ofs,OF_REOPEN|OF_DELETE);
						FilesRefresh();
						SendMessage(lb2,LB_SETCURSEL,0,0L);
						return 0;
						}
					}
				return 0;
				}
			// Process control notifications
			switch(LOWORD(wP)) // switch on child ID
				{
				case IDC_DIRTREE:
					switch(HIWORD(wP)) // switch on notificaton
						{
					/*	case LBN_DBLCLK:
							i=(int)SendMessage(dirTree,LB_GETCURSEL,0,0L);
							if(i==LB_ERR)
								return 0;
							SendMessage(dirTree,LB_GETTEXT,i,(LONG)(LPSTR)tbuf);
							if(tbuf[1]=='-') // if drive
								{
								if(_chdrive(tbuf[2]-'a'+1)) // Change drive
									return 0;
								}
							else // dir
								{
								tbuf[lstrlen(tbuf)-1]=0; // replace ']' with '\0'
								if(_chdir(tbuf+1)) // Change dir
									return 0;
								}
							FilesRefresh();
							LB2CurSel=0;
							dirTreeCurSel=0;
							SendMessage(dirTree,LB_SETCURSEL,0,0L);
							return 0;  
					*/
						case XN_TAB:
							SetFocus(lb2);
							return 0;
						case XN_BKTAB:
							SetFocus(IMWnd);
							return 0;
						case XN_KILLFOCUS:
					
						//	dirTreeCurSel=(int)SendMessage(dirTree,
						//		LB_GETCURSEL,0,0L);
						//	if(dirTreeCurSel==-1)
						//		dirTreeCurSel=0;
						//	SendMessage(dirTree,LB_SETCURSEL,-1,0L);
							return 0;
						case XN_SETFOCUS:
				//			FocusID=IDC_DIRTREE;
				//			if(-1==ListView_GetNextItem(dirTree,-1,LVNI_SELECTED))
				//				ListView_SetItemState(dirTree,LB2CurSel,LVIS_SELECTED,LVIS_SELECTED);
							return 0;
						break;
						}
					break;
				case IDC_LB2:
					switch(HIWORD(wP)) // switch on notificaton
						{
					//	case LBN_DBLCLK:
					//		SendMessage(hwnd,WM_COMMAND,IDM_RUN,0);
					//		return 0;
						case XN_TAB:
							SetFocus(IMWnd);
							return 0;
						case XN_BKTAB:
							SetFocus(dirTree);
							return 0;
						case XN_KILLFOCUS:
							LB2CurSel=ListView_GetNextItem(lb2,-1,LVNI_SELECTED);
							if(LB2CurSel==-1){
								LB2CurSel=0;
							}
					//		SendMessage(lb2,LB_SETCURSEL,-1,0L);
							return 0;
						case XN_SETFOCUS:
							FocusID=IDC_LB2;
							if(-1==ListView_GetNextItem(lb2,-1,LVNI_SELECTED))
							{
								ListView_SetItemState(lb2,LB2CurSel,
								LVIS_SELECTED|LVIS_FOCUSED,
								LVIS_SELECTED|LVIS_FOCUSED);
								ListView_EnsureVisible(lb2,LB2CurSel,FALSE);
							}
							return 0;
						break;
						}
					break;
				case IDC_IM:
					switch(HIWORD(wP))
						{
						case IMN_TAB:
							SetFocus(dirTree);
							return 0;
						case IMN_BKTAB:
							SetFocus(lb2);
							return 0;
						case IMN_SETFOCUS:
							FocusID=0;
							return 0;
						case IMN_ERROR:
							SendMessage(IMWnd,IMM_RESET,0,0);
							return 0;
						}
					return 0;
				}
			return 0;
			}
		}
	return DefWindowProc(hwnd,message,wP,lP);
	}

/***************************************************************************
	FILENAMEDESCDP

	FILENAME Dialog Proc
***************************************************************************/
BOOL CALLBACK FILENAMEDESCDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	static BOOL inEN_CHANGE=FALSE;

	switch(message)
		{
		case WM_INITDIALOG:
			{
			SetWindowText(hdlg,fndCaption);
			inEN_CHANGE=TRUE;
			// IDC_FILENAME
			SendDlgItemMessage(hdlg,IDC_FILENAME,EM_LIMITTEXT,MAXFILE,0);
			SetDlgItemText(hdlg,IDC_FILENAME,fndName);
			strcpy(EditorIMP.fn,fndName);   
			// IDC_DESC
			SendDlgItemMessage(hdlg,IDC_DESC,EM_LIMITTEXT,IMPLDESC,0);
			char i = (char)0xcd;
//			tbuf[30];
//			wsprintf(tbuf,"fndDesc: %c",fndDesc[0]);
//			MessageBox(NULL,tbuf,NULL,MB_OK);
//			wsprintf(tbuf,"char i: %c",i);
//			MessageBox(NULL,tbuf,NULL,MB_OK);
//			wsprintf(tbuf,"char i-1: %c",i-1);
//			MessageBox(NULL,tbuf,NULL,MB_OK);
			if(fndDesc[0]==i || fndDesc[0]==i-1)
			{
			//	MessageBox(NULL,"if","ERROR",MB_OK);
				SetDlgItemText(hdlg,IDC_DESC,"");
			
			}
			else{
				SetDlgItemText(hdlg,IDC_DESC,fndDesc);
				//	MessageBox(NULL,"else","ERROR",MB_OK);
			}
			inEN_CHANGE=FALSE;
			SetFocus(GetDlgItem(hdlg,IDC_FILENAME));
			return TRUE;
			}

		case WM_COMMAND: // Win32 changed
			switch(LOWORD(wP))
				{
				case IDOK:
					{
					char name[MAXFILE+1];
					char desc[IMPLDESC+1];
					GetDlgItemText(hdlg,IDC_FILENAME,name,sizeof(name));
					GetDlgItemText(hdlg,IDC_DESC,desc,sizeof(desc));
					// if desc not blank and (changed or no name)
					if(desc[0] && ((!name[0])||lstrcmp(desc,fndDesc)))
						{
						if(!FindFilenameByDesc(hdlg,desc,name))
							return TRUE;
						}
					lstrcpy(fndName,name);
					lstrcpy(fndDesc,desc);
					EndDialog(hdlg,TRUE);
					return TRUE;
					}
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				case IDC_FILENAME:
					if(HIWORD(wP)==EN_CHANGE && !inEN_CHANGE)
						{
						inEN_CHANGE=TRUE;
						SetDlgItemText(hdlg,IDC_DESC,"");
						inEN_CHANGE=FALSE;
						return TRUE;
						}
					return FALSE;
				case IDC_DESC:
					if(HIWORD(wP)==EN_CHANGE && !inEN_CHANGE)
						{
						inEN_CHANGE=TRUE;
						SetDlgItemText(hdlg,IDC_FILENAME,"");
						inEN_CHANGE=FALSE;
						return TRUE;
						}
					return FALSE;
				}
			return FALSE;
		}
	return FALSE;
	}

/***************************************************************************
	NEWFILEDP

	New file Dialog Proc
***************************************************************************/
BOOL CALLBACK NEWFILEDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
{
	HWND parent=GetParent(hdlg);
	switch(message)
	{
	case WM_INITDIALOG:
		{
			CheckDlgButton(hdlg,IDC_NEWIMP,BST_CHECKED);
			return TRUE;
		}
		
	case WM_COMMAND: // Win32 changed
		switch(LOWORD(wP))
		{
		case IDOK:
			{
				if (IsDlgButtonChecked(hdlg,IDC_NEWIMP)==BST_CHECKED){
				//	PostMessage(parent,IDM_NEWIMP,0,0);
					EditorIMP.fn[0]=0;
					if(!IMPNew(&EditorIMP))
						{
						SaveLB2CurSel=FALSE;
						SetMode(mEDIT);
						}
				}
				else{
				//	PostMessage(parent,IDM_NEWIMF,0,0);
					if(IMFNew(&EditIMF))
						{
						SaveLB2CurSel=FALSE;
						SetMode(mIMFEDIT);
						}
				}
				EndDialog(hdlg,TRUE);
				return TRUE;
			}
		case IDCANCEL:
			{
				EndDialog(hdlg,FALSE);
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}


/***************************************************************************
	RESOLVEFILENAMEDP

	RESOLVEFILENAME dialog proc
***************************************************************************/
BOOL CALLBACK RESOLVEFILENAMEDP(HWND hdlg,UINT message,WPARAM wP,LPARAM lP)
	{
	int nfound;
	//struct _find_t fi;
	char fdesc[IMPLDESC+1];
	char fname[MAXFILE+1];
	int sel;
	WIN32_FIND_DATA fData;
	HANDLE nxtFile;
	BOOL next;
	switch(message)
		{
		case WM_INITDIALOG:
			SetDlgItemText(hdlg,IDC_DESC,rfDesc);
			// Scan disk for files matching desc
			nfound=0;
			//if(!_dos_findfirst("*.*",_A_NORMAL,&fi)) ///old stuff
			nxtFile=FindFirstFile("*.*",&fData);
			if(nxtFile!=INVALID_HANDLE_VALUE)
			do	{
					if(MatchEndPattern(fData.cFileName,".IMP"))
						{
						if(IMPLoadDesc(fData.cFileName,fdesc))
							{
							if(!lstrcmp(rfDesc,fdesc)) // If match
								{
								GetShortPathName(fData.cFileName,fname,sizeof(fname));
								OemToAnsi(fData.cFileName,fname); // Clean it up
								SendDlgItemMessage(hdlg,IDC_FNLIST,
									LB_ADDSTRING,0,(LPARAM)(LPSTR)fname);
								nfound++;
								}
							}
						}
					next=FindNextFile(nxtFile,&fData);
					} while(next);
			/*	do	{
					if(MatchEndPattern(fi.name,".IMP"))
						{
						if(IMPLoadDesc(fi.name,fdesc))
							{
							if(!lstrcmp(rfDesc,fdesc)) // If match
								{
								OemToAnsi(fi.name,fname); // Clean it up
								AnsiLower(fname);         // and add it to FNLIST
								AddEndDot(fname);
								SendDlgItemMessage(hdlg,IDC_FNLIST,
									LB_ADDSTRING,0,(LPARAM)(LPSTR)fname);
								nfound++;
								}
							}
						}
					} while(!_dos_findnext(&fi));*/
			if(nfound==0) // If none found : cancel dialog
				PostMessage(hdlg,WM_COMMAND,IDCANCEL,0);
			else
				SendDlgItemMessage(hdlg,IDC_FNLIST,LB_SETCURSEL,0,0);
			SetFocus(GetDlgItem(hdlg,IDC_FNLIST));
			return TRUE;

		case WM_COMMAND: //Win32 changed
			switch(LOWORD(wP))
				{
				case IDOK:
					sel=(int)SendDlgItemMessage(hdlg,IDC_FNLIST,
						LB_GETCURSEL,0,0);
					SendDlgItemMessage(hdlg,IDC_FNLIST,
						LB_GETTEXT,sel,(LPARAM)rfName);
					EndDialog(hdlg,TRUE);
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg,FALSE);
					return TRUE;
				case IDC_FNLIST:
					if(HIWORD(wP)==LBN_DBLCLK)
						{
						PostMessage(hdlg,WM_COMMAND,IDOK,0);
						return TRUE;
						}
					return FALSE;
				}
			return FALSE;
		}
	return FALSE;
	}

/***************************************************************************
	STATIC FUNCTIONS
***************************************************************************/
/***************************************************************************
	FilesRefresh()

	Causes the files window to refresh its contents to reflect the current
	directory.
***************************************************************************/
static void FilesRefresh(void)
	{
	WIN32_FIND_DATA fi;
	int cw; // column width
	int ew; // entry width
	char fdesc[IMPLDESC+1];
	char fname[MAXFILE+1];
	int lname,ldesc;
	char entry[IMPLDESC+MAXFILE+4];
	int i;
	HCURSOR hcur;
	HANDLE nxtFile;
	BOOL next;
	hcur=SetCursor(LoadCursor(NULL,IDC_WAIT));

	// LB1
//	SendMessage(lb1,WM_SETREDRAW,FALSE,0L);
//	SendMessage(lb1,LB_RESETCONTENT,0,0L);
//	SendMessage(lb1,LB_DIR,0xc010,(LONG)(LPSTR)"*.*");
//	SendMessage(lb1,WM_SETREDRAW,TRUE,0L);

//	SendMessage(lb1,TVM_EXPAND,TVE_EXPAND,(LPARAM)dirTreeRoot);

	// GetCurrentDirectory(...)
	// ExpandDirectory(lb1,dir);


	// LB2
	SendMessage(lb2,WM_SETREDRAW,FALSE,0L);
//	SendMessage(lb2,LB_RESETCONTENT,0,0L);
	SendMessage(lb2,LVM_DELETEALLITEMS,0,0L);
	cw=sfftm.tmAveCharWidth*(MAXFILE+(iniShowDesc ? 3 : 1));
	nxtFile=FindFirstFile("*.*",&fi);
	if(nxtFile!=INVALID_HANDLE_VALUE)
		do	{
//			GetShortPathName(fi.cFileName,fname,sizeof(fname));
			OemToChar(fi.cFileName,fname);
			lname=lstrlen(fname);
			if(!(iniShowIMFsLast && MatchEndPattern(fname,".IMF")))
				{
				lstrcpy(entry,fname);
				if(MatchEndPattern(fname,".IMP"))
					{
					i=lstrlen(entry);
				//	while(i++ < MAXFILE)
				//		lstrcat(entry," ");
				//	ldesc=IMPLoadDesc(fi.cFileName,fdesc);
				//	lstrcat(entry," | ");
				//	lstrcat(entry,fdesc);
					int iItem = AddListViewItemWithImage(lb2,fname,1);
					if (iniShowDesc)
					{
						ldesc=IMPLoadDesc(fi.cFileName,fdesc);
						ListView_SetItemText(lb2,iItem,1,fdesc);
					//	formatTime(fi.ftLastAccessTime,tbuf);
					//	ListView_SetItemText(lb2,iItem,2,tbuf);
						formatTime(fi.ftLastWriteTime,tbuf);
						ListView_SetItemText(lb2,iItem,2,tbuf);
					}
					}
				else if(MatchEndPattern(fname,".IMF"))
					{
			//		i=lstrlen(entry);
			//		while(i++ < MAXFILE)
			//			lstrcat(entry," ");
			//		ldesc=IMFLoadDesc(fi.cFileName,fdesc);
			//		lstrcat(entry," | ");
			//		lstrcat(entry,fdesc);
					char tbuf[512];
					int iItem = AddListViewItemWithImage(lb2,fname,0);
					if (iniShowDesc)
					{
						ldesc=IMFLoadDesc(fi.cFileName,fdesc);
						ListView_SetItemText(lb2,iItem,1,fdesc);
						//####
				//		formatTime(fi.ftLastAccessTime,tbuf);
				//		ListView_SetItemText(lb2,iItem,2,tbuf);
						formatTime(fi.ftLastWriteTime,tbuf);
						ListView_SetItemText(lb2,iItem,2,tbuf);

					}
					}
			//	else
			//		{
			//		lstrcpy(fdesc,"");
			//		}
//				SendMessage(lb2,LB_ADDSTRING,0,(LPARAM)(LPSTR)entry);
			//	int iItem = AddListViewItem(lb2,fname);
			//	ListView_SetItemText(lb2,iItem,1,fdesc);

				ew=(lstrlen(entry)+1)*sfftm.tmAveCharWidth;
				cw=max(ew,cw);
				}
			next=FindNextFile(nxtFile,&fi);
			} while(next);

	nxtFile=FindFirstFile("*.IMF",&fi);
	if(iniShowIMFsLast)
		if(nxtFile!=INVALID_HANDLE_VALUE)
			do	{
				OemToAnsi(fi.cFileName,fname);
				lname=lstrlen(fname);
				lstrcpy(entry,fname);
			//	i=lstrlen(entry);
			//	while(i++ < MAXFILE)
			//		lstrcat(entry," ");
			//	ldesc=IMFLoadDesc(fi.cFileName,fdesc);
			//	lstrcat(entry," | ");
			//	lstrcat(entry,fdesc);
			//	SendMessage(lb2,LB_ADDSTRING,0,(LPARAM)(LPSTR)entry);
				int iItem = AddListViewItemWithImage(lb2,fname,0);
				if (iniShowDesc){
					ldesc=IMFLoadDesc(fi.cFileName,fdesc);
					ListView_SetItemText(lb2,iItem,1,fdesc);
		//			formatTime(fi.ftLastAccessTime,tbuf);
		//			ListView_SetItemText(lb2,iItem,2,tbuf);
					formatTime(fi.ftLastWriteTime,tbuf);
					ListView_SetItemText(lb2,iItem,2,tbuf);
				}
				ew=(lstrlen(entry)+1)*sfftm.tmAveCharWidth;
				cw=max(ew,cw);
				next=FindNextFile(nxtFile,&fi);
			} while(next);

	SendMessage(lb2,LB_SETCOLUMNWIDTH,cw,0L);
	SendMessage(lb2,WM_SETREDRAW,TRUE,0L);

	// DIR
	_getdcwd(0,tbuf,MAXPATH);
	SendMessage(dir,WM_SETTEXT,0,(LONG)(LPSTR)tbuf);
	

	SetCursor(hcur);
	}


/*****************************************************************************
	match=MatchEndPattern(s,p)

	Checks to see if a string ends in a given pattern (case insensitive)

	BOOL match = TRUE if p matches tail end of s
	char *s = string to check
	char *s = pattern to match
*****************************************************************************/
static BOOL MatchEndPattern(char *s,char *p)
	{
	int ls,lp;
	ls=lstrlen(s);
	lp=lstrlen(p);
	if(lp>ls)
		return FALSE;
	return (lstrcmpi(s+(ls-lp),p)==0);
	}

/******************************************************************************
	c = ClassifyFileSig(fn)

	Determines wheater a given file is an IMP Program, an
	IMP Fixture , or and unknown type.

	char c = classification 'P' for program, 'F' for fixture, 0 for unknown
	LPSTR fn = File name
******************************************************************************/
static char ClassifyFileSig(LPSTR fn)
	{
	HANDLE fh;

//	fh=OpenFile(NULL,ofs,OF_REOPEN|OF_READ); // Open file
	fh = CreateFile(fn,GENERIC_READ,0,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	DWORD err = GetLastError();
	if(fh==INVALID_HANDLE_VALUE) // On open error
		{
		ShowError(IDE_OPENFILE);
		return 0;
		}
	_lread32(fh,tbuf,MAXSIGNITURE);                  // Read signiture
	   
	// Close file
	CloseHandle(fh);
	if(IMPSigMatch(tbuf))
		return 'P';
	if(IMFSigMatch(tbuf))
		return 'F';
	ShowError(IDE_FILETYPE);
	return 0;
	}

/******************************************************************************
	ok=-FileNameDescDialog(hwnd,filename,filedesc,caption)

	Runs the FILENAMEDESC dialog to allow the user to ok a filename or
	select a differnet filename or search for a file by description
	before continuing into edit or run mode.


	HWND hwnd = Parent window
	BOOL ok = TRUE if user pressed ok.
	LPSTR filename=filename string (may be updated if ok)
	LPSTR filedesc=filedesc string (may be updated if ok)
	LPSTR caption=Dialog caption string
******************************************************************************/
static BOOL FileNameDescDialog(HWND hwnd,LPSTR filename,LPSTR filedesc,
	LPSTR caption)
	{
	BOOL ok;
//	char fntemp[512];
//	char tbuf[512];
//	LPSTR pPath;
	fndName=filename;
	fndDesc=filedesc;
	fndCaption=caption;
	ok=DialogBox(IMSInst,"FILENAMEDESC",hwnd,(DLGPROC)FILENAMEDESCDP);
/*	if(ok)
	{
		GetFullPathName(filename,sizeof(fntemp),fntemp,&pPath);
		GetPrivateProfileString("recent","1","NA",tbuf,sizeof(tbuf),IMSIni);
		if(!strcmp(tbuf,fntemp))
		{
			return ok;
		}
		GetPrivateProfileString("recent","3","NA",tbuf,sizeof(tbuf),IMSIni);
		if(strcmp(tbuf,"NA")&&strcmp(tbuf,fntemp))
		{
			WritePrivateProfileString("recent","4",tbuf,IMSIni);
		}
		GetPrivateProfileString("recent","2","NA",tbuf,sizeof(tbuf),IMSIni);
		if(strcmp(tbuf,"NA")&&strcmp(tbuf,fntemp))
		{
			WritePrivateProfileString("recent","3",tbuf,IMSIni);
		}
		GetPrivateProfileString("recent","1","NA",tbuf,sizeof(tbuf),IMSIni);
		if(strcmp(tbuf,"NA")&&strcmp(tbuf,fntemp))
		{
			WritePrivateProfileString("recent","2",tbuf,IMSIni);
		}
		WritePrivateProfileString("recent","1",fntemp,IMSIni);
	}*/
	return ok;
	}

/*****************************************************************************
	ok=FindFilenameByDesc(hwnd,desc,name)

	Searches for a file with the matching description.
	If 0 files are found and error message is displayed and FALSE is returned.
	If 1 file is found its name is copied to name and TRUE is returned.
	if >1 files found, a dialog box with a list is presented to the user to
	resolve the conflict. The user can either select a filename or cancle
	the operation.  On selection the name is copied to name and TRUE is
	returned on cancel FALSE is returned.

	BOOL ok=True on file found or resolved, false on 0 found or cancel
	HWND hwnd=parent window for RESOLVEFILENAME dialog if resolution needed.
	LPSTR desc=descripton to search for.
	LPSTR name=found or resolved filename.
*****************************************************************************/
static BOOL FindFilenameByDesc(HWND hwnd,LPSTR desc,LPSTR name)
	{
	WIN32_FIND_DATA fi;
	char fname[MAXFILE+1];
	char fdesc[IMPLDESC+1];
	int nfound;

	// Find first and possibly second file matching desc
	nfound=0;
	HANDLE nxtFile;
	BOOL next;
	nxtFile=FindFirstFile("*.*",&fi);
	if(nxtFile!=INVALID_HANDLE_VALUE)
		do	{
			if(MatchEndPattern(fi.cFileName,".IMP"))
				{
				if(IMPLoadDesc(fi.cFileName,fdesc))
					{
					if(!lstrcmp(desc,fdesc)) // If match
						{
						OemToAnsi(fi.cFileName,fname);
						nfound++;
						if(nfound>1) // Dont find more than 2 as the ...
							break;   // RESOLVEFILENAME dialog redoes the search
						}
					}
				}
			next=FindNextFile(nxtFile,&fi);
			} while(next);
	if(nfound==0) // If none found
		{
		ShowError(IDE_DESCNOTFOUND);
		return FALSE;
		}
	if(nfound==1)
		{
		lstrcpy(name,fname);
		return TRUE;
		}
	return ResolveFilenameByDesc(hwnd,desc,name);
	}

/*****************************************************************************
	ok = ResolveFilenameByDesc(hwnd,desc,name)

	Runs the RESOLVEFILENAME dialog which allows the user to
	pick from a list of filenames that have the given description
	or press cancel.

	BOOL ok = TRUE if name resolved, FALSE if cancle or error.
	HWND hwnd = parent window for dialog.
	LPSTR desc = file description to search for.
	LPSTR name = place to store name if resolution complete.
*****************************************************************************/
static BOOL ResolveFilenameByDesc(HWND hwnd,LPSTR desc,LPSTR name)
	{
	BOOL ok;
	rfDesc=desc;
	rfName=name;
	ok=DialogBox(IMSInst,"RESOLVEFILENAME",hwnd,(DLGPROC)RESOLVEFILENAMEDP);
	return ok;
	}

/*****************************************************************************
	ParseFileNameDesc(ibuf,fname,fdesc)

	Parses a files LB2 listbox entry into a seperate file name and desc.

	LPSTR ibuf = input buffer.
	LPSTR fname = place to store parsed file name
	LSPTR fdesc = place to store parsed file description
*****************************************************************************/
static void ParseFileNameDesc(LPSTR ibuf,LPSTR fname,LPSTR fdesc)
	{
	while(*ibuf)
		{
		if(*ibuf=='|')
			break;
		if(*ibuf!=' ')
			*fname++=*ibuf;
		ibuf++;
		}
	*fname=0;

	char temp[MAXFILE+1];
	lstrcpy(temp,fname);
	GetShortPathName(temp,fname,MAXFILE+1);

	// Skip seperator if it exists
	if(*ibuf=='|')
		{
		ibuf++;
		if(*ibuf==' ')
			ibuf++;
		}

	// Copy description if it exists
	while(*ibuf)
		*fdesc++=*ibuf++;
	*fdesc=0;
	}

/*****************************************************************************
	InitDirectoryTree(hTree)

	Initializes tree control to show directories

	HWND hTree = Handle to the tree control
****************************************************************************/
static int InitDirectoryTree(){
	bInitTree = TRUE;
	int nPos = 0;
	int nDrives = 0;

	char strDrive[] = "?:\\";

	DWORD dwDriveList = GetLogicalDrives();

	while(dwDriveList){
		if (dwDriveList & 1){
			strDrive[0] = 'A'+nPos;
			if (AddDrive(strDrive))
				nDrives++;
		}
		dwDriveList >>=1;
		nPos++;
	}

	char buf[MAX_PATH];
	_getdcwd(0,buf,MAX_PATH);
	InitializeDirectorySelection(buf);

	bInitTree = FALSE;
	return nDrives;
}

/*****************************************************************************
	AddDrive(hTree,strDrive)

	Adds a drive to the tree control

	HWND hTree = Handle to the tree control
	LPSTR strDrive = String identifying drive
****************************************************************************/
static BOOL AddDrive(LPSTR strDrive){
	
	UINT drvType = GetDriveType(strDrive);
	HTREEITEM item;

	char strItem[5];
	lstrcpy(strItem,"(");
	lstrcat(strItem,strDrive);
	strItem[3] = ')';
	strItem[4] = 0;

	switch(drvType){
	case DRIVE_REMOVABLE:
		item = AddTreeItemWithImages(dirTree,NULL,strItem,0,0);
		AddTreeItemWithImages(dirTree,item,"",4,5);
		break;
	case DRIVE_FIXED:
		item = AddTreeItemWithImages(dirTree,NULL,strItem,1,1);
		AddTreeItemWithImages(dirTree,item,"",4,5);
	//	item = AddTreeItem(dirTree,item,"waaaaaaa");
		break;
	case DRIVE_REMOTE:
		item = AddTreeItemWithImages(dirTree,NULL,strItem,3,3);
		AddTreeItemWithImages(dirTree,item,"",4,5);
		break;
	case DRIVE_CDROM:
		item = AddTreeItemWithImages(dirTree,NULL,strItem,2,2);
		AddTreeItemWithImages(dirTree,item,"",4,5);
		break;
	case DRIVE_RAMDISK:
		item = AddTreeItemWithImages(dirTree,NULL,strItem,3,3);
		AddTreeItemWithImages(dirTree,item,"",4,5);
		break;
	default:
		return FALSE;

	}
	return TRUE;

}

/*****************************************************************************
	OnDirectoryTreeExpand(pnmh)

	Function called when tree item is expanded

	LPNMHDR pnmh = WM_NOTIFY header pointer with into about the item
****************************************************************************/
static LRESULT OnDirectoryTreeExpand(LPNMHDR pnmh){
	NM_TREEVIEW* pTreeView = (NM_TREEVIEW*) pnmh;
	HTREEITEM expItem = pTreeView->itemNew.hItem;
	
	char path[MAX_PATH];
	char nodepath[MAX_PATH];
	GetPathFromTreeItem(expItem,path);
		
	if (pTreeView->action == TVE_EXPAND)
	{
		HANDLE hFind;
		WIN32_FIND_DATA fd;

		BOOL bResult = FALSE;
		HTREEITEM child = TreeView_GetChild(dirTree,expItem);
		// Delete child
		if (child){
			TreeView_DeleteItem(dirTree,child);
		}
		// Now add the dirs
		lstrcat(path,"*.*");
		// Nothing to add
		hFind = FindFirstFile(path,&fd);
		if (hFind == INVALID_HANDLE_VALUE){
			return FALSE;
		}
		
		// Add directories (That are not hidden) here
		do{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
			!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)){
				char fname[MAX_PATH];
				lstrcpy(fname,fd.cFileName);
				if (lstrcmp(fname,".") != 0 && lstrcmp(fname,"..") != 0){
					HTREEITEM newItem = AddTreeItemWithImages(dirTree,expItem,fname,4,5);
					lstrcpy(nodepath,path);
					nodepath[lstrlen(nodepath)-3]=0;  // Knock off the *.* on nodepath
					lstrcat(nodepath,fname);
					lstrcat(nodepath,"\\");
					SetExpansionState(newItem,nodepath);
					bResult = TRUE;
				}
			}
			
		} while (FindNextFile(hFind,&fd));
		//Sort the directories
		TreeView_SortChildren(dirTree,expItem,0);
							
//		CloseHandle(hFind);

		return bResult;
		
	} else if (pTreeView->action == TVE_COLLAPSE) 
	{ 
		// Delete the children (directories)
		HTREEITEM hItem,hNextItem,sItem;
		sItem = TreeView_GetSelection(dirTree);
		hItem = TreeView_GetChild(dirTree,expItem);
		if (!hItem){
			return FALSE;
		}
		// Here if we are deleting selected item, selection will keep cascading down the tree 	
		// which would keep calling FilesRefresh() - so set selection to the parent if needed
		if (sItem != expItem){
			while (sItem){
				sItem = TreeView_GetParent(dirTree,sItem); 
				if (sItem == expItem){
					TreeView_SelectItem(dirTree,sItem);
				}
			}
		}
		do {
			
			hNextItem = TreeView_GetNextSibling(dirTree,hItem);
			TreeView_DeleteItem(dirTree,hItem);
			hItem = hNextItem;
		} while (hItem);

		// Now refresh expansion state
		HTREEITEM parent = TreeView_GetParent(dirTree,expItem);
		if (parent){
			SetExpansionState(expItem,path);
		} else {
			AddTreeItemWithImages(dirTree,expItem,"",4,5);
		}
		
		return FALSE;
	}
	return FALSE;
}

/*****************************************************************************
	GetPathFromTreeItem(HTREEITEM itm, LPSTR path)

	Gets the path (directory) from the selected tree item

	HTREEITEM itm = Item to get path from
	LPSTR path = String for the path (size should be MAX_PATH)
****************************************************************************/
static void GetPathFromTreeItem(HTREEITEM itm, LPSTR path)
{	
	char dir[100];
	char tempPath[MAX_PATH] = "";
	char temp[MAX_PATH] = "";
	
	HTREEITEM hParent;
	
	while (itm){
		GetTreeItemText(dirTree,itm,dir,sizeof(dir));
		hParent = TreeView_GetParent(dirTree,itm);
		if (!hParent){  // At a drive root
			char driveString[] = "?:";
			driveString[0] = dir[1];
			lstrcpy(dir,driveString);
		}

		lstrcpy(temp,dir);
		lstrcat(temp,"\\");
		lstrcat(temp,tempPath);
		lstrcpy(tempPath,temp);

		itm = hParent;
	}
	lstrcpy(path,tempPath);
}

/*****************************************************************************
	InitializeDirectorySelection(LPSTR path)

	Initializes the selection to the item corresponding to the path, if it exists

	LPSTR path = Path to use to set selection
****************************************************************************/
static void InitializeDirectorySelection(LPSTR path){
	// Get drive
	
	BOOL bFound = FALSE;

	if (path[lstrlen(path)-1] != '\\'){
		lstrcat(path,"\\");
	}
	char itemText[100];
	char * p = path;
	char * p2 = strchr(path,'\\');
	strlwr(path);
	
	char dir[MAX_PATH];
	
	HTREEITEM hItem = TreeView_GetRoot(dirTree);  // Find drive in tree
	
	while (hItem){
		GetTreeItemText(dirTree,hItem,itemText,sizeof(itemText));
		strlwr(itemText);

			
		if (itemText[1] == path[0]){
			if (lstrcmp(p2,"\\") !=0)
				TreeView_Expand(dirTree,hItem,TVE_EXPAND);
			bFound = TRUE;
			break;
		}
		hItem = TreeView_GetNextSibling(dirTree,hItem);
	}
	if (!bFound){
		return;
	}
	// Do the rest of the path
	p=p2;
	while (true){
		bFound = FALSE;
		p++;  // Advance one char
		p2 = strchr(p,'\\');
		if (p2)
		{
			lstrcpy(dir,p);
			dir[p2-p] = 0;
			strlwr(dir);
			// Find subdirectory in tree
			hItem = TreeView_GetChild(dirTree,hItem);
			while (hItem){
				GetTreeItemText(dirTree,hItem,itemText,sizeof(itemText));
		
				
				strlwr(itemText);
				if (lstrcmp(dir,itemText)==0){
					if (lstrcmp(p2,"\\") != 0)
						TreeView_Expand(dirTree,hItem,TVE_EXPAND);
					bFound = TRUE;
					break;
				}
				hItem = TreeView_GetNextSibling(dirTree,hItem);
			}
			p = p2;
		} 
		else 
			break;
	}



	// Set the selection now and make sure we can see the parent directory
	HTREEITEM parent = TreeView_GetParent(dirTree,hItem);
	char imsProgs[MAX_PATH];
	GetPrivateProfileString(IMSName,"programs","",imsProgs,sizeof(imsProgs),IMSIni);
	strlwr(imsProgs);
	int len = lstrlen(imsProgs);
	if (imsProgs[len-1] != '\\')
		lstrcat(imsProgs,"\\");
	
	BOOL bShowParent = !lstrcmp(imsProgs,path);
	
	if (hItem){
		TreeView_SelectItem(dirTree,hItem);
		if (parent && bShowParent){
			TreeView_EnsureVisible(dirTree,parent);
		} else {
			// Do not scroll parent into view if no programs dir
			TreeView_EnsureVisible(dirTree,hItem);
		}
	}		
}

/*****************************************************************************
	SetExpansionState(hItem,path)

	Makes a tree node expandable if its directory has any subdirectories

    HTREEITEM hItem = Item to set expansion state of
	LPSTR path = Path corresponding to the node: must be correctly passed in and end in '\\'
****************************************************************************/
	
static void SetExpansionState(HTREEITEM hitem,LPSTR path){

	HANDLE hFind;
	WIN32_FIND_DATA fd;

	lstrcat(path,"*.*");

	hFind = FindFirstFile(path,&fd);
	if (hFind == INVALID_HANDLE_VALUE){
		return;
	}
		
	// Add dummy item if there is at least one directory - it will be deleted on expansion
	do{
		
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
			!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)){
			char fname[MAX_PATH];
			lstrcpy(fname,fd.cFileName);
			if (lstrcmp(fname,".") != 0 && lstrcmp(fname,"..") != 0){
				HTREEITEM newItem = AddTreeItemWithImages(dirTree,hitem,"",4,5);
				return;
			}
		}
		
	} while (FindNextFile(hFind,&fd));
}

/*****************************************************************************
	ReloadFileOptions

	Reloads the file options from the ini
****************************************************************************/
void ReloadFileOptions(){
	iniShowDesc=GetPrivateProfileInt(szFILES,
		"ShowDescriptions",1,IMSIni);

	iniShowIMFsLast=GetPrivateProfileInt(szFILES,
		"ShowIMFsLast",1,IMSIni);

	iniSuppressFilePopup=(BOOL)GetPrivateProfileInt(szFILES,
		"SuppressFilePopup",0,IMSIni);


}
void formatTime(FILETIME ft,LPSTR tbuf)
{
	SYSTEMTIME systime;
	ULARGE_INTEGER UTC;
	TIME_ZONE_INFORMATION tzi;
	UTC.LowPart=ft.dwLowDateTime;
	UTC.HighPart=ft.dwHighDateTime;
	DWORD dst=GetTimeZoneInformation(&tzi);
	int bDaylight = (dst == TIME_ZONE_ID_DAYLIGHT) ? 60 : 0;
	__int64 diff=(tzi.Bias-bDaylight)*60;
	diff*=10000000;
	UTC.QuadPart=UTC.QuadPart-diff;
	
	ft.dwLowDateTime=UTC.LowPart;
	ft.dwHighDateTime=UTC.HighPart;
	FileTimeToSystemTime(&ft,&systime);
	
	WORD hour;
	char ampm[3];
	if (systime.wHour == 0){
		hour = 12;
		lstrcpy(ampm,"AM");
	} else if (systime.wHour == 12){
		hour = 12;
		lstrcpy(ampm,"PM");
	} else if (systime.wHour > 12){
		hour = systime.wHour-12;
		lstrcpy(ampm,"PM");
	} else {
		hour = systime.wHour;
		lstrcpy(ampm,"AM");
	}

	wsprintf(tbuf,"%d/%d/%d %d:%02d %s",systime.wMonth,systime.wDay,systime.wYear,
		hour,systime.wMinute,ampm);

}