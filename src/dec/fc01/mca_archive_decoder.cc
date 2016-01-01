#include "dec/fc01/mca_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/fc01/common/custom_lzss.h"
#include "dec/fc01/common/util.h"
#include "err.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::dec::fc01;

static const bstr magic = "MCA "_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
    };
}

static bstr decrypt(const bstr &input, size_t output_size, u8 initial_key)
{
    bstr output(input.size());
    auto key = initial_key;
    for (auto i : algo::range(input.size()))
    {
        output[i] = common::rol8(input[i], 1) ^ key;
        key += input.size() - i;
    }
    return output;
}

struct McaArchiveDecoder::Priv final
{
    u8 key;
    bool key_set;
};

McaArchiveDecoder::McaArchiveDecoder() : p(new Priv())
{
}

McaArchiveDecoder::~McaArchiveDecoder()
{
}

void McaArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--mca-key"})
        ->set_value_name("KEY")
        ->set_description("Decryption key (0..255, same for all files)");
}

void McaArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("mca-key"))
        set_key(algo::from_string<int>(arg_parser.get_switch("mca-key")));
}

void McaArchiveDecoder::set_key(u8 key)
{
    p->key = key;
    p->key_set = true;
}

bool McaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> McaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(16);
    auto header_size = input_file.stream.read_u32_le();
    input_file.stream.skip(12);
    auto file_count = input_file.stream.read_u32_le();
    input_file.stream.seek(header_size);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = algo::format("%03d.png", i);
        entry->offset = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> McaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto encryption_type = input_file.stream.read_u32_le();
    input_file.stream.skip(8);
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    auto size_comp = input_file.stream.read_u32_le();
    auto size_orig = input_file.stream.read_u32_le();
    input_file.stream.skip(4);

    auto data = input_file.stream.read(size_comp);

    if (!p->key_set)
        throw err::UsageError("MCA decryption key not set");
    if (encryption_type == 0)
    {
        data = decrypt(data, size_orig, p->key);
    }
    else if (encryption_type == 1)
    {
        data = decrypt(data, size_orig, p->key);
        data = common::custom_lzss_decompress(data, size_orig);
    }
    else
    {
        throw err::NotSupportedError(algo::format(
            "Unknown encryption type: %d", encryption_type));
    }

    data = common::fix_stride(data, width, height, 24);
    res::Image image(width, height, data, res::PixelFormat::BGR888);
    return util::file_from_image(image, entry->path);
}

static auto _ = dec::register_decoder<McaArchiveDecoder>("fc01/mca");
