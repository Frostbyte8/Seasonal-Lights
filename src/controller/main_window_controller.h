#pragma once

#include <vector>
#include "../model/bulb.h"
#include "../model/direct2D.h"
#include <windows.h>

// TODO: Maybe change a few things to reduce the duplicating errors thing here.

namespace ControllerErrors {
    static const int SUCCESS                = 0;
    static const int OUT_OF_MEMORY          = -1;
    static const int COULD_NOT_LOAD_BULB    = -2;
}

struct MonitorDimensions {
    LONG xStart;
    LONG yStart;
    LONG width;
    LONG height;
};

struct SideInfo {

    // IDs will be wchar_t of the file name in the end. A file name cannot exceed 32 characters without the .bul, which works
    // for all the common bulbs I could find (The longest file name I found was 26 characters).
    //unsigned __int32        bulbIDs[5];     // Which Five bulbs do we look up?
    std::vector<__int16>    curFrame;       // For each and every bulb on the string, the current frame.
    LONG                    sideLength;     // How many pixels long is this side?

};

class MainWindowController {

    public:

        void releaseResources();
        const SideInfo& getSizeInfo(const BulbSide& side) const { return sideInfo[static_cast<size_t>(side)]; }
        void updateMonitorSize(const MONITORINFO& mi);
        int loadBulb(const std::string& fileName);
        bool initDirect2D();

        Direct2DLayer*                      getDirect2DLayer() { return &d2dLayer; }


    private:
        std::vector<ChristmasBulbPackage*>  christmasBulb;
        SideInfo                            sideInfo[static_cast<size_t>(BulbSide::NUM_SIDES)];
        MonitorDimensions                   monitorDimensions;
        Direct2DLayer                       d2dLayer;

};
