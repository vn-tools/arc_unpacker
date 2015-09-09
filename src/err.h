#pragma once

#include <stdexcept>
#include <string>

namespace au {
namespace err {

    struct UsageError : public std::runtime_error
    {
        UsageError(const std::string &description);
    };

    struct DataError : public std::runtime_error
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

    struct IoError : public std::runtime_error
    {
        IoError(const std::string &description);
    };

    struct FileNotFoundError : public IoError
    {
        FileNotFoundError(const std::string &description);
    };

    struct NotSupportedError : public std::runtime_error
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
