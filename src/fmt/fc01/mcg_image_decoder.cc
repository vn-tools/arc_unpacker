#include "fmt/fc01/mcg_image_decoder.h"
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "fmt/fc01/common/custom_lzss.h"
#include "fmt/fc01/common/util.h"
#include "fmt/fc01/common/mrg_decryptor.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "MCG "_b;

static bstr decrypt_v101(const bstr &input, size_t output_size, u8 initial_key)
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

static bstr transform_v200(
    bstr planes[3], const size_t width, const size_t height)
{
    for (const auto y : util::range(height - 1))
    for (const auto x : util::range(width - 1))
    for (const auto i : util::range(3))
    {
        const auto pos = y * width + x;
        int p00 = planes[i][pos];
        int p10 = planes[i][pos + width] - p00;
        int p01 = planes[i][pos + 1] - p00;
        p00 = std::abs(p01 + p10);
        p01 = std::abs(p01);
        p10 = std::abs(p10);
        s8 p11a;
        if (p00 >= p01 && p10 >= p01)
            p11a = planes[i][pos + width];
        else if (p00 < p10)
            p11a = planes[i][pos];
        else
            p11a = planes[i][pos + 1];
        planes[i][pos + width + 1] += p11a + 0x80;
    }

    bstr output(width * height * 3);
    auto output_ptr = output.get<u8>();
    for (const auto y : util::range(height))
    for (const auto x : util::range(width))
    {
        const auto pos = y * width + x;
        s8 b = -128 + static_cast<s8>(planes[2][pos]);
        s8 r = -128 + static_cast<s8>(planes[1][pos]);
        s8 g = planes[0][pos] - ((b + r) >> 2);
        *output_ptr++ = r + g;
        *output_ptr++ = g;
        *output_ptr++ = b + g;
    }
    return output;
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

bool McgImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

void McgImageDecoder::set_key(u8 key)
{
    p->key = key;
    p->key_set = true;
}

pix::Grid McgImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());
    const auto versionf = boost::lexical_cast<float>(file.stream.read(4).str());
    const auto version = static_cast<int>(100 * versionf + 0.5);

    file.stream.seek(16);
    const auto header_size = file.stream.read_u32_le();
    if (header_size != 64)
    {
        throw err::NotSupportedError(
            util::format("Unknown header size: %d", header_size));
    }
    const auto x = file.stream.read_u32_le();
    const auto y = file.stream.read_u32_le();
    const auto width = file.stream.read_u32_le();
    const auto height = file.stream.read_u32_le();
    const auto depth = file.stream.read_u32_le();
    const auto size_orig = file.stream.read_u32_le();

    if (!p->key_set)
        throw err::UsageError("MCG decryption key not set");

    file.stream.seek(header_size);
    bstr data = file.stream.read_to_eof();
    if (version == 101)
        data = decrypt_v101(data, size_orig, p->key);
    else if (version == 200)
    {
        common::MrgDecryptor decryptor(data, width * height);
        bstr planes[3];
        for (const auto i : util::range(3))
            planes[i] = decryptor.decrypt_with_key(p->key);
        data = transform_v200(planes, width, height);
    }
    else
        throw err::UnsupportedVersionError(version);
    data = common::fix_stride(data, width, height, depth);

    if (depth != 24)
        throw err::UnsupportedBitDepthError(depth);

    return pix::Grid(width, height, data, pix::Format::BGR888);
}

static auto dummy = fmt::register_fmt<McgImageDecoder>("fc01/mcg");
