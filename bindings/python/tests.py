import unittest
import AImg
import os
import io
import numpy as np

imagesDir = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + "/../../test_images")

class TestAImg(unittest.TestCase):
    
    def test_read_attrs(self):
        img = AImg.AImg(imagesDir + "/exr/grad_32.exr")

        self.assertEqual(img.detectedFileFormat, AImg.AImgFileFormats["EXR_IMAGE_FORMAT"])
        self.assertEqual(img.width, 64)
        self.assertEqual(img.height, 32)
        self.assertEqual(img.decodedImgFormat, AImg.AImgFormats["RGB32F"])
        self.assertEqual(img.colourProfile, None)
        self.assertEqual(img.profileName, "no_profile")

    def test_read_icc(self):
        img = AImg.AImg(imagesDir + "/png/ICC.png")

        self.assertEqual(img.detectedFileFormat, AImg.AImgFileFormats["PNG_IMAGE_FORMAT"])
        self.assertEqual(img.width, 400)
        self.assertEqual(img.height, 400)
        self.assertEqual(img.decodedImgFormat, AImg.AImgFormats["RGB8U"])
        self.assertEqual(len(img.colourProfile), 560)
        self.assertEqual(img.profileName, "ICC profile")

    def test_read_exr(self):
        img = AImg.AImg(imagesDir + "/exr/grad_32.exr")
        decoded = img.decode()

        groundTruth = np.fromfile(imagesDir + "/exr/grad_32.bin", dtype=np.float32)

        for y in range(img.height):
            for x in range(img.width):
                self.assertEqual(groundTruth[x + y*img.width], decoded[y][x][0])

    def test_write_exr(self):
        img = AImg.AImg(imagesDir + "/exr/grad_32.exr")
        decoded = img.decode()
        
        outFile = io.BytesIO()

        AImg.write(outFile, decoded, AImg.AImgFileFormats["EXR_IMAGE_FORMAT"], img.profileName, img.colourProfile)
        
        outFile.seek(0)
        img2 = AImg.AImg(outFile)

        decoded2 = img2.decode()

        for y in range(img2.height):
            for x in range(img2.width):
                for c in range(decoded.shape[2]):
                    self.assertEqual(decoded2[y][x][c], decoded[y][x][c])

    def test_write_tga(self):
        img = AImg.AImg(imagesDir + "/png/alpha.png")
        decoded = img.decode()
        
        outFile = io.BytesIO()

        AImg.write(outFile, decoded, AImg.AImgFileFormats["TGA_IMAGE_FORMAT"], img.profileName, img.colourProfile)

        outFile.seek(0)
        img2 = AImg.AImg(outFile)

        decoded2 = img2.decode()

        for y in range(img2.height):
            for x in range(img2.width):
                for c in range(decoded.shape[2]):
                    self.assertEqual(decoded2[y][x][c], decoded[y][x][c])

    def test_open_bad_file(self):
        with self.assertRaises(AImg.AImgUnsupportedFiletypeException) as context:
            img = AImg.AImg(__file__)

    def test_open_empty_stream(self):
        with self.assertRaises(AImg.AImgOpenFailedEmptyInputException) as context:
            stream = io.BytesIO()
            img = AImg.AImg(stream)

    def test_force_away_alpha(self):
        img = AImg.AImg(imagesDir + "/png/alpha.png")
        dataNoA = img.decode(forceImageFormat=AImg.AImgFormats["RGB8U"]) # real format is RGBA8U
        del img

        img = AImg.AImg(imagesDir + "/png/alpha.png")
        data = img.decode()

        width = img.width
        height = img.height
        del img

        for y in range(height):
            for x in range(width):
                if data[y][x][3] != 0:
                    self.assertEqual(data[y][x][0], dataNoA[y][x][0])
                    self.assertEqual(data[y][x][1], dataNoA[y][x][1])
                    self.assertEqual(data[y][x][2], dataNoA[y][x][2])
    
    def test_write_png_no_compression(self):
        img = AImg.AImg(imagesDir + "/png/alpha.png")
        decoded = img.decode()
        
        outFile = io.BytesIO()
        
        options = AImg.PngEncodingOptions(0, AImg.PngEncodingOptions.PNG_NO_FILTERS)
        AImg.write(outFile, decoded, AImg.AImgFileFormats["PNG_IMAGE_FORMAT"], img.profileName, img.colourProfile, encodeOptions=options)

        outFile.seek(0)
        img2 = AImg.AImg(outFile)

        decoded2 = img2.decode()

        for y in range(img2.height):
            for x in range(img2.width):
                for c in range(decoded.shape[2]):
                    self.assertEqual(decoded2[y][x][c], decoded[y][x][c])


    def test_icc_png(self):
        # Read image with colour profile
        img = AImg.AImg(imagesDir + "/png/ICC.png")
        # Decode image
        decoded = img.decode()
        
        outFile = open(imagesDir + "/png/ICC_out.png", mode='wb');

        # Write image with colour profile
        AImg.write(outFile, decoded, AImg.AImgFileFormats["PNG_IMAGE_FORMAT"], img.profileName, img.colourProfile)
        outFile.close()

        # Read the image back
        img2 = AImg.AImg(imagesDir + "/png/ICC_out.png")

        self.assertEqual(img.profileName, img2.profileName)
        self.assertEqual(len(img.colourProfile), len(img2.colourProfile))
        for i in range(len(img.colourProfile)):
            self.assertEqual(img.colourProfile[i], img2.colourProfile[i])

if __name__ == "__main__":
    unittest.main()
