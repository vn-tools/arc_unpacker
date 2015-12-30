#pragma once

#include <stdexcept>

namespace au {
namespace err {

    struct GeneralError : public std::runtime_error
    {
        virtual ~GeneralError() {}
        GeneralError(const std::string &description);
    };

    struct UsageError final : public GeneralError
    {
        UsageError(const std::string &description);
    };

    struct DataError : public GeneralError
    {
        virtual ~DataError() {}
    protected:
        DataError(const std::string &description);
    };

    struct RecognitionError final : public DataError
    {
        RecognitionError();
        RecognitionError(const std::string &description);
    };

    struct CorruptDataError final : public DataError
    {
        CorruptDataError(const std::string &description);
    };

    struct BadDataSizeError final : public DataError
    {
        BadDataSizeError();
    };

    struct BadDataOffsetError final : public DataError
    {
        BadDataOffsetError();
    };

    struct IoError : public GeneralError
    {
        virtual ~IoError() {}
        IoError(const std::string &description);
    };

    struct EofError final : public IoError
    {
        EofError();
    };

    struct FileNotFoundError final : public IoError
    {
        FileNotFoundError(const std::string &description);
    };

    struct NotSupportedError : public GeneralError
    {
        virtual ~NotSupportedError() {}
        NotSupportedError(const std::string &description);
    };

    struct UnsupportedBitDepthError final : public NotSupportedError
    {
        UnsupportedBitDepthError(size_t bit_depth);
    };

    struct UnsupportedChannelCountError final : public NotSupportedError
    {
        UnsupportedChannelCountError(size_t channel_count);
    };

    struct UnsupportedVersionError final : public NotSupportedError
    {
        UnsupportedVersionError();
        UnsupportedVersionError(int version);
    };

} }
