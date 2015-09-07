// Cronus GRP image
//
// Company:   Cronus
// Engine:    -
// Extension: -
// Archives:  GRP.PAK
//
// Known games:
// - Doki Doki Princess
// - Sweet Pleasure

#include <boost/filesystem/path.hpp>
#include "fmt/cronus/common.h"
#include "fmt/cronus/grp_converter.h"
#include "util/image.h"
#include "util/pack/lzss.h"
#include "util/plugin_mgr.hh"
#include "util/range.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::cronus;

namespace
{
    enum EncType
    {
        Delta,
        SwapBytes,
    };

    struct Plugin
    {
        u32 key1;
        u32 key2;
        u32 key3;
        EncType encryption_type;
    };
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

struct GrpConverter::Priv
{
    util::PluginManager<Plugin> plugin_mgr;
    Plugin plugin;
};

GrpConverter::GrpConverter() : p(new Priv)
{
    p->plugin_mgr.add(
        "dokidoki", "Doki Doki Princess",
        {0xA53CC35A, 0x35421005, 0xCF42355D, EncType::SwapBytes});

    p->plugin_mgr.add(
        "sweet", "Sweet Pleasure",
        {0x2468FCDA, 0x4FC2CC4D, 0xCF42355D, EncType::Delta});
}

GrpConverter::~GrpConverter()
{
}

bool GrpConverter::is_recognized_internal(File &file) const
{
    for (auto plugin : p->plugin_mgr.get_all())
    {
        file.io.seek(0);
        auto width = file.io.read_u32_le() ^ plugin.key1;
        auto height = file.io.read_u32_le() ^ plugin.key2;
        auto bpp = file.io.read_u32_le();
        file.io.skip(4);
        auto output_size = file.io.read_u32_le() ^ plugin.key3;

        size_t expected_output_size = width * height;
        if (bpp == 8)
            expected_output_size += 1024;
        else if (bpp == 24)
            expected_output_size *= 3;
        else
            return false;

        if (output_size == expected_output_size)
        {
            p->plugin = plugin;
            return true;
        }
    }
    return false;
}

std::unique_ptr<File> GrpConverter::decode_internal(File &file) const
{
    auto width = file.io.read_u32_le() ^ p->plugin.key1;
    auto height = file.io.read_u32_le() ^ p->plugin.key2;
    auto bpp = file.io.read_u32_le();
    file.io.skip(4);
    auto output_size = file.io.read_u32_le() ^ p->plugin.key3;
    file.io.skip(4);

    auto data = file.io.read_to_eof();
    boost::filesystem::path path(file.name);
    if (p->plugin.encryption_type == EncType::Delta)
        delta_decrypt(data, get_delta_key(path.filename().string()));
    else
        swap_decrypt(data, output_size);
    data = util::pack::lzss_decompress_bytewise(data, output_size);

    if (bpp == 8)
    {
        pix::Palette palette(256, data, pix::Format::BGRA8888);
        for (auto i : util::range(palette.size()))
            palette[i].a = 0xFF;
        pix::Grid pixels(width, height, data.substr(1024), palette);
        pixels.flip();
        return util::Image::from_pixels(pixels)->create_file(file.name);
    }
    else if (bpp == 24)
    {
        pix::Grid pixels(width, height, data, pix::Format::BGR888);
        pixels.flip();
        return util::Image::from_pixels(pixels)->create_file(file.name);
    }
    else
        util::fail("Unsupported BPP");
}

static auto dummy = fmt::Registry::add<GrpConverter>("cronus/grp");
