#pragma once

#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <wrl.h>

using namespace Microsoft::WRL;

struct DXInfo {

    // These are destroyed in the reverse order they are created.
    // Ex: D3DDevice is created first, destroyed last

    ComPtr<ID3D11Device>        D3DDevice;
    ComPtr<IDXGIDevice>         DXGIDevice;
    ComPtr<IDXGIFactory2>       dxFactory;
    ComPtr<IDXGISwapChain1>     swapChain;

    ComPtr<ID2D1Factory2>       d2Factory;
    ComPtr<ID2D1Device1>        d2Device;
    ComPtr<ID2D1DeviceContext>  dc;
    ComPtr<IDXGISurface2>       surface;
    ComPtr<ID2D1Bitmap1>        bitmap;

    ComPtr<IDCompositionDevice> dcompDevice;
    ComPtr<IDCompositionTarget> target;
    ComPtr<IDCompositionVisual> visual;

};

class Direct2DLayer {

    public:
        Direct2DLayer() : dxInfo(NULL) {
        }

        ~Direct2DLayer() {
            if(dxInfo) {
                delete dxInfo;
                dxInfo = NULL;
            }
        }

        // The functions here require no validation and are safe for the view to call them directly.

        bool init(const UINT& width, const UINT& height, const HWND& hWnd);
        __forceinline const DXInfo* getDXInfo() const { return dxInfo; }
        void shutdown();

    private:

        DXInfo* dxInfo;



};
