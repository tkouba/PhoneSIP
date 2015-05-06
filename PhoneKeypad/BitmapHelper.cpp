/*
 * Simple bitmap helper. 
 * Contains function for creating bitmap masks and drawing bitmaps.
 */
#include "stdafx.h"
#include "BitmapHelper.h"

// Create bitmap mask for transparent drawing
HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask;
    BITMAP bm;

    // Create monochrome (1 bit) mask bitmap.  
    GetObject(hbmColour, sizeof(BITMAP), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	if (hbmMask)
	{
		// Get some HDCs that are compatible with the display driver
		hdcMem = CreateCompatibleDC(NULL);
		hdcMem2 = CreateCompatibleDC(NULL);

		SelectBitmap(hdcMem, hbmColour);
		SelectBitmap(hdcMem2, hbmMask);

		// Set the background colour of the colour image to the colour
		// you want to be transparent.
		SetBkColor(hdcMem, crTransparent);

		// Copy the bits from the colour image to the B+W mask... everything
		// with the background colour ends up white while everythig else ends up
		// black...Just what we wanted.
		BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

		// Take our new mask and use it to turn the transparent colour in our
		// original colour image to black so the transparency effect will
		// work right.
		BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);
		
		// Clean up.
		if (hdcMem) DeleteDC(hdcMem);
		if (hdcMem2) DeleteDC(hdcMem2);
	}
    return hbmMask;
}

// Draw bitmap region to specified DC
BOOL DrawBitmap(HDC hdc, HBITMAP hbm, RECT rc)
{
	HDC hdcMem;
	BOOL ret = FALSE;

	hdcMem = CreateCompatibleDC(hdc);	
	if (hdcMem)
	{
		SelectBitmap(hdcMem, hbm);
		ret = BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, rc.left, rc.top, SRCCOPY);
		DeleteDC(hdcMem);
	}
	return ret;
}

// Draw bitmap to specified DC
BOOL DrawBitmap(HDC hdc, HBITMAP hbm)
{
	BITMAP bm;
	RECT rc;

	GetObject(hbm, sizeof(BITMAP), &bm);
	rc.left = 0;
	rc.top = 0;
	rc.right = bm.bmWidth;
	rc.bottom = bm.bmHeight;

	return DrawBitmap(hdc, hbm, rc);
}

// Draw transparent bitmap region to specified DC
BOOL DrawBitmap(HDC hdc, HBITMAP hbm, HBITMAP hbmMask, RECT rc)
{
	HDC hdcMem;
	BOOL ret = FALSE;

	hdcMem = CreateCompatibleDC(hdc);
	if (hdcMem)
	{
		SelectBitmap(hdcMem, hbmMask);
		ret = BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, rc.left, rc.top, SRCAND);

		if (ret)
		{
			SelectBitmap(hdcMem, hbm);
			ret = BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, rc.left, rc.top, SRCPAINT);
		}
		DeleteDC(hdcMem);
	}
	return ret;
}

// Draw transparent bitmap region to specified DC
BOOL DrawBitmap(HDC hdc, HBITMAP hbm, HBITMAP hbmMask, RECT rc, int srcLeft, int srcTop)
{
	HDC hdcMem;
	BOOL ret = FALSE;

	hdcMem = CreateCompatibleDC(hdc);
	if (hdcMem)
	{
		SelectBitmap(hdcMem, hbmMask);
		ret = BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, srcLeft, srcTop, SRCAND);

		if (ret)
		{
			SelectBitmap(hdcMem, hbm);
			ret = BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, srcLeft, srcTop, SRCPAINT);
		}
		DeleteDC(hdcMem);
	}
	return ret;
}