#include "fmt/bgi/cbg_image_decoder.h"
#include "err.h"
#include "fmt/bgi/cbg/cbg1_decoder.h"
#include "fmt/bgi/cbg/cbg2_decoder.h"
#include "util/image.h"

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

static Version get_version(io::IO &io)
{
    Version ret;
    io.peek(46, [&]()
    {
        ret = io.read_u16_le() == 2
            ? Version::Version2
            : Version::Version1;
    });
    return ret;
}

bool CbgImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CbgImageDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    auto version = get_version(file.io);
    if (version == Version::Version1)
    {
        cbg::Cbg1Decoder decoder;
        auto pixels = decoder.decode(file.io);
        return util::Image::from_pixels(*pixels)->create_file(file.name);
    }
    else if (version == Version::Version2)
    {
        cbg::Cbg2Decoder decoder;
        auto pixels = decoder.decode(file.io);
        return util::Image::from_pixels(*pixels)->create_file(file.name);
    }

    throw err::UnsupportedVersionError(static_cast<int>(version));
}

static auto dummy = fmt::Registry::add<CbgImageDecoder>("bgi/cbg");
