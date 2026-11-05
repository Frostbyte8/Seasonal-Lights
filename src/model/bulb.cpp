#include "bulb.h"
#include <stdlib.h>

__forceinline void CountSideIDs(const unsigned __int32 (&IDs)[ChristmasBulbConstants::MAX_FLAVORS], unsigned __int8& side) {

    /// Here we get the max index for a given side.

    for(size_t i = 0; i < ChristmasBulbConstants::MAX_FLAVORS; ++i) {
        if(IDs[i] == 0xFFFFFFFF) {
            side = i - 1;
            return;
        }
    }

    side = ChristmasBulbConstants::MAX_FLAVORS - 1;
    return;

}

//==============================================================================
// Destructor
//==============================================================================

ChristmasBulbPackage::~ChristmasBulbPackage() {

    // TODO: This should be in a function called destroy bitmaps.
    // TODO: Figure out what we really need to delete, and what the OS does
    // to avoid unnecessary cleanup.

    const size_t NUM_BMPS = bmpData.size();
    for(size_t i = 0; i < NUM_BMPS; ++i) {

        const size_t NUM_FRAMES = bmpData[i].frames.size();

        for(size_t k = 0; k < NUM_FRAMES; ++k) {
            if(bmpData[i].frames[k]) {
                bmpData[i].frames[k]->Release();
                bmpData[i].frames[k] = NULL;
            }
        }

    }

    const size_t NUM_GIFS = gifData.size();

    for(size_t i = 0; i < NUM_GIFS; ++i) {
        free(gifData[i].rawData);
        gifData[i].rawData = NULL;
    }

}

//==============================================================================
// Accessors
//==============================================================================

//==============================================================================
// Public Functions
//==============================================================================

//------------------------------------------------------------------------------
// loadBulb - Load a bulb from the path given.
//------------------------------------------------------------------------------

int ChristmasBulbPackage::loadBulb(const std::string& path) {

    FILE* fp = NULL;
    fopen_s(&fp, &path[0], "rb");

    if(fp == NULL) {
        return ChristmasBulbErrors::FILE_NOT_FOUND;
    }

    char headerID[sizeof(ChristmasBulbConstants::HEADER_ID)];
    fread(headerID, sizeof(char), sizeof(ChristmasBulbConstants::HEADER_ID), fp);

    if(strcmp(headerID, ChristmasBulbConstants::HEADER_ID)) {
        fclose(fp);
        return ChristmasBulbErrors::INVALID_HEADER;
    }

    // TODO: Read entire file into memory, then do work.
    // NOTE: The 2 is to skip over the two null terminators.
    fseek(fp, 2, SEEK_CUR);
    fread(&header, sizeof(ChristmasBulbHeader), 1, fp);

    // Update cached info about how many flavors are on each side for this bulb

    CountSideIDs(header.TIDs, cachedIDInfo.MAX_IDX_TIDS);
    CountSideIDs(header.RIDs, cachedIDInfo.MAX_IDX_RIDS);
    CountSideIDs(header.LIDs, cachedIDInfo.MAX_IDX_LIDS);
    CountSideIDs(header.BIDs, cachedIDInfo.MAX_IDX_BIDS);

    // Now read all the information about the GIFs in the file.
    fread(&infoTable[0], sizeof(GIFInfoTable), ChristmasBulbConstants::MAX_BULB_IDS, fp);

    // TODO: This needs to be properly checked
    cachedIDInfo.TOTAL_IDS = ChristmasBulbConstants::MAX_BULB_IDS;

    // Load GIF Data and track how many IDs we have.

    for(int i = 0; i < ChristmasBulbConstants::MAX_BULB_IDS; ++i) {

        if(!infoTable[i].size) {
            continue;
        }

        fseek(fp, infoTable[i].offset, SEEK_SET);

        GIFData data = {0};

        data.rawData = reinterpret_cast<char*>(malloc(infoTable[i].size));
        data.size = infoTable[i].size; // [TODO]: We might not need to cache this.
        fread(data.rawData, sizeof(char), data.size, fp);

        //fwrite(data.rawData, 1, data.size, stdout);

        gifData.push_back(data);

    }

    fclose(fp);

    return 0;

}

//------------------------------------------------------------------------------
// initBitmaps - Convert GIF data into BMP data for Direct2D to use.
//------------------------------------------------------------------------------

void ChristmasBulbPackage::initBitmaps(IWICImagingFactory* factory, IWICBitmapDecoder* decoder, ID2D1DeviceContext* dc) {

    // Now we need to convert the gif data into something Direct2D Can use.

    IWICStream*             imageStream     = NULL;
    IWICFormatConverter*    formatConverter = NULL;
    ID2D1Bitmap*            d2dBMP          = NULL;

    const size_t NUM_GIFS = gifData.size();

    for(size_t i = 0; i < NUM_GIFS; ++i) {

        // First we'll create a stream that we will use to load the GIF data into.
        HRESULT hr = factory->CreateStream(&imageStream);

        if(gifData[i].rawData != NULL) {

            // Copy the data into the IWIC stream. We will use this to convert our data into a series of
            // D2D Bitmaps.
            hr = imageStream->InitializeFromMemory((BYTE *)gifData[i].rawData, gifData[i].size);
            hr = factory->CreateDecoderFromStream(imageStream, NULL, WICDecodeMetadataCacheOnDemand, &decoder);

            UINT numFrames;
            decoder->GetFrameCount(&numFrames);

            D2DImageInfo bulbInfo;
            bulbInfo.width = *reinterpret_cast<unsigned __int16*>(&gifData[i].rawData[6]);
            bulbInfo.height = *reinterpret_cast<unsigned __int16*>(&gifData[i].rawData[8]);

            for(size_t k = 0; k < numFrames; ++k) {

                // We will now decode the current frame. We dont' have to advance the stream, the decoders
                // do that for us.
                IWICBitmapFrameDecode* decodedFrame     = NULL;
                IWICMetadataQueryReader* decodedMeta    = NULL;

                // TODO: warning C4267: 'argument': conversion from 'size_t' to 'UINT', possible loss of data
                decoder->GetFrame(k, &decodedFrame);

                UINT frameWidth;
                UINT frameHeight;
                decodedFrame->GetSize(&frameWidth, &frameHeight);

                factory->CreateFormatConverter(&formatConverter);
                formatConverter->Initialize(decodedFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);

                // Now we take our Stream of Data, and convert it into a D2D Bitmap.
                ID2D1Bitmap* bitmap = NULL;
                dc->CreateBitmapFromWicBitmap(*(&formatConverter), NULL, &bitmap);

                // If the frame Width doesn't match the width of the image already,
                // It's a composite frame.

                if (frameWidth != bulbInfo.width) {

                    decodedFrame->GetMetadataQueryReader(&decodedMeta);

                    PROPVARIANT var;
                    PROPVARIANT var2;
                    PropVariantInit(&var);
                    PropVariantInit(&var2);

                    HRESULT hr;
                    
                    // TODO: Check if these fail, and get some information about it for research

                    hr = decodedMeta->GetMetadataByName(L"/imgdesc/Left", &var);
                    LONG left = var.lVal;
                    hr = decodedMeta->GetMetadataByName(L"/imgdesc/Top", &var2);
                    LONG top = var2.lVal;
                    
                    ID2D1Bitmap* prevBitmap = bulbInfo.frames[bulbInfo.frames.size() - 1];

                    D2D1_SIZE_U size = { 0 };
                    D2D1_BITMAP_PROPERTIES bmpProp;
                    
                    size.width = prevBitmap->GetSize().width;
                    size.height = prevBitmap->GetSize().height;
                    prevBitmap->GetDpi(&bmpProp.dpiX, &bmpProp.dpiY);
                    bmpProp.pixelFormat = prevBitmap->GetPixelFormat();

                    ID2D1Bitmap* newBitmap = NULL;                   
                    dc->CreateBitmap(size, bmpProp, &newBitmap);
                    newBitmap->CopyFromBitmap(NULL, prevBitmap, NULL);
                    
                    D2D1_POINT_2U dest = { 0 };
                    dest.x = left;
                    dest.y = top;

                    newBitmap->CopyFromBitmap(&dest, bitmap, NULL);

                    bitmap->Release();
                    bitmap = newBitmap;



                }


                bulbInfo.frames.push_back(bitmap);

                // Clean up our resources
                decodedFrame->Release();
                decodedFrame = NULL;
                formatConverter = NULL;

            }

            imageStream->Release();
            imageStream = NULL;
            bmpData.push_back(bulbInfo);

        }
    }

}
