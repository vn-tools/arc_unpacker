#include "fmt/majiro/rct_image_decoder.h"
#include <boost/algorithm/hex.hpp>
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = util::utf8_to_sjis("六丁T"_b);

static bstr decrypt(const bstr &input, const bstr &key)
{
    u32 crc_table[0x100];
    for (const auto i : util::range(0x100))
    {
        u32 poly = i;
        for (const auto j : util::range(8))
            poly = (poly >> 1) ^ ((poly & 1) ? 0xEDB88320 : 0);
        crc_table[i] = poly;
    }

    u32 checksum = 0xFFFFFFFF;
    for (const auto c : key)
        checksum = (checksum >> 8) ^ crc_table[(c ^ (checksum & 0xFF)) & 0xFF];
    checksum ^= 0xFFFFFFFF;

    bstr derived_key(0x400);
    for (const auto i : util::range(0x100))
        derived_key.get<u32>()[i] = checksum ^ crc_table[(i + checksum) & 0xFF];

    bstr output(input);
    for (const auto i : util::range(output.size()))
        output[i] ^= derived_key[i % derived_key.size()];
    return output;
}

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::MemoryStream input_stream(input);

    bstr output(width * height * 3);

    auto output_ptr = output.get<u8>();
    auto output_start = output.get<const u8>();
    auto output_end = output.end<const u8>();

    std::vector<int> shift_table;
    for (auto i : util::range(6))
        shift_table.push_back((-1 - i) * 3);
    for (auto i : util::range(7))
        shift_table.push_back((3 - i - width) * 3);
    for (auto i : util::range(7))
        shift_table.push_back((3 - i - width * 2) * 3);
    for (auto i : util::range(7))
        shift_table.push_back((3 - i - width * 3) * 3);
    for (auto i : util::range(5))
        shift_table.push_back((2 - i - width * 4) * 3);

    if (output.size() < 3)
        return output;
    *output_ptr++ = input_stream.read_u8();
    *output_ptr++ = input_stream.read_u8();
    *output_ptr++ = input_stream.read_u8();

    while (output_ptr < output_end)
    {
        auto flag = input_stream.read_u8();
        if (flag & 0x80)
        {
            auto size = flag & 3;
            auto look_behind = (flag >> 2) & 0x1F;
            size = size == 3
                ? (input_stream.read_u16_le() + 4) * 3
                : size * 3 + 3;
            auto source_ptr = &output_ptr[shift_table[look_behind]];
            if (source_ptr < output_start || source_ptr + size >= output_end)
                return output;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = *source_ptr++;
        }
        else
        {
            auto size = flag == 0x7F
                ? (input_stream.read_u16_le() + 0x80) * 3
                : flag * 3 + 3;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = input_stream.read_u8();
        }
    }

    return output;
}

struct RctImageDecoder::Priv final
{
    bstr key;
};

RctImageDecoder::RctImageDecoder() : p(new Priv())
{
}

RctImageDecoder::~RctImageDecoder()
{
}

void RctImageDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--rct-key"})
        ->set_value_name("KEY")
        ->set_description("Decryption key (same for all files)");
    ImageDecoder::register_cli_options(arg_parser);
}

void RctImageDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("rct-key"))
    {
        std::string key;
        boost::algorithm::unhex(
            arg_parser.get_switch("rct-key"), std::back_inserter(key));
        set_key(key);
    }
    ImageDecoder::parse_cli_options(arg_parser);
}

void RctImageDecoder::set_key(const bstr &key)
{
    p->key = key;
}

bool RctImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Image RctImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    bool encrypted;
    auto tmp = input_file.stream.read_u8();
    if (tmp == 'S')
        encrypted = true;
    else if (tmp == 'C')
        encrypted = false;
    else
        throw err::NotSupportedError("Unexpected encryption flag");

    int version = boost::lexical_cast<int>(input_file.stream.read(2).str());
    if (version < 0 || version > 1)
        throw err::UnsupportedVersionError(version);

    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    auto data_size = input_file.stream.read_u32_le();

    std::string base_file;
    if (version == 1)
    {
        const auto name_size = input_file.stream.read_u16_le();
        base_file = input_file.stream.read(name_size).str();
    }

    auto data = input_file.stream.read(data_size);
    if (encrypted)
    {
        if (p->key.empty())
        {
            throw err::UsageError(
                "File is encrypted, but key not set. "
                "Please supply one with --rct-key switch.");
        }
        data = decrypt(data, p->key);
    }
    data = uncompress(data, width, height);
    pix::Image image(width, height, data, pix::Format::BGR888);

    if (version == 1)
    {
        for (auto y : util::range(height))
        for (auto x : util::range(width))
        {
            auto &p = image.at(x, y);
            if (p.r == 0xFF && p.g == 0 && p.g == 0)
                p.a = p.r = 0;
        }
    }

    return image;
}

static auto dummy = fmt::register_fmt<RctImageDecoder>("majiro/rct");
