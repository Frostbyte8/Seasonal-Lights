#include "bulb.h"
#include <stdlib.h>

ChristmasBulbPackage::~ChristmasBulbPackage() {

    // TODO: This should be in a function called destroy bitmaps.

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

int ChristmasBulbPackage::loadBulb(const std::string& path) {

    FILE* fp = NULL;
    fopen_s(&fp, &path[0], "rb");

    if(fp == NULL) {
        return -1;
    }

    char headerID[sizeof(ChristmasBulbConstants::HEADER_ID)];
    fread(headerID, sizeof(char), sizeof(ChristmasBulbConstants::HEADER_ID), fp);

    if(strcmp(headerID, ChristmasBulbConstants::HEADER_ID)) {
        fclose(fp);
        return -1;
    }

    // TODO: Read entire file into memory, then do work.
    fseek(fp, 2, SEEK_CUR);
    fread(&header, sizeof(ChristmasBulbHeader), 1, fp);

    // Now read all the information about the GIFs in the file.
    fread(&infoTable[0], sizeof(GIFInfoTable), ChristmasBulbConstants::MAX_BULB_IDS, fp);


    cachedIDInfo.TOTAL_IDS = ChristmasBulbConstants::MAX_BULB_IDS;

    // Load GIF Data and track how many IDs we have.

    for(int i = 0; i < ChristmasBulbConstants::MAX_BULB_IDS; ++i) {

        if(!infoTable[i].size) {
            cachedIDInfo.TOTAL_IDS = i - 1;
            break;
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
                decodedFrame->GetMetadataQueryReader(&decodedMeta);

                // TODO: Metadata might contain info about imgdesc/top and left, we can use that to support the other
                // type of GIF the original program did not support.

                factory->CreateFormatConverter(&formatConverter);
                formatConverter->Initialize(decodedFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);

                // Now we take our Stream of Data, and convert it into a D2D Bitmap.
                ID2D1Bitmap* bitmap = NULL;
                dc->CreateBitmapFromWicBitmap(*(&formatConverter), NULL, &bitmap);

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
