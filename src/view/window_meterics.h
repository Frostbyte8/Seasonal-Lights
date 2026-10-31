#pragma once

#include <windows.h>

struct ControlSpacing {

    int XWINDOW_MARGIN;
    int YWINDOW_MARGIN;

    int XLABELASSOC_MARGIN;
    int YLABELASSOC_MARGIN;

    int YLABELSAMELINE_MARGIN;

    int XRELATED_MARGIN;
    int YRELATED_MARGIN;

    int XUNRELATED_MARGIN;
    int YUNRELATED_MARGIN;

    int XGROUPBOX_MARGIN;
    int YFIRST_GROUPBOX_MARGIN;
    int YLAST_GROUPBOX_MARGIN;

};

struct ControlDimensions {
    int YLABEL;

    int XBUTTON;
    int YBUTTON;

    int XSCROLLBAR;
    int YSCROLLBAR;

    int YTEXTBOX_ONE_LINE_ALONE;
};

class WindowMetrics {

    public:
        WindowMetrics();
        ~WindowMetrics();
        void initWindowMetrics(float inScaleX = 96.0f, float inScaleY = 96.0f);

        inline int XDLU2PIX(const int& x) const;
        inline int YDLU2PIX(const int& y) const;
        const inline HFONT& GetCurrentFont() const { return curFont; }

        const inline ControlSpacing& getControlSpacing() const { return cs; }
        const inline ControlDimensions& getControlDimensions() const { return cd; }

        LONG calculateStringWidth(const wchar_t* str) const;

        const inline float& getDPI() const { return xScaleFactor; }


    private:
        HDC         fontHDC;
        HFONT       curFont;
        HFONT       oldFont;
        LONG        avgCharWidth;
        LONG        avgCharHeight;
        float       xScaleFactor;
        float       yScaleFactor;

        ControlSpacing      cs;
        ControlDimensions   cd;

};