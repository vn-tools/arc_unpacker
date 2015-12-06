#include "err.h"
#include "algo/format.h"

using namespace au;
using namespace au::err;

GeneralError::GeneralError(const std::string &desc) : std::runtime_error(desc)
{
}

UsageError::UsageError(const std::string &desc) : GeneralError(desc)
{
}

DataError::DataError(const std::string &desc) : GeneralError(desc)
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

IoError::IoError(const std::string &desc) : GeneralError(desc)
{
}

EofError::EofError() : IoError("Premature end of file")
{
}

FileNotFoundError::FileNotFoundError(const std::string &desc) : IoError(desc)
{
}

NotSupportedError::NotSupportedError(const std::string &desc)
    : GeneralError(desc)
{
}

UnsupportedBitDepthError::UnsupportedBitDepthError(size_t bit_depth)
    : NotSupportedError(algo::format("Unsupported bit depth: %d", bit_depth))
{
}

UnsupportedChannelCountError::UnsupportedChannelCountError(size_t channel_count)
    : NotSupportedError(algo::format(
        "Unsupported channel count: %d", channel_count))
{
}

UnsupportedVersionError::UnsupportedVersionError(int version)
    : NotSupportedError(algo::format("Unsupported version: %d", version))
{
}

UnsupportedVersionError::UnsupportedVersionError()
    : NotSupportedError("Unsupported version")
{
}
