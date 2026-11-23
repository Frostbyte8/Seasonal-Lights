#include "main_window_controller.h"

//==============================================================================
// Public Functions
//==============================================================================

//------------------------------------------------------------------------------
// releaseResources - Release all the data held.
//------------------------------------------------------------------------------

void MainWindowController::releaseResources() {
    for(size_t i = 0; i < christmasBulb.size(); ++i) {
        delete christmasBulb[i];
        christmasBulb[i] = NULL;
    }
}

//------------------------------------------------------------------------------
// loadBulb - Attempts to load a bulb into memory
//------------------------------------------------------------------------------

int MainWindowController::loadBulb(const std::string &fileName) {

    ChristmasBulbPackage* bulbPackage = new (std::nothrow) ChristmasBulbPackage();

    if(!bulbPackage) {
        return ControllerErrors::OUT_OF_MEMORY;
    }

    if(bulbPackage->loadBulb(fileName) != ChristmasBulbErrors::SUCCESS) {
        delete bulbPackage;
        bulbPackage = NULL;
        return ControllerErrors::COULD_NOT_LOAD_BULB;
    }

    // TODO: Check for error.
    bulbPackage->initBitmaps(wicFactory.Get(), gifDecoder.Get(), d2dLayer.getDXInfo()->dc.Get());

    christmasBulb.push_back(bulbPackage);

    return true;

}

//------------------------------------------------------------------------------
// updateMonitorSize - Updates the size of the monitor, and readjust bulbs
// to fit the given size.
//------------------------------------------------------------------------------

void MainWindowController::updateMonitorSize(const MONITORINFO &mi) {

    // Update Position and Size

    monitorDimensions.xStart = mi.rcMonitor.left;
    monitorDimensions.yStart = mi.rcMonitor.top;
    monitorDimensions.width =  abs(mi.rcMonitor.right - mi.rcMonitor.left);
    monitorDimensions.height =  abs(mi.rcMonitor.top - mi.rcMonitor.bottom);

    // Update Lengths of sides (TODO: Very incomplete)

    SideInfo& si = sideInfo[static_cast<size_t>(BulbSide::TOP)];

    si.sideLength = abs(mi.rcMonitor.right - mi.rcMonitor.left);
    si.sideLength -= (christmasBulb[0]->getBulbInfo(BulbSide::TOP_LEFT, 0)->width + christmasBulb[0]->getBulbInfo(BulbSide::TOP_RIGHT, 0)->width);

    LONG realLen = 0;
    size_t numBulbs = 0;
    size_t idx = 0;
    size_t flavor = 0;

    BulbInfo bi = {0};

    // TODO: Figure out a good number to put here as a constant, and make sure corners only reserve one.
    si.bulbInfo.reserve(60);

    for(int i = 0;; ++i) {

        const D2DImageInfo* imgInfo = christmasBulb[idx]->getBulbInfo(BulbSide::TOP, flavor);

        if(realLen + imgInfo->width > si.sideLength) {
            break;
        }

        realLen += imgInfo->width;

        bi.currentFrame = 0;
        bi.maxFrames = imgInfo->frames.size();
        bi.whichFlavor = 0; // TODO: how to make sure flavor exists

        si.bulbInfo.push_back(bi);

        numBulbs++;
        idx++;

        if(idx >= christmasBulb.size()) {
            idx = 0;
            //flavor++;
        }

    }

    si.sideLength = realLen;
    // TODO: Possibly shrink to fit?
    //si.bulbInfo.resize(numBulbs);


    // Reset D2D related stuff

}

//------------------------------------------------------------------------------
// updateBulbs - Animate bulbs (or not) based on what the user has selected
//------------------------------------------------------------------------------

void MainWindowController::updateBulbs() {

    //D2DImageInfo* imf = NULL;

    //SideInfo& si = sideInfo[static_cast<size_t>(BulbSide::TOP)];
    std::vector<BulbInfo>& bulbInfo = sideInfo[static_cast<size_t>(BulbSide::TOP)].bulbInfo;

    for(size_t i = 0; i < bulbInfo.size() ; ++i) {

        bulbInfo[i].currentFrame++;
        if(bulbInfo[i].maxFrames == bulbInfo[i].currentFrame) {
            bulbInfo[i].currentFrame = 0;
        }

    }

}
