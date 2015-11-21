#include "fmt/bgi/cbg_image_decoder.h"
#include "err.h"
#include "fmt/bgi/cbg/cbg1_decoder.h"
#include "fmt/bgi/cbg/cbg2_decoder.h"

using namespace au;
using namespace au::fmt::bgi;

namespace
{
    enum class Version : u8
    {
        Version1 = 1,
        Version2 = 2,
    };
}

static const bstr magic = "CompressedBG___\x00"_b;

static Version get_version(io::Stream &stream)
{
    Version ret;
    stream.peek(46, [&]()
    {
        ret = stream.read_u16_le() == 2
            ? Version::Version2
            : Version::Version1;
    });
    return ret;
}

bool CbgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid CbgImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    auto version = get_version(input_file.stream);
    if (version == Version::Version1)
    {
        cbg::Cbg1Decoder decoder;
        return *decoder.decode(input_file.stream);
    }
    else if (version == Version::Version2)
    {
        cbg::Cbg2Decoder decoder;
        return *decoder.decode(input_file.stream);
    }

    throw err::UnsupportedVersionError(static_cast<int>(version));
}

static auto dummy = fmt::register_fmt<CbgImageDecoder>("bgi/cbg");
