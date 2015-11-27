#include "fmt/will/wipf_image_archive_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/file_from_image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::will;

static const bstr magic = "WIPF"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t width;
        size_t height;
        size_t size_comp;
        size_t size_orig;
        size_t depth;
    };
}

// Modified LZSS routine
// - repetition count and look behind pos differs
// - non-standard initial dictionary pos
// - non-standard minimal match size

static bstr custom_lzss_decompress(io::Stream &input, size_t output_size)
{
    const size_t dict_size = 0x1000;
    size_t dict_pos = 1;
    u8 dict[dict_size] {};
    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    u16 control = 0;
    while (output_ptr < output_end && !input.eof())
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input.read_u8() | 0xFF00;
        if (control & 1)
        {
            if (input.eof())
                break;
            auto byte = input.read_u8();
            dict[dict_pos++] = *output_ptr++ = byte;
            dict_pos %= dict_size;
            continue;
        }
        if (input.eof())
            break;
        u8 di = input.read_u8();
        if (input.eof())
            break;
        u8 tmp = input.read_u8();
        u32 look_behind_pos = (((di << 8) | tmp) >> 4);
        u16 repetitions = (tmp & 0xF) + 2;
        while (repetitions-- && output_ptr < output_end)
        {
            dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
            look_behind_pos %= dict_size;
            dict_pos %= dict_size;
        }
    }
    return output;
}

static bstr custom_lzss_decompress(const bstr &input, size_t output_size)
{
    io::MemoryStream stream(input);
    return custom_lzss_decompress(stream, output_size);
}

fmt::IDecoder::NamingStrategy WipfImageArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

bool WipfImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    WipfImageArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto meta = std::make_unique<ArchiveMeta>();
    auto file_count = input_file.stream.read_u16_le();
    auto depth = input_file.stream.read_u16_le();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->width = input_file.stream.read_u32_le();
        entry->height = input_file.stream.read_u32_le();
        input_file.stream.skip(12);
        entry->size_comp = input_file.stream.read_u32_le();
        entry->size_orig = entry->width * entry->height * (depth >> 3);
        entry->depth = depth;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<pix::Image> WipfImageArchiveDecoder::read_image(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    std::unique_ptr<pix::Palette> palette;
    if (entry->depth == 8)
    {
        palette.reset(new pix::Palette(
            256, input_file.stream, pix::PixelFormat::BGRA8888));
        for (auto &c : *palette)
            c.a ^= 0xFF;
    }

    auto data = input_file.stream.read(entry->size_comp);
    data = custom_lzss_decompress(data, entry->size_orig);

    auto w = entry->width;
    auto h = entry->height;

    std::unique_ptr<pix::Image> image;
    if (entry->depth == 8)
    {
        image = std::make_unique<pix::Image>(w, h, data, *palette);
    }
    else if (entry->depth == 24)
    {
        image = std::make_unique<pix::Image>(w, h);
        for (auto y : util::range(h))
        for (auto x : util::range(w))
        {
            image->at(x, y).b = data[w * h * 0 + y * w + x];
            image->at(x, y).g = data[w * h * 1 + y * w + x];
            image->at(x, y).r = data[w * h * 2 + y * w + x];
            image->at(x, y).a = 0xFF;
        }
    }
    else
        throw err::UnsupportedBitDepthError(entry->depth);

    return image;
}

std::unique_ptr<io::File> WipfImageArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    return util::file_from_image(*read_image(input_file, m, e), e.name);
}

std::vector<std::shared_ptr<pix::Image>>
    WipfImageArchiveDecoder::unpack_to_images(io::File &input_file) const
{
    auto meta = read_meta(input_file);
    std::vector<std::shared_ptr<pix::Image>> output;
    for (auto &entry : meta->entries)
        output.push_back(read_image(input_file, *meta, *entry));
    return output;
}

static auto dummy = fmt::register_fmt<WipfImageArchiveDecoder>("will/wipf");
