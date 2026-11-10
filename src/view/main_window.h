#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <wincodec.h>
#include <wrl.h>

#include "window_meterics.h"
#include "subclassed_controls.h"
#include "../controller/main_window_controller.h"
#include "../interface/main_window_interface.h"
#include "../model/bulb.h"

class MainWindowView : public WndAsClass<MainWindowView>, public MainWindowInterface {
    friend WndAsClass;

    public:

        MainWindowView(const HINSTANCE hIn);
        ~MainWindowView();
        bool createWindow();
        bool initCOM();
        bool registerSelf();
        bool initDirect2D();
        void shutdown();
        void loadBulb();
        void initSize();
        WPARAM doLoop();

        void implDisplayMessage(const wchar_t* message, const wchar_t* title) {
            MessageBoxW(NULL, L"Not yet implemented.", L"", MB_OK);
        }

    private:

        // Disable Copy Constructor

        MainWindowView(const MainWindowView&);
        MainWindowView& operator = (const MainWindowView&);

        bool onPaint();

        LRESULT windowProc(const UINT& msg, const WPARAM wParam, const LPARAM lParam);

        // Member Variables
        HWND hWnd;
        const HINSTANCE hInstance;
        WindowMetrics   metrics;
        HWND activeModalDialog;
        MainWindowController mainWindowController;

        // TODO: Most of this is going to moved into the controller if necessary.

        DXInfo* dxInfo;
        //ChristmasBulbPackage                christmasBulb;

        SideInfo                            sideInfo;

        ComPtr<IWICImagingFactory>      wicFactory;
        ComPtr<IWICBitmapDecoder>       gifDecoder;
};
