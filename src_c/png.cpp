#include "AIL.h"
#include "png.h"
#include "AIL_internal.h"
#include <vector>
#include <png.h>
#include <string.h>

namespace AImg
{
    typedef struct CallbackData
    {
            ReadCallback readCallback;
            TellCallback tellCallback;
            SeekCallback seekCallback;
            WriteCallback writeCallback;
            void * callbackData;

    } CallbackData;

    int32_t PNGImageLoader::initialise()
    {
        return AImgErrorCode::AIMG_SUCCESS;
    }

    bool PNGImageLoader::canLoadImage(ReadCallback readCallback, TellCallback tellCallback, SeekCallback seekCallback, void *callbackData)
    {
        int32_t startingPosition = tellCallback(callbackData);
        std::vector<uint8_t> header(8);
        readCallback(callbackData, &header[0], 8);

        seekCallback(callbackData, startingPosition);

        uint8_t png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

        return ((int32_t)(memcmp(&header[0], &png_signature[0], 8))) == 0;
    }

    void png_custom_read_data(png_struct* png_ptr, png_byte* data, png_size_t length)
    {
        CallbackData callbackData = *((CallbackData *)png_get_io_ptr(png_ptr));

        callbackData.readCallback(callbackData.callbackData, data, length);
    }

    void png_custom_write_data(png_struct* png_ptr, png_byte* data, png_size_t length)
    {
        CallbackData callbackData = *((CallbackData *)png_get_io_ptr(png_ptr));

        callbackData.writeCallback(callbackData.callbackData, data, length);
    }

    std::string PNGImageLoader::getFileExtension()
    {
        return "PNG";
    }

    int32_t PNGImageLoader::getAImgFileFormatValue()
    {
        return PNG_IMAGE_FORMAT;
    }

    void flush_data_noop_func(png_struct* png_ptr)
    {
        AIL_UNUSED_PARAM(png_ptr);
    }

    class PNGFile : public AImgBase
    {
        public:
            CallbackData * data;
            png_info * png_info_ptr;
            png_struct * png_read_ptr;
            uint32_t width;
            uint32_t height;
            uint8_t colour_type;
            uint8_t bit_depth;
            uint8_t numChannels;

            PNGFile()
            {
                data = new CallbackData();
            }

            virtual ~PNGFile()
            {
                delete data;
                png_destroy_read_struct(&png_read_ptr, &png_info_ptr, (png_infopp)NULL);
                png_destroy_info_struct(png_read_ptr, &png_info_ptr);
            }

            virtual int32_t getImageInfo(int32_t *width, int32_t *height, int32_t *numChannels, int32_t *bytesPerChannel, int32_t *floatOrInt, int32_t *decodedImgFormat)
            {
                *width = this->width;
                *height = this->height;
                *numChannels = this->numChannels;

                if (bit_depth / 8 == 0)
                    *bytesPerChannel = -1;
                else
                    *bytesPerChannel = bit_depth/8;

                *floatOrInt = AImgFloatOrIntType::FITYPE_INT;

                *decodedImgFormat = getDecodeFormat();

                return AImgErrorCode::AIMG_SUCCESS;
            }

            int32_t getDecodeFormat()
            {
                if (bit_depth == 8)
                {
                    if (numChannels == 1)
                        return AImgFormat::R8U;
                    else if (numChannels == 2)
                        return AImgFormat::RG8U;
                    else if (numChannels == 3)
                        return AImgFormat::RGB8U;
                    else if (numChannels == 4)
                        return AImgFormat::RGBA8U;
                }

                else if (bit_depth == 16)
                {
                    if (numChannels == 1)
                        return AImgFormat::R16U;
                    else if (numChannels == 2)
                        return AImgFormat::RG16U;
                    else if (numChannels == 3)
                        return AImgFormat::RGB16U;
                    else if (numChannels == 4)
                        return AImgFormat::RGBA16U;
                }

                return AImgFormat::INVALID_FORMAT;

            }

            virtual int32_t decodeImage(void *destBuffer, int32_t forceImageFormat)
            {
                // This sets a restore point for libpng if reading fails internally
                // Crazy old C exceptions without exceptions

                if (setjmp(png_jmpbuf(png_read_ptr)))
                {
                    AISetLastErrorDetails("[PNGImageLoader::PNGFile::decodeImage] Failed to read file");
                    return AImgErrorCode::AIMG_LOAD_FAILED_INTERNAL;
                }

                void **ptrs = (void **)malloc(height * sizeof(size_t));

                for (uint32_t y = 0; y < height; y++)
                    ptrs[y] = (void *)((size_t)destBuffer + (y*width * (bit_depth/8) * numChannels));


                png_read_image(png_read_ptr, (png_bytepp)ptrs);

                if (forceImageFormat != AImgFormat::INVALID_FORMAT)
                {
                    int32_t numChannels, bytesPerChannel, floatOrInt;
                    AIGetFormatDetails(forceImageFormat, &numChannels, &bytesPerChannel, &floatOrInt);

                    std::vector<uint8_t> convertBuffer(width * height * numChannels * bytesPerChannel);

                    int32_t convertError = AImgConvertFormat(destBuffer, &convertBuffer[0], width, height, getDecodeFormat(), forceImageFormat);

                    if (convertError != AImgErrorCode::AIMG_SUCCESS)
                        return convertError;

                }
                free(ptrs);
                return AImgErrorCode::AIMG_SUCCESS;
            }
    };


    AImgBase* PNGImageLoader::openImage(ReadCallback readCallback, TellCallback tellCallback, SeekCallback seekCallback, void *callbackData)
    {
        PNGFile* png = new PNGFile();
        png->data->readCallback = readCallback;
        png->data->tellCallback = tellCallback;
        png->data->seekCallback = seekCallback;
        png->data->callbackData = callbackData;

        png->png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png->png_info_ptr = png_create_info_struct(png->png_read_ptr);

        png_set_read_fn(png->png_read_ptr, (void *)(png->data), png_custom_read_data);
        png_read_info(png->png_read_ptr, png->png_info_ptr);

        png->width = png_get_image_width(png->png_read_ptr, png->png_info_ptr);
        png->height = png_get_image_height(png->png_read_ptr, png->png_info_ptr);
        png->bit_depth = png_get_bit_depth(png->png_read_ptr, png->png_info_ptr);
        png->numChannels = png_get_channels(png->png_read_ptr, png->png_info_ptr);
        png->colour_type = png_get_color_type(png->png_read_ptr, png->png_info_ptr);

        return png;
    }

    AImgFormat PNGImageLoader::getWhatFormatWillBeWrittenForData(int32_t inputFormat)
    {
        int32_t numChannels, bytesPerChannel, floatOrInt;
        AIGetFormatDetails(inputFormat, &numChannels, &bytesPerChannel, &floatOrInt);

        if(floatOrInt == AImgFloatOrIntType::FITYPE_FLOAT)
            return (AImgFormat) (AImgFormat::R16U + bytesPerChannel - 1); // convert to 16U version with same channelNum

        if(inputFormat >= AImgFormat::R8U && inputFormat <= AImgFormat::RGBA8U)
            return (AImgFormat)inputFormat;

        if(inputFormat >= AImgFormat::R16U && inputFormat <= AImgFormat::RGBA16U)
            return (AImgFormat)inputFormat;

        return AImgFormat::INVALID_FORMAT;
    }

    int32_t PNGImageLoader::writeImage(void *data, int32_t width, int32_t height, int32_t inputFormat, WriteCallback writeCallback, TellCallback tellCallback, SeekCallback seekCallback, void *callbackData)
    {
        AIL_UNUSED_PARAM(tellCallback);
        AIL_UNUSED_PARAM(seekCallback);

        png_struct * png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_info * png_info_ptr = png_create_info_struct(png_write_ptr);

        CallbackData * callbackDataStruct = new CallbackData();

        callbackDataStruct->writeCallback = writeCallback;
        callbackDataStruct->callbackData = callbackData;

        png_set_write_fn(png_write_ptr, (void *)callbackDataStruct, png_custom_write_data, flush_data_noop_func);

        int32_t writeFormat = getWhatFormatWillBeWrittenForData(inputFormat);

        std::vector<uint8_t> convertBuffer(0);

        if (writeFormat != inputFormat)
        {
            int32_t numChannels, bytesPerChannel, floatOrInt;
            AIGetFormatDetails(writeFormat, &numChannels, &bytesPerChannel, &floatOrInt) ;
            convertBuffer.resize(width * height * numChannels * bytesPerChannel);

            int32_t convertError = AImgConvertFormat(data, &convertBuffer[0], width, height, inputFormat, writeFormat);

            if (convertError != AImgErrorCode::AIMG_SUCCESS)
                return convertError;
            data = &convertBuffer[0];
        }

        png_byte colour_type;
        png_byte bit_depth;

        switch (writeFormat)
        {
            case R8U:
                colour_type = PNG_COLOR_TYPE_GRAY;
                bit_depth = 8;
                break;
            case RG8U:
                colour_type = PNG_COLOR_TYPE_RGB;
                bit_depth = 8;
                break;
            case RGB8U:
                colour_type = PNG_COLOR_TYPE_RGB;
                bit_depth = 8;
                break;
            case RGBA8U:
                colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
                bit_depth = 8;
                break;

            case R16U:
                colour_type = PNG_COLOR_TYPE_GRAY;
                bit_depth = 16;
                break;
            case RG16U:
                colour_type = PNG_COLOR_TYPE_RGB;
                bit_depth = 16;
                break;
            case RGB16U:
                colour_type = PNG_COLOR_TYPE_RGB;
                bit_depth = 16;
                break;
            case RGBA16U:
                colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
                bit_depth = 16;
                break;
        }

        if (setjmp(png_jmpbuf(png_write_ptr)))
        {
            AISetLastErrorDetails("[PNGImageLoader::writeImage] Failed to write PNG header");
            return AImgErrorCode::AIMG_WRITE_FAILED_EXTERNAL;
        }

        int32_t numChannels, bytesPerChannel, floatOrInt;
        AIGetFormatDetails(inputFormat, &numChannels, &bytesPerChannel, &floatOrInt);

        void ** ptrs = (void **)malloc(sizeof(size_t) * height);

        for (int32_t y=0; y < height; y++)
            ptrs[y] = (void *)((size_t)data + y*width * numChannels * bytesPerChannel);

        png_set_IHDR(png_write_ptr, png_info_ptr, width, height, bit_depth, colour_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png_write_ptr, png_info_ptr);

        if (setjmp(png_jmpbuf(png_write_ptr)))
        {
            AISetLastErrorDetails("[PNGImageLoader::writeImage] Failed to write file");
            return AImgErrorCode::AIMG_WRITE_FAILED_EXTERNAL;
        }

        png_write_image(png_write_ptr, (png_bytepp)ptrs);

        if (setjmp(png_jmpbuf(png_write_ptr)))
        {
            AISetLastErrorDetails("[PNGImageLoader::writeImage] Failed to finalize write");
            return AImgErrorCode::AIMG_WRITE_FAILED_EXTERNAL;
        }

        png_write_end(png_write_ptr, png_info_ptr);

        free(ptrs);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
        png_destroy_info_struct(png_write_ptr, &png_info_ptr);
        delete callbackDataStruct;
        return AImgErrorCode::AIMG_SUCCESS;
    }
}