// PhoneInput.cpp : Implementation of CPhoneInput

#include "stdafx.h"
#include "PhoneInput.h"
#include "BitmapHelper.h"

static const TCHAR g_pwszClassName[] = TEXT("PhoneIMWndClass");
static const COLORREF g_colorMask = RGB(255, 0, 255);
static const DWORD g_KeyTimeout = 1000;
static const LPTSTR g_pwszBackground[2] = {TEXT("P16.png"), TEXT("L16.png")};
static const LPTSTR g_pwszDown[2] = {TEXT("P16-down.png"), TEXT("L16-down.png")};
static const LPTSTR g_pwszCtrl[2] = {TEXT("P16-ctrl.png"), TEXT("L16-ctrl.png")};

extern HINSTANCE g_hInstDll = 0;
extern TCHAR g_modulePath[MAX_PATH] = TEXT("");

static HWND g_hwndMain = 0;
static IIMCallback *g_pIMCallback = NULL;

static HIMAGELIST g_hImagelistIcon = NULL;

static HBITMAP g_hbmBackground[2];
static HBITMAP g_hbmDown[2];
static HBITMAP g_hbmMask[2];
static HBITMAP g_hbmCtrl[2];
static HBITMAP g_hbmCtrlMask[2];

static int currentOrientation = 0;

static const int g_nKeyCount = 16;

static TCHAR const orientationId[2] = {'P', 'L'};

typedef struct
{
	TCHAR key;
	LPTSTR alternateKey;
	RECT location[2];
	bool pressed;
} KEYENTRY;

static SIZE keyboardSize[2] = {{480, 305}, {800, 155}};

static KEYENTRY keyList[g_nKeyCount] = {
	{'0', NULL /*TEXT("+0")*/, {{138, 228, 268, 298}, {422, 78, 522, 148}}, false}, // 0
	{'1', NULL /*TEXT(".,-?!'@:;/()1")*/, {{3, 3, 133, 73}, {6, 3, 106, 73}}, false}, // 1
	{'2', NULL /*TEXT("abc2")*/, {{138, 3, 268, 73}, {110, 3, 210, 73}}, false}, // 2
	{'3', NULL /*TEXT("def3")*/, {{273, 3, 403, 73}, {214, 3, 314, 73}}, false}, // 3
	{'4', NULL /*TEXT("ghi4")*/, {{3, 78, 133, 148}, {318, 3, 418, 73}}, false}, // 4
	{'5', NULL /*TEXT("jkl5")*/, {{138, 78, 268, 148}, {422, 3, 522, 73}}, false}, // 5
	{'6', NULL /*TEXT("mno6")*/, {{273, 78, 403, 148}, {6, 78, 106, 148}}, false}, // 6
	{'7', NULL /*TEXT("pqrs7")*/, {{3, 153, 133, 223}, {110, 78, 210, 148}}, false}, // 7
	{'8', NULL /*TEXT("tuv8")*/, {{138, 153, 268, 223}, {214, 78, 314, 148}}, false}, // 8
	{'9', NULL /*TEXT("wxyz9")*/, {{273, 153, 403, 223}, {318, 78, 418, 148}}, false}, // 9
	{'+', NULL, {{408, 153, 477, 223}, {630, 78, 710, 148}}, false}, // 10
	{'*', NULL, {{3, 228, 133, 298}, {526, 3, 626, 73}}, false}, // 11
	{'#', TEXT(" #"), {{273, 228, 403, 298}, {526, 78, 626, 148}}, false}, // 12
	{VK_BACK, NULL, {{408, 3, 477, 73}, {630, 3, 710, 73}}, false}, // 13
	{VK_RETURN, NULL, {{408, 228, 477, 298}, {714, 78, 794, 148}}, false}, // 14
	{VK_CONTROL, NULL, {{408, 78, 477, 148}, {714, 3, 794, 73}}, false}, // 15
};

static int lastKey = NO_KEY;
static size_t keyPosition = 0; // Position in alternate key string
static DWORD lastKeyPressedTime = 0;
static int keyboardState = 0; // Keyboard mode
static int keyboardStateKey = NO_KEY; // Key mode switch key

// Get file name
LPTSTR GetFileName(LPTSTR path, LPTSTR file)
{
	_stprintf(path, L"%s\\%s", g_modulePath, file);
	return path;
}

// Get file name with user language prefix
LPTSTR GetFileNameLang(LPTSTR path, LPTSTR file)
{
	// LANGID lid = GetUserDefaultUILanguage(); - UI language, user cannot modify freely
	LANGID lid = GetUserDefaultLangID(); // User can modify throw the Control Panel 
	_stprintf(path, L"%s\\%i\\%s", g_modulePath, lid, file);
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
	{
		GetFileName(path, file);
	}
	return path;
}

int GetOrientation()
{
	HDC hdc = GetDC(GetDesktopWindow()); 
	int w = GetDeviceCaps(hdc, HORZRES);
	int h = GetDeviceCaps(hdc, VERTRES);

#ifdef _DEBUG_
	TCHAR str[80];
	_stprintf(str, L"%i x %i", h, w);
	MessageBox(NULL,str,L"Size",MB_OK);
#endif

	return h > w ? 0 : 1;
}

void LoadBitmaps(int orientation)
{
	TCHAR szFileName[MAX_PATH];
	GetFileName(szFileName, g_pwszBackground[orientation]);
	g_hbmBackground[orientation] = SHLoadImageFile(szFileName);
	GetFileName(szFileName, g_pwszDown[orientation]);
	g_hbmDown[orientation] = SHLoadImageFile(szFileName);
	g_hbmMask[orientation] = CreateBitmapMask(g_hbmDown[orientation], g_colorMask);
	GetFileName(szFileName, g_pwszCtrl[orientation]);
	g_hbmCtrl[orientation] = SHLoadImageFile(szFileName);
	g_hbmCtrlMask[orientation] = CreateBitmapMask(g_hbmCtrl[orientation], g_colorMask);
}

BOOL DestroyBitmap(HBITMAP *obj)
{
	BOOL ret = DeleteObject(obj);
	obj = NULL;
	return ret;
}

void LoadStrings()
{
#ifdef _DEBUG_
	TCHAR str[80];
#endif
	//_stprintf(str, L"%i  %x  %i  %x", lid, lid, GetUserDefaultLangID(), GetUserDefaultLangID());
	//MessageBox(NULL,str,L"Lang ID",MB_OK);
	TCHAR szFileName[MAX_PATH];
	GetFileNameLang(szFileName, TEXT("keys.txt"));
#ifdef _DEBUG_
	MessageBox(NULL,szFileName,L"Keys file",MB_OK);
#endif
	FILE * in;
	TCHAR line[MAX_KEY_LINE];

	in = _tfopen(szFileName, TEXT("rt"));
	if (in != NULL)
	{
		for (int i = 0; i < 10; i++)
		{
			if (_fgetts(line, MAX_KEY_LINE, in) != NULL)
			{
				// Trim cr lf
				TCHAR* original = line + _tcslen(line) - 1;
				while((*original == TCHAR('\r')) || (*original == TCHAR('\n')))
					original--;
				*(original + 1) = TCHAR('\0');
				keyList[i].alternateKey = _tcsdup(line);
#ifdef _DEBUG_
				_stprintf(str, L"Line: '%s'", line);
				MessageBox(NULL,str,L"Lang ID",MB_OK);
#endif
			}
		}
	}
	fclose(in);
}

void DestroyStrings()
{
	for (int i = 0; i < 10; i++)
	{
		if (keyList[i].alternateKey != NULL)
		{
			free(keyList[i].alternateKey);
		}
	}
}

int GetKeyFromPosition(int x, int y)
{
	for (int i = 0; i < g_nKeyCount; i++)
	{
		if ((keyList[i].location[currentOrientation].left <= x) &&
			(keyList[i].location[currentOrientation].right >= x) &&
			(keyList[i].location[currentOrientation].top <= y) &&
			(keyList[i].location[currentOrientation].bottom >= y))
			return i;
	}
	return NO_KEY;
}

int GetKeyFromChar(TCHAR key)
{
	for (int i = 0; i < g_nKeyCount; i++)
	{
		if (keyList[i].key == key)
			return i;
	}
	return NO_KEY;
}

void SendCharEvents(UINT key, UINT ch)
{
	UINT keyChar;
	KEY_STATE_FLAGS keyStateFlag;
	UINT *pChar;
    HRESULT hRes;

	keyChar = key;
	pChar = &ch;
	keyStateFlag = KeyStateDownFlag;
    hRes = g_pIMCallback->SendCharEvents(
        keyChar,
        keyStateFlag,
        1,
        &keyStateFlag,
        pChar);

	keyStateFlag = KeyStatePrevDownFlag | KeyShiftNoCharacterFlag;
    hRes = g_pIMCallback->SendCharEvents(
        keyChar,
        keyStateFlag,
        1,
        &keyStateFlag,
        pChar);

}

int SendKey(int key)
{
#ifdef _DEBUG_
	TCHAR str[80];
#endif

	if (key == NO_KEY) return 0;

	if (keyboardState == STATE_NUM)
	{
		SendCharEvents(keyList[key].key, keyList[key].key);
	}
	else
	{
		if ((keyList[key].alternateKey == NULL) || (_tcslen(keyList[key].alternateKey) == 0))
		{
			SendCharEvents(keyList[key].key, keyList[key].key);
		}
		else
		{
			DWORD currentTime = GetTickCount();
			if ((lastKey == key) && ((currentTime - lastKeyPressedTime) < g_KeyTimeout) && (_tcslen(keyList[key].alternateKey) > 1))
			{
				SendCharEvents(VK_BACK, VK_BACK);
				keyPosition ++;
				if (keyPosition >= _tcslen(keyList[key].alternateKey))
					keyPosition = 0;
#ifdef _DEBUG_
				_stprintf(str, L"%i >= %i", keyPosition, _tcslen(keyList[key].alternateKey[keyboardState - 1]));
				MessageBox(NULL,str,L"Size",MB_OK);
#endif
			}
			else
			{
				keyPosition = 0;
			}
			lastKeyPressedTime = currentTime;
			TCHAR ch = keyList[key].alternateKey[keyPosition];

			if (keyboardState == STATE_UPR)
				ch = _totupper(ch);
#ifdef _DEBUG_
			_stprintf(str, L"%i => %c", keyPosition, ch);
			MessageBox(NULL,str,L"Size",MB_OK);
#endif
			SendCharEvents(keyList[key].key, ch);
		}
		lastKey = key;
	}

	return 0;
}

/*
 * dc - to draw
 * key - key to draw
 * force - redraw key if unpressed state (false only after paint background :-) )
 */
void DrawKey(HDC dc, int key, bool force)
{
	if (keyList[key].pressed)
	{
		DrawBitmap(dc, 
			g_hbmDown[currentOrientation], 
			g_hbmMask[currentOrientation], 
			keyList[key].location[currentOrientation]);
	}
	else
	{
		if (force)
		{
			DrawBitmap(dc, 
				g_hbmBackground[currentOrientation], 
				keyList[key].location[currentOrientation]);
		}
	}
	if (key == keyboardStateKey)
	{
		DrawBitmap(dc,
			g_hbmCtrl[currentOrientation], 
			g_hbmCtrlMask[currentOrientation], 
			keyList[key].location[currentOrientation], 
			(keyList[key].location[currentOrientation].right - keyList[key].location[currentOrientation].left) * keyboardState,
			0);
	}
}

LRESULT WINAPI OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);

	//HBRUSH hbrush = CreateSolidBrush(RGB(127,127,255));
	//FillRect(ps.hdc, &ps.rcPaint, hbrush);
	//DeleteObject(hbrush);

	if (DrawBitmap(ps.hdc, g_hbmBackground[currentOrientation]))
	{
		for (int i = 0; i < g_nKeyCount; i++)
		{
			DrawKey(ps.hdc, i, false);
		}
	}
	else
	{
		MessageBox(hwnd, TEXT("Background bitmap drawing failed!"),
			TEXT("FATAL Error"), MB_ICONERROR + MB_OK);
	}
	EndPaint(hwnd, &ps);
	return 0;
}

LRESULT WINAPI OnMouseDown(HWND hwnd, int x, int y)
{
	int key = GetKeyFromPosition(x, y);
	if (key >= 0)
	{
		keyList[key].pressed = true;
		HDC tempDc = GetDC(g_hwndMain);
		DrawKey(tempDc, key, false);
		ReleaseDC(g_hwndMain, tempDc);
	}
	else
	{
#ifdef _DEBUG_
		MessageBox(NULL, L"Key not found.",L"Error",MB_OK+MB_ICONSTOP);
#endif
	}
	return 0;
}

LRESULT WINAPI OnMouseUp(HWND hwnd, int x, int y)
{
	int key = GetKeyFromPosition(x, y);
	if (key >= 0)
	{
		if (keyList[key].pressed)
		{
			keyList[key].pressed = false;
			if (key == keyboardStateKey)
			{
				keyboardState++;
				if (keyboardState > STATE_MAX)
					keyboardState = 0;
				lastKey = NO_KEY;
				keyPosition = 0;
			}
			HDC tempDc = GetDC(g_hwndMain);
			DrawKey(tempDc, key, true);
			ReleaseDC(g_hwndMain, tempDc);
			if (key != keyboardStateKey)
				SendKey(key);
		}
		else
		{
			for (int i = 0; i < g_nKeyCount; i++)
			{
				if (keyList[i].pressed)
				{
					keyList[i].pressed = false;
					HDC tempDc = GetDC(g_hwndMain);
					DrawKey(tempDc, key, true);
					ReleaseDC(g_hwndMain, tempDc);
				}
			}
		}
	}
	else
	{
#ifdef _DEBUG_
		MessageBox(NULL, L"Key not found.",L"Error",MB_OK+MB_ICONSTOP);
#endif
	}
	return 0;
}

LRESULT WINAPI ImWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {

    case WM_PAINT:
		return OnPaint(hwnd);
    case WM_LBUTTONDOWN:
		return OnMouseDown(hwnd, LOWORD(lParam), HIWORD(lParam));
    case WM_LBUTTONUP:
		return OnMouseUp(hwnd, LOWORD(lParam), HIWORD(lParam));
    //case WM_MOUSEMOVE:
    //case WM_LBUTTONDBLCLK:
    //    return IM_OnMouseEvent(hwnd, msg, wParam, lParam);
    //        
    } // switch(message)

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


// CPhoneInput

HRESULT CPhoneInput::Select(HWND hwndSip)
{
#ifdef _DEBUG_
	MessageBox(NULL,L"Select",L"",MB_OK);
#endif

	LoadStrings();

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = ImWndProc;
    wc.hInstance = g_hInstDll;
    wc.hbrBackground = NULL;
    wc.lpszClassName = g_pwszClassName;
	wc.lpszMenuName = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	if(!RegisterClass(&wc)) 
    {
        return E_FAIL;
    }

	//currentOrientation = GetOrientation();

    g_hwndMain = CreateWindow(g_pwszClassName, TEXT(""), WS_CHILD,
		0, 0, 
		keyboardSize[currentOrientation].cx, 
		keyboardSize[currentOrientation].cy, 
		hwndSip, (HMENU)NULL, g_hInstDll, NULL);

	if (g_hwndMain == NULL)
	{
        return E_FAIL;
	}

	keyboardStateKey = GetKeyFromChar(VK_CONTROL);
	if (keyboardStateKey == NO_KEY)
	{
        return E_FAIL;
	}

    ShowWindow(g_hwndMain, SW_SHOWNOACTIVATE);

	return S_OK;
}

HRESULT CPhoneInput::Deselect(void)
{
#ifdef _DEBUG_
	MessageBox(NULL,L"Deselect",L"",MB_OK);
#endif
	// Free the currently allocated image list.
	if (g_hImagelistIcon)
		ImageList_Destroy(g_hImagelistIcon);
	g_hImagelistIcon = NULL;

	// Free the currently allocated images.
    if (g_hbmBackground[0]) 
		DestroyBitmap(&g_hbmBackground[0]);
    if (g_hbmBackground[1]) 
		DestroyBitmap(&g_hbmBackground[1]);
    if (g_hbmDown[0]) 
	    DestroyBitmap(&g_hbmDown[0]);
    if (g_hbmDown[1]) 
	    DestroyBitmap(&g_hbmDown[1]);
    if (g_hbmMask[0]) 
	    DestroyBitmap(&g_hbmMask[0]);
    if (g_hbmMask[1]) 
	    DestroyBitmap(&g_hbmMask[1]);

	// Free and unregister window
    DestroyWindow(g_hwndMain);
    UnregisterClass(g_pwszClassName, g_hInstDll);

    return S_OK;
}


HRESULT CPhoneInput::Showing(void)
{
#ifdef _DEBUG_
	MessageBox(NULL,L"Showing",L"",MB_OK);
#endif
	currentOrientation = GetOrientation();

	if (!g_hbmBackground[currentOrientation])
		LoadBitmaps(currentOrientation);

	//SetWindowPos(g_hwndMain, HWND_TOP, 0, 0, 
	//	keyboardSize[currentOrientation].x, 
	//	keyboardSize[currentOrientation].y, 
	//	SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
	MoveWindow(
        g_hwndMain,
        0,
        0,
		keyboardSize[currentOrientation].cx, 
		keyboardSize[currentOrientation].cy, 
        FALSE );

	ShowWindow(g_hwndMain, SW_SHOW);
    return S_OK;
}


HRESULT CPhoneInput::Hiding(void)
{
#ifdef _DEBUG_
	MessageBox(NULL,L"Hiding",L"",MB_OK);
#endif
    return S_OK;
}

HRESULT CPhoneInput::GetInfo(/* [out] */ IMINFO *pInfo)
{
#ifdef _DEBUG_
	MessageBox(NULL,L"GetInfo",L"",MB_OK);
#endif
    HBITMAP hbm;

	// Free the currently allocated image list.
	if (g_hImagelistIcon)
		ImageList_Destroy(g_hImagelistIcon);

	// Allocate new image list.
    g_hImagelistIcon = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 1, 1);

	// 16X16 pen icon
	if (hbm = LoadBitmap(g_hInstDll, MAKEINTRESOURCE(IDB_BITMAP_PHONE_KEYPAD))) 
	{
		ImageList_AddMasked(g_hImagelistIcon, hbm, RGB(255,255,255));
		DeleteObject(hbm);
	}
	ASSERT(hbm); 
	
    pInfo->fdwFlags = SIPF_DOCKED | SIPF_ON;
    pInfo->hImageNarrow = (HANDLE)g_hImagelistIcon;
    pInfo->hImageWide = (HANDLE)g_hImagelistIcon;
    pInfo->rcSipRect.left = 0;
	pInfo->rcSipRect.top = 0;
    pInfo->rcSipRect.right = keyboardSize[currentOrientation].cx; //BITMAP_WIDTH;
    pInfo->rcSipRect.bottom = keyboardSize[currentOrientation].cx; //BITMAP_HEIGHT;

    /*
	pInfo->iNarrow = pInfo->iWide = 0;
    pInfo->rcSipRect.left = 0;
    pInfo->rcSipRect.right = PannelWidth;
    pInfo->rcSipRect.bottom = 320-26;
    pInfo->rcSipRect.top = pInfo->rcSipRect.bottom - PannelHeight;
	*/

    return NOERROR;
}

HRESULT CPhoneInput::ReceiveSipInfo(SIPINFO *psi)
{
#ifdef _DEBUG_
	MessageBox(NULL,L"ReceiveSipInfo",L"",MB_OK);
#endif

	currentOrientation = GetOrientation();

	MoveWindow(
        g_hwndMain,
        0,
        0,
        psi->rcSipRect.right - psi->rcSipRect.left,
        psi->rcSipRect.bottom - psi->rcSipRect.top,
        FALSE );

	if (psi->rcSipRect.bottom - psi->rcSipRect.top != keyboardSize[currentOrientation].cy)
	{
#ifdef _DEBUG_
		MessageBox(g_hwndMain,L"Not Big Enough",L"",MB_OK);
#endif
		if (g_pIMCallback)
		{
#ifdef _DEBUG_
			MessageBox(g_hwndMain,L"Resizing...",L"",MB_OK);
#endif
			IMINFO Info;
			Info.cbSize = sizeof(Info);
			Info.fdwFlags = SIPF_DOCKED | SIPF_OFF;
			Info.hImageNarrow = (HANDLE)g_hImagelistIcon;
			Info.hImageWide = (HANDLE)g_hImagelistIcon;
			Info.iNarrow = Info.iWide = 0;
			Info.rcSipRect.left = 0;
			Info.rcSipRect.right = keyboardSize[currentOrientation].cx;
			Info.rcSipRect.bottom = psi->rcSipRect.bottom; //800-26;
			Info.rcSipRect.top = Info.rcSipRect.bottom - keyboardSize[currentOrientation].cy;
			g_pIMCallback->SetImInfo(&Info);
		}
	}

	return S_OK;
}

HRESULT CPhoneInput::RegisterCallback(IIMCallback *pIMCallback)
{
	g_pIMCallback = pIMCallback;
    return S_OK;
}