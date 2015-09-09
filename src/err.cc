#include "err.h"
#include "util/format.h"

using namespace au;
using namespace au::err;

UsageError::UsageError(const std::string &desc) : std::runtime_error(desc)
{
}

DataError::DataError(const std::string &desc) : std::runtime_error(desc)
{
}

RecognitionError::RecognitionError() : DataError("Data not recognized")
{
}

RecognitionError::RecognitionError(const std::string &desc) : DataError(desc)
{
}

CorruptDataError::CorruptDataError(const std::string &desc) : DataError(desc)
{
}

BadDataSizeError::BadDataSizeError() : DataError("Bad data size")
{
}

BadDataOffsetError::BadDataOffsetError() : DataError("Bad data offset")
{
}

IoError::IoError(const std::string &desc) : std::runtime_error(desc)
{
}

FileNotFoundError::FileNotFoundError(const std::string &desc) : IoError(desc)
{
}

NotSupportedError::NotSupportedError(const std::string &desc)
    : std::runtime_error(desc)
{
}

UnsupportedBitDepthError::UnsupportedBitDepthError(size_t bit_depth)
    : NotSupportedError(util::format("Unsupported bit depth: %d", bit_depth))
{
}

UnsupportedChannelCountError::UnsupportedChannelCountError(size_t channel_count)
    : NotSupportedError(util::format(
        "Unsupported channel count: %d", channel_count))
{
}

UnsupportedVersionError::UnsupportedVersionError(int version)
    : NotSupportedError(util::format("Unsupported version: %d", version))
{
}

UnsupportedVersionError::UnsupportedVersionError()
    : NotSupportedError("Unsupported version")
{
}
