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
#include "../interface/main_window_interface.h"
#include "../model/bulb.h"

using namespace Microsoft::WRL;

struct DXInfo {

    // These are destroyed in the reverse order they are created.
    // Ex: D3DDevice is created first, destroyed

    ComPtr<ID3D11Device>        D3DDevice;
    ComPtr<IDXGIDevice>         DXGIDevice;
    ComPtr<IDXGIFactory2>       dxFactory;
    ComPtr<IDXGISwapChain1>     swapChain;


    // Our Graphics stuff is destroyed
    ComPtr<ID2D1Factory2>       d2Factory;
    ComPtr<ID2D1Device1>        d2Device;
    ComPtr<ID2D1DeviceContext>  dc;
    ComPtr<IDXGISurface2>       surface;
    ComPtr<ID2D1Bitmap1>        bitmap;

    ComPtr<IDCompositionDevice> dcompDevice;
    ComPtr<IDCompositionTarget> target;
    ComPtr<IDCompositionVisual> visual;
};

struct SideInfo {
    // IDs will be wchar_t of the file name in the end. A file name cannot exceed 32 characters without the .bul, which works
    // for all the common bulbs I could find (The longest file name I found was 26 characters).
    //unsigned __int32        bulbIDs[5];     // Which Five bulbs do we look up?
    std::vector<__int16>    curFrame;       // For each and every bulb on the string, the current frame.
    LONG                    sideLength;     // How many pixels long is this side?
};

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

        DXInfo* dxInfo;

        //ChristmasBulbPackage                christmasBulb;
        std::vector<ChristmasBulbPackage*>  christmasBulb;
        SideInfo                            sideInfo;

        ComPtr<IWICImagingFactory>      wicFactory;
        ComPtr<IWICBitmapDecoder>       gifDecoder;
};
