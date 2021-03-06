﻿using System;
using NUnit.Framework;
using Artomatix.ImageLoader;
using System.Reflection;
using System.IO;
using System.Runtime.CompilerServices;
using Artomatix.ImageLoader.ImgEncodingOptions;

namespace ArtomatixImageLoaderTests
{
    [TestFixture]
    public class TestAImg
    {
        private static string getCsFilePath([CallerFilePath] string filePath = "")
        {
            return filePath;
        }

        private static string getImagesDir()
        {
            string thisFile = getCsFilePath();
            string thisDir = Path.GetDirectoryName(thisFile);

            return Path.GetFullPath(thisDir + "/../../../../test_images");
        }

        [Test]
        public static void TestReadAttrs()
        {
            using (AImg img = new AImg(File.Open(getImagesDir() + "/exr/grad_32.exr", FileMode.Open)))
            {
                Assert.AreEqual(64, img.width);
                Assert.AreEqual(32, img.height);
                Assert.AreEqual(4, img.bytesPerChannel);
                Assert.AreEqual(3, img.numChannels);
                Assert.AreEqual(AImgFloatOrIntType.FITYPE_FLOAT, img.floatOrInt);
                Assert.AreEqual(AImgFileFormat.EXR_IMAGE_FORMAT, img.detectedFileFormat);
                Assert.AreEqual(AImgFormat.RGB32F, img.decodedImgFormat);
                Assert.AreEqual(0, img.colourProfile.Length);
                Assert.AreEqual("no_profile", img.colourProfileName);
            }
        }

        [Test]
        public static void TestReadImage()
        {
            using (AImg img = new AImg(File.Open(getImagesDir() + "/exr/grad_32.exr", FileMode.Open)))
            {
                float[] data = new float[img.width * img.height * img.decodedImgFormat.numChannels()];
                img.decodeImage(data);

                using (var dataStream = File.Open(getImagesDir() + "/exr/grad_32.bin", FileMode.Open))
                {
                    var reader = new BinaryReader(dataStream);
                    var realData = new float[dataStream.Length / 4];

                    for (int i = 0; i < realData.Length; i++)
                        realData[i] = reader.ReadSingle();

                    Assert.AreEqual(realData.Length * 3, data.Length);

                    for (int i = 0; i < realData.Length; i++)
                        Assert.AreEqual(realData[i], data[i * 3]);
                }
            }
        }

        [Test]
        public static void TestForceImageFormat()
        {
            float[] fData;
            byte[] bData;

            using (AImg img = new AImg(File.Open(getImagesDir() + "/png/8-bit.png", FileMode.Open)))
            {
                fData = new float[img.width * img.height * 3];
                img.decodeImage(fData, AImgFormat.RGB32F);
            }

            using (AImg img = new AImg(File.Open(getImagesDir() + "/png/8-bit.png", FileMode.Open)))
            {
                bData = new byte[img.width * img.height * 3];
                img.decodeImage(bData, AImgFormat.RGB8U);

                for (int y = 0; y < img.height; y++)
                {
                    for (int x = 0; x < img.width; x++)
                    {
                        float groundTruth = ((float)bData[(x + y * img.width) * 3]) / 255.0f;
                        float forced = fData[(x + y * img.width) * 3];

                        Assert.AreEqual(groundTruth, forced);
                    }
                }
            }
        }

        [Test]
        public static void TestGetWhatFormatWIllBeWritten()
        {
            AImgFormat res = AImg.getWhatFormatWillBeWrittenForData(AImgFileFormat.EXR_IMAGE_FORMAT, AImgFormat.RGBA32F, AImgFormat.INVALID_FORMAT);
            Assert.AreEqual(AImgFormat.RGBA32F, res);
        }

        [Test]
        public static void TestSupportedFormat()
        {
            Assert.True(AImg.IsFormatSupported(AImgFileFormat.TIFF_IMAGE_FORMAT, AImgFormat._8BITS));
            Assert.True(AImg.IsFormatSupported(AImgFileFormat.TIFF_IMAGE_FORMAT, AImgFormat._16BITS));
            Assert.True(AImg.IsFormatSupported(AImgFileFormat.TIFF_IMAGE_FORMAT, AImgFormat._32BITS));

            Assert.False(AImg.IsFormatSupported(AImgFileFormat.EXR_IMAGE_FORMAT, AImgFormat._8BITS));
            Assert.True(AImg.IsFormatSupported(AImgFileFormat.EXR_IMAGE_FORMAT, AImgFormat._16BITS));
            Assert.True(AImg.IsFormatSupported(AImgFileFormat.EXR_IMAGE_FORMAT, AImgFormat._32BITS));

            Assert.True(AImg.IsFormatSupported(AImgFileFormat.JPEG_IMAGE_FORMAT, AImgFormat._8BITS));
            Assert.False(AImg.IsFormatSupported(AImgFileFormat.JPEG_IMAGE_FORMAT, AImgFormat._16BITS));
            Assert.False(AImg.IsFormatSupported(AImgFileFormat.JPEG_IMAGE_FORMAT, AImgFormat._32BITS));

            Assert.True(AImg.IsFormatSupported(AImgFileFormat.PNG_IMAGE_FORMAT, AImgFormat._8BITS));
            Assert.True(AImg.IsFormatSupported(AImgFileFormat.PNG_IMAGE_FORMAT, AImgFormat._16BITS));
            Assert.False(AImg.IsFormatSupported(AImgFileFormat.PNG_IMAGE_FORMAT, AImgFormat._32BITS));

            Assert.True(AImg.IsFormatSupported(AImgFileFormat.TGA_IMAGE_FORMAT, AImgFormat._8BITS));
            Assert.False(AImg.IsFormatSupported(AImgFileFormat.TGA_IMAGE_FORMAT, AImgFormat._16BITS));
            Assert.False(AImg.IsFormatSupported(AImgFileFormat.TGA_IMAGE_FORMAT, AImgFormat._32BITS));
        }

        [Test]
        public static void TestWriteExr()
        {
            using (AImg img = new AImg(File.Open(getImagesDir() + "/exr/grad_32.exr", FileMode.Open)))
            {
                float[] data = new float[img.width * img.height * 3];
                img.decodeImage(data, AImgFormat.RGB32F);

                var colourProfileName = img.colourProfileName;
                var colourProfile = img.colourProfile;

                using (var writeStream = new MemoryStream())
                {
                    var wImg = new AImg(AImgFileFormat.EXR_IMAGE_FORMAT);
                    wImg.writeImage(data, img.width, img.height, AImgFormat.RGB32F, colourProfileName, colourProfile, writeStream);
                    writeStream.Seek(0, SeekOrigin.Begin);

                    using (AImg img2 = new AImg(writeStream))
                    {
                        float[] data2 = new float[img.width * img.height * 3];
                        img2.decodeImage(data2);

                        for (int i = 0; i < data.Length; i++)
                            Assert.AreEqual(data[i], data2[i]);
                    }
                }
            }
        }

        [Test]
        public static void TestWriteUncompressedPng()
        {
            using (AImg img = new AImg(File.Open(getImagesDir() + "/png/8-bit.png", FileMode.Open)))
            {
                byte[] data = new byte[img.width * img.height * img.decodedImgFormat.numChannels() * img.decodedImgFormat.bytesPerChannel()];
                img.decodeImage(data);

                var colourProfileName = img.colourProfileName;
                var colourProfile = img.colourProfile;

                using (var writeStream = new MemoryStream())
                {
                    var wImg = new AImg(AImgFileFormat.PNG_IMAGE_FORMAT);

                    PngEncodingOptions options = new PngEncodingOptions(0, PngEncodingOptions.Filter.PNG_NO_FILTERS);

                    wImg.writeImage(data, img.width, img.height, img.decodedImgFormat, colourProfileName, colourProfile, writeStream, options);
                    writeStream.Seek(0, SeekOrigin.Begin);

                    using (AImg img2 = new AImg(writeStream))
                    {
                        byte[] data2 = new byte[img.width * img.height * img.decodedImgFormat.numChannels() * img.decodedImgFormat.bytesPerChannel()];
                        img2.decodeImage(data2);

                        for (int i = 0; i < data.Length; i++)
                            Assert.AreEqual(data[i], data2[i]);
                    }
                }
            }
        }

        [Test]
        public static void TestWrite32BitToPng()
        {
            using (AImg img = new AImg(File.Open(getImagesDir() + "/exr/grad_32.exr", FileMode.Open)))
            {
                Assert.AreEqual(AImgFormat.RGB32F, img.decodedImgFormat);

                float[] data = new float[img.width * img.height * img.decodedImgFormat.numChannels()];
                img.decodeImage(data);

                var colourProfileName = img.colourProfileName;
                var colourProfile = img.colourProfile;

                using (var writeStream = new MemoryStream())
                {
                    var wImg = new AImg(AImgFileFormat.PNG_IMAGE_FORMAT);
                    wImg.writeImage(data, img.width, img.height, img.decodedImgFormat, colourProfileName, colourProfile, writeStream);
                    writeStream.Seek(0, SeekOrigin.Begin);

                    using (AImg img2 = new AImg(writeStream))
                    {
                        Assert.AreEqual(img2.decodedImgFormat, AImgFormat.RGB16U);

                        UInt16[] data2 = new UInt16[img2.width * img2.height * img2.decodedImgFormat.numChannels()];
                        img2.decodeImage(data2);

                        for (int y = 0; y < img.height; y++)
                        {
                            for (int x = 0; x < img.width; x++)
                            {
                                var fR = data[((x + y * img.width) * img.decodedImgFormat.numChannels()) + 0];
                                var fG = data[((x + y * img.width) * img.decodedImgFormat.numChannels()) + 1];
                                var fB = data[((x + y * img.width) * img.decodedImgFormat.numChannels()) + 2];

                                var hR = data2[((x + y * img.width) * img2.decodedImgFormat.numChannels()) + 0];
                                var hG = data2[((x + y * img.width) * img2.decodedImgFormat.numChannels()) + 1];
                                var hB = data2[((x + y * img.width) * img2.decodedImgFormat.numChannels()) + 2];

                                Assert.AreEqual((UInt16)(fR * 65535), hR);
                                Assert.AreEqual((UInt16)(fG * 65535), hG);
                                Assert.AreEqual((UInt16)(fB * 65535), hB);
                            }
                        }
                    }
                }
            }
        }

        [Test]
        public static void TestOpenEmptyStream()
        {
            Assert.Throws<AImgOpenFailedEmptyInputException>(delegate
            {
                var s = new MemoryStream();
                AImg img = new AImg(s);
            });
        }

        public static void TestWriteIMG<T>(int width, int height, AImgFormat format, AImgFileFormat fileformat, float allowedDelta = 0, bool deleteAfterwards = true) where T : struct
        {
            var img = new AImg(fileformat);
            T[] data = new T[width * height * format.numChannels()];

            var r = new Random(123);

            for (int i = 0; i < data.Length; i++)
            {
                if (format.bytesPerChannel() == 2)
                    data[i] = (T)Convert.ChangeType(r.Next(0, ushort.MaxValue), typeof(T));
                else if (format.bytesPerChannel() > 1)
                    data[i] = (T)Convert.ChangeType(r.Next(0, 255) / 255.0f, typeof(T));
                else
                    data[i] = (T)Convert.ChangeType(r.Next(0, 255), typeof(T));
            }

            using (var f = new FileStream(getImagesDir() + "/testOut", FileMode.Create))
                img.writeImage<T>(data, width, height, format, null, null, f);

            T[] readBackData = null;
            using (AImg f = new AImg(new FileStream(getImagesDir() + "/testOut", FileMode.Open)))
            {
                readBackData = new T[width * height * f.decodedImgFormat.sizeInBytes()];
                f.decodeImage(readBackData, format);
            }

            for (int i = 0; i < data.Length; i++)
                Assert.That(data[i], Is.EqualTo(readBackData[i]).Within(allowedDelta));

            if (deleteAfterwards)
            {
                try
                {
                    Directory.Delete(getImagesDir() + "/testOut");
                }
                catch { }
            }
        }

        [Test]
        public static void TestICCProfilePng()
        {
            // Read image with colour profile
            using (AImg img = new AImg(File.Open(getImagesDir() + "/png/ICC.png", FileMode.Open)))
            {
                float[] data = new float[img.width * img.height * img.decodedImgFormat.numChannels()];
                // Decode image
                img.decodeImage(data);

                var colourProfileName = img.colourProfileName;
                var colourProfile = img.colourProfile;

                // Write image with colour profile
                using (var dataStream = File.Open(getImagesDir() + "/png/ICC_out.png", FileMode.Create))
                {
                    img.writeImage(data, img.width, img.height, img.decodedImgFormat, colourProfileName, colourProfile, dataStream);
                    dataStream.Close();

                    // Read the image back
                    using (AImg img2 = new AImg(File.Open(getImagesDir() + "/png/ICC_out.png", FileMode.Open)))
                    {
                        Assert.AreEqual(img.colourProfileName, img2.colourProfileName);
                        Assert.AreEqual(img.colourProfile.Length, img2.colourProfile.Length);
                        for (int i = 0; i < img.colourProfile.Length; i++)
                            Assert.AreEqual(img.colourProfile[i], img2.colourProfile[i]);
                    }
                }
            }
            try
            {
                File.Delete(getImagesDir() + "/png/ICC_out.png");
            }
            catch { }
        }

        [Test]
        public static void TestICCProfileTiff()
        {
            // Read image with colour profile
            using (AImg img = new AImg(File.Open(getImagesDir() + "/tiff/ICC.tif", FileMode.Open)))
            {
                float[] data = new float[img.width * img.height * img.decodedImgFormat.numChannels()];
                // Decode image
                img.decodeImage(data);

                var colourProfileName = img.colourProfileName;
                var colourProfile = img.colourProfile;

                // Write image with colour profile
                using (var dataStream = File.Open(getImagesDir() + "/tiff/ICC_out.tif", FileMode.Create))
                {
                    img.writeImage(data, img.width, img.height, img.decodedImgFormat, colourProfileName, colourProfile, dataStream);
                    dataStream.Close();

                    // Read the image back
                    using (AImg img2 = new AImg(File.Open(getImagesDir() + "/tiff/ICC_out.tif", FileMode.Open)))
                    {
                        Assert.AreEqual(img.colourProfileName, img2.colourProfileName);
                        Assert.AreEqual(img.colourProfile.Length, img2.colourProfile.Length);
                        for (int i = 0; i < img.colourProfile.Length; i++)
                            Assert.AreEqual(img.colourProfile[i], img2.colourProfile[i]);
                    }
                }
            }
            try
            {
                File.Delete(getImagesDir() + "/tiff/ICC_out.tif");
            }
            catch { }
        }

        [Test]
        public static void TestWriteTiffs()
        {
            TestWriteIMG<float>(128, 128, AImgFormat.RGBA32F, AImgFileFormat.TIFF_IMAGE_FORMAT);
            TestWriteIMG<ushort>(128, 128, AImgFormat.RGBA16F, AImgFileFormat.TIFF_IMAGE_FORMAT);
            TestWriteIMG<byte>(128, 128, AImgFormat.RGBA8U, AImgFileFormat.TIFF_IMAGE_FORMAT);
            TestWriteIMG<ushort>(128, 128, AImgFormat.RGBA16U, AImgFileFormat.TIFF_IMAGE_FORMAT);
            TestWriteIMG<ushort>(128, 128, AImgFormat.R16U, AImgFileFormat.TIFF_IMAGE_FORMAT);
        }

        [Test]
        public static void TestWritePngs()
        {
            TestWriteIMG<float>(128, 128, AImgFormat.RGBA32F, AImgFileFormat.PNG_IMAGE_FORMAT);
            TestWriteIMG<byte>(128, 128, AImgFormat.RGBA8U, AImgFileFormat.PNG_IMAGE_FORMAT);
            TestWriteIMG<ushort>(128, 128, AImgFormat.RGBA16U, AImgFileFormat.PNG_IMAGE_FORMAT);
        }

        [Test]
        public static void TestWriteEXRs()
        {
            TestWriteIMG<float>(128, 128, AImgFormat.RGBA32F, AImgFileFormat.EXR_IMAGE_FORMAT);
            TestWriteIMG<ushort>(128, 128, AImgFormat.RGBA16F, AImgFileFormat.EXR_IMAGE_FORMAT);
            TestWriteIMG<ushort>(128, 128, AImgFormat.RGBA16U, AImgFileFormat.EXR_IMAGE_FORMAT, 20);
        }

        [Test]
        public static void TestWriteTGAs()
        {
            TestWriteIMG<byte>(128, 128, AImgFormat.RGBA8U, AImgFileFormat.TGA_IMAGE_FORMAT);
            TestWriteIMG<byte>(128, 128, AImgFormat.RGB8U, AImgFileFormat.TGA_IMAGE_FORMAT);
            TestWriteIMG<byte>(128, 128, AImgFormat.RG8U, AImgFileFormat.TGA_IMAGE_FORMAT);
            TestWriteIMG<byte>(128, 128, AImgFormat.R8U, AImgFileFormat.TGA_IMAGE_FORMAT);
        }

        [Test]
        public static void TestWrite2ChannelPNGs()
        {
            TestWriteIMG<byte>(128, 128, AImgFormat.RG8U, AImgFileFormat.PNG_IMAGE_FORMAT);
        }
    }
}