#pragma once

#include <vector>
#include <string>
#include <d2d1_2.h>
#include <wincodec.h>

// [TODO]:
// At some point, I plan to split this all so the GUI stuff only needs the Header + Preview Bulb
// and the renderer only gets the image data it needs.

// [TODO]:
// Packed structs

namespace ChristmasBulbConstants {
    static const char               HEADER_ID[] = "bluBrgiT\x04";
    static const unsigned __int32   ID_OFFSET = 360;
    static const unsigned __int32   LOOKUP_TABLE_OFFSET = 508;
    static const unsigned __int32   CATEGORY_INFO_OFFSET = 1396;
    static const unsigned int       MAX_STR_SIZE = 80;
    static const unsigned __int32   MAX_BULB_IDS = 37;
    static const unsigned int       MAX_FLAVORS = 8;
}

struct ChristmasBulbHeader {

    // Milliseconds since Windows was started when this bulb was made.
    // Possibly meant as a Unique ID system? Updated when edited?
    unsigned __int32 msCreationTime;

    // Attribution when sending the Bulb in for review:
    // 01 = You are the artist. 02 = Not Artist, but have permission.
    // 03 = In older versions, you could specify the bulb art was public domain.
    unsigned __int32    attribution;

    // Whether the Bulb is locked from editing or not. Bulbs downloaded have this
    // set so that people could not alter the bulb. 0 = Can Edit, 1 = Cannot edit.
    // In Holiday Lights we ignore this and will 0 this upon saving.
    unsigned __int32    locked;

    // Controls whether the bulb could be sent in. Possibly even got set when it
    // sent successfully. All downloaded bulb have this set to 0, but newly created
    // ones have this set to 1.
    // In Holiday Lights we will be setting this to 0.
    unsigned __int32    canSend;

    // These are all 80 character Null Terminated, ASCII strings.
    char name[ChristmasBulbConstants::MAX_STR_SIZE];
    char desc[ChristmasBulbConstants::MAX_STR_SIZE];
    char copyright[ChristmasBulbConstants::MAX_STR_SIZE];
    char contactInfo[ChristmasBulbConstants::MAX_STR_SIZE];

    // Information Regarding the Preview Bulb
    unsigned __int32    xPreview;
    unsigned __int32    yPreview;
    unsigned __int32    prevBulbID; // Note that there is a max of 37 possible IDs.

    // This area down is all required from drawing.

    // Corner Bulbs can only have 1 Image simply because they can only ever show 1.
    // If they're not set, they use topLeftID[0] for the two two bulbs, and bottomRightId for
    // the two bottom bulbs
    unsigned __int32    topLeftID;
    unsigned __int32    topRightID;
    unsigned __int32    bottomRightID;
    unsigned __int32    bottomLeftID;

    // The sides however, can have up to 8 "Flavors" defined
    // FF FF FF FF that the previous ID was the last ID and that there is nothing beyond
    // this frame. So a NULL terminator.
    unsigned __int32    TIDs[ChristmasBulbConstants::MAX_FLAVORS];
    unsigned __int32    RIDs[ChristmasBulbConstants::MAX_FLAVORS];
    unsigned __int32    LIDs[ChristmasBulbConstants::MAX_FLAVORS];
    unsigned __int32    BIDs[ChristmasBulbConstants::MAX_FLAVORS];

    // Just before the GIF Data, it has the information about Categories. That's a [TODO] for now.

};

struct CachedIDInfo {
    unsigned __int8 TOTAL_IDS;
    // [TODO]: Useful?
    unsigned __int8 TOTAL_TIDS;
    unsigned __int8 TOTAL_RIDS;
    unsigned __int8 TOTAL_LIDS;
    unsigned __int8 TOTAL_BIDS;
};

struct GIFData {
    char*               rawData;
    unsigned __int32    size;
};

struct GIFInfoTable {

    unsigned __int32    size;
    unsigned __int32    offset;

    // This is a BZIP2 CRC32 hash.
    unsigned __int32    hash;

    // How many times this gif is with the same CRC32 hash is reused. 09 Means it's used for all sides + preview
    unsigned __int32    unknown1;

    // Not sure what this does. Sometimes set to 1?
    unsigned __int32    unknown2;

};

struct D2DImageInfo {
    unsigned __int16    width;
    unsigned __int16    height;
    std::vector<ID2D1Bitmap *> frames;
    D2DImageInfo() : width(0), height(0) {
    }
};

class ChristmasBulbPackage {

    public:
        int loadBulb(const std::string& path);
        void initBitmaps(IWICImagingFactory* factory, IWICBitmapDecoder* decoder, ID2D1DeviceContext* dc);
        ~ChristmasBulbPackage();

        __forceinline const D2DImageInfo* getBulbInfo(const size_t& index) const {
            return &(bmpData[index]);
        }

    private:

        ChristmasBulbHeader header;
        CachedIDInfo cachedIDInfo;
        GIFInfoTable infoTable[37];
        unsigned int numDefined;
        std::vector<GIFData> gifData;
        std::vector<D2DImageInfo> bmpData;

};
