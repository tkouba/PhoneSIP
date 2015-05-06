/*
 * Simple driver that only calls CoFreeUnusedLibrariesEx() 
 * inside the device.exe process
 */
#include "stdafx.h"
#include <windows.h>

// For COM
#include <objbase.h>

// Need to link against ole32.lib as well for all this COM stuff
#pragma comment(lib, "ole32.lib")

// Thread that does actual call to CoFreeUnusedLibrariesEx()
DWORD CALLBACK FreeUnusedLibrariesThread(LPVOID)
{
	// Need to initialize COM as this is the separate thread, 
	// where COM was not used yet
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		// Do our important call
		CoFreeUnusedLibrariesEx(0, 0);

		// Finalize COM for this thread
		CoUninitialize();

		return 0;
	}
	else
		return 1;
}

// Macro that simplifies declarations of driver entry points, 
// i.e. it marks function as exported from this DLL and also 
// adds the same prefix to all entry point names
#define DRIVER_ENTRY(ret_type, name) \
	extern "C" ret_type __declspec(dllexport) UIM_ ## name

// Function that is called by device.exe just after driver has been loaded
DRIVER_ENTRY(DWORD, Init)(
	LPCTSTR pContext,
	LPCVOID lpvBusContext)
{	
	// Create a seperate thread for CoFreeUnusedLibrariesEx, 
	// because COM checks owner process ID (which is ID of process 
	// where we call RegisterDevice() in our case) to be equal 
	// to current process ID (device.exe)
	HANDLE hThread = CreateThread(NULL, 0, FreeUnusedLibrariesThread, NULL, 0, NULL);
	if (hThread)
	{		
		// Waiting for completition of our thread
		if (WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0)
			return 1;
	}

	return 0;
}

// Function that is called just before unloading of the driver 
// from device.exe process
DRIVER_ENTRY(BOOL, Deinit)(DWORD hDeviceContext)
{
	// Successfully do nothing in our case
	return TRUE;
}

//
// The functions bellow are called when user code works with 
// our device driver using CreateFile()/DeviceIOControl()/CloseHandle() 
// API functions, but in our case they are needed only as labels for 
// the device manager to treat our DLL as a driver
//

DRIVER_ENTRY(DWORD, Open)(
	DWORD hDeviceContext,
	DWORD AccessCode,
	DWORD ShareMode 
)
{
	// Our device cannot and need not to be opened
	return 0;
}

DRIVER_ENTRY(BOOL, Close)(
	DWORD hOpenContext 
)
{
	// Should be never called 
	return FALSE;
}

DRIVER_ENTRY(BOOL, IOControl)(
	DWORD hOpenContext,
	DWORD dwCode,
	PBYTE pBufIn,
	DWORD dwLenIn,
	PBYTE pBufOut,
	DWORD dwLenOut,
	PDWORD pdwActualOut 
)
{
	// Should be never called 
	return FALSE;
}
