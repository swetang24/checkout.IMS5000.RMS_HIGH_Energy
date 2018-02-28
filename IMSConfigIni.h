// IMSConfigIni.h: interface for the IMSConfigIni class.
//
//////////////////////////////////////////////////////////////////////

#ifndef IMSCFGINI
#define IMSCFGINI

// IMS Settings...
BOOL CALLBACK MARKINGDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK UID06CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK UID06CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK UID05DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK UID12CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK UID12CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK COMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK COLORSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK SYSTEMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK FILESDIRSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK LOCATORDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
BOOL CALLBACK EDITTIMEDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
int CALLBACK ConfigBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
    
class IMSConfigIni  
{	
	char AppName[20];
	char AppIni[MAX_PATH];
	char errmsg[256];
	HWND hWndPaint;
	COLORREF refEdgeColor;
	COLORREF fixtureColor;
	COLORREF noPartColor;
	COLORREF partColor;
	COLORREF startedPartColor;
	COLORREF markedPartColor;
	COLORREF passedPartColor;
	COLORREF failedPartColor;
	COLORREF evenBandColor;
	COLORREF oddBandColor;
	COLORREF inkColor;
	COLORREF selColor;
	COLORREF objectColor;
	COLORREF originColor;

	BOOL needRestart;
	BOOL bNewName;
public:
	IMSConfigIni();
	~IMSConfigIni();
	BOOL initialize(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni);
	BOOL initializeCOM(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni);
	BOOL initializeUID06(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni);
BOOL initializeUID05(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni);
BOOL initializeUID12(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni);
	BOOL initializeUID(HWND hwnd,HINSTANCE hinst,LPSTR LName,LPSTR LIni);           //-----------------test--
	BOOL MARKINGDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL UID06CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL UID05DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL UID12CON1DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL UID06CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL UID12CON2DP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
    BOOL COMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL COLORSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL SYSTEMDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL FILESDIRSDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);
	BOOL LOCATORDP(HWND hDlg,UINT msg,WPARAM wP,LPARAM lp);

	BOOL CheckMarkSettings(HWND hDlg);
	BOOL CheckUIDSettings(HWND hDlg);
	BOOL CheckFileSettings(HWND hDlg);
	BOOL CheckColorSettings(HWND hDlg);
	void ChangeColor(HWND hDlg, DWORD dlgItem, COLORREF & colorChanged);

	BOOL CheckRGBValues(HWND hDlg, DWORD dlgItemRed, DWORD dlgItemGreen, DWORD dlgItemBlue);

	BOOL FindFileInEnvPath(LPSTR filename);

	BOOL StringToInteger(LPSTR temp,int & i);
	BOOL StringToDouble(LPSTR temp,double & d);
	BOOL HexStringToInteger(LPSTR temp,int & i);

	void UpdateStartPointControls(HWND hDlg);

	int AddTimeName(HWND lCtrl,LPSTR name,LPSTR value,LPARAM data=0);
	int FindTime(HWND lCtrl,LPSTR name);
	int FindTimeName(HWND lCtrl,LPSTR name);

	BOOL NeedsRestart();

	BOOL EDITTIMEDP(HWND hwnd,UINT message,WPARAM wP,LPARAM lP);

	
	



};

#endif IMSCFGINI


