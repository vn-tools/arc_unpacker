#include "fmt/microsoft/dds_image_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/endian.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::microsoft;

namespace
{
    enum class D3d10ResourceDimension : u32
    {
        Unknown    = 0,
        Buffer     = 1,
        Texture1D  = 2,
        Texture2D  = 3,
        Texture3D  = 4,
    };

    enum DdsPixelFormatFlags
    {
        DDPF_ALPHAPIXELS = 0x1,
        DDPF_ALPHA = 0x2,
        DDPF_FOURCC = 0x4,
        DDPF_RGB = 0x40,
        DDPF_YUV = 0x200,
        DDPF_LUMINACE = 0x20000,
    };

    struct DdsPixelFormat final
    {
        u32 size;
        DdsPixelFormatFlags flags;
        bstr four_cc;
        u32 rgb_bit_count;
        u32 r_bit_mask;
        u32 g_bit_mask;
        u32 b_bit_mask;
        u32 a_bit_mask;
    };

    struct DdsHeaderDx10 final
    {
        u32 dxgi_format;
        D3d10ResourceDimension resource_dimension;
        u32 misc_flag;
        u32 array_size;
        u32 misc_flags2;
    };

    enum DdsHeaderFlags
    {
        DDSD_CAPS        = 0x1,
        DDSD_HEIGHT      = 0x2,
        DDSD_WIDTH       = 0x4,
        DDSD_PITCH       = 0x8,
        DDSD_PIXELFORMAT = 0x1000,
        DDSD_MIPMAPCOUNT = 0x20000,
        DDSD_LINEARSIZE  = 0x80000,
        DDSD_DEPTH       = 0x800000,
    };

    struct DdsHeader final
    {
        u32 size;
        DdsHeaderFlags flags;
        u32 height;
        u32 width;
        u32 pitch_or_linear_size;
        u32 depth;
        u32 mip_map_count;
        DdsPixelFormat pixel_format;
        u32 caps[4];
    };
}

static const bstr magic = "DDS\x20"_b;
static const bstr magic_dxt1 = "DXT1"_b;
static const bstr magic_dxt2 = "DXT2"_b;
static const bstr magic_dxt3 = "DXT3"_b;
static const bstr magic_dxt4 = "DXT4"_b;
static const bstr magic_dxt5 = "DXT5"_b;
static const bstr magic_dx10 = "DX10"_b;

static void fill_pixel_format(io::IO &io, DdsPixelFormat &pixel_format)
{
    pixel_format.size = io.read_u32_le();
    pixel_format.flags = static_cast<DdsPixelFormatFlags>(io.read_u32_le());
    pixel_format.four_cc = io.read(4);
    pixel_format.rgb_bit_count = io.read_u32_le();
    pixel_format.r_bit_mask = io.read_u32_le();
    pixel_format.g_bit_mask = io.read_u32_le();
    pixel_format.b_bit_mask = io.read_u32_le();
    pixel_format.a_bit_mask = io.read_u32_le();
}

static std::unique_ptr<DdsHeader> read_header(io::IO &io)
{
    auto header = std::make_unique<DdsHeader>();
    header->size = io.read_u32_le();
    header->flags = static_cast<DdsHeaderFlags>(io.read_u32_le());
    header->height = io.read_u32_le();
    header->width = io.read_u32_le();
    header->pitch_or_linear_size = io.read_u32_le();
    header->depth = io.read_u32_le();
    header->mip_map_count = io.read_u32_le();
    io.skip(4 * 11);
    fill_pixel_format(io, header->pixel_format);
    for (auto i : util::range(4))
        header->caps[i] = io.read_u32_le();
    io.skip(4);
    return header;
}

static std::unique_ptr<DdsHeaderDx10> read_header_dx10(io::IO &io)
{
    auto header = std::make_unique<DdsHeaderDx10>();
    header->dxgi_format = io.read_u32_le();
    header->resource_dimension = static_cast<D3d10ResourceDimension>(
        io.read_u32_le());
    header->misc_flag = io.read_u32_le();
    header->array_size = io.read_u32_le();
    header->misc_flags2 = io.read_u32_le();
    return header;
}

static std::unique_ptr<pix::Grid> create_pixels(
    size_t width, size_t height)
{
    return std::make_unique<pix::Grid>(
        ((width + 3) / 4) * 4, ((height + 3) / 4) * 4);
}

static void decode_dxt1_block(io::IO &io, pix::Pixel output_colors[4][4])
{
    pix::Pixel colors[4];
    bstr tmp = io.read(4);
    const u8 *tmp_ptr = tmp.get<u8>();
    colors[0] = pix::read<pix::Format::BGR565>(tmp_ptr);
    colors[1] = pix::read<pix::Format::BGR565>(tmp_ptr);
    bool transparent
        = colors[0].b <= colors[1].b
        && colors[0].g <= colors[1].g
        && colors[0].r <= colors[1].r
        && colors[0].a <= colors[1].a;

    for (auto i : util::range(4))
    {
        if (!transparent)
        {
            colors[2][i] = ((colors[0][i] << 1) + colors[1][i]) / 3;
            colors[3][i] = ((colors[1][i] << 1) + colors[0][i]) / 3;
        }
        else
        {
            colors[2][i] = (colors[0][i] + colors[1][i]) >> 1;
            colors[3][i] = 0;
        }
    }

    auto lookup = io.read_u32_le();
    for (auto y : util::range(4))
    for (auto x : util::range(4))
    {
        size_t index = lookup & 3;
        output_colors[y][x] = colors[index];
        lookup >>= 2;
    }
}

static void decode_dxt5_block(io::IO &io, u8 output_alpha[4][4])
{
    u8 alpha[8];
    alpha[0] = io.read_u8();
    alpha[1] = io.read_u8();

    if (alpha[0] > alpha[1])
    {
        for (auto i : util::range(2, 8))
            alpha[i] = ((8. - i) * alpha[0] + ((i - 1.) * alpha[1])) / 7.;
    }
    else
    {
        for (auto i : util::range(2, 6))
            alpha[i] = ((6. - i) * alpha[0] + ((i - 1.) * alpha[1])) / 5.;
        alpha[6] = 0;
        alpha[7] = 255;
    }

    for (auto i : util::range(2))
    {
        u32 lookup = util::from_big_endian<u32>(
            (io.read_u16_be() << 16) | (io.read_u8() << 8));
        for (auto j : util::range(8))
        {
            u8 index = lookup & 7;
            size_t pos = i * 8 + j;
            size_t x = pos % 4;
            size_t y = pos / 4;
            lookup >>= 3;
            output_alpha[y][x] = alpha[index];
        }
    }
}

static std::unique_ptr<pix::Grid> decode_dxt1(
    io::IO &io, size_t width, size_t height)
{
    auto pixels = create_pixels(width, height);
    for (auto block_y : util::range(0, height, 4))
    for (auto block_x : util::range(0, width, 4))
    {
        pix::Pixel colors[4][4];
        decode_dxt1_block(io, colors);
        for (auto y : util::range(4))
        for (auto x : util::range(4))
            pixels->at(block_x + x, block_y + y) = colors[y][x];
    }
    return pixels;
}

static std::unique_ptr<pix::Grid> decode_dxt3(
    io::IO &io, size_t width, size_t height)
{
    auto pixels = create_pixels(width, height);
    for (auto block_y : util::range(0, height, 4))
    for (auto block_x : util::range(0, width, 4))
    {
        u8 alpha[4][4];
        for (auto y : util::range(4))
        {
            for (auto x : util::range(0, 4, 2))
            {
                u8 b = io.read_u8();
                alpha[y][x + 0] = b & 0xF0;
                alpha[y][x + 1] = (b & 0x0F) << 4;
            }
        }

        pix::Pixel colors[4][4];
        decode_dxt1_block(io, colors);
        for (auto y : util::range(4))
        for (auto x : util::range(4))
        {
            colors[y][x].a = alpha[y][x];
            pixels->at(block_x + x, block_y + y) = colors[y][x];
        }
    }
    return pixels;
}

static std::unique_ptr<pix::Grid> decode_dxt5(
    io::IO &io, size_t width, size_t height)
{
    auto pixels = create_pixels(width, height);
    for (auto block_y : util::range(0, height, 4))
    for (auto block_x : util::range(0, width, 4))
    {
        u8 alpha[4][4];
        decode_dxt5_block(io, alpha);

        pix::Pixel colors[4][4];
        decode_dxt1_block(io, colors);
        for (auto y : util::range(4))
        for (auto x : util::range(4))
        {
            colors[y][x].a = alpha[y][x];
            pixels->at(block_x + x, block_y + y) = colors[y][x];
        }
    }
    return pixels;
}

bool DdsImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid DdsImageDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());

    auto header = read_header(file.io);
    if (header->pixel_format.four_cc == magic_dx10)
        read_header_dx10(file.io);

    auto width = header->width;
    auto height = header->height;

    std::unique_ptr<pix::Grid> pixels(nullptr);
    if (header->pixel_format.flags & DDPF_FOURCC)
    {
        if (header->pixel_format.four_cc == magic_dxt1)
            pixels = decode_dxt1(file.io, width, height);
        else if (header->pixel_format.four_cc == magic_dxt3)
            pixels = decode_dxt3(file.io, width, height);
        else if (header->pixel_format.four_cc == magic_dxt5)
            pixels = decode_dxt5(file.io, width, height);
        else
        {
            throw err::NotSupportedError(util::format(
                "%s textures are not supported",
                header->pixel_format.four_cc.get<char>()));
        }
    }
    else if (header->pixel_format.flags & DDPF_RGB)
    {
        if (header->pixel_format.rgb_bit_count == 32)
        {
            pixels.reset(new pix::Grid(
                width, height, file.io, pix::Format::BGRA8888));
        }
    }

    if (pixels == nullptr)
        throw err::NotSupportedError("Unsupported pixel format");

    return *pixels;
}

static auto dummy = fmt::register_fmt<DdsImageDecoder>("microsoft/dds");
