﻿using System;
using System.Runtime.Serialization;

namespace Artomatix.ImageLoader
{
    public class AImgException : Exception
    {
        protected AImgException(string msg) : base(msg)
        {
        }

        public static void checkErrorCode(IntPtr img, Int32 errorCode)
        {
            if (errorCode != 0)
            {
                string msg = "";

                if (img != IntPtr.Zero)
                    msg = ImgLoader.AImgGetLastErrorDetails(img);

                switch (errorCode)
                {
                    case -1:
                        throw new AImgUnsupportedFiletypeException(msg);
                    case -2:
                        throw new AImgLoadFailedExternalException(msg);
                    case -3:
                        throw new AImgLoadFailedInternalException(msg);
                    case -4:
                        throw new AImgConversionFailedBadFormatException(msg);
                    case -5:
                        throw new AImgWriteFailedExternalException(msg);
                    case -6:
                        throw new AImgWriteFailedInternalException(msg);
                    case -7:
                        throw new AImgLoadFailedUnsupportedTiffException(msg);
                    case -8:
                        throw new AImgOpenFailedEmptyInputException(msg);
                    case -9:
                        throw new AImgInvalidEncodeArgsException(msg);
                    case -10:
                        throw new AImgWriteNotSupportedForFormat(msg);

                    default:
                        throw new AImgException("Unknown error code: " + errorCode + " " + msg);
                }
            }
        }
    }

    internal class AImgWriteNotSupportedForFormat : Exception
    {
        public AImgWriteNotSupportedForFormat(string message) : base(message)
        {
        }
    }

    public class AImgUnsupportedFiletypeException : AImgException
    {
        public AImgUnsupportedFiletypeException(string msg) : base(msg)
        {
        }
    }

    public class AImgLoadFailedExternalException : AImgException
    {
        public AImgLoadFailedExternalException(string msg) : base(msg)
        {
        }
    }

    public class AImgLoadFailedInternalException : AImgException
    {
        public AImgLoadFailedInternalException(string msg) : base(msg)
        {
        }
    }

    public class AImgConversionFailedBadFormatException : AImgException
    {
        public AImgConversionFailedBadFormatException(string msg) : base(msg)
        {
        }
    }

    public class AImgWriteFailedExternalException : AImgException
    {
        public AImgWriteFailedExternalException(string msg) : base(msg)
        {
        }
    }

    public class AImgWriteFailedInternalException : AImgException
    {
        public AImgWriteFailedInternalException(string msg) : base(msg)
        {
        }
    }

    public class AImgOpenFailedEmptyInputException : AImgException
    {
        public AImgOpenFailedEmptyInputException(string msg) : base(msg)
        {
        }
    }

    public class AImgLoadFailedUnsupportedTiffException : AImgException
    {
        public AImgLoadFailedUnsupportedTiffException(string msg) : base(msg)
        {
        }
    }

    public class AImgInvalidEncodeArgsException : AImgException
    {
        public AImgInvalidEncodeArgsException(string msg) : base(msg)
        {
        }
    }
}