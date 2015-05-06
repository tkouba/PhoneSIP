// PhoneKeypad.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include <windows.h>

#ifdef POCKETPC2003_UI_MODEL
#include "resource.h"
#endif 

#include "PhoneKeypad.h"

#include "PhoneInput.h"

extern HINSTANCE g_hInstDll;
extern TCHAR g_modulePath[MAX_PATH];

// CLSID Keyboard ...
const LPTSTR defaultSipClsidString = TEXT("{42429667-ae04-11d0-a4f8-00aa00a749b9}");
CLSID defaultSipClsid;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_PhoneInput, CPhoneInput)
END_OBJECT_MAP()


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, (HINSTANCE)hInstance);
		g_hInstDll = (HINSTANCE)hInstance;
		GetModuleFileName(g_hInstDll, g_modulePath, MAX_PATH);
		LPTSTR sl = _tcsrchr(g_modulePath, '\\');
		if (sl)
		{
			*sl = '\0';
		}
		else
		{
			g_modulePath[0] = '\0';
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;    // ok
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer(TRUE);
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _Module.UnregisterServer();

	if (SUCCEEDED(CLSIDFromString(defaultSipClsidString, &defaultSipClsid)))
		SipSetCurrentIM(&defaultSipClsid);

	TCHAR path[MAX_PATH];
	GetModuleFileName(g_hInstDll, path, MAX_PATH);
		// Load our special driver. Note, that full name needs to be here 
	// unless our driver resides in \windows folder	
	HANDLE hDevice = RegisterDevice(L"UIM", 8, path, 0);
	if (hDevice)		
	{
		// All ok, we did it, unloading our driver		
		DeregisterDevice(hDevice);
	}

	return hr;
}

