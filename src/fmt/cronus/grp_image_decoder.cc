#include "fmt/cronus/grp_image_decoder.h"
#include <boost/filesystem/path.hpp>
#include "err.h"
#include "fmt/cronus/common.h"
#include "util/pack/lzss.h"
#include "util/plugin_mgr.hh"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cronus;

namespace
{
    enum EncType
    {
        Delta,
        SwapBytes,
        None,
    };

    struct Header final
    {
        size_t width;
        size_t height;
        size_t bpp;
        size_t input_offset;
        size_t output_size;
        bool flip;
        bool use_transparency;
        EncType encryption_type;
    };

    using HeaderFunc = std::function<Header(io::IO&)>;
}

static void swap_decrypt(bstr &input, size_t encrypted_size)
{
    u16 *input_ptr = input.get<u16>();
    const u16 *input_end = input.end<const u16>();
    size_t repetitions = encrypted_size >> 1;
    while (repetitions-- && input_ptr < input_end)
    {
        *input_ptr = ((*input_ptr << 8) | (*input_ptr >> 8)) ^ 0x33CC;
        input_ptr++;
    }
}

struct GrpImageDecoder::Priv final
{
    util::PluginManager<HeaderFunc> plugin_mgr;
    Header header;
};

static bool validate_header(const Header &header)
{
    size_t expected_output_size = header.width * header.height;
    if (header.bpp == 8)
        expected_output_size += 1024;
    else if (header.bpp == 24)
        expected_output_size *= 3;
    else if (header.bpp == 32)
        expected_output_size *= 4;
    else
        return false;
    return header.output_size == expected_output_size;
}

static HeaderFunc reader_v1(u32 key1, u32 key2, u32 key3, EncType enc_type)
{
    return [=](io::IO &file_io)
    {
        Header header;
        header.width = file_io.read_u32_le() ^ key1;
        header.height = file_io.read_u32_le() ^ key2;
        header.bpp = file_io.read_u32_le();
        file_io.skip(4);
        header.output_size = file_io.read_u32_le() ^ key3;
        file_io.skip(4);
        header.use_transparency = false;
        header.flip = true;
        header.encryption_type = enc_type;
        return header;
    };
}

static HeaderFunc reader_v2()
{
    return [](io::IO &file_io)
    {
        Header header;
        file_io.skip(4);
        header.output_size = file_io.read_u32_le();
        file_io.skip(8);
        header.width = file_io.read_u32_le();
        header.height = file_io.read_u32_le();
        header.bpp = file_io.read_u32_le();
        file_io.skip(8);
        header.flip = false;
        header.use_transparency = file_io.read_u32_le();
        header.encryption_type = EncType::None;
        return header;
    };
}

GrpImageDecoder::GrpImageDecoder() : p(new Priv)
{
    p->plugin_mgr.add(
        "dokidoki", "Doki Doki Princess",
        reader_v1(0xA53CC35A, 0x35421005, 0xCF42355D, EncType::SwapBytes));

    p->plugin_mgr.add(
        "sweet", "Sweet Pleasure",
        reader_v1(0x2468FCDA, 0x4FC2CC4D, 0xCF42355D, EncType::Delta));

    p->plugin_mgr.add("nursery", "Nursery Song", reader_v2());
}

GrpImageDecoder::~GrpImageDecoder()
{
}

bool GrpImageDecoder::is_recognized_impl(File &file) const
{
    for (auto header_func : p->plugin_mgr.get_all())
    {
        file.io.seek(0);
        try
        {
            p->header = header_func(file.io);
            if (!validate_header(p->header))
                continue;
            p->header.input_offset = file.io.tell();
            return true;
        }
        catch (...)
        {
            continue;
        }
    }
    return false;
}

pix::Grid GrpImageDecoder::decode_impl(File &file) const
{
    file.io.seek(p->header.input_offset);
    auto data = file.io.read_to_eof();

    boost::filesystem::path path(file.name);
    if (p->header.encryption_type == EncType::Delta)
        delta_decrypt(data, get_delta_key(path.filename().string()));
    else if (p->header.encryption_type == EncType::SwapBytes)
        swap_decrypt(data, p->header.output_size);
    data = util::pack::lzss_decompress_bytewise(data, p->header.output_size);

    std::unique_ptr<pix::Grid> grid;

    if (p->header.bpp == 8)
    {
        pix::Palette palette(256, data, pix::Format::BGRA8888);
        grid.reset(new pix::Grid(
            p->header.width, p->header.height, data.substr(1024), palette));
    }
    else if (p->header.bpp == 24)
    {
        grid.reset(new pix::Grid(
            p->header.width, p->header.height, data, pix::Format::BGR888));
    }
    else if (p->header.bpp == 32)
    {
        grid.reset(new pix::Grid(
            p->header.width, p->header.height, data, pix::Format::BGRA8888));
    }
    else
        throw err::UnsupportedBitDepthError(p->header.bpp);

    if (!p->header.use_transparency)
    {
        for (auto x : util::range(p->header.width))
        for (auto y : util::range(p->header.height))
            grid->at(x, y).a = 0xFF;
    }
    if (p->header.flip)
        grid->flip();

    return *grid;
}

static auto dummy = fmt::Registry::add<GrpImageDecoder>("cronus/grp");
