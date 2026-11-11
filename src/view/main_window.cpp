#include "main_window.h"
#include <d2d1.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <debugapi.h>
#include <dxgi1_3.h>
#include <winuser.h>

//==============================================================================
// Namespaces / Enums / Constants
//==============================================================================

static const DWORD WINDOW_STYLE = WS_VISIBLE | WS_POPUP;
static const DWORD WINDOW_STYLE_EX = WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST;

//==============================================================================
// Constructors
//==============================================================================

MainWindowView::MainWindowView(const HINSTANCE hIn) : hWnd(NULL), activeModalDialog(NULL),
hInstance(hIn) {
    //windowPresenter.setMainWindowInterface(this);
}

MainWindowView::~MainWindowView() {
}

//==============================================================================
// Public Functions
//==============================================================================

//------------------------------------------------------------------------------
// createWindow - Creates the main Window.
//------------------------------------------------------------------------------

bool MainWindowView::createWindow() {

    if(hWnd) {
        return false;
    }

    hWnd = CreateWindowExW(WINDOW_STYLE_EX, L"SeasonalLights", L"", WINDOW_STYLE, 0, 0, 320, 240, NULL, NULL, hInstance, this);

    if(hWnd == NULL) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // TODO: If the monitor size should ever change we need to reset this and re-create the D2D stuff.

    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
    MoveWindow(hWnd, mi.rcMonitor.left, mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top, FALSE);

    return true;
}

//------------------------------------------------------------------------------
// doLoop - Standard run of the mill message loop
//------------------------------------------------------------------------------

WPARAM MainWindowView::doLoop() {

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) > 0) {

        if (activeModalDialog && IsDialogMessage(activeModalDialog, &msg)) {
        }
        else if (!IsDialogMessageW(hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

    }

    return msg.wParam;
}

//------------------------------------------------------------------------------
// initCOM - Initialize the COM interface, as well as a few COM related objects.
//------------------------------------------------------------------------------

bool MainWindowView::initCOM() {

    if(CoInitializeEx(NULL, 0) != S_OK) {
        MessageBoxW(NULL, L"Error Initializing COM.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    switch(mainWindowController.initWIC()) {

        case ControllerErrors::WIC_INIT_ERROR:
            MessageBoxW(NULL, L"Error Initializing WIC Imaging Factory.", L"Error", MB_OK | MB_ICONERROR);
            return false;
            break;

        case ControllerErrors::GIF_INIT_ERROR:
            MessageBoxW(NULL, L"Error Initializing GIF decoder.", L"Error", MB_OK | MB_ICONERROR);
            return false;
            break;

    }

    return true;
}

//------------------------------------------------------------------------------
// initDirect2D - Initialize Direct2D
//------------------------------------------------------------------------------

bool MainWindowView::initDirect2D() {

    RECT rect = { 0 };
    GetClientRect(hWnd, &rect);
    return mainWindowController.getDirect2DLayer()->init(rect.right - rect.left, rect.bottom - rect.top, hWnd);

}

void MainWindowView::shutdown() {

    // Release the resources held by Direct2D
    mainWindowController.releaseResources();

}

void MainWindowView::loadBulb() {

    if(!mainWindowController.loadBulb("D:\\dump\\12.bul")) {
        MessageBoxW(hWnd, L"TODO: Catch 'loadBulb' Error", L"", MB_ICONERROR | MB_OK);
    }
    SetTimer(hWnd, 0, 100, NULL);

}

//------------------------------------------------------------------------------
// registerSelf - Registers the Window class.
//------------------------------------------------------------------------------

bool MainWindowView::registerSelf() {

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowView::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"SeasonalLights";
    wc.hIconSm = LoadIconW(NULL, IDI_APPLICATION);

    //wc.hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, iconXSize, iconYSize, 0);
    //wc.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, iconSmallXSize, iconSmallYSize, 0);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Error registering window class.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

//==============================================================================
// Window Procedure
//==============================================================================

//------------------------------------------------------------------------------
// windowProc - Standard window procedure for a window
//------------------------------------------------------------------------------

LRESULT MainWindowView::windowProc(const UINT& msg, const WPARAM wParam, const LPARAM lParam) {

    INT_PTR retVal = 0;

    switch (msg) {

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
        case WM_PAINT:
            onPaint();
            // If this doesn't work, DefWindowProc might do more than validate the region,
            // but this *should* work.
            ValidateRect(hWnd, NULL);
            return 0;
            //return DefWindowProcW(hWnd, msg, wParam, lParam);


        case WM_TIMER:
            InvalidateRect(hWnd, NULL, TRUE);
            SetTimer(hWnd, 0, 100, NULL);
            break;

        case WM_CLOSE:
            //onClose();
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return 0;
}

bool MainWindowView::onPaint() {

    // TODO: Handle no valid bulb

    const DXInfo* dxInfo = mainWindowController.getDirect2DLayer()->getDXInfo();


    dxInfo->dc->BeginDraw();
    dxInfo->dc->Clear();

    //const D2DImageInfo* bulbInfo = christmasBulb.getBulbInfo(BulbSide::TOP, 0);

    const std::vector<ChristmasBulbPackage*>& christmasBulb = mainWindowController.getChristmasBulbs();
    const SideInfo sideInfo = mainWindowController.getSizeInfo(BulbSide::TOP);

    MONITORINFO mi = {0};
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
    int midPoint = abs(mi.rcMonitor.right - mi.rcMonitor.left);
    FLOAT offset = (midPoint - sideInfo.sideLength) / 2;



    D2D1_RECT_F dest = { offset, 0, offset, 0};
    size_t idx = 0;
    size_t flavor = 0;
    for(size_t i = 0; i < sideInfo.curFrame.size(); ++i) {

        const D2DImageInfo* bulbInfo = christmasBulb[idx]->getBulbInfo(BulbSide::TOP, flavor);

        dest.bottom = bulbInfo->height;
        dest.right += bulbInfo->width;

        dxInfo->dc->DrawBitmap(bulbInfo->frames[sideInfo.curFrame[i]], &dest);

        // TODO: Update all frames all at one, not while drawing.
        /*
        sideInfo.curFrame[i]++;
        if (sideInfo.curFrame[i] >= bulbInfo->frames.size()) {
            sideInfo.curFrame[i] = 0;

        }
        */

        dest.left = dest.right;
        idx++;

        if(idx >= christmasBulb.size()) {
            idx = 0;
            flavor++;
        }

    }

    // TODO: Check for errors

    HRESULT status = dxInfo->dc->EndDraw();
    //__D3D_IS_OK__(status);

    status = dxInfo->swapChain->Present(1, 0);
    //__D3D_IS_OK__(status);

    return true;

}

void MainWindowView::initSize() {

    MONITORINFO mi = {0};
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
    mainWindowController.updateMonitorSize(mi);

    // TODO: What if there is no corner bulb?

    /*
    sideInfo.sideLength = abs(mi.rcMonitor.right - mi.rcMonitor.left);
    sideInfo.sideLength -= (christmasBulb[0]->getBulbInfo(BulbSide::TOP_LEFT, 0)->width + christmasBulb[0]->getBulbInfo(BulbSide::TOP_RIGHT, 0)->width);

    //const LONG height = abs(mi.rcMonitor.bottom - mi.rcMonitor.top);

    LONG realLen = 0;
    size_t numBulbs = 0;
    size_t idx = 0;
    size_t flavor = 0;

    for(int i = 0;; ++i) {

        const D2DImageInfo* bulbInfo = christmasBulb[idx]->getBulbInfo(BulbSide::TOP, flavor);

        if(realLen + bulbInfo->width > sideInfo.sideLength) {
            break;
        }

        realLen += bulbInfo->width;
        numBulbs++;
        idx++;

        if(idx >= christmasBulb.size()) {
            idx = 0;
            flavor++;
        }

    }

    sideInfo.sideLength = realLen;
    sideInfo.curFrame.resize(numBulbs);
    */

    /*
    char buf[256] = {0};
    sprintf(buf, "Side Len: %d  | Bulbs: %d", sideInfo.sideLength, sideInfo.curFrame.size());
    MessageBoxA(NULL, buf, "", MB_OK);
    */


}

//==============================================================================
// Private Functions
//==============================================================================
