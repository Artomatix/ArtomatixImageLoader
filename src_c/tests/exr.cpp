#include <gtest/gtest.h>
#include "testCommon.h"

#include "../AIL.h"

#ifdef HAVE_EXR

#include <half.h>

void WriteImageTest(AImgFormat decodeFormat, AImgFormat writeFormat, AImgFormat expectedWritten = AImgFormat::INVALID_FORMAT)
{
    if (expectedWritten < 0)
    {
        expectedWritten = writeFormat;
    }

    auto data = readFile<uint8_t>(getImagesDir() + "/exr/grad_32.exr");

    ReadCallback readCallback = NULL;
    WriteCallback writeCallback = NULL;
    TellCallback tellCallback = NULL;
    SeekCallback seekCallback = NULL;
    void* callbackData = NULL;

    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &data[0], (int32_t)data.size());

    AImgHandle img = NULL;
    AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);

    int32_t width = 64;
    int32_t height = 32;

    int32_t numChannels, bytesPerChannel, floatOrInt;
    AIGetFormatDetails(decodeFormat, &numChannels, &bytesPerChannel, &floatOrInt);

    int32_t fileSize = width*height*numChannels*bytesPerChannel;

    std::vector<uint8_t> imgData(fileSize, 0);

    AImgDecodeImage(img, &imgData[0], decodeFormat);
    AImgClose(img);
    AIDestroySimpleMemoryBufferCallbacks(readCallback, writeCallback, tellCallback, seekCallback, callbackData);

    std::vector<uint8_t> fileData(fileSize);
    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &fileData[0], (int32_t)fileData.size());

    AImgHandle wImg = AImgGetAImg(AImgFileFormat::EXR_IMAGE_FORMAT);
    AImgWriteImage(wImg, &imgData[0], width, height, decodeFormat, writeFormat, NULL, NULL, 0, writeCallback, tellCallback, seekCallback, callbackData, NULL);
    AImgClose(wImg);

    seekCallback(callbackData, 0);

    AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);

    int32_t _, writtenFormat;
    AImgGetInfo(img, &_, &_, &_, &_, &_, &writtenFormat, NULL);
    ASSERT_EQ(writtenFormat, expectedWritten);

    std::vector<uint8_t> imgData2(fileSize, 0);
    AImgDecodeImage(img, &imgData2[0], decodeFormat);
    AImgClose(img);
    AIDestroySimpleMemoryBufferCallbacks(readCallback, writeCallback, tellCallback, seekCallback, callbackData);

    for (uint32_t i = 0; i < imgData2.size(); i++)
        ASSERT_LE(abs(imgData[i] - imgData2[i]), 1);
}

TEST(Exr, TestDetectExr)
{
    ASSERT_TRUE(detectImage("/exr/grad_32.exr", EXR_IMAGE_FORMAT));
}

TEST(Exr, TestReadExrAttrs)
{
    ASSERT_TRUE(validateImageHeaders("/exr/grad_32.exr", 64, 32, 3, 4, AImgFloatOrIntType::FITYPE_FLOAT, AImgFormat::RGB32F));
}

TEST(Exr, TestCompareForceImageFormat1)
{
    ASSERT_TRUE(compareForceImageFormat("/exr/grad_32.exr"));
}

TEST(Exr, TestCompareForceImageFormat2)
{
    ASSERT_TRUE(compareForceImageFormat("/exr/neal_half.exr"));
}

// THIS HAS NOTHING TO DO WITH EXRS

TEST(Exr, TestMemoryCallbacksRead)
{
    std::vector<uint8_t> data(10);
    for(size_t i = 0; i < data.size(); i++)
        data[i] = (uint8_t)i;

    ReadCallback readCallback = NULL;
    WriteCallback writeCallback = NULL;
    TellCallback tellCallback = NULL;
    SeekCallback seekCallback = NULL;
    void* callbackData = NULL;

    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &data[0], (int32_t)data.size());

    std::vector<uint8_t> readBuf(data.size());
    std::fill(readBuf.begin(), readBuf.end(), 100);

    int32_t read = readCallback(callbackData, &readBuf[0], 12);
    ASSERT_EQ(read, 10);
    for(size_t i = 0; i < data.size(); i++)
        ASSERT_EQ(readBuf[i], i);

    std::fill(readBuf.begin(), readBuf.end(), 100);
    seekCallback(callbackData, 1);
    read = readCallback(callbackData, &readBuf[0], 12);
    ASSERT_EQ(read, 9);
    for(size_t i = 0; i < data.size()-1; i++)
        ASSERT_EQ(readBuf[i], i+1);

    seekCallback(callbackData, 0);
    ASSERT_EQ(tellCallback(callbackData), 0);

    std::fill(readBuf.begin(), readBuf.end(), 100);
    readCallback(callbackData, &readBuf[0], 1);
    ASSERT_EQ(tellCallback(callbackData), 1);
    ASSERT_EQ(readBuf[0], 0);
    ASSERT_EQ(readBuf[1], 100);

    std::fill(readBuf.begin(), readBuf.end(), 100);
    readCallback(callbackData, &readBuf[0], 2);
    ASSERT_EQ(tellCallback(callbackData), 3);
    ASSERT_EQ(readBuf[0], 1);
    ASSERT_EQ(readBuf[1], 2);
    ASSERT_EQ(readBuf[2], 100);

    AIDestroySimpleMemoryBufferCallbacks(readCallback, writeCallback, tellCallback, seekCallback, callbackData);
}

TEST(Exr, TestReadExr)
{
    auto data = readFile<uint8_t>(getImagesDir() + "/exr/grad_32.exr");

    ReadCallback readCallback = NULL;
    WriteCallback writeCallback = NULL;
    TellCallback tellCallback = NULL;
    SeekCallback seekCallback = NULL;
    void* callbackData = NULL;

    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &data[0], (int32_t)data.size());

    AImgHandle img = NULL;
    AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);

    int32_t width = 64;
    int32_t height = 32;

    std::vector<float> imgData(width*height*3, 0.0f);

    AImgDecodeImage(img, &imgData[0], AImgFormat::INVALID_FORMAT);

    auto knownData = readFile<float>(getImagesDir() + "/exr/grad_32.bin");

    for(int32_t y = 0; y < height; y++)
    {
        for(int32_t x = 0; x < width; x++)
        {
            ASSERT_EQ(knownData[x + width*y], imgData[(x + width*y) * 3]);
        }
    }

    AImgClose(img);
    AIDestroySimpleMemoryBufferCallbacks(readCallback, writeCallback, tellCallback, seekCallback, callbackData);
}

TEST(Exr, TestWriteExr)
{
    auto data = readFile<uint8_t>(getImagesDir() + "/exr/grad_32.exr");

    ReadCallback readCallback = NULL;
    WriteCallback writeCallback = NULL;
    TellCallback tellCallback = NULL;
    SeekCallback seekCallback = NULL;
    void* callbackData = NULL;

    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &data[0], (int32_t)data.size());

    AImgHandle img = NULL;
    AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);

    int32_t width = 64;
    int32_t height = 32;

    std::vector<float> imgData(width*height*3, 0.0f);

    AImgDecodeImage(img, &imgData[0], AImgFormat::INVALID_FORMAT);
    AImgClose(img);
    AIDestroySimpleMemoryBufferCallbacks(readCallback, writeCallback, tellCallback, seekCallback, callbackData);

    std::vector<char> fileData(4096); // fixed size buffer for a file write, not the best but it'll do for this test
    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &fileData[0], (int32_t)fileData.size());

    AImgHandle wImg = AImgGetAImg(AImgFileFormat::EXR_IMAGE_FORMAT);
    AImgWriteImage(wImg, &imgData[0], width, height, AImgFormat::RGB32F, AImgFormat::INVALID_FORMAT, NULL, NULL, 0, writeCallback, tellCallback, seekCallback, callbackData, NULL);
    AImgClose(wImg);

    seekCallback(callbackData, 0);


    AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);
    std::vector<float> imgData2(width*height*3, 0.0f);
    AImgDecodeImage(img, &imgData2[0], AImgFormat::INVALID_FORMAT);
    AImgClose(img);
    AIDestroySimpleMemoryBufferCallbacks(readCallback, writeCallback, tellCallback, seekCallback, callbackData);

    for(uint32_t i = 0; i < imgData2.size(); i++)
        ASSERT_EQ(imgData[i], imgData2[i]);
}

TEST(Exr, TestConvertDataFormat)
{
    int32_t width = 16;
    int32_t height = 16;

    std::vector<uint8_t> startingData(width*height);

    for(int32_t y = 0; y < height; y++)
    {
        for(int32_t x = 0; x < width; x++)
        {
            startingData[x + y*width] = x+y;
        }
    }

    std::vector<float> convertedF(width*height*4, 0);

    AImgConvertFormat(&startingData[0], &convertedF[0], width, height, AImgFormat::R8U, AImgFormat::RGBA32F);

    for(int32_t y = 0; y < height; y++)
    {
        for(int32_t x = 0; x < width; x++)
        {
            ASSERT_EQ((uint8_t)(convertedF[(x + y*width) * 4] * 255.0f), startingData[x + y*width]);
        }
    }

    std::vector<uint8_t> convertedBack(width*height, 0);

    AImgConvertFormat(&convertedF[0], &convertedBack[0], width, height, AImgFormat::RGBA32F, AImgFormat::R8U);

    for(uint32_t i = 0; i < convertedBack.size(); i++)
        ASSERT_EQ(startingData[i], convertedBack[i]);
}

TEST(Exr, TestOpenBadImage)
{
    std::string data = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor"
                       "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis"
                       "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."
                       "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu"
                       "fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in"
                       "culpa qui officia deserunt mollit anim id est laborum.";



    ReadCallback readCallback = NULL;
    WriteCallback writeCallback = NULL;
    TellCallback tellCallback = NULL;
    SeekCallback seekCallback = NULL;
    void* callbackData = NULL;

    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, &data[0], (int32_t)data.size());

    AImgHandle img = NULL;
    int32_t err = AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);

    ASSERT_EQ(err, AImgErrorCode::AIMG_UNSUPPORTED_FILETYPE);
    ASSERT_EQ((size_t)img, (size_t)NULL);
}

TEST(Exr, TestOpenEmptyStream)
{
    ReadCallback readCallback = NULL;
    WriteCallback writeCallback = NULL;
    TellCallback tellCallback = NULL;
    SeekCallback seekCallback = NULL;
    void* callbackData = NULL;

    AIGetSimpleMemoryBufferCallbacks(&readCallback, &writeCallback, &tellCallback, &seekCallback, &callbackData, NULL, 0);

    AImgHandle img = NULL;
    int32_t err = AImgOpen(readCallback, tellCallback, seekCallback, callbackData, &img, NULL);

    ASSERT_EQ(err, AImgErrorCode::AIMG_OPEN_FAILED_EMPTY_INPUT);
    ASSERT_EQ((size_t)img, (size_t)NULL);
}

TEST(Exr, TestWriteExr16)
{
    WriteImageTest(AImgFormat::RGB16F, AImgFormat::RGB16F);
}

TEST(Exr, TestWriteConvert32To16bits)
{
    WriteImageTest(AImgFormat::RGB32F, AImgFormat::RGB16F);
}
TEST(Exr, TestWriteConvert16To32bits)
{
    WriteImageTest(AImgFormat::RGB16F, AImgFormat::RGB32F);
}

TEST(Exr, TestWrite8bits)
{
    WriteImageTest(AImgFormat::RGB8U, AImgFormat::INVALID_FORMAT, AImgFormat::RGB16F);
}
TEST(Exr, TestWrite16bitsU)
{
    WriteImageTest(AImgFormat::R16U, AImgFormat::R16U, AImgFormat::R16F);
}

TEST(Exr, TestSupportedFormat)
{
    ASSERT_FALSE(AImgIsFormatSupported(AImgFileFormat::EXR_IMAGE_FORMAT, AImgFormat::_8BITS));
    ASSERT_TRUE(AImgIsFormatSupported(AImgFileFormat::EXR_IMAGE_FORMAT, AImgFormat::_16BITS | AImgFormat::FLOAT_FORMAT));
    ASSERT_TRUE(AImgIsFormatSupported(AImgFileFormat::EXR_IMAGE_FORMAT, AImgFormat::_32BITS | AImgFormat::FLOAT_FORMAT));
}

#endif

int main(int argc, char **argv) 
{
    AImgInitialise();

    ::testing::InitGoogleTest(&argc, argv);
    int retval = RUN_ALL_TESTS();

    AImgCleanUp();

    return retval;
}
