#ifndef ARTOMATIX_IMAGE_LOADER_BASE_H
#define ARTOMATIX_IMAGE_LOADER_BASE_H

#include <string>

#include "AIL.h"

namespace AImg
{
    class AImgBase
    {
        public:
            virtual ~AImgBase();

            virtual int32_t getImageInfo(int32_t* width, int32_t* height, int32_t* numChannels, int32_t* bytesPerChannel, int32_t* floatOrInt, int32_t* decodedImgFormat) = 0;
            virtual int32_t decodeImage(void* destBuffer, int32_t forceImageFormat) = 0;
    };

    class ImageLoaderBase
    {
        public:
            virtual ~ImageLoaderBase();

            virtual int32_t initialise() = 0;
            virtual bool canLoadImage(ReadCallback readCallback, TellCallback tellCallback, SeekCallback seekCallback, void* callbackData) = 0;
            virtual std::string getFileExtension() = 0;
            virtual int32_t getAImgFileFormatValue() = 0;
            virtual AImgBase* openImage(ReadCallback readCallback, TellCallback tellCallback, SeekCallback seekCallback, void* callbackData) = 0;
    };
}

#endif // ARTOMATIX_IMAGE_LOADER_BASE_H
