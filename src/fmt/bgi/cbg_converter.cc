// CBG image
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC
//
// Known games:
// - [07th Expansion] [020810] Higurashi No Naku Koro Ni
// - [Lump of Sugar] [20130531] Magical Charming
// - [Overdrive] [110930] Go! Go! Nippon! ~My First Trip to Japan~

#include "err.h"
#include "fmt/bgi/cbg/cbg1_decoder.h"
#include "fmt/bgi/cbg/cbg2_decoder.h"
#include "fmt/bgi/cbg_converter.h"
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

bool CbgConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CbgConverter::decode_internal(File &file) const
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

static auto dummy = fmt::Registry::add<CbgConverter>("bgi/cbg");
