
#include <windows.h>
#include <windowsx.h>


HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent);

BOOL DrawBitmap(HDC hdc, HBITMAP hbm);
BOOL DrawBitmap(HDC hdc, HBITMAP hbm, RECT rc);
BOOL DrawBitmap(HDC hdc, HBITMAP hbm, HBITMAP hbmMask, RECT rc);
BOOL DrawBitmap(HDC hdc, HBITMAP hbm, HBITMAP hbmMask, RECT rc, int srcLeft, int srcTop);
