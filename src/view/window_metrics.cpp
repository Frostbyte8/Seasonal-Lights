#include "window_meterics.h"

WindowMetrics::WindowMetrics() : fontHDC(0), curFont(0), oldFont(0), avgCharWidth(0), avgCharHeight(0), xScaleFactor(96.0f), yScaleFactor(96.0f) {
}

WindowMetrics::~WindowMetrics() {
    if(curFont) {
        SelectObject(fontHDC, oldFont);
        DeleteObject(curFont);
        DeleteDC(fontHDC);
    }
}

void WindowMetrics::initWindowMetrics(float inScaleX, float inScaleY) {

    xScaleFactor = inScaleX;
    yScaleFactor = inScaleY;

    if(curFont) {
        SelectObject(fontHDC, oldFont);
        DeleteObject(curFont);
        DeleteDC(fontHDC);
    }

    NONCLIENTMETRICS ncm;
    ZeroMemory(&ncm, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

	OSVERSIONINFOEX osVer;
	ZeroMemory(&osVer, sizeof(osVer));
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    // The iPaddedBorderWidth method does not exist prior to Windows Vista
    #pragma warning( push )
	#pragma warning( disable: 4996)
	GetVersionEx(reinterpret_cast<OSVERSIONINFO *>(&osVer));
	#pragma warning( pop )

	if(osVer.dwMajorVersion < 6) {
		ncm.cbSize -= sizeof(ncm.iPaddedBorderWidth);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm) - sizeof(int),
							 &ncm, 0);
	}
	else {
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm),
							 &ncm, 0);
    }

    TEXTMETRIC tm;
    ZeroMemory(&tm, sizeof(TEXTMETRIC));

    // Scale the DPI as needed.

    ncm.lfMessageFont.lfWidth *= static_cast<LONG>(xScaleFactor / 96.0f);
    ncm.lfMessageFont.lfHeight *= static_cast<LONG>(yScaleFactor / 96.0f);

    fontHDC     = CreateCompatibleDC(NULL);
    curFont     = CreateFontIndirect(&ncm.lfMessageFont);
    oldFont     = reinterpret_cast<HFONT>(SelectObject(fontHDC, curFont));
    GetTextMetrics(fontHDC, &tm);
    // We only care about these two settings.
    avgCharWidth    = tm.tmAveCharWidth;
    avgCharHeight   = tm.tmHeight;


    // Margins / Spacing

    cs.XWINDOW_MARGIN               = XDLU2PIX(7);
    cs.YWINDOW_MARGIN               = YDLU2PIX(7);

    cs.XLABELASSOC_MARGIN           = XDLU2PIX(3);
    cs.YLABELASSOC_MARGIN           = YDLU2PIX(3);

    // MSDN Says 3, but because of the comment below, it's actually 2.
    cs.YLABELSAMELINE_MARGIN        = YDLU2PIX(2);

    cs.XRELATED_MARGIN              = XDLU2PIX(4);
    cs.YRELATED_MARGIN              = YDLU2PIX(4);

    cs.XUNRELATED_MARGIN            = XDLU2PIX(7);
    cs.YUNRELATED_MARGIN            = YDLU2PIX(7);

    cs.XGROUPBOX_MARGIN             = XDLU2PIX(6);
    cs.YFIRST_GROUPBOX_MARGIN       = YDLU2PIX(11);
    cs.YLAST_GROUPBOX_MARGIN        = YDLU2PIX(7);

    // Dimensions

    cd.YLABEL                       = YDLU2PIX(8);

    cd.XBUTTON                      = XDLU2PIX(50);
    cd.YBUTTON                      = YDLU2PIX(14);

    // MSDN says to use 14, however it seems that many applications, MS
    // applications included, use 12 if the button is alone, and only use the 14
    // that is documented when the textbox is on the same row as a button,
	// 14 just so happens to be the same height as a button.

    // You can see this for yourself in the Internet Options dialog box: The
    // the text boxes are 12 DLUs (20px~), while the one in the Accessibility
    // sub-dialog for the style sheet is 14 (23px~) since it's on the same row
    // as a button! Maybe MSDN is wrong? Maybe they forgot to mention this?

    // Dialer.exe also uses 12 DLUs, which only leads me to believe MSDN is wrong.

    cd.YTEXTBOX_ONE_LINE_ALONE     = YDLU2PIX(12);

    // A Few extras that are obtained via GetSystemMetrics
#ifdef _DPI_AWARE_
    cd.XSCROLLBAR                   = GetSystemMetricsForDpi(SM_CXVSCROLL, static_cast<UINT>(xScaleFactor));
    cd.YSCROLLBAR                   = GetSystemMetricsForDpi(SM_CYHSCROLL, static_cast<UINT>(xScaleFactor));
#else
    cd.XSCROLLBAR                   = GetSystemMetrics(SM_CXVSCROLL);
    cd.YSCROLLBAR                   = GetSystemMetrics(SM_CYHSCROLL);
#endif // _DPI_AWARE_

}

inline int WindowMetrics::XDLU2PIX(const int& x) const {
    return MulDiv(x, avgCharWidth, 4);
}

inline int WindowMetrics::YDLU2PIX(const int& y) const {
    return MulDiv(y, avgCharHeight, 8);
}

LONG WindowMetrics::calculateStringWidth(const wchar_t* str) const {
    const int numChars = static_cast<int>(wcslen(str));
    SIZE size;
    GetTextExtentPoint32(fontHDC, str, numChars, &size);
    return size.cx;
}
