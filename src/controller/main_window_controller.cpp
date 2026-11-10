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

    // Update Lengths of sides
    // Reset D2D related stuff

}
