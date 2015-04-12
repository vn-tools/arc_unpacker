// DDS image (DirectDraw surface)
//
// Company:   Microsoft
// Engine:    DirectX
// Extension: .dds
// Archives:  -
//
// Known games:
// - Touhou 13.5 - Hopeless Masquerade

#include "formats/microsoft/dds_converter.h"
#include "io/buffered_io.h"
#include "util/colors.h"
#include "util/endian.h"
#include "util/image.h"
using namespace Formats::Microsoft;

namespace
{
    const std::string magic("DDS\x20", 4);
    const std::string magic_dxt1("DXT1", 4);
    const std::string magic_dxt2("DXT2", 4);
    const std::string magic_dxt3("DXT3", 4);
    const std::string magic_dxt4("DXT4", 4);
    const std::string magic_dxt5("DXT5", 4);
    const std::string magic_dx10("DX10", 4);

    enum class D3d10ResourceDimension : uint32_t
    {
        Unknown    = 0,
        Buffer     = 1,
        Texture1D  = 2,
        Texture2D  = 3,
        Texture3D  = 4,
    };

    typedef enum
    {
        DDPF_ALPHAPIXELS = 0x1,
        DDPF_ALPHA = 0x2,
        DDPF_FOURCC = 0x4,
        DDPF_RGB = 0x40,
        DDPF_YUV = 0x200,
        DDPF_LUMINACE = 0x20000,
    } DdsPixelFormatFlags;

    typedef struct
    {
        uint32_t size;
        DdsPixelFormatFlags flags;
        std::string four_cc;
        uint32_t rgb_bit_count;
        uint32_t r_bit_mask;
        uint32_t g_bit_mask;
        uint32_t b_bit_mask;
        uint32_t a_bit_mask;
    } DdsPixelFormat;

    typedef struct
    {
        uint32_t dxgi_format;
        D3d10ResourceDimension resource_dimension;
        uint32_t misc_flag;
        uint32_t array_size;
        uint32_t misc_flags2;
    } DdsHeaderDx10;

    typedef enum
    {
        DDSD_CAPS        = 0x1,
        DDSD_HEIGHT      = 0x2,
        DDSD_WIDTH       = 0x4,
        DDSD_PITCH       = 0x8,
        DDSD_PIXELFORMAT = 0x1000,
        DDSD_MIPMAPCOUNT = 0x20000,
        DDSD_LINEARSIZE  = 0x80000,
        DDSD_DEPTH       = 0x800000,
    } DdsHeaderFlags;

    typedef struct
    {
        uint32_t size;
        DdsHeaderFlags flags;
        uint32_t height;
        uint32_t width;
        uint32_t pitch_or_linear_size;
        uint32_t depth;
        uint32_t mip_map_count;
        DdsPixelFormat pixel_format;
        uint32_t caps[4];
    } DdsHeader;

    void fill_pixel_format(IO &io, DdsPixelFormat &pixel_format)
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

    std::unique_ptr<DdsHeader> read_header(IO &io)
    {
        std::unique_ptr<DdsHeader> header(new DdsHeader);
        header->size = io.read_u32_le();
        header->flags = static_cast<DdsHeaderFlags>(io.read_u32_le());
        header->height = io.read_u32_le();
        header->width = io.read_u32_le();
        header->pitch_or_linear_size = io.read_u32_le();
        header->depth = io.read_u32_le();
        header->mip_map_count = io.read_u32_le();
        io.skip(4 * 11);
        fill_pixel_format(io, header->pixel_format);
        for (size_t i = 0; i < 4; i ++)
            header->caps[i] = io.read_u32_le();
        io.skip(4);
        return header;
    }

    std::unique_ptr<DdsHeaderDx10> read_header_dx10(IO &io)
    {
        std::unique_ptr<DdsHeaderDx10> header(new DdsHeaderDx10);
        header->dxgi_format = io.read_u32_le();
        header->resource_dimension = static_cast<D3d10ResourceDimension>(
            io.read_u32_le());
        header->misc_flag = io.read_u32_le();
        header->array_size = io.read_u32_le();
        header->misc_flags2 = io.read_u32_le();
        return header;
    }

    inline int max(int a, int b ) { return a > b ? a : b; }
}

namespace
{
    std::unique_ptr<BufferedIO> create_io(size_t width, size_t height)
    {
        std::unique_ptr<BufferedIO> output_io(new BufferedIO);
        output_io->reserve(((width + 3) / 4) * ((height + 3) / 4) * 64);
        return output_io;
    }

    void decode_dxt1_block(IO &io, uint32_t output_colors[4][4])
    {
        uint32_t colors[4];
        colors[0] = io.read_u16_le();
        colors[1] = io.read_u16_le();
        bool transparent = colors[0] < colors[1];
        colors[0] = rgb565(colors[0]);
        colors[1] = rgb565(colors[1]);

        uint8_t rgba[4][4];
        for (size_t i = 0; i < 2; i ++)
            split_channels(colors[i], rgba[i]);

        for (size_t i = 0; i < 4; i ++)
        {
            if (!transparent)
            {
                rgba[2][i] = ((rgba[0][i] << 1) + rgba[1][i]) / 3;
                rgba[3][i] = ((rgba[1][i] << 1) + rgba[0][i]) / 3;
            }
            else
            {
                rgba[2][i] = (rgba[0][i] + rgba[1][i]) >> 1;
                rgba[3][i] = 0;
            }
        }

        for (size_t i = 2; i < 4; i ++)
            merge_channels(rgba[i], colors[i]);

        uint32_t lookup = io.read_u32_le();
        for (size_t y = 0; y < 4; y ++)
        {
            for (size_t x = 0; x < 4; x ++)
            {
                size_t index = lookup & 3;
                output_colors[y][x] = colors[index];
                lookup >>= 2;
            }
        }
    }

    void decode_dxt5_block(IO &io, uint8_t output_alpha[4][4])
    {
        uint8_t alpha[8];
        alpha[0] = io.read_u8();
        alpha[1] = io.read_u8();

        if (alpha[0] > alpha[1])
        {
            for (size_t i = 2; i < 8; i ++)
                alpha[i] = ((8. - i) * alpha[0] + ((i - 1.) * alpha[1])) / 7.;
        }
        else
        {
            for (size_t i = 2; i < 6; i ++)
                alpha[i] = ((6. - i) * alpha[0] + ((i - 1.) * alpha[1])) / 5.;
            alpha[6] = 0;
            alpha[7] = 255;
        }

        for (size_t i = 0; i < 2; i ++)
        {
            uint32_t lookup = be32toh(
                (io.read_u16_be() << 16) | (io.read_u8() << 8));
            for (size_t j = 0; j < 8; j ++)
            {
                uint8_t index = lookup & 7;
                size_t pos = i * 8 + j;
                size_t x = pos % 4;
                size_t y = pos / 4;
                lookup >>= 3;
                output_alpha[y][x] = alpha[index];
            }
        }
    }

    std::unique_ptr<BufferedIO> decode_dxt1(IO &io, size_t width, size_t height)
    {
        auto output_io = create_io(width, height);
        for (size_t block_y = 0; block_y < height; block_y += 4)
        for (size_t block_x = 0; block_x < width; block_x += 4)
        {
            uint32_t colors[4][4];
            decode_dxt1_block(io, colors);
            for (size_t y = 0; y < 4; y ++)
            {
                output_io->seek(((block_y + y) * width + block_x) * 4);
                for (size_t x = 0; x < 4; x ++)
                    output_io->write_u32_le(colors[y][x]);
            }
        }
        return output_io;
    }

    std::unique_ptr<BufferedIO> decode_dxt3(IO &io, size_t width, size_t height)
    {
        auto output_io = create_io(width, height);
        for (size_t block_y = 0; block_y < height; block_y += 4)
        for (size_t block_x = 0; block_x < width; block_x += 4)
        {
            uint8_t alpha[4][4];
            for (size_t y = 0; y < 4; y ++)
            {
                for (size_t x = 0; x < 4; x += 2)
                {
                    uint8_t b = io.read_u8();
                    alpha[y][x + 0] = b & 0xf0;
                    alpha[y][x + 1] = (b & 0x0f) << 4;
                }
            }

            uint32_t colors[4][4];
            decode_dxt1_block(io, colors);
            for (size_t y = 0; y < 4; y ++)
            {
                output_io->seek(((block_y + y) * width + block_x) * 4);
                for (size_t x = 0; x < 4; x ++)
                {
                    set_channel(colors[y][x], 3, alpha[y][x]);
                    output_io->write_u32_le(colors[y][x]);
                }
            }
        }
        return output_io;
    }

    std::unique_ptr<BufferedIO> decode_dxt5(IO &io, size_t width, size_t height)
    {
        auto output_io = create_io(width, height);
        for (size_t block_y = 0; block_y < height; block_y += 4)
        for (size_t block_x = 0; block_x < width; block_x += 4)
        {
            uint8_t alpha[4][4];
            decode_dxt5_block(io, alpha);

            uint32_t colors[4][4];
            decode_dxt1_block(io, colors);
            for (size_t y = 0; y < 4; y ++)
            {
                output_io->seek(((block_y + y) * width + block_x) * 4);
                for (size_t x = 0; x < 4; x ++)
                {
                    set_channel(colors[y][x], 3, alpha[y][x]);
                    output_io->write_u32_le(colors[y][x]);
                }
            }
        }
        return output_io;
    }
}

bool DdsConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> DdsConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    auto header = read_header(file.io);
    if (header->pixel_format.four_cc == magic_dx10)
        read_header_dx10(file.io);

    std::unique_ptr<IO> pixels_io(nullptr);
    if (header->pixel_format.flags & DDPF_FOURCC)
    {
        if (header->pixel_format.four_cc == magic_dxt1)
            pixels_io = decode_dxt1(file.io, header->width, header->height);
        else if (header->pixel_format.four_cc == magic_dxt3)
            pixels_io = decode_dxt3(file.io, header->width, header->height);
        else if (header->pixel_format.four_cc == magic_dxt5)
            pixels_io = decode_dxt5(file.io, header->width, header->height);
        else
        {
            throw std::runtime_error(
                header->pixel_format.four_cc + " textures are not supported");
        }
    }
    else if (header->pixel_format.flags & DDPF_RGB)
    {
        if (header->pixel_format.rgb_bit_count == 32)
        {
            pixels_io = std::unique_ptr<IO>(new BufferedIO(file.io.read(
                header->width * header->height * 4)));
        }
    }

    if (pixels_io == nullptr)
        throw std::runtime_error("Not supported");

    pixels_io->seek(0);
    std::unique_ptr<Image> image = Image::from_pixels(
        header->width,
        header->height,
        pixels_io->read_until_end(),
        PixelFormat::BGRA);
    return image->create_file(file.name);
}
