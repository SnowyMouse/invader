// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__ERROR_HPP
#define INVADER__ERROR_HPP

#include <stdexcept>

namespace Invader {
    class Exception : public std::runtime_error {
    protected:
        Exception(const char *what) : std::runtime_error(what) {}
        Exception(const Exception &) = default;
        Exception(Exception &&) = default;
        virtual ~Exception() = 0;
    };
    
    #define DEFINE_EXCEPTION(exception_class, error_text) \
    class exception_class : public Exception {\
    public: \
        exception_class() : Exception(error_text) {} \
        exception_class(const exception_class &) = default; \
        exception_class(exception_class &&) = default; \
    }

    /**
     * This is thrown when invalid parameters were given to a function.
     */
    DEFINE_EXCEPTION(InvalidArgumentException, "invalid arguments were given");

    /**
     * This is thrown when a map has an invalid name.
     */
    DEFINE_EXCEPTION(InvalidScenarioNameException, "scenario name is invalid");

    /**
     * This is thrown when a pointer is invalid.
     */
    DEFINE_EXCEPTION(InvalidPointerException, "pointer is invalid");

    /**
     * This is thrown when a dependency is invalid.
     */
    DEFINE_EXCEPTION(InvalidDependencyException, "dependency is invalid");

    /**
     * This is thrown when tag data exceeds the maximum tag data size
     */
    DEFINE_EXCEPTION(MaximumTagDataSizeException, "maximum tag data size exceeded");

    /**
     * This is thrown when tag data exceeds the maximum file size
     */
    DEFINE_EXCEPTION(MaximumFileSizeException, "maximum file size exceeded");

    /**
     * This is thrown when tag data is out of bounds.
     */
    DEFINE_EXCEPTION(OutOfBoundsException, "data is out of bounds");

    /**
     * This is thrown when an invalid cyclic loop is present, likely resulting in infinite recursion.
     */
    DEFINE_EXCEPTION(InvalidCyclicLoopException, "invalid cyclic loop");

    /**
     * This is thrown when tag data remains
     */
    DEFINE_EXCEPTION(ExtraTagDataException, "expected EOF found extra tag data");

    /**
     * This is thrown when a tag cannot be compiled because its tag class is wrong
     */
    DEFINE_EXCEPTION(UnexpectedTagClassException, "unexpected tag class");

    /**
     * This is thrown when a tag cannot be compiled because it's unknown
     */
    DEFINE_EXCEPTION(UnknownTagClassException, "unknown tag class");

    /**
     * This is thrown when a tag cannot be compiled because it's not fully implemented
     */
    DEFINE_EXCEPTION(UnimplementedTagClassException, "unimplemented tag class");

    /**
     * This is thrown when a file cannot be compiled because it's not found or could not be opened
     */
    DEFINE_EXCEPTION(FailedToOpenFileException, "failed to open a file");

    /**
     * This is thrown when some other tag related error occurs.
     */
    DEFINE_EXCEPTION(TagErrorException, "an error occured with a tag");

    /**
     * This is thrown when a map isn't valid and tried to be opened
     */
    DEFINE_EXCEPTION(InvalidMapException, "tried to open an invalid map");

    /**
     * This is thrown when a tag path is too long or is otherwise invalid
     */
    DEFINE_EXCEPTION(InvalidTagPathException, "tag path is invalid");

    /**
     * This is thrown when an invalid bitmap input is given when making a bitmap tag
     */
    DEFINE_EXCEPTION(InvalidInputBitmapException, "input bitmap is invalid");

    /**
     * This is thrown when an invalid bitmap format is used
     */
    DEFINE_EXCEPTION(InvalidBitmapFormatException, "bitmap format is invalid");

    /**
     * This is thrown when an invalid sound input is given when making a sound tag
     */
    DEFINE_EXCEPTION(InvalidInputSoundException, "input sound is invalid");

    /**
     * This is thrown when an error occurs when encoding sound
     */
    DEFINE_EXCEPTION(SoundEncodeFailureException, "sound failed to encode");

    /**
     * This is thrown when a map needs decompressed, first
     */
    DEFINE_EXCEPTION(MapNeedsDecompressedException, "compressed map needs to be decompressed");

    /**
     * This is thrown when a map needs compressed, first
     */
    DEFINE_EXCEPTION(MapNeedsCompressedException, "decompressed map needs to be compressed");

    /**
     * This is thrown when a map engine is unsupported
     */
    DEFINE_EXCEPTION(UnsupportedMapEngineException, "map engine is unsupported");

    /**
     * This is thrown when compression failed
     */
    DEFINE_EXCEPTION(CompressionFailureException, "failed to compress");

    /**
     * This is thrown when decompression failed
     */
    DEFINE_EXCEPTION(DecompressionFailureException, "failed to decompress");

    /**
     * This is thrown when tag data is invalid
     */
    DEFINE_EXCEPTION(InvalidTagDataException, "tag data is invalid");

    /**
     * This is thrown when a HEK tag data header is invalid
     */
    DEFINE_EXCEPTION(InvalidTagDataHeaderException, "tag data header is invalid");

    /**
     * This is thrown when a resource map was not supplied when it should have
     */
    DEFINE_EXCEPTION(ResourceMapRequiredException, "no resource map was supplied");
}
#endif
