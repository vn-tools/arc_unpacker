#pragma once

#include <stdexcept>

namespace au {
namespace err {

    struct GeneralError : public std::runtime_error
    {
        GeneralError(const std::string &description);
    };

    struct UsageError : public GeneralError
    {
        UsageError(const std::string &description);
    };

    struct DataError : public GeneralError
    {
    protected:
        DataError(const std::string &description);
    };

    struct RecognitionError : public DataError
    {
        RecognitionError();
        RecognitionError(const std::string &description);
    };

    struct CorruptDataError : public DataError
    {
        CorruptDataError(const std::string &description);
    };

    struct BadDataSizeError : public DataError
    {
        BadDataSizeError();
    };

    struct BadDataOffsetError : public DataError
    {
        BadDataOffsetError();
    };

    struct IoError : public GeneralError
    {
        IoError(const std::string &description);
    };

    struct EofError : public IoError
    {
        EofError();
    };

    struct FileNotFoundError : public IoError
    {
        FileNotFoundError(const std::string &description);
    };

    struct NotSupportedError : public GeneralError
    {
        NotSupportedError(const std::string &description);
    };

    struct UnsupportedBitDepthError : public NotSupportedError
    {
        UnsupportedBitDepthError(size_t bit_depth);
    };

    struct UnsupportedChannelCountError : public NotSupportedError
    {
        UnsupportedChannelCountError(size_t channel_count);
    };

    struct UnsupportedVersionError : public NotSupportedError
    {
        UnsupportedVersionError();
        UnsupportedVersionError(int version);
    };

} }
