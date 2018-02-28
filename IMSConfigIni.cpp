// IMSConfigIni.cpp: implementation of the IMSConfigIni class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IMSConfigIni.h"
#include "resource.h"
#include <imslib32/imslib32.h>
#include <stdlib.h>
#include <shlobj.h>
#include "commdlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMSConfigIni::IMSConfigIni()
{
	InitCommonControls();
	COLORREF defColor = GetSysColor(COLOR_BTNFACE); //RGB(128,128,128); 
	refEdgeColor = defColor;
	fixtureColor = defColor;
	noPartColor = defColor;
	partColor = defColor;
	startedPartColor = defColor;
	markedPartColor = defColor;
	passedPartColor = defColor;
	failedPartColor = defColor;	
	evenBandColor = defColor;
	oddBandColor = defColor;
	inkColor = defColor;
	selColor = defColor;
	objectColor = defColor;
	originColor = defColor;
	needRestart = FALSE;
}

IMSConfigIni::~IMSConfigIni()
{

}

#define INISTRLEN 100

BOOL IMSConfigIni::initialize(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni){

	needRestart = FALSE;

	lstrcpy(AppName,LName);
	lstrcpy(AppIni,LIni);
	char title[INISTRLEN];
	wsprintf(title,"CONFIGURATION: ROTARY %s",AppName);

	PROPERTYSHEET p;
	p.AddPage("Marking Options",hinst,"MARKING",(DLGPROC)::MARKINGDP);
	//p.AddPage("UID Format 06 Setting",hinst,"UID06",(DLGPROC)::UIDDP);
	//p.AddPage("UID Format 05 Setting",hinst,"UID05",(DLGPROC)::UIDDP);
	//p.AddPage("UID Format 12 Setting",hinst,"UID12",(DLGPROC)::UIDDP);
	p.AddPage("System Settings",hinst,"SYSTEM",(DLGPROC)::SYSTEMDP);
	p.AddPage("Files/Directories",hinst,"FILESDIRS",(DLGPROC)::FILESDIRSDP);
	p.AddPage("Colors/Graphics",hinst,"COLORS",(DLGPROC)::COLORSDP);
	if (true){
		p.AddPage("Locator Settings",hinst,"LOCATOR",(DLGPROC)::LOCATORDP);
	}

	return p.DoDialog(title,hwnd);
}

BOOL IMSConfigIni::initializeCOM(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni)       //----------Linghai-4-14-09---
{

	needRestart = FALSE;

	lstrcpy(AppName,LName);
	lstrcpy(AppIni,LIni);
	char title[INISTRLEN];
	wsprintf(title,"CONFIGURATION: ROTARY %s",AppName);

	PROPERTYSHEET p;
	//p.AddPage("Marking Options",hinst,"MARKING",(DLGPROC)::MARKINGDP);
	p.AddPage("Company Information.",hinst,"UIDCOM",(DLGPROC)::COMDP);
	//p.AddPage("UID Format 06 Setting.",hinst,"UID06",(DLGPROC)::UIDDP);
	//p.AddPage("UID Format 05 Setting.",hinst,"UID05",(DLGPROC)::UIDDP);
	//p.AddPage("UID Format 12 Setting.",hinst,"UID12",(DLGPROC)::UIDDP);
	//p.AddPage("System Settings",hinst,"SYSTEM",(DLGPROC)::SYSTEMDP);
	//p.AddPage("Files/Directories",hinst,"FILESDIRS",(DLGPROC)::FILESDIRSDP);
	//p.AddPage("Colors/Graphics",hinst,"COLORS",(DLGPROC)::COLORSDP);
	

	return p.DoDialog(title,hwnd);
}

BOOL IMSConfigIni::initializeUID06(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni)       //----------Linghai-4-14-09---
{

	needRestart = FALSE;

	lstrcpy(AppName,LName);
	lstrcpy(AppIni,LIni);
	char title[INISTRLEN];
	wsprintf(title,"CONFIGURATION: ROTARY %s",AppName);

	PROPERTYSHEET p;
	//p.AddPage("Marking Options",hinst,"MARKING",(DLGPROC)::MARKINGDP);
	//p.AddPage("UID Company Settings.",hinst,"UIDCOM",(DLGPROC)::COMDP);
	p.AddPage("Construct #1",hinst,"UID06CON1",(DLGPROC)::UID06CON1DP);
	p.AddPage("Construct #2",hinst,"UID06CON2",(DLGPROC)::UID06CON2DP);
	//p.AddPage("System Settings",hinst,"SYSTEM",(DLGPROC)::SYSTEMDP);
	//p.AddPage("Files/Directories",hinst,"FILESDIRS",(DLGPROC)::FILESDIRSDP);
	//p.AddPage("Colors/Graphics",hinst,"COLORS",(DLGPROC)::COLORSDP);
	

	return p.DoDialog(title,hwnd);
}

BOOL IMSConfigIni::initializeUID05(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni)       //----------Linghai-4-14-09---
{

	needRestart = FALSE;

	lstrcpy(AppName,LName);
	lstrcpy(AppIni,LIni);
	char title[INISTRLEN];
	wsprintf(title,"CONFIGURATION: ROTARY %s",AppName);

	PROPERTYSHEET p;
	//p.AddPage("Marking Options",hinst,"MARKING",(DLGPROC)::MARKINGDP);
	//p.AddPage("UID Company Settings.",hinst,"UIDCOM",(DLGPROC)::COMDP);
	//p.AddPage("UID Format 06 Setting.",hinst,"UID06",(DLGPROC)::UID06DP);
	p.AddPage("UID Format 05 Setting.",hinst,"UID05",(DLGPROC)::UID05DP);
	//p.AddPage("UID Format 12 Setting.",hinst,"UID12",(DLGPROC)::UID12DP);
	//p.AddPage("System Settings",hinst,"SYSTEM",(DLGPROC)::SYSTEMDP);
	//p.AddPage("Files/Directories",hinst,"FILESDIRS",(DLGPROC)::FILESDIRSDP);
	//p.AddPage("Colors/Graphics",hinst,"COLORS",(DLGPROC)::COLORSDP);
	

	return p.DoDialog(title,hwnd);
}
BOOL IMSConfigIni::initializeUID(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni)       //----------Linghai-4-14-09---
{

	needRestart = FALSE;

	lstrcpy(AppName,LName);
	lstrcpy(AppIni,LIni);
	char title[INISTRLEN];
	wsprintf(title,"CONFIGURATION: ROTARY %s",AppName);

	PROPERTYSHEET p;
	//p.AddPage("Marking Options",hinst,"MARKING",(DLGPROC)::MARKINGDP);
	p.AddPage("06 Construct #1",hinst,"UID06CON1",(DLGPROC)::UID06CON1DP);
	p.AddPage("06 Construct #2",hinst,"UID06CON2",(DLGPROC)::UID06CON2DP);
	p.AddPage("UID Format 05 Setting.",hinst,"UID05",(DLGPROC)::UID05DP);
	p.AddPage("12 Construct #1",hinst,"UID12CON1",(DLGPROC)::UID12CON1DP);
	p.AddPage("12 Construct #2",hinst,"UID12CON2",(DLGPROC)::UID12CON2DP);
	//p.AddPage("UID Format 12 Setting.",hinst,"UID12",(DLGPROC)::UID12DP);
	//p.AddPage("System Settings",hinst,"SYSTEM",(DLGPROC)::SYSTEMDP);
	//p.AddPage("Files/Directories",hinst,"FILESDIRS",(DLGPROC)::FILESDIRSDP);
	//p.AddPage("Colors/Graphics",hinst,"COLORS",(DLGPROC)::COLORSDP);
	

	return p.DoDialog(title,hwnd);
}

BOOL IMSConfigIni::initializeUID12(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni)       //----------Linghai-4-14-09---
{

	needRestart = FALSE;

	lstrcpy(AppName,LName);
	lstrcpy(AppIni,LIni);
	char title[INISTRLEN];
	wsprintf(title,"CONFIGURATION: ROTARY %s",AppName);

    //p.AddPage("Construct #1",hinst,"UID06CON1",(DLGPROC)::UID06CON1DP);
	//p.AddPage("Construct #2",hinst,"UID06CON2",(DLGPROC)::UID06CON2DP);

	PROPERTYSHEET p;
	//p.AddPage("Marking Options",hinst,"MARKING",(DLGPROC)::MARKINGDP);
	//p.AddPage("UID Company Settings.",hinst,"UIDCOM",(DLGPROC)::COMDP);
	p.AddPage("Construct #1",hinst,"UID12CON1",(DLGPROC)::UID12CON1DP);
	p.AddPage("Construct #2",hinst,"UID12CON2",(DLGPROC)::UID12CON2DP);
	//p.AddPage("UID Format 12 Setting.",hinst,"UID12",(DLGPROC)::UID12DP);
	//p.AddPage("System Settings",hinst,"SYSTEM",(DLGPROC)::SYSTEMDP);
	//p.AddPage("Files/Directories",hinst,"FILESDIRS",(DLGPROC)::FILESDIRSDP);
	//p.AddPage("Colors/Graphics",hinst,"COLORS",(DLGPROC)::COLORSDP);
	

	return p.DoDialog(title,hwnd);
}

BOOL IMSConfigIni::COMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)            //-----------------Linghai-4-14-09---
{
	char COM[] = "COMPANY";
	char buf[INISTRLEN];
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			GetPrivateProfileString(COM,"COMPANY NAME","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_COMNAME,buf);

			GetPrivateProfileString(COM,"STREET","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_STREET,buf);

			GetPrivateProfileString(COM,"CAGECODE","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_CAGECODE,buf);

			GetPrivateProfileString(COM,"DUNS","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_DUNS,buf);

			GetPrivateProfileString(COM,"CITY","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_CITY,buf);

			GetPrivateProfileString(COM,"STATE","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_STATE,buf);

			GetPrivateProfileString(COM,"ZIP","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_ZIPCODE,buf);

			GetPrivateProfileString(COM,"UFNAME","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_UFNAME,buf);

			GetPrivateProfileString(COM,"ULNAME","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_ULNAME,buf);

			GetPrivateProfileString(COM,"UPHONE","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_UPHONE,buf);

			GetPrivateProfileString(COM,"UEMAIL","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_UEMAIL,buf);

			GetPrivateProfileString(COM,"QFNAME","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_QFNAME,buf);

			GetPrivateProfileString(COM,"QLNAME","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_QLNAME,buf);

			GetPrivateProfileString(COM,"QPHONE","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_QPHONE,buf);

			GetPrivateProfileString(COM,"QEMAIL","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,UID_QEMAIL,buf);


		}
		break;
	case WM_COMMAND:
		{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
			{
			case UID_COMNAME:
			case UID_STREET:
			case UID_CITY:
			case UID_STATE:
			case UID_ZIPCODE:
			case UID_UFNAME:
			case UID_ULNAME:
			case UID_UPHONE:
			case UID_UEMAIL:
			case UID_QFNAME:
			case UID_QLNAME:
			case UID_QPHONE:
			case UID_QEMAIL:
				
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				
			
			}
		}
		break;
	case WM_NOTIFY:
		{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
			{
				case PSN_KILLACTIVE:
					//bResult = !CheckMarkSettings(hDlg);
					break;
				case PSN_APPLY:
					{
					// Starting point group box
					char temp[100];
					//BOOL b;
					//int i;
					GetDlgItemText(hDlg,UID_COMNAME,temp,sizeof(temp));
					WritePrivateProfileString(COM,"COMPANY NAME",temp,AppIni);
					WritePrivateProfileString(COM,"Mil-Std 130 REV","N",AppIni);
					WritePrivateProfileString(COM,"Effective Date","01 DECEMBER 2016",AppIni);

					GetDlgItemText(hDlg,UID_CAGECODE,temp,sizeof(temp));
					WritePrivateProfileString(COM,"CAGECODE",temp,AppIni);

					GetDlgItemText(hDlg,UID_DUNS,temp,sizeof(temp));
					WritePrivateProfileString(COM,"DUNS",temp,AppIni);

					GetDlgItemText(hDlg,UID_STREET,temp,sizeof(temp));
					WritePrivateProfileString(COM,"STREET",temp,AppIni);

					GetDlgItemText(hDlg,UID_CITY,temp,sizeof(temp));
					WritePrivateProfileString(COM,"CITY",temp,AppIni);

					GetDlgItemText(hDlg,UID_STATE,temp,sizeof(temp));
					WritePrivateProfileString(COM,"STATE",temp,AppIni);

					GetDlgItemText(hDlg,UID_ZIPCODE,temp,sizeof(temp));
					WritePrivateProfileString(COM,"ZIP",temp,AppIni);

					GetDlgItemText(hDlg,UID_UFNAME,temp,sizeof(temp));
					WritePrivateProfileString(COM,"UFNAME",temp,AppIni);

					GetDlgItemText(hDlg,UID_ULNAME,temp,sizeof(temp));
					WritePrivateProfileString(COM,"ULNAME",temp,AppIni);

					GetDlgItemText(hDlg,UID_UPHONE,temp,sizeof(temp));
					WritePrivateProfileString(COM,"UPHONE",temp,AppIni);

					GetDlgItemText(hDlg,UID_UEMAIL,temp,sizeof(temp));
					WritePrivateProfileString(COM,"UEMAIL",temp,AppIni);

					GetDlgItemText(hDlg,UID_QFNAME,temp,sizeof(temp));
					WritePrivateProfileString(COM,"QFNAME",temp,AppIni);

					GetDlgItemText(hDlg,UID_QLNAME,temp,sizeof(temp));
					WritePrivateProfileString(COM,"QLNAME",temp,AppIni);

					GetDlgItemText(hDlg,UID_QPHONE,temp,sizeof(temp));
					WritePrivateProfileString(COM,"QPHONE",temp,AppIni);

					GetDlgItemText(hDlg,UID_QEMAIL,temp,sizeof(temp));
					WritePrivateProfileString(COM,"QEMAIL",temp,AppIni);

					}
			}
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;
		}
		break;
	}
	return FALSE;
}

BOOL IMSConfigIni::UID06CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)          
{
	char UID[] = "UID";
	char uidformat[] = "06";
	switch (msg)
		{
		
		case WM_INITDIALOG:
			{
				//return TRUE;
			}
			break;
	
		case WM_COMMAND:
			{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
				
				case IDC_CAGE1:
				case IDC_DUNS1:
				//case IDC_GS11:
				//case IDC_DODAAC:
				//case IDC_OTHERA:
				case IDC_SNPI1:
				case IDC_PARTS1:
				case IDC_LOTB1:
				//case IDC_PARTC1:
				//case IDC_LOTBA:
				//case IDC_CUII1:
				//case IDC_UII1:
				//case IDC_VIN:
				//case IDC_ESN1:
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
			}
				
				
			break;
		case WM_NOTIFY:
			{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
				{
				case PSN_KILLACTIVE:
					//bResult = !CheckUIDSettings(hDlg);
					break;
				case PSN_APPLY:
					{
					BOOL CAGE1 = IsDlgButtonChecked(hDlg,IDC_CAGE1) == BST_CHECKED;
					BOOL DUNS1 = IsDlgButtonChecked(hDlg,IDC_DUNS1) == BST_CHECKED;
					BOOL GS11 = IsDlgButtonChecked(hDlg,IDC_GS11) == BST_CHECKED;
					BOOL DODA = IsDlgButtonChecked(hDlg,IDC_DODAAC) == BST_CHECKED;
					BOOL OTHE = IsDlgButtonChecked(hDlg,IDC_OTHERA) == BST_CHECKED;

					WritePrivateProfileString(UID,"FORMAT","06",AppIni);
					WritePrivateProfileString(UID,"CONSTRUCT","2",AppIni);
WritePrivateProfileString(UID,"CAGE","0",AppIni);
						WritePrivateProfileString(UID,"DUNS","0",AppIni);
						WritePrivateProfileString(UID,"GS1","0",AppIni);
						WritePrivateProfileString(UID,"DODAAC","0",AppIni);
						WritePrivateProfileString(UID,"OTHER","0",AppIni);
						WritePrivateProfileString(UID,"SERNEID","0",AppIni);
						WritePrivateProfileString(UID,"SERNPIN","0",AppIni);
						WritePrivateProfileString(UID,"Part Num","0",AppIni);
						WritePrivateProfileString(UID,"Lot/Batch","0",AppIni);
						WritePrivateProfileString(UID,"Comp UII","0",AppIni);
						WritePrivateProfileString(UID,"UIIIAC","0",AppIni);
						WritePrivateProfileString(UID,"VIN","0",AppIni);
						WritePrivateProfileString(UID,"ESN","0",AppIni);
						WritePrivateProfileString(UID,"CPN","0",AppIni);
						WritePrivateProfileString(UID,"LOTBA","0",AppIni);
					

					if (CAGE1)
						{
						WritePrivateProfileString(UID,"CAGE","17V",AppIni);
						}

					else if (DUNS1)
						{
						WritePrivateProfileString(UID,"DUNS","12V",AppIni);
						}
					else if (GS11)
					{
						WritePrivateProfileString(UID,"GS1","3V",AppIni);
					}
					else if(DODA)
					{
						WritePrivateProfileString(UID,"DODAAC","7L",AppIni);
					}
					else if(OTHE)
					{
						WritePrivateProfileString(UID,"OTHER","18V",AppIni);
					}
					//else
					//{
					//	MessageBox(hDlg,"You should choose ONE EID!",NULL,MB_OK);
					//}
					
					

					BOOL SNP1 = IsDlgButtonChecked(hDlg,IDC_SNPI1) == BST_CHECKED;
					if(SNP1)
					{
						WritePrivateProfileString(UID,"SERNPIN","S",AppIni);
					}
					//else
					//{
					//	MessageBox(hDlg,"You should choose Serial Number",NULL,MB_OK);
					//}
					
				

					BOOL OPN1 = IsDlgButtonChecked(hDlg,IDC_PARTS1) == BST_CHECKED;
					BOOL LOT1 = IsDlgButtonChecked(hDlg,IDC_LOTB1) == BST_CHECKED;
					if(OPN1)
					{
						WritePrivateProfileString(UID,"Part Num","1P",AppIni);
					}
					else if(LOT1)
					{
						WritePrivateProfileString(UID,"Lot/Batch","1T",AppIni);
					}
					//else
					//{
					//	MessageBox(hDlg,"You should choose Part Number or Lot/Batch Number",NULL,MB_OK);
					//}
					break;
					}
				}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

			}
			break;
		}
	return FALSE;
}

BOOL IMSConfigIni::UID06CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)          
{
	char UID[] = "UID";
	char uidformat[] = "06";
	switch (msg)
		{
		
		case WM_INITDIALOG:
			{
				//return TRUE;
			
			}
			break;
	
		case WM_COMMAND:
			{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
				
				case IDC_PARTC1:
				case IDC_CUII1:
				case IDC_UII1:
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
			}
				
				
			break;
		case WM_NOTIFY:
			{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
				{
				case PSN_KILLACTIVE:
					//bResult = !CheckUIDSettings(hDlg);
					break;
				case PSN_APPLY:
					{
					WritePrivateProfileString(UID,"FORMAT","06",AppIni);
					WritePrivateProfileString(UID,"CONSTRUCT","1",AppIni);
WritePrivateProfileString(UID,"CAGE","0",AppIni);
						WritePrivateProfileString(UID,"DUNS","0",AppIni);
						WritePrivateProfileString(UID,"GS1","0",AppIni);
						WritePrivateProfileString(UID,"DODAAC","0",AppIni);
						WritePrivateProfileString(UID,"OTHER","0",AppIni);
						WritePrivateProfileString(UID,"SERNEID","0",AppIni);
						WritePrivateProfileString(UID,"SERNPIN","0",AppIni);
						WritePrivateProfileString(UID,"Part Num","0",AppIni);
						WritePrivateProfileString(UID,"Lot/Batch","0",AppIni);
						WritePrivateProfileString(UID,"Comp UII","0",AppIni);
						WritePrivateProfileString(UID,"UIIIAC","0",AppIni);
						WritePrivateProfileString(UID,"VIN","0",AppIni);
						WritePrivateProfileString(UID,"ESN","0",AppIni);
						WritePrivateProfileString(UID,"CPN","0",AppIni);
						WritePrivateProfileString(UID,"LOTBA","0",AppIni);
					

					BOOL UII1 = IsDlgButtonChecked(hDlg,IDC_CUII1) == BST_CHECKED;
					BOOL UIAC1 = IsDlgButtonChecked(hDlg,IDC_UII1) == BST_CHECKED;
					if(UII1)
					{
						WritePrivateProfileString(UID,"Comp UII","25S",AppIni);
					}
					else if(UIAC1)
					{
						WritePrivateProfileString(UID,"UIIIAC","18S",AppIni);
					}
					//else
					//{
					//	MessageBox(hDlg,"You should choose UII or UII w/s",NULL,MB_OK);
					//}

					BOOL CPN1 = IsDlgButtonChecked(hDlg,IDC_PARTC1) == BST_CHECKED;

					if(CPN1)
					{
						WritePrivateProfileString(UID,"CPN","30P",AppIni);
					}
					//else
					//{
					//	MessageBox(hDlg,"You should choose Current Part Number",NULL,MB_OK);
					//}
					
					
					break;
					}
				}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

			}
			break;
		}
	return FALSE;
}

BOOL IMSConfigIni::UID05DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)          
{
	char UID[] = "UID";
	switch (msg)
		{
		
		case WM_INITDIALOG:
			{
			//return TRUE;
			}
			break;
	
		case WM_COMMAND:
			{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
				
				case IDC_UIDF5EID:
				case IDC_UIDF5SN:
				case IDC_PARTC2:
				case IDC_GIAI:
				
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
			}
				
				
			break;
		case WM_NOTIFY:
			{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
				{
				case PSN_KILLACTIVE:
					//bResult = !CheckUIDSettings(hDlg);
					break;
				case PSN_APPLY:
					{
WritePrivateProfileString(UID,"Format","05",AppIni);
WritePrivateProfileString(UID,"CAGE","0",AppIni);
						WritePrivateProfileString(UID,"DUNS","0",AppIni);
						WritePrivateProfileString(UID,"GS1","0",AppIni);
						WritePrivateProfileString(UID,"DODAAC","0",AppIni);
						WritePrivateProfileString(UID,"OTHER","0",AppIni);
						WritePrivateProfileString(UID,"SERNEID","0",AppIni);
						WritePrivateProfileString(UID,"SERNPIN","0",AppIni);
						WritePrivateProfileString(UID,"Part Num","0",AppIni);
						WritePrivateProfileString(UID,"Lot/Batch","0",AppIni);
						WritePrivateProfileString(UID,"Comp UII","0",AppIni);
						WritePrivateProfileString(UID,"UIIIAC","0",AppIni);
						WritePrivateProfileString(UID,"VIN","0",AppIni);
						WritePrivateProfileString(UID,"ESN","0",AppIni);
						WritePrivateProfileString(UID,"CPN","0",AppIni);
						WritePrivateProfileString(UID,"LOTBA","0",AppIni);
					BOOL UIDF5EID = IsDlgButtonChecked(hDlg,IDC_UIDF5EID) == BST_CHECKED;
					BOOL UIDF5SN = IsDlgButtonChecked(hDlg,IDC_UIDF5SN) == BST_CHECKED;
					BOOL PARTC2 = IsDlgButtonChecked(hDlg,IDC_PARTC2) == BST_CHECKED;
					BOOL GIAI = IsDlgButtonChecked(hDlg,IDC_GIAI) == BST_CHECKED;

					if (UIDF5EID)
						{
						WritePrivateProfileString(UID,"UIDF5EID","95",AppIni);
						
						}
					if(UIDF5SN)
					{
						WritePrivateProfileString(UID,"UIDF5SN","21",AppIni);
						
					}
					if(PARTC2)
					{
						WritePrivateProfileString(UID,"CPN","240",AppIni);
					}
					if(GIAI)
					{
						WritePrivateProfileString(UID,"GIAI","8004",AppIni);
					}

					break;
					}
				}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

			}
			break;
		}
	return FALSE;
}

BOOL IMSConfigIni::UID12CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)          
{
	char UID[] = "UID";
	switch (msg)
		{
		
		case WM_INITDIALOG:
			{
			
			//return TRUE;
			}
			break;
	
		case WM_COMMAND:
			{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
				
				
				case IDC_CAGE2:
				case IDC_CAGE3:
				case IDC_CAGE4:
				case IDC_DUNS2:
				case IDC_GS12:

				case IDC_SNEI1:
				case IDC_SNEI2:
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
			}
				
				
			break;
		case WM_NOTIFY:
			{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
				{
				case PSN_KILLACTIVE:
					//bResult = !CheckUIDSettings(hDlg);
					break;
				case PSN_APPLY:
					{
						WritePrivateProfileString(UID,"FORMAT","12",AppIni);
						WritePrivateProfileString(UID,"CONSTRUCT","1",AppIni);
						WritePrivateProfileString(UID,"CAGE","0",AppIni);
						WritePrivateProfileString(UID,"DUNS","0",AppIni);
						WritePrivateProfileString(UID,"GS1","0",AppIni);
						WritePrivateProfileString(UID,"DODAAC","0",AppIni);
						WritePrivateProfileString(UID,"OTHER","0",AppIni);
						WritePrivateProfileString(UID,"SERNEID","0",AppIni);
						WritePrivateProfileString(UID,"SERNPIN","0",AppIni);
						WritePrivateProfileString(UID,"Part Num","0",AppIni);
						WritePrivateProfileString(UID,"Lot/Batch","0",AppIni);
						WritePrivateProfileString(UID,"Comp UII","0",AppIni);
						WritePrivateProfileString(UID,"UIIIAC","0",AppIni);
						WritePrivateProfileString(UID,"VIN","0",AppIni);
						WritePrivateProfileString(UID,"ESN","0",AppIni);
						WritePrivateProfileString(UID,"CPN","0",AppIni);
						WritePrivateProfileString(UID,"LOTBA","0",AppIni);

					BOOL CAGE2 = IsDlgButtonChecked(hDlg,IDC_CAGE2) == BST_CHECKED;
					BOOL CAGE3 = IsDlgButtonChecked(hDlg,IDC_CAGE3) == BST_CHECKED;
					BOOL CAGE4 = IsDlgButtonChecked(hDlg,IDC_CAGE4) == BST_CHECKED;
					BOOL DUNS2 = IsDlgButtonChecked(hDlg,IDC_DUNS2) == BST_CHECKED;
					BOOL GS12 = IsDlgButtonChecked(hDlg,IDC_GS12) == BST_CHECKED;

					BOOL SNEI1 = IsDlgButtonChecked(hDlg,IDC_SNEI1) == BST_CHECKED;
					BOOL SNEI2 = IsDlgButtonChecked(hDlg,IDC_SNEI2) == BST_CHECKED;

					if(CAGE2)
					{
						WritePrivateProfileString(UID,"CAGE","MFR",AppIni);
					}
					else if(CAGE3)
					{
						WritePrivateProfileString(UID,"CAGE","SPL",AppIni);
					}
					else if(CAGE4)
					{
						WritePrivateProfileString(UID,"CAGE","CAG",AppIni);
					}

					
					else if (DUNS2)
						{
						WritePrivateProfileString(UID,"DUNS","DUN",AppIni);
						}
					
					else if (GS12)
						{
						
						WritePrivateProfileString(UID,"GS1","EUC",AppIni);
						}
					

					if(SNEI1)
					{
						WritePrivateProfileString(UID,"SERNEID","SER",AppIni);
					}
					else if(SNEI2)
					{
						WritePrivateProfileString(UID,"SERNEID","UCN",AppIni);
					}
					
					break;
					}
				}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

			}
			break;
		}
	return FALSE;
}

BOOL IMSConfigIni::UID12CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)          
{
	char UID[] = "UID";
	switch (msg)
		{
		
		case WM_INITDIALOG:
			{
			//return TRUE;
			}
			break;
	
		case WM_COMMAND:
			{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
				
				
				case IDC_CAGE2:
				case IDC_CAGE3:
				case IDC_CAGE4:
				case IDC_DUNS2:
				case IDC_GS12:

				case IDC_SNPI2:
				case IDC_PARTS2:
				case IDC_LOTB2:
				case IDC_LOTB3:
				case IDC_LOTB4:
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
			}
				
				
			break;
		case WM_NOTIFY:
			{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
				{
				case PSN_KILLACTIVE:
					//bResult = !CheckUIDSettings(hDlg);
					break;
				case PSN_APPLY:
					{
WritePrivateProfileString(UID,"FORMAT","12",AppIni);
					WritePrivateProfileString(UID,"CONSTRUCT","2",AppIni);
WritePrivateProfileString(UID,"CAGE","0",AppIni);
						WritePrivateProfileString(UID,"DUNS","0",AppIni);
						WritePrivateProfileString(UID,"GS1","0",AppIni);
						WritePrivateProfileString(UID,"DODAAC","0",AppIni);
						WritePrivateProfileString(UID,"OTHER","0",AppIni);
						WritePrivateProfileString(UID,"SERNEID","0",AppIni);
						WritePrivateProfileString(UID,"SERNPIN","0",AppIni);
						WritePrivateProfileString(UID,"Part Num","0",AppIni);
						WritePrivateProfileString(UID,"Lot/Batch","0",AppIni);
						WritePrivateProfileString(UID,"Comp UII","0",AppIni);
						WritePrivateProfileString(UID,"UIIIAC","0",AppIni);
						WritePrivateProfileString(UID,"VIN","0",AppIni);
						WritePrivateProfileString(UID,"ESN","0",AppIni);
						WritePrivateProfileString(UID,"CPN","0",AppIni);
						WritePrivateProfileString(UID,"LOTBA","0",AppIni);

					BOOL CAGE2 = IsDlgButtonChecked(hDlg,IDC_CAGE2) == BST_CHECKED;
					BOOL CAGE3 = IsDlgButtonChecked(hDlg,IDC_CAGE3) == BST_CHECKED;
					BOOL CAGE4 = IsDlgButtonChecked(hDlg,IDC_CAGE4) == BST_CHECKED;
					BOOL DUNS2 = IsDlgButtonChecked(hDlg,IDC_DUNS2) == BST_CHECKED;
					BOOL GS12 = IsDlgButtonChecked(hDlg,IDC_GS12) == BST_CHECKED;

					BOOL PARTS2 = IsDlgButtonChecked(hDlg,IDC_PARTS2) == BST_CHECKED;
					BOOL LOTB2 = IsDlgButtonChecked(hDlg,IDC_LOTB2) == BST_CHECKED;
					BOOL LOTB3 = IsDlgButtonChecked(hDlg,IDC_LOTB3) == BST_CHECKED;
					BOOL LOTB4 = IsDlgButtonChecked(hDlg,IDC_LOTB4) == BST_CHECKED;

					BOOL SNPI2 = IsDlgButtonChecked(hDlg,IDC_SNPI2) == BST_CHECKED;

					if(CAGE2)
					{
						WritePrivateProfileString(UID,"CAGE","MFR",AppIni);
						
					}
					else if(CAGE3)
					{
						WritePrivateProfileString(UID,"CAGE","SPL",AppIni);
						
					}
					else if(CAGE4)
					{
						WritePrivateProfileString(UID,"CAGE","CAG",AppIni);
						
					}

					else if (DUNS2)
						{
						WritePrivateProfileString(UID,"DUNS","DUN",AppIni);
						}
					else if (GS12)
						{
						WritePrivateProfileString(UID,"GS1","EUC",AppIni);
						}

					if(SNPI2)
					{
						WritePrivateProfileString(UID,"SERNPIN","SEQ",AppIni);
					}

					
					if(PARTS2)
					{
						WritePrivateProfileString(UID,"Part Num","PNO",AppIni);
					}
					else if(LOTB2)
					{
						WritePrivateProfileString(UID,"Lot/Batch","LOT",AppIni);
					}
					else if(LOTB3)
					{
						WritePrivateProfileString(UID,"Lot/Batch","LNT",AppIni);
					}
					else if(LOTB4)
					{
						WritePrivateProfileString(UID,"Lot/Batch","BII",AppIni);
					}

					break;
					}
				}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

			}
			break;
		}
	return FALSE;
}


BOOL IMSConfigIni::MARKINGDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP)
	{
	char buf[INISTRLEN];
	HWND cbVarPop = GetDlgItem(hDlg,IDC_CBVARPOP);
	HWND cbVarFoc = GetDlgItem(hDlg,IDC_CBVARFOC);
	HWND cbHistLev = GetDlgItem(hDlg,IDC_CBHSTLEV);
	char sIMF[] = "IMF";

	switch (msg)
		{
		
		case WM_INITDIALOG:
			{
			// Starting point group box
			BOOL b;
			int i;
			double sp[3] = {0,0,1};
			
			GetPrivateProfileVect(AppName,"StartPoint",(double*)&sp,3,AppIni);
			ftoa(buf,sp[0],4);
			SetDlgItemText(hDlg,IDC_ESTARTX,buf);
			ftoa(buf,sp[1],4);
			SetDlgItemText(hDlg,IDC_ESTARTY,buf);
			ftoa(buf,sp[2],4);
			SetDlgItemText(hDlg,IDC_ESTARTZ,buf);
			
			GetPrivateProfileVect(AppName,"RehomePoint",(double*)&sp,3,AppIni);
			ftoa(buf,sp[0],4);
			SetDlgItemText(hDlg,IDC_EREHOMEX,buf);
			ftoa(buf,sp[1],4);
			SetDlgItemText(hDlg,IDC_EREHOMEY,buf);
			ftoa(buf,sp[2],4);
			SetDlgItemText(hDlg,IDC_EREHOMEZ,buf);

			b = GetPrivateProfileInt(AppName,"XStartOnly",0,AppIni);
			CheckDlgButton(hDlg,IDC_CZONLY,b);

			b = GetPrivateProfileInt(AppName,"ReturnToStart",0,AppIni);
			CheckDlgButton(hDlg,IDC_CRET,b);

			UpdateStartPointControls(hDlg);

			b = GetPrivateProfileInt(AppName,"WarnOnBlankVars",1,AppIni);
			CheckDlgButton(hDlg,IDC_CVARWARN,b);

			b = GetPrivateProfileInt(AppName,"VarAutoAdvance",0,AppIni);
			CheckDlgButton(hDlg,IDC_CVARADV,b);

			b = GetPrivateProfileInt(AppName,"Propogate",0,AppIni);
			CheckDlgButton(hDlg,IDC_CVARPROP,b);

			b = GetPrivateProfileInt(AppName,"ReplaceZeros",0,AppIni);
			CheckDlgButton(hDlg,IDC_CVARREP,b);

			i = GetPrivateProfileInt(AppName,"VarAutoPopKey",0,AppIni);

			SendMessage(cbVarPop,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Return Key");
			SendMessage(cbVarPop,CB_ADDSTRING,0,(LPARAM)(LPSTR)"All Keys");
			SendMessage(cbVarPop,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Virtual Key");

			SendMessage(GetDlgItem(hDlg,IDC_EVARPOP),EM_SETLIMITTEXT,1,0);

			if (i==0)
				{
				SendMessage(cbVarPop,CB_SETCURSEL,0,0);
				EnableWindow(GetDlgItem(hDlg,IDC_SVARPOP),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_EVARPOP),FALSE);
				}
			else if (i<0)
				{
				SendMessage(cbVarPop,CB_SETCURSEL,1,0);
				EnableWindow(GetDlgItem(hDlg,IDC_SVARPOP),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_EVARPOP),FALSE);
				}
			else
				{
				SendMessage(cbVarPop,CB_SETCURSEL,2,0);
				// Set the key code besed on the value
				lstrcpy(buf," ");
				buf[0] = (char) i;
				SetDlgItemText(hDlg,IDC_EVARPOP,buf);
				}

			i = GetPrivateProfileInt(AppName,"FocusVarsFirst",0,AppIni);

			SendMessage(cbVarFoc,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Never");
			SendMessage(cbVarFoc,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Once");
			SendMessage(cbVarFoc,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Every Cycle");

			if (i>=0 && i<=2)
				SendMessage(cbVarFoc,CB_SETCURSEL,i,0);
			else
				SendMessage(cbVarFoc,CB_SETCURSEL,0,0);

			// Units group box

			if (GetPrivateProfileInt(AppName,"MetricMM",0,AppIni))
				CheckDlgButton(hDlg,IDC_RMM,TRUE);
			else if (GetPrivateProfileInt(AppName,"Metric",0,AppIni))
				CheckDlgButton(hDlg,IDC_RCM,TRUE);
	
			else
				CheckDlgButton(hDlg,IDC_RIN,TRUE);

			// Run Settings group box

			b = GetPrivateProfileInt(AppName,"QuitButtonOnStop",1,AppIni);
			CheckDlgButton(hDlg,IDC_CRUNQUIT,b);

			b = GetPrivateProfileInt(AppName,"PassFailButtonOnStop",0,AppIni);
			CheckDlgButton(hDlg,IDC_CRUNPASS,b);

			b = GetPrivateProfileInt(AppName,"PauseAfterReads",0,AppIni);
			CheckDlgButton(hDlg,IDC_CRUNPAUSE,b);

			// History group box
			i = GetPrivateProfileInt(AppName,"UniqueHistorySize",0,AppIni);
			wsprintf(buf,"%d",i);
			SetDlgItemText(hDlg,IDC_EHSTSIZE,buf);

			i = GetPrivateProfileInt(AppName,"UniqueOverrideLevel",0,AppIni);

			SendMessage(cbHistLev,CB_ADDSTRING,0,(LPARAM)(LPSTR)"0");
			SendMessage(cbHistLev,CB_ADDSTRING,0,(LPARAM)(LPSTR)"1");
			SendMessage(cbHistLev,CB_ADDSTRING,0,(LPARAM)(LPSTR)"2");
			SendMessage(cbHistLev,CB_ADDSTRING,0,(LPARAM)(LPSTR)"3");
			SendMessage(cbHistLev,CB_ADDSTRING,0,(LPARAM)(LPSTR)"4");

			if (i>=0 && i<=4)
				SendMessage(cbHistLev,CB_SETCURSEL,i,0);
			else
				SendMessage(cbHistLev,CB_SETCURSEL,0,0);

		//	b = GetPrivateProfileInt(AppName,"UniqueHistoryDebug",0,AppIni);
		//	CheckDlgButton(hDlg,IDC_CHSTDBG,b);
			
			// Fixture
			b = GetPrivateProfileInt(sIMF,"RefRightEdge",0,AppIni);
			CheckDlgButton(hDlg,IDC_CREFRT,b);
			
			b = GetPrivateProfileInt(sIMF,"RefBottomEdge",0,AppIni);
			CheckDlgButton(hDlg,IDC_CREFBT,b);
		

			return TRUE;
			}
			break;
	
		case WM_COMMAND:
			{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id)
				{
				// Edit controls
				case IDC_EVARPOP:
				case IDC_ESTARTX:
				case IDC_ESTARTY:
				case IDC_ESTARTZ:
				case IDC_EREHOMEX:
				case IDC_EREHOMEY:
				case IDC_EREHOMEZ:
				case IDC_EHSTSIZE:
					if (ncode == EN_KILLFOCUS)
						{
						CheckMarkSettings(hDlg);
						return TRUE;
						}
					if (ncode == EN_CHANGE)
						{
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
						return TRUE;
						}
					break;
			
				// Checkboxes
				case IDC_CZONLY:
					{
					UpdateStartPointControls(hDlg);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;	
					}
				
				case IDC_CRET:
					{
					UpdateStartPointControls(hDlg);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
					}
				case IDC_CVARWARN:
				case IDC_CVARADV:
				case IDC_CVARPROP:
				case IDC_CVARREP:
				case IDC_CRUNQUIT:
				case IDC_CRUNPASS:
				case IDC_CRUNPAUSE:
				case IDC_CREFRT:
				case IDC_CREFBT:
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				// Radio buttons
				case IDC_RIN:
				case IDC_RCM:
				case IDC_RMM:
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				// Combo boxes
				case IDC_CBVARPOP:
					if (ncode == CBN_SELCHANGE)
						{
						int sel = SendMessage(cbVarPop,CB_GETCURSEL,0,0);
						if (sel == 2)
							{
							EnableWindow(GetDlgItem(hDlg,IDC_SVARPOP),TRUE);
							EnableWindow(GetDlgItem(hDlg,IDC_EVARPOP),TRUE);
							}
						else
							{
							EnableWindow(GetDlgItem(hDlg,IDC_SVARPOP),FALSE);
							EnableWindow(GetDlgItem(hDlg,IDC_EVARPOP),FALSE);
							}
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
						return TRUE;
						}
					break;
				case IDC_CBVARFOC:
				case IDC_CBHSTLEV:
					if (ncode == CBN_SELCHANGE)
						{
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
						return TRUE;
						}
					break;
				} // End of id switch
			}
			break;
		case WM_NOTIFY:
			{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code)
				{
				case PSN_KILLACTIVE:
					bResult = !CheckMarkSettings(hDlg);
					break;
				case PSN_APPLY:
					{
					// Starting point group box
					char temp[20];
					BOOL b;
					int i;
					GetDlgItemText(hDlg,IDC_ESTARTX,temp,sizeof(temp));
					lstrcpy(buf,temp);
					lstrcat(buf," ");

					GetDlgItemText(hDlg,IDC_ESTARTY,temp,sizeof(temp));
					lstrcat(buf,temp);
					lstrcat(buf," ");

					GetDlgItemText(hDlg,IDC_ESTARTZ,temp,sizeof(temp));
					lstrcat(buf,temp);

					WritePrivateProfileString(AppName,"StartPoint",buf,AppIni);

					GetDlgItemText(hDlg,IDC_EREHOMEX,temp,sizeof(temp));
					lstrcpy(buf,temp);
					lstrcat(buf," ");

					GetDlgItemText(hDlg,IDC_EREHOMEY,temp,sizeof(temp));
					lstrcat(buf,temp);
					lstrcat(buf," ");

					GetDlgItemText(hDlg,IDC_EREHOMEZ,temp,sizeof(temp));
					lstrcat(buf,temp);

					WritePrivateProfileString(AppName,"RehomePoint",buf,AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CZONLY) == BST_CHECKED;
					WritePrivateProfileString(AppName,"XStartOnly",(b ? "1" : "0"),AppIni);
					
					b = IsDlgButtonChecked(hDlg,IDC_CRET) == BST_CHECKED;
					WritePrivateProfileString(AppName,"ReturnToStart",(b ? "1" : "0"),AppIni);


					b = IsDlgButtonChecked(hDlg,IDC_CVARWARN) == BST_CHECKED;
					WritePrivateProfileString(AppName,"WarnOnBlankVars",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CVARADV) == BST_CHECKED;
					WritePrivateProfileString(AppName,"VarAutoAdvance",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CVARPROP) == BST_CHECKED;
					WritePrivateProfileString(AppName,"Propogate",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CVARREP) == BST_CHECKED;
					WritePrivateProfileString(AppName,"ReplaceZeros",(b ? "1" : "0"),AppIni);

					i = SendMessage(cbVarPop,CB_GETCURSEL,0,0);
					if (i==0)
						WritePrivateProfileString(AppName,"VarAutoPopKey","0",AppIni);
					else if (i==1)
						WritePrivateProfileString(AppName,"VarAutoPopKey","-1",AppIni);
					else
						{
						GetDlgItemText(hDlg,IDC_EVARPOP,buf,sizeof(buf));
						if (lstrlen(buf) > 0)
							{
							int j = (int) buf[0];
							if (j>=97 && j<=122)  // Make lowercase into uppercase
								j-=32;
							wsprintf(temp,"%d",j);
							WritePrivateProfileString(AppName,"VarAutoPopKey",temp,AppIni);
							}
						}
					
					
					i =	SendMessage(cbVarFoc,CB_GETCURSEL,0,0);
					wsprintf(buf,"%d",i);

					if (i)
						WritePrivateProfileString(AppName,"FocusVarsFirst",buf,AppIni);
					else
						WritePrivateProfileString(AppName,"FocusVarsFirst","0",AppIni);

					// Units 

					BOOL bIN = IsDlgButtonChecked(hDlg,IDC_RIN) == BST_CHECKED;
					BOOL bCM = IsDlgButtonChecked(hDlg,IDC_RCM) == BST_CHECKED;
					BOOL bMM = IsDlgButtonChecked(hDlg,IDC_RMM) == BST_CHECKED;

					if (bCM)
						{
						WritePrivateProfileString(AppName,"Metric","1",AppIni);
						WritePrivateProfileString(AppName,"MetricMM","0",AppIni);
						}
					else if (bMM)
						{
						WritePrivateProfileString(AppName,"Metric","0",AppIni);
						WritePrivateProfileString(AppName,"MetricMM","1",AppIni);
						}
					else
						{
						WritePrivateProfileString(AppName,"Metric","0",AppIni);
						WritePrivateProfileString(AppName,"MetricMM","0",AppIni);
						}
					
					// Run Settings

					b = IsDlgButtonChecked(hDlg,IDC_CRUNQUIT) == BST_CHECKED;
					WritePrivateProfileString(AppName,"QuitButtonOnStop",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CRUNPASS) == BST_CHECKED;
					WritePrivateProfileString(AppName,"PassFailButtonOnStop",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CRUNPAUSE) == BST_CHECKED;
					WritePrivateProfileString(AppName,"PauseAfterReads",(b ? "1" : "0"),AppIni);
				
					// History 

					GetDlgItemText(hDlg,IDC_EHSTSIZE,buf,sizeof(buf));
					WritePrivateProfileString(AppName,"UniqueHistorySize",buf,AppIni);

					i = SendMessage(cbHistLev,CB_GETCURSEL,0,0);
					wsprintf(buf,"%d",i);

					if (i)
						WritePrivateProfileString(AppName,"UniqueOverrideLevel",buf,AppIni);
					else
						WritePrivateProfileString(AppName,"UniqueOverrideLevel","0",AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CREFRT) == BST_CHECKED;
					WritePrivateProfileString(sIMF,"RefRightEdge",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CREFBT) == BST_CHECKED;
					WritePrivateProfileString(sIMF,"RefBottomEdge",(b ? "1" : "0"),AppIni);
		
					bResult = PSNRET_NOERROR;
					break;
					}
				}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

			}
			break;
		}
	return FALSE;
	}




BOOL IMSConfigIni::COLORSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP){
//	char rbuf[5];
//	char gbuf[5];
//	char bbuf[5];
	char buf[INISTRLEN];
	char sCDEV[] = "CDEV";
	char sIMF[] = "IMF";
	char sBANDVIEW[] = "BANDVIEW";
	HBRUSH bgBrush = CreateSolidBrush(RGB(0,0,0));
	
	switch (msg){

	case WM_DESTROY:
		{
			DeleteObject(bgBrush);
		}
	case WM_CTLCOLORSTATIC:
		{
			HWND hw=(HWND)lP;
			HDC hdc=(HDC)wP;
			
			if(hw==GetDlgItem(hDlg,IDC_SREFRGB))
			{
				if (IsDlgButtonChecked(hDlg,IDC_CSHOWREF) == BST_CHECKED){
					bgBrush = CreateSolidBrush(refEdgeColor);
				} else {
					bgBrush = CreatePatternBrush(LoadBitmap(IMSInst,"PATTERN1"));
				}
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SFIXRGB)){
				bgBrush = CreateSolidBrush(fixtureColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SNOPRGB)){
				bgBrush = CreateSolidBrush(noPartColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SPARRGB)){
				bgBrush = CreateSolidBrush(partColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SSTARGB)){
				bgBrush = CreateSolidBrush(startedPartColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SMARRGB)){
				bgBrush = CreateSolidBrush(markedPartColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SPASRGB)){
				bgBrush = CreateSolidBrush(passedPartColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SFAIRGB)){
				bgBrush = CreateSolidBrush(failedPartColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SEVERGB)){
				bgBrush = CreateSolidBrush(evenBandColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SODDRGB)){
				bgBrush = CreateSolidBrush(oddBandColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SINKRGB)){
				bgBrush = CreateSolidBrush(inkColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SSELRGB)){
				bgBrush = CreateSolidBrush(selColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SOBJRGB)){
				bgBrush = CreateSolidBrush(objectColor);
				return (BOOL)bgBrush;
			} else if (hw==GetDlgItem(hDlg,IDC_SORIRGB)){
				bgBrush = CreateSolidBrush(originColor);
				return (BOOL)bgBrush;
			} 
			break;
		}
	case WM_INITDIALOG:
		{
			
			
			// Fixture display colors
			COLORREF cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"RefEdgeColor",RGB(0,255,0),AppIni);
			refEdgeColor = cr;

			cr=GetPrivateProfileCOLORREF(sIMF,"FixtureColor",RGB(192,192,192),AppIni);
			fixtureColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"NoPartColor",RGB(128,128,128),AppIni);
			noPartColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"PartColor",RGB(128,128,255),AppIni);
			partColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"StartedPartColor",RGB(255,128,255),AppIni);
			startedPartColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"MarkedPartColor",RGB(255,255,128),AppIni);
			markedPartColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"PassedPartColor",RGB(128,255,128),AppIni);
			passedPartColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sIMF,"FailedPartColor",RGB(255,128,128),AppIni);
			failedPartColor = cr;
			
			//Band view colors
			cr=GetPrivateProfileCOLORREF(sBANDVIEW,"OddBandColor",RGB(0,255,255),AppIni);
			oddBandColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sBANDVIEW,"EvenBandColor",RGB(255,255,0),AppIni);
			evenBandColor = cr;
			
			//CDEV colors
			cr=GetPrivateProfileCOLORREF(sCDEV,"SelColor",RGB(0,0,255),AppIni);
			selColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sCDEV,"OriginColor",RGB(255,0,0),AppIni);
			originColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sCDEV,"InkColor",RGB(0,0,0),AppIni);
			inkColor = cr;
			
			cr=GetPrivateProfileCOLORREF(sCDEV,"ObjColor",RGB(255,255,255),AppIni);
			objectColor = cr;
			
			// Boolean values
			BOOL b;
			b = GetPrivateProfileInt(sIMF,"ShowRefEdge",0,AppIni);
			CheckDlgButton(hDlg,IDC_CSHOWREF,b);

			if (!b){
//				EnableWindow(GetDlgItem(hDlg,IDC_CREFRT),FALSE);
//				EnableWindow(GetDlgItem(hDlg,IDC_CREFBT),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_SREFRGB),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_BREFRGB),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_SREF),FALSE);
			}
				
			b = GetPrivateProfileInt(sBANDVIEW,"Enable",1,AppIni);
			CheckDlgButton(hDlg,IDC_CBPENABLE,b);
			
			b = GetPrivateProfileInt(sCDEV,"Zoom",1,AppIni);
			CheckDlgButton(hDlg,IDC_CCDZOOM,b);
			
//			

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			
			switch (id){
				// Buttons
			case IDC_BREFRGB:
				{
					ChangeColor(hDlg,IDC_SREFRGB,refEdgeColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BFIXRGB:
				{
					ChangeColor(hDlg,IDC_SFIXRGB,fixtureColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BNOPRGB:
				{
					ChangeColor(hDlg,IDC_SNOPRGB,noPartColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BPARRGB:
				{
					ChangeColor(hDlg,IDC_SPARRGB,partColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BMARRGB:
				{
					ChangeColor(hDlg,IDC_SMARRGB,markedPartColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BSTARGB:
				{
					ChangeColor(hDlg,IDC_SSTARGB,startedPartColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BPASRGB:
				{
					ChangeColor(hDlg,IDC_SPASRGB,passedPartColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BFAIRGB:
				{
					ChangeColor(hDlg,IDC_SFAIRGB,failedPartColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BEVERGB:
				{
					ChangeColor(hDlg,IDC_SEVERGB,evenBandColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BODDRGB:
				{
					ChangeColor(hDlg,IDC_SODDRGB,oddBandColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BINKRGB:
				{
					ChangeColor(hDlg,IDC_SINKRGB,inkColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BSELRGB:
				{
					ChangeColor(hDlg,IDC_SSELRGB,selColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BOBJRGB:
				{
					ChangeColor(hDlg,IDC_SOBJRGB,objectColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_BORIRGB:
				{
					ChangeColor(hDlg,IDC_SORIRGB,originColor);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}

				//Checkboxes
			case IDC_CSHOWREF:
				{
					BOOL ck = IsDlgButtonChecked(hDlg,IDC_CSHOWREF) == BST_CHECKED;
//					EnableWindow(GetDlgItem(hDlg,IDC_CREFRT),ck);
//					EnableWindow(GetDlgItem(hDlg,IDC_CREFBT),ck);
					EnableWindow(GetDlgItem(hDlg,IDC_SREFRGB),ck);
					EnableWindow(GetDlgItem(hDlg,IDC_BREFRGB),ck);
					EnableWindow(GetDlgItem(hDlg,IDC_SREF),ck);
					InvalidateRect(GetDlgItem(hDlg,IDC_SREFRGB),NULL,FALSE);
					
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}

			case IDC_CBPENABLE:
			case IDC_CCDZOOM:
			//	HDC hdc;
			//	SendMessage(hDlg,WM_CTLCOLOREDIT,hdc,
				SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
				return TRUE;
				
			} //End of switch(id)
		}
		break;
	case WM_NOTIFY:
		{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code){
			case PSN_KILLACTIVE:
				bResult = !CheckColorSettings(hDlg);
				break;
			case PSN_APPLY:
				{
					
					BOOL b;

					b = IsDlgButtonChecked(hDlg,IDC_CBPENABLE) == BST_CHECKED;
					WritePrivateProfileString(sBANDVIEW,"Enable",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CCDZOOM) == BST_CHECKED;
					WritePrivateProfileString(sCDEV,"Zoom",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CSHOWREF) == BST_CHECKED;
					WritePrivateProfileString(sIMF,"ShowRefEdge",(b ? "1" : "0"),AppIni);

					if (b){
				
						wsprintf(buf,"%d %d %d",GetRValue(refEdgeColor),GetGValue(refEdgeColor),GetBValue(refEdgeColor));
						WritePrivateProfileString(sIMF,"RefEdgeColor",buf,AppIni);

					}

					wsprintf(buf,"%d %d %d",GetRValue(fixtureColor),
						GetGValue(fixtureColor),GetBValue(fixtureColor));
					WritePrivateProfileString(sIMF,"FixtureColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(noPartColor),
						GetGValue(noPartColor),GetBValue(noPartColor));
					WritePrivateProfileString(sIMF,"NoPartColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(partColor),
						GetGValue(partColor),GetBValue(partColor));
					WritePrivateProfileString(sIMF,"PartColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(startedPartColor),
						GetGValue(startedPartColor),GetBValue(startedPartColor));
					WritePrivateProfileString(sIMF,"StartedPartColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(markedPartColor),
						GetGValue(markedPartColor),GetBValue(markedPartColor));
					WritePrivateProfileString(sIMF,"MarkedPartColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(passedPartColor),
						GetGValue(passedPartColor),GetBValue(passedPartColor));
					WritePrivateProfileString(sIMF,"PassedPartColor",buf,AppIni);
					
					wsprintf(buf,"%d %d %d",GetRValue(failedPartColor),
						GetGValue(failedPartColor),GetBValue(failedPartColor));
					WritePrivateProfileString(sIMF,"FailedPartColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(evenBandColor),
						GetGValue(evenBandColor),GetBValue(evenBandColor));
					WritePrivateProfileString(sBANDVIEW,"EvenBandColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(oddBandColor),
						GetGValue(oddBandColor),GetBValue(oddBandColor));
					WritePrivateProfileString(sBANDVIEW,"OddBandColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(inkColor),
						GetGValue(inkColor),GetBValue(inkColor));
					WritePrivateProfileString(sCDEV,"InkColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(selColor),
						GetGValue(selColor),GetBValue(selColor));
					WritePrivateProfileString(sCDEV,"SelColor",buf,AppIni);

					wsprintf(buf,"%d %d %d",GetRValue(objectColor),
						GetGValue(objectColor),GetBValue(objectColor));
					WritePrivateProfileString(sCDEV,"ObjColor",buf,AppIni);
					
					wsprintf(buf,"%d %d %d",GetRValue(originColor),
						GetGValue(originColor),GetBValue(originColor));
					WritePrivateProfileString(sCDEV,"OriginColor",buf,AppIni);

					bResult = PSNRET_NOERROR;
					break;
				}
			}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;
			
		}
		break;
	}
	return FALSE;
}


BOOL IMSConfigIni::SYSTEMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP){
	char buf[INISTRLEN],buf2[INISTRLEN];
	char sTIMENAMES[] = "TimeNames";
	char sDDERUN[] = "DDERUN";
	char sHIDEOPT[] = "HideOpt";

	HWND cbSysType = GetDlgItem(hDlg,IDC_CBSYSTYPE);
	HWND lvTime = GetDlgItem(hDlg,IDC_LVTIME);
	switch (msg){
		
	case WM_INITDIALOG:
		{
			
			BOOL b;
		//	int i;

			ListView_SetExtendedListViewStyle(lvTime,LVS_EX_FULLROWSELECT);

			// System type
			SendMessage(cbSysType,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Downspray");
			SendMessage(cbSysType,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Upspray");
			
			b = GetPrivateProfileInt(AppName,"Upspray",0,AppIni);
			
			if (b){
				SendMessage(cbSysType,CB_SETCURSEL,1,0);
			} else {
				SendMessage(cbSysType,CB_SETCURSEL,0,0);
			}

			// Time names

			AddListViewColumn(lvTime,"Seconds Past Midnight",0,125);
			AddListViewColumn(lvTime,"Name",1,110);

			char buf[INISTRLEN];
			static char keys[128];

			char *key;
			int idx=0;

			// map time names
			GetPrivateProfileString(sTIMENAMES,NULL,"",keys,sizeof(keys),AppIni);
			key=keys;
			while(*key)
			{
				
				if(GetPrivateProfileString(sTIMENAMES,key,"",buf,sizeof(buf),AppIni))
				{
					AddTimeName(lvTime,key,buf);
				}
				while(*key++);
				idx++;
			}

			EnableWindow(GetDlgItem(hDlg,IDC_WNREMOVE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_WNCHANGE),FALSE);

			// DDE server options

			b = GetPrivateProfileInt(sDDERUN,"MinimizeOnUserQuit",0,AppIni);
			CheckDlgButton(hDlg,IDC_CDDEUSERMIN,b);

			b = GetPrivateProfileInt(sDDERUN,"MinimizeOnQuit",0,AppIni);
			CheckDlgButton(hDlg,IDC_CDDEMIN,b);

			b = GetPrivateProfileInt(sDDERUN,"MaximizeOnOpen",0,AppIni);
			CheckDlgButton(hDlg,IDC_CDDEMAX,b);

			b = GetPrivateProfileInt(sDDERUN,"SendPDONE",0,AppIni);
			CheckDlgButton(hDlg,IDC_CDDEPDONE,b);

			b = GetPrivateProfileInt(sDDERUN,"NBFChangeData",0,AppIni);
			CheckDlgButton(hDlg,IDC_CDDENOTIFY,b);

			// Menu options

			b = GetPrivateProfileInt(AppName,"NoFontMarks",0,AppIni);
			CheckDlgButton(hDlg,IDC_CNOFONT,b);

			b = GetPrivateProfileInt(AppName,"NoBitmapMarks",0,AppIni);
			CheckDlgButton(hDlg,IDC_CNOBMP,b);

			b = GetPrivateProfileInt(AppName,"NoAlign",0,AppIni);
			CheckDlgButton(hDlg,IDC_CNOALIGN,b);

			b = GetPrivateProfileInt(AppName,"NoCal",0,AppIni);
			CheckDlgButton(hDlg,IDC_CNOCAL,b);

			// Optional attributes - HideOpt section - use negation

			b = GetPrivateProfileInt(sHIDEOPT,"Conveyor",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTCONV,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"Init",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTINIT,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"Inspect",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTINSP,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"Link",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTLINK,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"MarkRotation",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTMARKR,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"MarkSlant",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTMARKS,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"ModeMenu",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTMODE,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"Read",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTREAD,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"RotaryIMF",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTRIMF,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"RotaryDEG",0,AppIni);   // deg
			CheckDlgButton(hDlg,IDC_COPTDEG,!b);

			b = GetPrivateProfileInt(sHIDEOPT,"Rotation",0,AppIni);
			CheckDlgButton(hDlg,IDC_COPTROT,!b);

			return TRUE;
		}
		break;
	
	case WM_COMMAND:
		{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id){
			
			// Checkboxes
			case IDC_CDDEMIN:	case IDC_CDDEMAX:	case IDC_CDDEPDONE:	case IDC_CDDENOTIFY:
			case IDC_CNOFONT:	case IDC_CNOBMP:	case IDC_CNOALIGN:	case IDC_CNOCAL:
			case IDC_COPTCONV:	case IDC_COPTINIT:	case IDC_COPTINSP:	case IDC_COPTLINK: 
			case IDC_COPTMARKR: case IDC_COPTMARKS: case IDC_COPTMODE:	case IDC_COPTREAD: 
			case IDC_COPTRIMF: case IDC_COPTDEG:  case IDC_COPTROT:
			case IDC_CDDEUSERMIN:
				SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
				return TRUE;

			// Combo box
			case IDC_CBSYSTYPE:
				if (ncode == CBN_SELCHANGE){
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
				break;
			// Buttons
			case IDC_WNADD:
				{
					bNewName = TRUE;
					BOOL bAdded = DialogBox(IMSInst,"EDITTIME",hDlg,(DLGPROC)::EDITTIMEDP);
					if (bAdded){
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					}
					return TRUE;
				}
				
			case IDC_WNREMOVE:
				{
					int selItem = ListView_GetNextItem(lvTime,-1,LVNI_SELECTED);
					ListView_DeleteItem(lvTime,selItem);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
			case IDC_WNCHANGE:
				{
					bNewName = FALSE;
					BOOL bAdded = DialogBox(IMSInst,"EDITTIME",hDlg,(DLGPROC)::EDITTIMEDP);
					if (bAdded){
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					}
					return TRUE;
				}
			} // End of id switch
		}
		break;
	case WM_NOTIFY:
		{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->idFrom){
			case IDC_LVTIME:
				{
					switch (pNmhdr->code){
					case LVN_ITEMCHANGED:
						
						NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lP;
						
						if (pNMListView->uOldState & LVIS_SELECTED){
							// Selection was lost
							int sel = ListView_GetNextItem(lvTime,-1,LVNI_SELECTED);
							EnableWindow(GetDlgItem(hDlg,IDC_WNREMOVE),sel != -1);
							EnableWindow(GetDlgItem(hDlg,IDC_WNCHANGE),sel != -1);
						}
						if (pNMListView->uNewState & LVIS_SELECTED){
							// New selection 
							int sel = ListView_GetNextItem(lvTime,-1,LVNI_SELECTED);
							EnableWindow(GetDlgItem(hDlg,IDC_WNREMOVE),sel != -1);
							EnableWindow(GetDlgItem(hDlg,IDC_WNCHANGE),sel != -1);
						}
						return TRUE;
					}
					
				}
			default:
				{
					switch (pNmhdr->code){
					case PSN_SETACTIVE:
						{
					//		BOOL b = GetPrivateProfileInt(AppName,"Upspray",0,AppIni);
							
					//		if (b){
					//			SendMessage(cbSysType,CB_SETCURSEL,1,0);
					//		} else {
					//			SendMessage(cbSysType,CB_SETCURSEL,0,0);
					//		}
							
							bResult = FALSE;
							break;
						}
					case PSN_KILLACTIVE:
							// Apparently this lovely page needs no validation
							
							bResult = FALSE;
						break;
					case PSN_APPLY:
						{
							int i;
							BOOL b;

							// System type
							
							i = SendMessage(cbSysType,CB_GETCURSEL,0,0);
							wsprintf(buf,"%d",i);

							if (i==0 || i==1){
								WritePrivateProfileString(AppName,"Upspray",buf,AppIni);
							} else {
								WritePrivateProfileString(AppName,"Upspray","0",AppIni);
							}

							// Time names

							int count = ListView_GetItemCount(lvTime);
							
							WritePrivateProfileString(sTIMENAMES,NULL,NULL,AppIni);
							for (i=0;i<count;i++){
								ListView_GetItemText(lvTime,i,0,buf,sizeof(buf));
								ListView_GetItemText(lvTime,i,1,buf2,sizeof(buf2));
								WritePrivateProfileString(sTIMENAMES,buf,buf2,AppIni);
							}
							
							
							// DDE server mode options
							b = IsDlgButtonChecked(hDlg,IDC_CDDEUSERMIN) == BST_CHECKED;
							WritePrivateProfileString(sDDERUN,"MinimizeOnUserQuit",(b ? "1" : "0") ,AppIni);

							b = IsDlgButtonChecked(hDlg,IDC_CDDEMIN) == BST_CHECKED;
							WritePrivateProfileString(sDDERUN,"MinimizeOnQuit",(b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_CDDEMAX) == BST_CHECKED;
							WritePrivateProfileString(sDDERUN,"MaximizeOnOpen",(b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_CDDEPDONE) == BST_CHECKED;
							WritePrivateProfileString(sDDERUN,"SendPDONE",(b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_CDDENOTIFY) == BST_CHECKED;
							WritePrivateProfileString(sDDERUN,"NBFChangeData",(b ? "1" : "0") ,AppIni);
							
							
							
							// Menu options
							
							b = IsDlgButtonChecked(hDlg,IDC_CNOFONT) == BST_CHECKED;
							WritePrivateProfileString(AppName,"NoFontMarks",(b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_CNOBMP) == BST_CHECKED;
							WritePrivateProfileString(AppName,"NoBitmapMarks",(b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_CNOALIGN) == BST_CHECKED;
							WritePrivateProfileString(AppName,"NoAlign",(b ? "1" : "0") ,AppIni);

							b = IsDlgButtonChecked(hDlg,IDC_CNOCAL) == BST_CHECKED;
							WritePrivateProfileString(AppName,"NoCal",(b ? "1" : "0") ,AppIni);
							
							
							// Optional attributes - HideOpt section - use negation
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTCONV) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"Conveyor",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTINIT) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"Init",(!b ? "1" : "0") ,AppIni);
								
							b = IsDlgButtonChecked(hDlg,IDC_COPTINSP) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"Inspect",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTLINK) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"Link",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTMARKR) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"MarkRotation",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTMARKS) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"MarkSlant",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTMODE) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"ModeMenu",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTREAD) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"Read",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTRIMF) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"RotaryIMF",(!b ? "1" : "0") ,AppIni);

							b = IsDlgButtonChecked(hDlg,IDC_COPTDEG) == BST_CHECKED;         //  deg
							WritePrivateProfileString(sHIDEOPT,"RotaryDEG",(!b ? "1" : "0") ,AppIni);
							
							b = IsDlgButtonChecked(hDlg,IDC_COPTROT) == BST_CHECKED;
							WritePrivateProfileString(sHIDEOPT,"Rotation",(!b ? "1" : "0") ,AppIni);
							
							bResult = PSNRET_NOERROR;
							break;
						}
					}
					
					SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
					return bResult;
				}
				
			}
			
		}
		break;
	}
	return FALSE;
}

BOOL IMSConfigIni::FILESDIRSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP){
	char buf[INISTRLEN];
	char sFILES[] = "Files";
	//HWND cbVarPop = GetDlgItem(hDlg,IDC_CBVARPOP);
	//HWND cbVarFoc = GetDlgItem(hDlg,IDC_CBVARFOC);
	//HWND cbHistLev = GetDlgItem(hDlg,IDC_CBHSTLEV);
	static BOOL ready=FALSE;
	switch (msg){
		
	case WM_INITDIALOG:
		{
			ready=FALSE;
			
			BOOL b;
		
			// Files
			GetPrivateProfileString(AppName,"Shell","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_ESHELL,buf);

			GetPrivateProfileString(AppName,"Run","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_ERUN,buf);

			GetPrivateProfileString(AppName,"Load","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_ELOAD,buf);

			// File options
			b = GetPrivateProfileInt(sFILES,"ShowDescriptions",1,AppIni);
			CheckDlgButton(hDlg,IDC_CSHOWDESC,b);

			b = GetPrivateProfileInt(sFILES,"ShowIMFsLast",1,AppIni);
			CheckDlgButton(hDlg,IDC_CIMFLAST,b);

			b = GetPrivateProfileInt(sFILES,"SuppressFilePopup",0,AppIni);
			CheckDlgButton(hDlg,IDC_CPOPUP,b);

			// Directories
			GetPrivateProfileString(AppName,"programs","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_EPROG,buf);

			GetPrivateProfileString(AppName,"fixtures","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_EFIXT,buf);

			GetPrivateProfileString(AppName,"BMPDir","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_EBMPS,buf);

			GetPrivateProfileString(AppName,"VisObjects","",buf,sizeof(buf),AppIni);
			SetDlgItemText(hDlg,IDC_EVISOBJ,buf);

			ready=TRUE;
			return TRUE;
		}
		break;
	
	case WM_COMMAND:
		{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);
			switch (id){
			// Buttons
			case IDC_PBROWSE:
				{
					
					LPMALLOC pMalloc;
					
					if (!SUCCEEDED(::SHGetMalloc(&pMalloc)))
						return FALSE;
					
					char dirname[MAX_PATH];

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					
					BROWSEINFO bi;
					
					bi.hwndOwner = hDlg;
					bi.pidlRoot = NULL;
					bi.pszDisplayName = &dirname[0];
					bi.lpszTitle = "Select IMS Programs Directory";
					bi.ulFlags = BIF_RETURNONLYFSDIRS;
					bi.iImage = 0;
					bi.lpfn = ConfigBrowseCallbackProc;
					bi.lParam = (LPARAM)(LPCSTR) initdir;
					
					ITEMIDLIST* itemList = ::SHBrowseForFolder(&bi);
					
					if (!itemList){
						pMalloc->Release();
						return FALSE;
					}
					
					if (!::SHGetPathFromIDList(itemList, &dirname[0])) {
						pMalloc->Free(itemList);
						pMalloc->Release();
						return FALSE;
					}

					SetDlgItemText(hDlg,IDC_EPROG,dirname);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					
					return TRUE;
				}
			case IDC_FBROWSE:
				{
					
					LPMALLOC pMalloc;
					
					if (!SUCCEEDED(::SHGetMalloc(&pMalloc)))
						return FALSE;
					
					char dirname[MAX_PATH];

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					
					BROWSEINFO bi;
					
					bi.hwndOwner = hDlg;
					bi.pidlRoot = NULL;
					bi.pszDisplayName = &dirname[0];
					bi.lpszTitle = "Select IMS Fixtures Directory";
					bi.ulFlags = BIF_RETURNONLYFSDIRS;
					bi.iImage = 0;
					bi.lpfn = ConfigBrowseCallbackProc;
					bi.lParam = (LPARAM)(LPCSTR) initdir;
					
					ITEMIDLIST* itemList = ::SHBrowseForFolder(&bi);
					
					if (!itemList){
						pMalloc->Release();
						return FALSE;
					}
					
					if (!::SHGetPathFromIDList(itemList, &dirname[0])) {
						pMalloc->Free(itemList);
						pMalloc->Release();
						return FALSE;
					}

					SetDlgItemText(hDlg,IDC_EFIXT,dirname);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					
					return TRUE;
				}
			case IDC_BBROWSE:
				{
					
					LPMALLOC pMalloc;
					
					if (!SUCCEEDED(::SHGetMalloc(&pMalloc)))
						return FALSE;
					
					char dirname[MAX_PATH];

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					
					BROWSEINFO bi;
					
					bi.hwndOwner = hDlg;
					bi.pidlRoot = NULL;
					bi.pszDisplayName = &dirname[0];
					bi.lpszTitle = "Select IMS Bitmap Directory";
					bi.ulFlags = BIF_RETURNONLYFSDIRS;
					bi.iImage = 0;
					bi.lpfn = ConfigBrowseCallbackProc;
					bi.lParam = (LPARAM)(LPCSTR) initdir;
					
					ITEMIDLIST* itemList = ::SHBrowseForFolder(&bi);
					
					if (!itemList){
						pMalloc->Release();
						return FALSE;
					}
					
					if (!::SHGetPathFromIDList(itemList, &dirname[0])) {
						pMalloc->Free(itemList);
						pMalloc->Release();
						return FALSE;
					}

					SetDlgItemText(hDlg,IDC_EBMPS,dirname);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					
					return TRUE;
				}
			case IDC_VBROWSE:
				{
					
					LPMALLOC pMalloc;
					
					if (!SUCCEEDED(::SHGetMalloc(&pMalloc)))
						return FALSE;
					
					char dirname[MAX_PATH];

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					
					BROWSEINFO bi;
					
					bi.hwndOwner = hDlg;
					bi.pidlRoot = NULL;
					bi.pszDisplayName = &dirname[0];
					bi.lpszTitle = "Select IMS Vision Object Directory";
					bi.ulFlags = BIF_RETURNONLYFSDIRS;
					bi.iImage = 0;
					bi.lpfn = ConfigBrowseCallbackProc;
					bi.lParam = (LPARAM)(LPCSTR) initdir;
					
					ITEMIDLIST* itemList = ::SHBrowseForFolder(&bi);
					
					if (!itemList){
						pMalloc->Release();
						return FALSE;
					}
					
					if (!::SHGetPathFromIDList(itemList, &dirname[0])) {
						pMalloc->Free(itemList);
						pMalloc->Release();
						return FALSE;
					}

					SetDlgItemText(hDlg,IDC_EVISOBJ,dirname);
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					
					return TRUE;
				}
			case IDC_SBROWSE:
				{
					OPENFILENAME ofn; // common dialog box structure
					char pathname[MAX_PATH]; // buffer for file name
					char title[MAX_PATH]; // buffer for file title
					//		HWND hwnd; // owner window
					//		HANDLE hf; // file handle

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;
					//
					// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
					// use the contents of szFile to initialize itself.
					//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					ofn.lpstrFilter = "Programs (.exe)\0*.EXE\0";
					ofn.nFilterIndex = 0;
					
					ofn.lpstrInitialDir = initdir;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					
					// Display the Open dialog box. 
					
					if (GetOpenFileName(&ofn) && ofn.lpstrFile){
						SetDlgItemText(hDlg,IDC_ESHELL,ofn.lpstrFile); 
					}
					
					return TRUE;
				} 
			case IDC_RBROWSE:
				{
					OPENFILENAME ofn; // common dialog box structure
					char pathname[MAX_PATH]; // buffer for file name
					char title[MAX_PATH]; // buffer for file title
					//		HWND hwnd; // owner window
					//		HANDLE hf; // file handle

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;
					//
					// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
					// use the contents of szFile to initialize itself.
					//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					ofn.lpstrFilter = "Programs (.exe)\0*.EXE\0";
					ofn.nFilterIndex = 0;
					
					ofn.lpstrInitialDir = initdir;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					
					// Display the Open dialog box. 
					
					if (GetOpenFileName(&ofn) && ofn.lpstrFile){
						SetDlgItemText(hDlg,IDC_ERUN,ofn.lpstrFile); 
					}
					
					return TRUE;
				}
			case IDC_LBROWSE:
				{
						OPENFILENAME ofn; // common dialog box structure
					char pathname[MAX_PATH]; // buffer for file name
					char title[MAX_PATH]; // buffer for file title
					//		HWND hwnd; // owner window
					//		HANDLE hf; // file handle

					char initdir[MAX_PATH];
					lstrcpy(initdir,IMSDir);
					lstrcat(initdir,"\\");

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = pathname;
					ofn.lpstrFileTitle = title;
					//
					// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
					// use the contents of szFile to initialize itself.
					//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(pathname);
					ofn.nMaxFileTitle = sizeof(title);
					ofn.lpstrFilter = "Programs (.exe)\0*.EXE\0";
					ofn.nFilterIndex = 0;
					
					ofn.lpstrInitialDir = initdir;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					
					// Display the Open dialog box. 
					
					if (GetOpenFileName(&ofn) && ofn.lpstrFile){
						SetDlgItemText(hDlg,IDC_ELOAD,ofn.lpstrFile); 
					}
					
					return TRUE;
				}
				
			
			// Checkboxes
			
			case IDC_CSHOWDESC:
			case IDC_CIMFLAST:
			case IDC_CPOPUP:
			
				SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
				return TRUE;

				// Edit controls
			case IDC_ESHELL:
			case IDC_EPROG:
			case IDC_EFIXT:
			case IDC_EBMPS:
			case IDC_EVISOBJ:
				if (ncode == EN_CHANGE){
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
				break;
			case IDC_ERUN:
			case IDC_ELOAD:
				if (ncode == EN_CHANGE){
					if (ready){
						needRestart=TRUE;
					}
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
				
			} // End of id switch
		}
		break;
	case WM_NOTIFY:
		{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code){
			case PSN_KILLACTIVE:
				bResult = !CheckFileSettings(hDlg);
				break;
			case PSN_APPLY:
				{
					
					
					BOOL b;
					char * buf2;
					
				
					
					// Files
					buf2 = NULL;
					GetDlgItemText(hDlg,IDC_ESHELL,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"Shell",buf2,AppIni);

					buf2 = NULL;
					GetDlgItemText(hDlg,IDC_ERUN,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"run",buf2,AppIni);

					buf2 = NULL;
					GetDlgItemText(hDlg,IDC_ELOAD,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"load",buf2,AppIni);

					// Dirs
					buf2 = NULL;	
					GetDlgItemText(hDlg,IDC_EPROG,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"programs",buf2,AppIni);

					buf2 = NULL;
					GetDlgItemText(hDlg,IDC_EFIXT,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"fixtures",buf2,AppIni);

					buf2 = NULL;
					GetDlgItemText(hDlg,IDC_EBMPS,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"BMPDir",buf2,AppIni);

					buf2 = NULL;
					GetDlgItemText(hDlg,IDC_EVISOBJ,buf,sizeof(buf));
					strtrimts(buf);
					strtrimls(buf);
					if (lstrcmp(buf,"") != 0){
						buf2 = buf;
					}
					WritePrivateProfileString(AppName,"VisObjects",buf2,AppIni);

					// Options

					b = IsDlgButtonChecked(hDlg,IDC_CSHOWDESC) == BST_CHECKED;
					WritePrivateProfileString(sFILES,"ShowDescriptions",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CIMFLAST) == BST_CHECKED;
					WritePrivateProfileString(sFILES,"ShowIMFsLast",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CPOPUP) == BST_CHECKED;
					WritePrivateProfileString(sFILES,"SuppressFilePopup",(b ? "1" : "0"),AppIni);
				
					bResult = PSNRET_NOERROR;
					break;
				}
			}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;

		}
		break;
	}
	return FALSE;
}

BOOL IMSConfigIni::LOCATORDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lP){
	
	HWND cbLocName = GetDlgItem(hDlg,IDC_CBLOCNAME);
	HWND cbLocPrep = GetDlgItem(hDlg,IDC_CBLOCPREP);
	char buf[INISTRLEN] , buf2[INISTRLEN];
	switch (msg){
	case WM_INITDIALOG:
		{
			BOOL b;
			int i,j;
			// Request the vision devices from IM layer
			i = IMVisCount();
			SendMessage(cbLocName,CB_ADDSTRING,0,(LONG)(LPSTR)"(None)");
			//SendMessage(cbLocPrep,CB_ADDSTRING,0,(LONG)(LPSTR)"(None)");
			for (j=0;j<i;j++){
				b = IMVisName(buf,j);
				if (b){
					SendMessage(cbLocName,CB_ADDSTRING,0,(LONG)(LPSTR)buf);
				}	
			}
			
			// Add devices to list, select the one from the INI if it's there
			GetPrivateProfileString(AppName,"StdLocator","",buf,sizeof(buf),AppIni);
			if (strcmp(buf,"") != 0){
				int idx = SendMessage(cbLocName,CB_FINDSTRINGEXACT,-1,(LONG)(LPSTR)buf);
				if (idx >= 0){
					SendMessage(cbLocName,CB_SETCURSEL,idx,0);
					// Now get the preps
			/*		char preps[512];
					LPSTR s = preps;
					b = IMVisPreps(buf,preps);
					if (b){
						s = ParseCSV(s,prep);
						while (strcmp(prep,"") != 0){
							SendMessage(cbLocPrep,CB_ADDSTRING,0,(LONG)(LPSTR)prep);
							s = ParseCSV(s,prep); 
						}
					}           
					*/
				} else {
					sprintf(errmsg,"Selected locator %s not installed",buf);
					MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
					SendMessage(cbLocName,CB_SETCURSEL,0,0);
				}
			} else {
				SendMessage(cbLocName,CB_SETCURSEL,0,0);
			}
			
			//Select the prep from the INI if it's valid
			GetPrivateProfileString(AppName,"StdLocatorPrep","",buf2,sizeof(buf2),AppIni);
			SetDlgItemText(hDlg,IDC_PREP,buf2);

			b = GetPrivateProfileInt(AppName,"StdLocatorVisLamp",0,AppIni);
			CheckDlgButton(hDlg,IDC_CUSELAMP,b);

			b = GetPrivateProfileInt(AppName,"VisInterleave",0,AppIni);
			CheckDlgButton(hDlg,IDC_CINSPMARKS,b);

			return TRUE;
		} 
	case WM_COMMAND:
		{
			WORD id = LOWORD(wP);
			WORD ncode = HIWORD(wP);

			switch(id){
			case IDC_PREP:
				if (ncode == EN_CHANGE){
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
						return TRUE;
					}
				break;
				// Combo boxes
			case IDC_CBLOCNAME:
				{
					if (ncode == CBN_SELCHANGE){
						SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
						return TRUE;
					}
				}
				break;
			case IDC_CBLOCPREP:
				if (ncode == CBN_SELCHANGE){
					SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
					return TRUE;
				}
				break;
			// Checkboxes
			case IDC_CUSELAMP:
			case IDC_CINSPMARKS:
				SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)hDlg,0);
				return TRUE;  // Or else...
			} // end of id switch
		
		} 
		break; // end of WM_COMMAND
	case WM_NOTIFY:
		{
			BOOL bResult = FALSE;
			LPNMHDR pNmhdr = (LPNMHDR) lP;
			switch (pNmhdr->code){
			case PSN_KILLACTIVE:
				// No validation required here
				break;
			case PSN_APPLY:
				{
					
					BOOL b;

					int sel = SendMessage(cbLocName,CB_GETCURSEL,0,0);
					SendMessage(cbLocName,CB_GETLBTEXT,sel,(LPARAM) buf);

					if (sel > 0){
						WritePrivateProfileString(AppName,"StdLocator",buf,AppIni);
					} else {
						WritePrivateProfileString(AppName,"StdLocator",NULL,AppIni);
					}

					GetDlgItemText(hDlg,IDC_PREP,buf,sizeof(buf));
					WritePrivateProfileString(AppName,"StdLocatorPrep",buf,AppIni);
					
					b = IsDlgButtonChecked(hDlg,IDC_CUSELAMP) == BST_CHECKED;
					WritePrivateProfileString(AppName,"StdLocatorVisLamp",(b ? "1" : "0"),AppIni);

					b = IsDlgButtonChecked(hDlg,IDC_CINSPMARKS) == BST_CHECKED;
					WritePrivateProfileString(AppName,"VisInterleave",(b ? "1" : "0"),AppIni);
					
					bResult = PSNRET_NOERROR;
					break;
				}
			}
			
			SetWindowLong(hDlg,DWL_MSGRESULT,bResult);
			return bResult;
			
		}
		break;
	}
	
	return FALSE;
}

BOOL IMSConfigIni::CheckUIDSettings(HWND hDlg)                                 //--------------------Linghai-4-13-09------
{
	return TRUE;
}

BOOL IMSConfigIni::CheckMarkSettings(HWND hDlg)
	{
	char tbuf[INISTRLEN];
	double val;
	int intVal;
	BOOL bValid;

	DWORD dlgItm;
	
	dlgItm = IDC_ESTARTX;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToDouble(tbuf,val);
		
	if(!bValid)
		{	
		lstrcpy(errmsg,"Please enter a numeric value for startpoint X");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
		}
		
	dlgItm = IDC_ESTARTY;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToDouble(tbuf,val);
		
	if(!bValid)
		{	
		lstrcpy(errmsg,"Please enter a numeric value for startpoint Y");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
		}

	dlgItm = IDC_ESTARTZ;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToDouble(tbuf,val);
		
	if(!bValid)
		{	
		lstrcpy(errmsg,"Please enter a numeric value for startpoint Z");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
		}
		
	dlgItm = IDC_EREHOMEX;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToDouble(tbuf,val);
	if(!bValid)
		{	
		lstrcpy(errmsg,"Please enter a numeric value for rehomepoint X");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
		}

	dlgItm = IDC_EREHOMEY;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToDouble(tbuf,val);
	if(!bValid)
		{	
		lstrcpy(errmsg,"Please enter a numeric value for rehomepoint Y");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
		}

	dlgItm = IDC_EREHOMEZ;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToDouble(tbuf,val);
	if(!bValid)
		{	
		lstrcpy(errmsg,"Please enter a numeric value for rehomepoint Z");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
		}


	dlgItm = IDC_EHSTSIZE;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToInteger(tbuf,intVal);

	if(!bValid || intVal<0)
	{	
		lstrcpy(errmsg,"Please enter a valid queue size");
		MessageBox(hDlg,errmsg,AppName,MB_OK);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	int sel = SendMessage(GetDlgItem(hDlg,IDC_CBVARPOP),CB_GETCURSEL,0,0);
	if (sel == 2){
		
		dlgItm = IDC_EVARPOP;
		GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
		char ch = tbuf[0];
		bValid = (ch >= 'A' && ch <='Z') || (ch >= 'a' && ch <='z') || (ch >= '0' && ch <='9');
		if(!bValid)
		{	
			lstrcpy(errmsg,"Please enter an alphanumeric key code");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			SetFocus(GetDlgItem(hDlg,dlgItm));
			return FALSE;
		}
	}
	return TRUE;
}

void IMSConfigIni::ChangeColor(HWND hDlg, DWORD dlgItem, COLORREF & colorChanged){

	CHOOSECOLOR cc;                 // common dialog box structure 
	static COLORREF acrCustClr[16]; // array of custom colors 
	HWND hctrl = GetDlgItem(hDlg,dlgItem);
				
	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hDlg;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = colorChanged;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if (ChooseColor(&cc)==TRUE) {
		colorChanged = cc.rgbResult; 
		InvalidateRect(hctrl,NULL,FALSE);
	}
}

BOOL IMSConfigIni::CheckColorSettings(HWND hDlg){

//	BOOL ck = IsDlgButtonChecked(hDlg,IDC_CSHOWREF) == BST_CHECKED;
/*	
	if (ck){
		if (!CheckRGBValues(hDlg,IDC_EREFR,IDC_EREFG,IDC_EREFB)){
				lstrcpy(errmsg,"Please enter a valid RGB value for referenced edge color");
				MessageBox(hDlg,errmsg,AppName,MB_OK);
				return FALSE;
		}
	}

	if (!CheckRGBValues(hDlg,IDC_EFIXR,IDC_EFIXG,IDC_EFIXB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for fixture color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_ENOPR,IDC_ENOPG,IDC_ENOPB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for no part color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EPARR,IDC_EPARG,IDC_EPARB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for part color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EMARR,IDC_EMARG,IDC_EMARB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for marked part color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EPASR,IDC_EPASG,IDC_EPASB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for passed part color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_ESTAR,IDC_ESTAG,IDC_ESTAB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for started part color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EFAIR,IDC_EFAIG,IDC_EFAIB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for failed part color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EEVER,IDC_EEVEG,IDC_EEVEB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for even band color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EODDR,IDC_EODDG,IDC_EODDB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for odd band color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EORIR,IDC_EORIG,IDC_EORIB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for origin color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_ESELR,IDC_ESELG,IDC_ESELB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for selection color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EINKR,IDC_EINKG,IDC_EINKB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for ink color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}

	if (!CheckRGBValues(hDlg,IDC_EOBJR,IDC_EOBJG,IDC_EOBJB)){
			lstrcpy(errmsg,"Please enter a valid RGB value for object color");
			MessageBox(hDlg,errmsg,AppName,MB_OK);
			return FALSE;
	}
*/
	return TRUE;
}

BOOL IMSConfigIni::NeedsRestart(){
	return needRestart;
}

BOOL IMSConfigIni::CheckRGBValues(HWND hDlg, DWORD dlgItemRed, DWORD dlgItemGreen, DWORD dlgItemBlue){
	DWORD dlgItm;

	int intVal;
	BOOL bValid;

	dlgItm = dlgItemRed;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToInteger(tbuf,intVal);

	if(!bValid || intVal<0 || intVal>255)
	{	
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = dlgItemGreen;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToInteger(tbuf,intVal);

	if(!bValid || intVal<0 || intVal>255)
	{	
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = dlgItemBlue;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	bValid = StringToInteger(tbuf,intVal);

	if(!bValid || intVal<0 || intVal>255)
	{	
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	return TRUE;
}

BOOL IMSConfigIni::CheckFileSettings(HWND hDlg){
	char tbuf[INISTRLEN];
	
	BOOL bValid;
	DWORD dlgItm;

	dlgItm = IDC_ESHELL;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	char * p = strrchr(tbuf,'.');
	if (!p){
		strfnextend(tbuf,"exe");
	} else {
		p++;
		strlwr(p);
		if (lstrcmp(p,"exe") != 0){
			lstrcat(tbuf,".exe");
		}
	}
	bValid = FindFileInEnvPath(tbuf);

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The system could not find the file '%s'. Please enter a valid filename",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = IDC_ERUN;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	p = strrchr(tbuf,'.');
	if (!p){
		strfnextend(tbuf,"exe");
	} else {
		p++;
		strlwr(p);
		if (lstrcmp(p,"exe") != 0){
			lstrcat(tbuf,".exe");
		}
	}
	bValid = FindFileInEnvPath(tbuf);

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The system could not find the file '%s'. Please enter a valid filename",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = IDC_ELOAD;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	p = strrchr(tbuf,'.');
	if (!p){
		strfnextend(tbuf,"exe");
	} else {
		p++;
		strlwr(p);
		if (lstrcmp(p,"exe") != 0){
			lstrcat(tbuf,".exe");
		}
	}
	bValid = FindFileInEnvPath(tbuf);

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The system could not find the file '%s'. Please enter a valid filename",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = IDC_EPROG;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	bValid = access(tbuf,0)!= -1;

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The folder '%s' does not exist. Please select a valid folder",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = IDC_EFIXT;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	bValid = access(tbuf,0)!= -1;

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The folder '%s' does not exist. Please select a valid folder",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = IDC_EBMPS;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	bValid = access(tbuf,0)!= -1;

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The folder '%s' does not exist. Please select a valid folder",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}

	dlgItm = IDC_EVISOBJ;
	GetDlgItemText(hDlg,dlgItm,tbuf,sizeof(tbuf));
	strtrimls(tbuf);
	strtrimts(tbuf);
	bValid = access(tbuf,0)!= -1;

	if (!bValid && lstrcmp(tbuf,"")!=0){
		wsprintf(errmsg,"The folder '%s' does not exist. Please select a valid folder",tbuf);
		int i = MessageBox(hDlg,errmsg,AppName,MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg,dlgItm));
		return FALSE;
	}




	

	return TRUE;
}

BOOL IMSConfigIni::StringToInteger(LPSTR temp,int & i)
{
	// Trim the string
	strtrimls(temp);
	strtrimts(temp);

	if(strcmp(temp,"")==0){
		i = 0;
		return false;
	}

	char* stopstring;

	int val = strtol(temp,&stopstring,10);
	if(strcmp(stopstring,"")!=0)
	{
		i = 0;
		return false;
	}
	i = val;
	return true;
}

BOOL IMSConfigIni::HexStringToInteger(LPSTR temp,int & i)
{
	// Trim the string
	strtrimls(temp);
	strtrimts(temp);

	if(strcmp(temp,"")==0){
		i = 0;
		return false;
	}
	
	char* stopstring;

    int val = strtol(temp,&stopstring,16);
	if(strcmp(stopstring,"")!=0)
	{
		i = 0;
		return false;
	}
	i = val;
	return true;
}

BOOL IMSConfigIni::StringToDouble(LPSTR temp, double & d)
{
	// Trim the string
	strtrimls(temp);
	strtrimts(temp);

	if(strcmp(temp,"")==0){
		d = 0;
		return false;
	}

	// Check it now
	char* stopstring;

    double val = strtod(temp,&stopstring);
	if(strcmp(stopstring,"")!=0)
	{
		d = 0;
		return false;
	}
	d = val;
	return true;
}

BOOL IMSConfigIni::FindFileInEnvPath(LPSTR filename){
	char buf[INISTRLEN],buf2[INISTRLEN];
	char *path = getenv("PATH");
	char *p;
	
	// First check the current directory
	lstrcpy(buf2,filename);
	if(access(buf2,0) != -1){
		return TRUE;
	}
	
	// Check IMS directory
	lstrcpy(buf2,IMSDir);
	lstrcat(buf2,"\\");
	lstrcat(buf2,filename);
	if(access(buf2,0) != -1){
		return TRUE;
	}
	
    // Now check all paths in env variable PATH
	while (p = strchr(path,';')){
		*p = ',';      // Replace the semicolons with commas
	}
	
	LPSTR s = path;
	s = ParseCSV(s,buf);
	while (lstrcmp(buf,"")!=0){
		lstrcpy(buf2,buf);
		lstrcat(buf2,"\\");
		lstrcat(buf2,filename);
		if(access(buf2,0) != -1){
			return TRUE;
		}
		s = ParseCSV(s,buf);
		
	}
    // File not found	
	return FALSE;
}


void IMSConfigIni::UpdateStartPointControls(HWND hDlg){
	BOOL bReturn,bZOnly;
	
	bReturn = IsDlgButtonChecked(hDlg,IDC_CRET) == BST_CHECKED;
	bZOnly = IsDlgButtonChecked(hDlg,IDC_CZONLY) == BST_CHECKED;
	
	if (bZOnly)
		{
		EnableWindow(GetDlgItem(hDlg,IDC_CRET),FALSE);
		CheckDlgButton(hDlg,IDC_CRET,FALSE);
		
		EnableWindow(GetDlgItem(hDlg,IDC_SSTARTX),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_ESTARTX),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_SSTARTZ),FALSE);  //changed ---STARTY to STARTZ ---sam
		EnableWindow(GetDlgItem(hDlg,IDC_ESTARTZ),FALSE);   //changed ---STARTY to STARTZ---sam
		}
	else
		{
		if (bReturn)
			{
			EnableWindow(GetDlgItem(hDlg,IDC_CRET),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_SSTARTX),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ESTARTX),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_SSTARTY),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ESTARTY),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_SSTARTZ),FALSE);  //sam
			EnableWindow(GetDlgItem(hDlg,IDC_ESTARTZ),FALSE);   //sam
		}
		else
			{
			EnableWindow(GetDlgItem(hDlg,IDC_CRET),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_SSTARTX),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_ESTARTX),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_SSTARTY),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_ESTARTY),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_SSTARTZ),TRUE);   //sam
			EnableWindow(GetDlgItem(hDlg,IDC_ESTARTZ),TRUE);   //sam
			}
		}
	}

int IMSConfigIni::AddTimeName(HWND lCtrl,LPSTR secs,LPSTR tname,LPARAM data){	
	char tex[INISTRLEN];
	int ret,i=0;
	int nItems = ListView_GetItemCount(lCtrl);
	if (nItems == 0)
		{
		ret = AddListViewItem(lCtrl,secs,data);
		}
	else
		{
		for(i=0;i<nItems;i++)
			{
			ListView_GetItemText(lCtrl,i,0,tex,sizeof(tex));
			int iSecs = atoi(secs);
			int iTex = atoi(tex);
			if (iTex > iSecs)
				break;
			}
		ret = InsertListViewItem(lCtrl,secs,i,data);
		}
	ListView_SetItemText(lCtrl,i,1,tname);
	return ret;
	}

BOOL IMSConfigIni::EDITTIMEDP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP){
	HWND hSheet = GetParent(hwnd);
	HWND hPage = (HWND)SendMessage(hSheet,PSM_GETCURRENTPAGEHWND,0,0);
	HWND hListCtrl =  GetDlgItem(hPage,IDC_LVTIME);
	int selItem;
	char newtime[INISTRLEN],newtname[INISTRLEN];

	switch (message){
		
	case WM_INITDIALOG:
		{
			
			if (!bNewName){  // Edit
				selItem = ListView_GetNextItem(hListCtrl,-1,LVNI_SELECTED);
				ListView_GetItemText(hListCtrl,selItem,0,newtime,sizeof(newtime));
				ListView_GetItemText(hListCtrl,selItem,1,newtname,sizeof(newtname));
				SetDlgItemText(hwnd,IDC_ETIME,newtime);
				SetDlgItemText(hwnd,IDC_ENAME,newtname);
				SetWindowText(hwnd,"Edit Time Name");
			} else {
				SetWindowText(hwnd,"Add Time Name");
			}

			
			return TRUE;
		} // end WM_INITDIALOG
	case WM_COMMAND:
		{
			if(LOWORD(wP)==IDOK)
			{
				GetDlgItemText(hwnd,IDC_ETIME,newtime,sizeof(newtime));
				GetDlgItemText(hwnd,IDC_ENAME,newtname,sizeof(newtname));

				int tmidx,tnidx;
				int secs;

				BOOL bValid = StringToInteger(newtime,secs);
				bValid = bValid && secs >=0 && secs <=86400;

				
				if (strcmp(newtime,"")==0 || !bValid){
					MessageBox(hwnd,"Please enter a number of seconds between 0 and 86400",AppName,MB_ICONEXCLAMATION);
					return FALSE;
				}
				if (strcmp(newtname,"")==0){
					MessageBox(hwnd,"Please enter a time name",AppName,MB_ICONEXCLAMATION);
					return FALSE;
				}

				if (bNewName)  // Add new name
				{
					// Check to see if mapping exists
					tmidx = FindTime(hListCtrl,newtime);
					if (tmidx >= 0){
						wsprintf(errmsg,"The time specified already exists");
						MessageBox(hwnd,errmsg,AppName,MB_ICONEXCLAMATION);
						return FALSE;
					}
					
					tnidx = FindTimeName(hListCtrl,newtname);
					if (tnidx >= 0){
						wsprintf(errmsg,"The time name specified already exists");
						MessageBox(hwnd,errmsg,AppName,MB_ICONEXCLAMATION);
						return FALSE;
					}
					
					int nItem = AddTimeName(hListCtrl,newtime,newtname);
					/* Scroll the new item into view and select it -
					 *   If 3rd parameter of ListView_EnsureVisible() TRUE, no scrolling 
					 *   occurs if the item is partially visible.
					 */
					if (nItem>=0){
						ListView_EnsureVisible(hListCtrl,nItem,FALSE);
						ListView_SetItemState(hListCtrl,nItem,LVIS_SELECTED,LVIS_SELECTED);
					}
				} else  // Edit input	
				{
					selItem = ListView_GetNextItem(hListCtrl,-1,LVNI_SELECTED);

					// Check to see if mapping exists - but this time it could be the selection
					tmidx = FindTime(hListCtrl,newtime);
					if (tmidx >= 0 && tmidx != selItem){
						wsprintf(errmsg,"The time specified already exists");
						MessageBox(hwnd,errmsg,AppName,MB_ICONEXCLAMATION);
						return FALSE;
					}
					
					tnidx = FindTimeName(hListCtrl,newtname);
					if (tnidx >= 0 && tnidx != selItem){
						wsprintf(errmsg,"The time name specified already exists");
						MessageBox(hwnd,errmsg,AppName,MB_ICONEXCLAMATION);
						return FALSE;
					}
					
					ListView_DeleteItem(hListCtrl,selItem);
					int nItem = AddTimeName(hListCtrl,newtime,newtname);
					if (nItem>=0){
						ListView_EnsureVisible(hListCtrl,nItem,FALSE);
						ListView_SetItemState(hListCtrl,nItem,LVIS_SELECTED,LVIS_SELECTED);
					}
				} 
				
				EndDialog(hwnd,TRUE);
			}
			else if(LOWORD(wP)==IDCANCEL)
			{	
				EndDialog(hwnd,FALSE);	
			} 
		} // end WM_COMMAND
		break;
	}
	return FALSE;

}

int IMSConfigIni::FindTime(HWND lCtrl,LPSTR name){
	char buf[INISTRLEN];
	int ct = ListView_GetItemCount(lCtrl);
	for (int i=0;i<ct;i++){
		ListView_GetItemText(lCtrl,i,0,buf,sizeof(buf));
		if (strcmp(buf,name) == 0){
			return i;
		}
	}
	return -1;
}
int IMSConfigIni::FindTimeName(HWND lCtrl,LPSTR name){
	char buf[INISTRLEN];
	int ct = ListView_GetItemCount(lCtrl);
	for (int i=0;i<ct;i++){
		ListView_GetItemText(lCtrl,i,1,buf,sizeof(buf));
		if (strcmp(buf,name) == 0){
			return i;
		}
	}
	return -1;
}

int CALLBACK ConfigBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData){
   	switch (uMsg){
	case BFFM_INITIALIZED:
		SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData);
		break;
	case BFFM_SELCHANGED:
		break;
	}
	return 0;
}

