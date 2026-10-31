#include <Windows.h>
#include <winuser.h>
#include "view/main_window.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(_MSC_VER) && defined(_DEBUG)

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif // defined(_MSC_VER) && defined(_DEBUG)

    MainWindowView mainWindowView(hInstance);
    UINT_PTR retVal = 0;

    if(!mainWindowView.initCOM()) {
        MessageBoxW(NULL, L"Could not initialize COM. The program will now close.", L"COM Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    if(!mainWindowView.registerSelf()) {
        MessageBoxW(NULL, L"Could not initialize COM. The program will now close.", L"Window Registration Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    if (!mainWindowView.createWindow()) {
        MessageBoxW(NULL, L"Could not create Window. The program will now close.", L"Window Creation Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    if(!mainWindowView.initDirect2D()) {
        MessageBoxW(NULL, L"Could not initialize Direct2D. The program will now close.", L"Direct2D Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    mainWindowView.loadBulb();

    retVal = mainWindowView.doLoop();

    mainWindowView.shutdown();

    return static_cast<unsigned int>(retVal);

}
