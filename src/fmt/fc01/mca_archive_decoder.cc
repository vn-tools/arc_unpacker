#include "fmt/fc01/mca_archive_decoder.h"
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "fmt/fc01/common/custom_lzss.h"
#include "fmt/fc01/common/util.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "MCA "_b;

static bstr decrypt(const bstr &input, size_t output_size, u8 initial_key)
{
    bstr output(input.size());
    auto key = initial_key;
    for (auto i : util::range(input.size()))
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
    ArchiveDecoder::register_cli_options(arg_parser);
}

void McaArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("mca-key"))
        set_key(boost::lexical_cast<int>(arg_parser.get_switch("mca-key")));
    ArchiveDecoder::parse_cli_options(arg_parser);
}

void McaArchiveDecoder::set_key(u8 key)
{
    p->key = key;
    p->key_set = true;
}

bool McaArchiveDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

void McaArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.seek(16);
    auto header_size = arc_file.io.read_u32_le();
    arc_file.io.skip(12);
    auto file_count = arc_file.io.read_u32_le();
    arc_file.io.seek(header_size);

    std::vector<size_t> offsets(file_count);
    for (auto i : util::range(offsets.size()))
        offsets[i] = arc_file.io.read_u32_le();

    if (!p->key_set)
        throw err::UsageError("MCA decryption key not set");

    for (auto i : util::range(offsets.size()))
    {
        arc_file.io.seek(offsets[i]);
        auto encryption_type = arc_file.io.read_u32_le();
        arc_file.io.skip(8);
        auto width = arc_file.io.read_u32_le();
        auto height = arc_file.io.read_u32_le();
        auto size_comp = arc_file.io.read_u32_le();
        auto size_orig = arc_file.io.read_u32_le();
        arc_file.io.skip(4);

        auto data = arc_file.io.read(size_comp);

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
            throw err::NotSupportedError(util::format(
                "Unknown encryption type: %d", encryption_type));
        }

        data = common::fix_stride(data, width, height, 24);

        pix::Grid pixels(width, height, data, pix::Format::BGR888);
        saver.save(util::grid_to_boxed(pixels, util::format("%03d.png", i)));
    }
}

static auto dummy = fmt::Registry::add<McaArchiveDecoder>("fc01/mca");
