#include "main_window.h"
#include <d2d1.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi1_3.h>

#define __D3D_IS_OK__(x) if(x != S_OK) { return false; }

//==============================================================================
// Namespaces / Enums / Constants
//==============================================================================

static const DWORD WINDOW_STYLE = WS_VISIBLE | WS_POPUP;
static const DWORD WINDOW_STYLE_EX = WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST;

//==============================================================================
// Constructors
//==============================================================================

MainWindowView::MainWindowView(const HINSTANCE hIn) : hWnd(NULL), activeModalDialog(NULL),
hInstance(hIn), dxInfo(NULL), wicFactory(NULL), gifDecoder(NULL) {
    //windowPresenter.setMainWindowInterface(this);
}

MainWindowView::~MainWindowView() {
    if(dxInfo) {
        delete dxInfo;
        dxInfo = NULL;
    }
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

    if(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory)) != S_OK) {
        MessageBoxW(NULL, L"Error Initializing WIC Imaging Factory.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if(CoCreateInstance(CLSID_WICGifDecoder, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&gifDecoder)) != S_OK) {
        MessageBoxW(NULL, L"Error Initializing GIF decoder.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// initDirect2D - Initialize Direct2D
//------------------------------------------------------------------------------

bool MainWindowView::initDirect2D() {

    HRESULT status;

    // TODO: According to my notes, this won't release any resources.
    // I'm not sure why you'd need to init D2D twice, but it's here for now just in case.

    if(dxInfo) {
        delete dxInfo;
    }

    dxInfo = new DXInfo();

    // I'll be documenting this the best I can since MSDN has such poor documentation.

    // First we need to create a D3D11 Device, and our Device Context for the Device.

    status = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                               NULL, 0, D3D11_SDK_VERSION, &(dxInfo->D3DDevice), NULL, NULL);

    __D3D_IS_OK__(status);

    // Now that we have our Device, we can use it to get the DirectX Graphics Infrastructure.
    // We use this to manage display outputs, enumerate hardware, and presenting frames to the
    // monitor.

    status = dxInfo->D3DDevice.As(&(dxInfo->DXGIDevice));
    __D3D_IS_OK__(status);

    // We use the infrastructure to create our Factory. The Factory's job is to find information
    // about the physical graphics hardware, and bridge it to a window's swap chain.
    // TODO: DXGI_CREATE_FACTORY_DEBUG is probably not needed on Release builds.
    status = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(dxInfo->dxFactory), reinterpret_cast<void**>(dxInfo->dxFactory.GetAddressOf()));
    __D3D_IS_OK__(status);

    // Now for the Swap chain. The swap chain has two buffers: Primary and Back Buffer. Once the Back Buffer is done being draw, it "swaps"
    // the two buffers so you can begin drawing the next frame.
    // So my understanding is: The factory is used to create our swap chain, and our swap chain makes frames.

    // TODO: Release references to Back Buffer,
    // IDXGISwapChain -> ResizeBuffers
    // Recreate Render Target View
    // Reset View Port
    // This can be done in the WM_SIZE message.


    DXGI_SWAP_CHAIN_DESC1 desc = { 0 };
    desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount        = 2;
    desc.SampleDesc.Count   = 1;
    desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode          = DXGI_ALPHA_MODE_PREMULTIPLIED;

    RECT rect ={ 0 };
    GetClientRect(hWnd, &rect);
    desc.Width = rect.right - rect.left;
    desc.Height = rect.bottom - rect.top;

    status = dxInfo->dxFactory->CreateSwapChainForComposition(dxInfo->DXGIDevice.Get(), &desc, nullptr, dxInfo->swapChain.GetAddressOf());
    __D3D_IS_OK__(status);

    // Now, we already have an infrastructure factory, we now need a Direct2D Factory.
    // This will allow D2D to talk to D3D, which is how everything is drawn under the hood.
    // TODO: What options do we need for release mode?

    D2D1_FACTORY_OPTIONS const options = { D2D1_DEBUG_LEVEL_INFORMATION };
    status = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, dxInfo->d2Factory.GetAddressOf());
    __D3D_IS_OK__(status);

    // And now we need to create a Device and Device Context that Direct2D Can use to do it's drawing and communication
    // to D3D under the hood.
    status = dxInfo->d2Factory->CreateDevice(dxInfo->DXGIDevice.Get(), dxInfo->d2Device.GetAddressOf());
    __D3D_IS_OK__(status);

    status = dxInfo->d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, dxInfo->dc.GetAddressOf());
    __D3D_IS_OK__(status);

    // We also need to get access to the Back Buffer from the Swap Chain. This is so we can make compatible bitmaps from
    // this surface.
    status = dxInfo->swapChain->GetBuffer(0, __uuidof(dxInfo->surface), reinterpret_cast<void**>(dxInfo->surface.GetAddressOf()));
    __D3D_IS_OK__(status);

    // Now, we create our compatible bitmap that we can draw to.
	D2D1_BITMAP_PROPERTIES1 properties = {};
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

	status = dxInfo->dc->CreateBitmapFromDxgiSurface(dxInfo->surface.Get(), properties, dxInfo->bitmap.GetAddressOf());
    __D3D_IS_OK__(status);

    // And we need to make sure we set the Device Context's target.
    dxInfo->dc->SetTarget(dxInfo->bitmap.Get());

    // Now we just need a Composition object. We need this for click-through transparency,
    // DPI related issues, and to tell DWM how to blend our window related stuff to the screen.
    // Apparently it also stops the white flash glitch.
    // Normally we might not need this, but for Seasonal Lights we sure do.

    status = DCompositionCreateDevice(dxInfo->DXGIDevice.Get(), __uuidof(dxInfo->dcompDevice), reinterpret_cast<void**>(dxInfo->dcompDevice.GetAddressOf()));
    __D3D_IS_OK__(status);

    // Now we tell our DirectComposition Device which window to use for our composition
    status = dxInfo->dcompDevice->CreateTargetForHwnd(hWnd, true, dxInfo->target.GetAddressOf());
    __D3D_IS_OK__(status);

    // Now we create a blank layer object for it to use
    status = dxInfo->dcompDevice->CreateVisual(dxInfo->visual.GetAddressOf());
    __D3D_IS_OK__(status);

    // Attach our swap chain. So we tell the visual about it.
    status = dxInfo->visual->SetContent(dxInfo->swapChain.Get());
    __D3D_IS_OK__(status);

    // Then we take the target we created with CreateTargetForHwnd, and make it aware of the visual.
    status = dxInfo->target->SetRoot(dxInfo->visual.Get());
    __D3D_IS_OK__(status);

    // Commit here tells the GPU to compile all the instructions we just did and display the graphics
    // inside our window.
    status = dxInfo->dcompDevice->Commit();
    __D3D_IS_OK__(status);

    return true;
}

void MainWindowView::shutdown() {

    // Even though they're in ComPTRs, at shutdown we need to make sure they are released as code and drivers
    // outside our control could still be referencing our objects.

    if(dxInfo->target) {
        dxInfo->target->SetRoot(NULL);
    }

    if(dxInfo->visual) {
        dxInfo->visual->SetContent(NULL);
    }

    dxInfo->visual.Reset();
    dxInfo->target.Reset();
    dxInfo->dcompDevice.Reset();

    dxInfo->dc.Reset();
    dxInfo->d2Device.Reset();
    dxInfo->d2Factory.Reset();

    dxInfo->swapChain.Reset();
    dxInfo->dxFactory.Reset();
    dxInfo->D3DDevice.Reset();

}

void MainWindowView::loadBulb() {

    christmasBulb.loadBulb("D:\\dump\\12.bul");
    christmasBulb.initBitmaps(wicFactory.Get(), gifDecoder.Get(), dxInfo->dc.Get());
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

    dxInfo->dc->BeginDraw();
    dxInfo->dc->Clear();

    static int i = 0;   // Frame
    static int k = 4;   // Current Flavor (TODO: Current Flavor only updates after all bulbs for the side have been drawn)

    const D2DImageInfo* bulbInfo = christmasBulb.getBulbInfo(k);

    D2D1_RECT_F dest = { 0, 0, static_cast<float>(bulbInfo->width), static_cast<float>(bulbInfo->height)};

    dxInfo->dc->DrawBitmap(bulbInfo->frames[i], &dest);
    i++;
    //k++;

    if(i == bulbInfo->frames.size()) {
        i = 0;
    }

    if(k == 8) {
        k = 0;
    }

    HRESULT status = dxInfo->dc->EndDraw();
    __D3D_IS_OK__(status);

    status = dxInfo->swapChain->Present(1, 0);
    __D3D_IS_OK__(status);

    return true;

}

//==============================================================================
// Private Functions
//==============================================================================
