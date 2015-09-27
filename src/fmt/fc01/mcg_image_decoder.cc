#include "fmt/fc01/mcg_image_decoder.h"
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "fmt/fc01/common/custom_lzss.h"
#include "fmt/fc01/common/util.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "MCG "_b;

static bstr decrypt(const bstr &input, size_t output_size, u8 initial_key)
{
    auto data = input;
    auto key = initial_key;
    for (auto i : util::range(data.size()))
    {
        data[i] = common::rol8(data[i], 1) ^ key;
        key += data.size() - 1 - i;
    }
    return common::custom_lzss_decompress(data, output_size);
}

struct McgImageDecoder::Priv final
{
    u8 key;
    bool key_set;
};

McgImageDecoder::McgImageDecoder() : p(new Priv())
{
}

McgImageDecoder::~McgImageDecoder()
{
}

void McgImageDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--mcg-key"})
        ->set_value_name("KEY")
        ->set_description("Decryption key (0..255, same for all files)");
    ImageDecoder::register_cli_options(arg_parser);
}

void McgImageDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("mcg-key"))
        set_key(boost::lexical_cast<int>(arg_parser.get_switch("mcg-key")));
    ImageDecoder::parse_cli_options(arg_parser);
}

bool McgImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

void McgImageDecoder::set_key(u8 key)
{
    p->key = key;
    p->key_set = true;
}

pix::Grid McgImageDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto version_float = boost::lexical_cast<float>(file.io.read(4).str());
    auto version = static_cast<int>(100 * version_float + 0.5);

    if (version != 101)
        throw err::UnsupportedVersionError(version);

    file.io.seek(16);
    auto header_size = file.io.read_u32_le();
    if (header_size != 64)
    {
        throw err::NotSupportedError(
            util::format("Unknown header size: %d", header_size));
    }
    auto x = file.io.read_u32_le();
    auto y = file.io.read_u32_le();
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto depth = file.io.read_u32_le();
    auto output_size = file.io.read_u32_le();

    file.io.seek(header_size);
    if (!p->key_set)
        throw err::UsageError("MCG decryption key not set");
    auto data = decrypt(file.io.read_to_eof(), output_size, p->key);
    data = common::fix_stride(data, width, height, depth);

    if (depth != 24)
        throw err::UnsupportedBitDepthError(depth);

    return pix::Grid(width, height, data, pix::Format::BGR888);
}

static auto dummy = fmt::Registry::add<McgImageDecoder>("fc01/mcg");
