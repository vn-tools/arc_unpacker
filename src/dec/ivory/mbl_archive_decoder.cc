#include "dec/ivory/mbl_archive_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/ivory/prs_image_decoder.h"
#include "dec/ivory/wady_audio_decoder.h"
#include "err.h"
#include "util/plugin_mgr.h"

using namespace au;
using namespace au::dec::ivory;

namespace
{
    using PluginFunc = std::function<void(bstr &)>;

    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };

    struct ArchiveMetaImpl final : dec::ArchiveMeta
    {
        bool encrypted;
        PluginFunc decrypt;
    };
}

static bool verify_version(
    io::IStream &input_stream,
    const size_t file_count,
    const size_t name_size)
{
    try
    {
        input_stream.skip((file_count - 1) * (name_size + 8));
        input_stream.skip(name_size);
        const auto last_file_offset = input_stream.read_u32_le();
        const auto last_file_size = input_stream.read_u32_le();
        return last_file_offset + last_file_size == input_stream.size();
    }
    catch (...)
    {
        return false;
    }
}

static int detect_version(io::IStream &input_stream)
{
    {
        input_stream.seek(0);
        const auto file_count = input_stream.read_u32_le();
        if (verify_version(input_stream, file_count, 16))
            return 1;
    }

    {
        input_stream.seek(0);
        const auto file_count = input_stream.read_u32_le();
        const auto name_size = input_stream.read_u32_le();
        if (verify_version(input_stream, file_count, name_size))
            return 2;
    }

    throw err::RecognitionError();
}

struct MblArchiveDecoder::Priv final
{
    util::PluginManager<PluginFunc> plugin_mgr;
};

MblArchiveDecoder::MblArchiveDecoder() : p(new Priv)
{
    p->plugin_mgr.add("noop", "Unencrypted games", [](bstr &) { });

    p->plugin_mgr.add(
        "candy",
        "Candy Toys",
        [](bstr &data)
        {
            for (auto i : algo::range(data.size()))
                data[i] = -data[i];
        });

    p->plugin_mgr.add(
        "wanko",
        "Wanko to Kurasou",
        [](bstr &data)
        {
            static const bstr key =
                "\x82\xED\x82\xF1\x82\xB1\x88\xC3\x8D\x86\x89\xBB"_b;
            for (auto i : algo::range(data.size()))
                data[i] ^= key[i % key.size()];
        });
}

MblArchiveDecoder::~MblArchiveDecoder()
{
}

void MblArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    p->plugin_mgr.register_cli_options(
        arg_parser, "Specifies plugin for decoding dialog files.");
}

void MblArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    p->plugin_mgr.parse_cli_options(arg_parser);
}

void MblArchiveDecoder::set_plugin(const std::string &plugin_name)
{
    p->plugin_mgr.set(plugin_name);
}

bool MblArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return detect_version(input_file.stream) > 0;
}

std::unique_ptr<dec::ArchiveMeta> MblArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMetaImpl>();
    const auto version = detect_version(input_file.stream);
    meta->encrypted
        = input_file.path.name().find("mg_data") != std::string::npos;
    meta->decrypt = p->plugin_mgr.is_set() ? p->plugin_mgr.get() : nullptr;
    input_file.stream.seek(0);

    const auto file_count = input_file.stream.read_u32_le();
    const auto name_size = version == 2
        ? input_file.stream.read_u32_le()
        : 16;
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(name_size)).str();
        entry->offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> MblArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    if (meta->encrypted)
    {
        if (!meta->decrypt)
        {
            throw err::UsageError(
                "File is encrypted, but plugin not set. "
                "Please supply one with --plugin switch.");
        }
        meta->decrypt(data);
    }

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> MblArchiveDecoder::get_linked_formats() const
{
    return {"ivory/prs", "ivory/wady"};
}

static auto _ = dec::register_decoder<MblArchiveDecoder>("ivory/mbl");
