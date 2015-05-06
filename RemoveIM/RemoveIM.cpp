#include <windows.h>
#include <string.h>
#include <sipapi.h>

// Default SIP CLSID
const LPTSTR defaultSipClsidString = TEXT("{42429667-ae04-11d0-a4f8-00aa00a749b9}");
CLSID defaultSipClsid;

/*
// Not used
int SipEnumIMProc(IMENUMINFO *pIMInfo)
{
	TCHAR text[200];
	LPOLESTR pwszClsid;
	CLSID* pCLSID = new CLSID;
	memcpy(pCLSID, &pIMInfo->clsid, sizeof(CLSID));

	
	StringFromCLSID(*pCLSID, &pwszClsid);

	_stprintf(text, L"Name: %s CLSID: %s", &pIMInfo->szName, pwszClsid);

	if (defaultSipClsid == *pCLSID)
		MessageBox(NULL,text,L"Keypad",MB_OK);
	return 1;

}
*/

int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPWSTR szCmdLine, 
	int nShowCmd)
{	

	// Select some other SIP here, for example default one
	if (SUCCEEDED(CLSIDFromString(defaultSipClsidString, &defaultSipClsid)))
		SipSetCurrentIM(&defaultSipClsid);

	TCHAR path[MAX_PATH];

	GetModuleFileName( NULL, path, MAX_PATH );
	TCHAR *bs = wcsrchr(path, TCHAR('\\'));
	bs++;
	*bs = TCHAR('\0');
	wcsncat(path, L"PhoneKeypad.dll", MAX_PATH);
	//MessageBox(NULL,path,L"Path",MB_OK);

	// Load our special driver. Note, that full name needs to be here 
	// unless our driver resides in \windows folder	
	HANDLE hDevice = RegisterDevice(L"UIM", 8, path, 0);
	if (hDevice)		
	{
		// All ok, we did it, unloading our driver		
		DeregisterDevice(hDevice);
		MessageBox(NULL, L"Done", L"Info", MB_OK | MB_ICONINFORMATION);

	}
	else
	{
		MessageBox(NULL, L"Error", L"Error", MB_OK | MB_ICONSTOP);
	}

	return 0;
}
